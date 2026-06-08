import json

from sqlalchemy.orm import Session
from sqlalchemy.exc import IntegrityError
from exceptions.exception import ServiceException
from typing import List, Dict

from module_device.constant.device_constant import (
    DeviceGroupMap,
    DeviceTypeMap,
    RtdbDevType,
    web_device_type_to_rtdb,
    LOCAL_SYSTEM_DEVICE_ID,
    LOCAL_SYSTEM_DEVICE_NAME,
    apply_local_system_display_name,
)
from module_device.entity.vo.device_vo import *
from module_device.dao.device_dao import DeviceDao
from module_device.dao.point_dao import PointDao
from module_device.dao.template_dao import TemplateDao
from module_device.dao.topology_dao import TopologyDao
from module_device.service.business_db_sync import sync_delete_devices_to_business
from module_device.dao.port_dao import PortDao
from module_device.service.template_service import TemplateService
from module_device.entity.vo.device_vo import DeleteDeviceModel, DeviceCreateUpdateModel, DeviceModel, \
    DevicePageQueryModel, CrudResponseModel
from module_device.entity.do.point_do import Point
from module_device.entity.vo.template_vo import TemplatePointModel
from utils.common_util import CamelCaseUtil
from module_device.service.rtdb_service import get_rtdb_point_value


class DeviceService:
    """
    设备管理模块服务层
    """

    @staticmethod
    def prepare_device_content(device_object: DeviceCreateUpdateModel) -> dict:
        """
        将 device_object 的字段处理为 content 格式
        """
        content_fields = {'address', 'baud', 'data', 'ip', 'netId', 'parity', 'port', 'stop', 'timeout', 'uId', 'uartId', 'canId'}
        content_data = {
            key: value
            for key, value in device_object.model_dump().items()
            if key in content_fields and value is not None
        }

        device_dict = {
            "id": device_object.id,
            "sn": device_object.sn,
            "type": device_object.type,
            "code": device_object.code,
            "name": device_object.name,
            "protocol_type": device_object.protocol_type,
            "template_id": device_object.template_id,
            "sort_order": device_object.sort_order,
            "remark": device_object.remark,
            "status": device_object.status,
        }
        
        protocol_dict = {
            # "id": device_object.protocol_type,
            "related_id": device_object.id,
            "template_type": 0,
            "type": device_object.protocol_type,
            "name": device_object.name,
            "content": json.dumps(content_data),
            # "remark": device_object.protocol_remark,
        }

        try:
            if device_object.id is not None and int(device_object.id) == LOCAL_SYSTEM_DEVICE_ID:
                device_dict["name"] = LOCAL_SYSTEM_DEVICE_NAME
                protocol_dict["name"] = LOCAL_SYSTEM_DEVICE_NAME
        except (TypeError, ValueError):
            pass

        return device_dict, protocol_dict

    @classmethod
    async def get_device_list_services(
            cls, query_db: Session, query_object: DevicePageQueryModel, is_page: bool = True
    ):
        """
        获取设备列表信息service

        :param query_db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 设备列表信息对象
        """
        device_list_result = await DeviceDao.get_device_list(query_db, query_object, is_page)
        rows = getattr(device_list_result, "rows", None)
        if rows:
            for row in rows:
                apply_local_system_display_name(row)

        return device_list_result
    

    @classmethod
    async def get_all_device_services(cls, query_db: Session, name: str = None):
        """
        获取所有设备信息service
        """
        device_result = await DeviceDao.get_all_device(query_db, name)
        if isinstance(device_result, list):
            for row in device_result:
                apply_local_system_display_name(row)
        else:
            rows = getattr(device_result, "rows", None)
            if rows:
                for row in rows:
                    apply_local_system_display_name(row)

        return device_result

    @classmethod
    async def _validate_device_template(cls, query_db: Session, device_object: DeviceCreateUpdateModel):
        """创建设备时仅能选一个模板，且须属于该设备类型（及协议类型）。"""
        if device_object.template_id is None:
            raise ServiceException(message='Template is required')
        template = await TemplateDao.get_template_detail_by_id(query_db, device_object.template_id)
        if not template:
            raise ServiceException(message='Template does not exist')
        if int(template.type) != 0:
            raise ServiceException(message='Invalid device configuration template')
        if int(template.protocol_type) != int(device_object.protocol_type):
            raise ServiceException(message='Template protocol type does not match device')
        if template.dev_type is not None and int(template.dev_type) != int(device_object.type):
            raise ServiceException(message='Template does not match device type')

    @classmethod
    async def add_device_services(cls, query_db: Session, device_object: DeviceCreateUpdateModel):
        """
        新增设备信息service
        """
        try:
            await cls._validate_device_template(query_db, device_object)
            # 查询数据库中是否有同名设备
            existing_device = DeviceDao.check_device_exists_by_name(query_db, device_object.name)
            if existing_device:
                raise ServiceException(message='Device name already exists')
            # 查询当前已有的同类型设备数量
            existing_count = await DeviceDao.count_devices_by_type(query_db, device_object.type)
            new_code = existing_count + 1  # 计算新设备的 code

            # 处理 content 字段
            device_data, protocol_data = cls.prepare_device_content(device_object)
            device_data["code"] = str(new_code)  # 设置新设备的 code

            # 新增设备
            new_device = await DeviceDao.add_device_dao(query_db, device_data)
            protocol_data["related_id"] = new_device.id
            await DeviceDao.add_protocol_dao(query_db, protocol_data)

            # 判断是否需要copy测点
            if not await cls.judge_if_points_exist(query_db, device_object.template_id, device_object.type):
                # 获取模板点位
                template_points = await TemplateService.get_template_points_services(query_db, device_object.template_id)
                
                # 构造批量插入数据
                point_data_list = [
                    {
                        # "id": point["id"],
                        "template_id": point["templateId"],
                        "protocol_type": new_device.protocol_type,
                        "dev_type": new_device.type,
                        "group_type": point["groupType"],
                        "code": point["code"],
                        "name": point["name"],
                        "ename": point["ename"],
                        "data_type": point["dataType"],
                        "attribute": point["attribute"],
                        "func": point["func"],
                        "reg_addr": point["regAddr"],
                        "bit_pos": point["bitPos"],
                        "bit_num": point["bitNum"],
                        "endian": point["endian"],
                        "precision": point["precision"],
                        "ratio": point["ratio"],
                        "offset": point["offset"],
                        "is_persisted": point["isPersisted"],
                        "storage_interval": point["storageInterval"],
                        "mutate_bound": point["mutateBound"],
                        "default_value": point["defaultValue"],
                        "min_value": point["minValue"],
                        "max_value": point["maxValue"],
                        "unit": point["unit"],
                        "is_show": point["isShow"],
                    }
                    for point in template_points
                ]
                
                if point_data_list:
                    await PointDao.add_device_points_batch_dao(query_db, point_data_list)

            query_db.commit()
            return CrudResponseModel(is_success=True, message=f'Added successfully, device code: {new_code}')
        except Exception as e:
            query_db.rollback()
            raise e
        
    @classmethod
    async def judge_if_points_exist(cls, query_db: Session, template_id: int,dev_type: int):
        """
        新增设备时，判断是否需要拉取测点
        """
        return PointDao.check_point_exists(query_db, template_id, dev_type)
    
    @classmethod
    async def judge_if_devices_exist(cls, query_db: Session, template_id: int,dev_type: int):
        """
        删除设备时，判断是否需要删除测点
        """
        return DeviceDao.check_device_exists(query_db, template_id, dev_type)

    @classmethod
    async def edit_device_services(cls, query_db: Session, device_object: DeviceCreateUpdateModel):
        """
        编辑设备信息 service，同时更新 protocol 表的相关信息
        """
        device_info = await cls.device_detail_services(query_db, device_object.id)
        if device_info.id:
            try:
                if device_object.template_id is not None:
                    await cls._validate_device_template(query_db, device_object)
                # 处理设备数据
                device_data, protocol_data = cls.prepare_device_content(device_object)

                # 更新设备表
                await DeviceDao.edit_device_dao(query_db, device_data)

                # 检查 protocol 是否存在
                protocol_info = await DeviceDao.get_protocol_by_device_id(query_db, device_object.id)
                if protocol_info:
                    # 更新 protocol 表
                    await DeviceDao.edit_protocol_dao(query_db, protocol_data)
                else:
                    # 若不存在，则插入新的 protocol 记录
                    protocol_data["related_id"] = device_object.id
                    await DeviceDao.add_protocol_dao(query_db, protocol_data)

                query_db.commit()
                return CrudResponseModel(is_success=True, message='Update successful')

            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Device does not exist')

    @classmethod
    async def delete_device_services(cls, query_db: Session, device_object: DeleteDeviceModel):
        """
        删除设备信息 service，同时删除 protocol 表的相关信息，并重新编号 code
        """
        if not device_object.device_ids:
            raise ServiceException(message='Device ID is empty')

        device_id_list = device_object.device_ids.split(',')

        try:
            # 查询要删除的设备信息
            devices_to_delete = await DeviceDao.get_devices_by_ids(query_db, device_id_list)
            if not devices_to_delete:
                raise ServiceException(message="Device does not exist")
            
            # 检查设备是否在设备拓扑中
            for device in devices_to_delete:
                # 本机 BBMS 设备为系统基础设备，禁止删除，避免系统无法运行
                if int(device.type) == DeviceTypeMap.LOCAL_SYSTEM or int(device.id) == 1:
                    raise ServiceException(message=f"Device {device.name} is a system base device and cannot be deleted")
                is_in_topology = await TopologyDao.check_device_in_topology(query_db, device.id)
                if is_in_topology:
                    raise ServiceException(message=f"Device {device.name} has been added to device topology and cannot be deleted")

            # 先删除 protocol 表中对应的 related_id 记录
            await DeviceDao.delete_protocol_dao(query_db, device_id_list)

            # 删除设备表中的设备数据
            await DeviceDao.delete_device_dao(query_db, device_id_list)

            # 删除对应设备类型和协议类型的测点
            for device in devices_to_delete:
                if await cls.judge_if_devices_exist(query_db, device.template_id, device.type):
                    continue
                await PointDao.delete_points_by_template_id_and_dev_type(query_db, device.template_id, device.type)
                
            query_db.commit()
            try:
                sync_delete_devices_to_business(devices_to_delete)
            except Exception:
                pass
            return CrudResponseModel(is_success=True, message='Delete successful')

        except Exception as e:
            query_db.rollback()
            raise e


    @classmethod
    async def device_detail_services(cls, query_db: Session, device_id: int):
        """
        获取设备详细信息service

        :param query_db: orm对象
        :param device_id: 设备id
        :return: 设备id对应的信息
        """
        device = await DeviceDao.get_device_detail_by_id(query_db, device_id=device_id)
        if device:
            d = CamelCaseUtil.transform_result(device)
            if isinstance(d, dict):
                apply_local_system_display_name(d)
                result = DeviceCreateUpdateModel(**d)
            else:
                result = DeviceCreateUpdateModel(**dict())
        else:
            result = DeviceCreateUpdateModel(**dict())

        return result
    
    @classmethod
    async def get_device_points_services(cls, query_db: Session, device_id: int):
        """
        获取设备点位信息service
        """
        device_info = await DeviceDao.get_device_detail_by_id(query_db, device_id)
        if not device_info:
            return []
        tid = getattr(device_info, "template_id", None)
        if tid is None:
            return []
        points = await PointDao.get_point_list_by_device_template(query_db, device_info.template_id, device_info.type)
        return points

    @classmethod
    async def get_device_points_with_status_services(cls, query_db: Session, device_id: int):
        """
        获取设备点位信息service
        """
        device_info = await DeviceDao.get_device_detail_by_id(query_db, device_id)
        if not device_info:
            return []
        tid = getattr(device_info, "template_id", None)
        if tid is None or int(tid) == 0:
            return []
        try:
            points = await PointDao.get_point_list_by_device_template(query_db, device_info.template_id, device_info.type)
            rtdb_t = web_device_type_to_rtdb(int(device_info.type))
            for point in points:
                point["value"] = get_rtdb_point_value(rtdb_t, int(device_info.code) - 1, point["code"])
            return points
        except Exception as e:
            raise e
    
    @classmethod
    async def add_device_point_services(cls, query_db: Session, add_point: TemplatePointModel):
        """
        增加设备测点信息service
        """
        try:
            point_data = add_point.model_dump(by_alias=False)
            db_point = await PointDao.add_point_dao(query_db, point_data)
            query_db.commit()
            return CrudResponseModel(is_success=True, message='Added successfully', result=db_point.id)
        except Exception as e:
            query_db.rollback()
            raise e
        
    @classmethod
    async def edit_device_point_services(cls, query_db: Session, edit_point: TemplatePointModel):
        """
        编辑设备测点信息service
        """
        edit_point = edit_point.model_dump(exclude_unset=True)
        point_info = await PointDao.get_device_point_detail_by_id(query_db, edit_point.get('id'))
        if point_info.id:
            try:
                await PointDao.edit_point_dao(query_db, edit_point)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Update successful')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Device point does not exist')

    @classmethod
    async def delete_device_point_services(cls, query_db: Session, point_ids: str):
        """
        删除设备测点信息service
        """
        if point_ids:
            point_id_list = point_ids.split(',')
            try:
                await PointDao.delete_point_by_id(query_db, point_id_list)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Delete successful')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Point ID is empty')
        
    @classmethod
    async def batch_add_devices_services(cls, db: Session, devices_data: List[dict]):
        """
        批量新增设备服务
        """
        with db.begin():
            try:
                # 准备设备模型并校验
                device_models, type_counter = await cls.prepare_and_validate_batch_devices(db, devices_data)
                
                results = []
                for model in device_models:
                    # 创建单个设备
                    device_result = await cls.create_device_with_points(db, model, type_counter)
                    results.append(device_result)
                    # 更新类型计数器
                    type_counter += 1
                
                # 处理测点模板，只需检查和增加一次
                first_model = device_models[0]
                if not await cls.judge_if_points_exist(db, first_model.template_id, first_model.type):
                    template_points = await TemplateService.get_template_points_services(db, first_model.template_id)
                    if template_points:
                        point_data_list = [
                            {
                                "template_id": point["templateId"],
                                "protocol_type": first_model.protocol_type,
                                "dev_type": first_model.type,
                                "group_type": point["groupType"],
                                "code": point["code"],
                                "name": point["name"],
                                "ename": point["ename"],
                                "data_type": point["dataType"],
                                "attribute": point["attribute"],
                                "func": point["func"],
                                "reg_addr": point["regAddr"],
                                "bit_pos": point["bitPos"],
                                "bit_num": point["bitNum"],
                                "endian": point["endian"],
                                "precision": point["precision"],
                                "ratio": point["ratio"],
                                "offset": point["offset"],
                                "is_persisted": point["isPersisted"],
                                "storage_interval": point["storageInterval"],
                                "mutate_bound": point["mutateBound"],
                                "default_value": point["defaultValue"],
                                "min_value": point["minValue"],
                                "max_value": point["maxValue"],
                                "unit": point["unit"],
                                "is_show": point["isShow"],
                            }
                            for point in template_points
                        ]
                        await PointDao.add_device_points_batch_dao(db, point_data_list)

                db.commit()
                return results
            except Exception as e:
                db.rollback()
                raise ServiceException(message="Database operation error, please check the unique constraint")

    @classmethod
    async def prepare_and_validate_batch_devices(cls, db: Session, devices_data: List[dict]):
        """
        预处理和校验批量设备
        """
        if not devices_data:
            raise ServiceException(message="Device data is empty")

        device_models = []
        name_set = set()
        identifier_set = set()

        # 获取第一个设备的类型和协议类型
        first_device_type = devices_data[0].get('type')
        first_protocol_type = devices_data[0].get('protocolType')
        if first_device_type is None or first_protocol_type is None:
            raise ServiceException(message="Device type or protocol type is not defined")

        # 初始化类型计数器
        type_counter = await DeviceDao.count_devices_by_type(db, first_device_type)

        # 从数据库中查询所有当前type的设备名称
        existing_device_names = {device.name for device in DeviceDao.get_devices_by_type(db, first_device_type)}

        # 从数据库中查询所有与当前设备类型和协议类型相同的设备
        existing_devices = DeviceDao.get_devices_by_type_and_protocol(db, first_device_type, first_protocol_type)
        existing_identifiers = set()
        for device in existing_devices:
            protocol = await DeviceDao.get_protocol_by_device_id(db, device.id)
            if protocol:
                content = json.loads(protocol.content)
                identifier = cls.get_device_identifier_from_content(content)
                if identifier:
                    existing_identifiers.add(identifier)

        # 生成模型并校验
        for data in devices_data:
            model = ProtocolFactory.create_protocol_model(data)
            await cls._validate_device_template(db, model)
            device_models.append(model)

            # 名称重复校验
            if model.name in name_set or model.name in existing_device_names:
                raise ServiceException(message=f"Device name {model.name} is duplicate or already exists")
            name_set.add(model.name)

            # 协议唯一性校验
            identifier = cls.get_device_identifier_from_content(model)
            if identifier:
                if identifier in identifier_set or identifier in existing_identifiers:
                    raise ServiceException(message=f"Unique identifier {identifier} is duplicate or already exists, please check the data")
                identifier_set.add(identifier)

        return device_models, type_counter

    @staticmethod
    def get_device_identifier_from_content(content):
        """从协议内容中获取设备唯一标识"""
        if hasattr(content, 'ip') and hasattr(content, 'port') and hasattr(content, 'uId'):
            return f"{content.ip}_{content.port}_{content.uId}"
        elif hasattr(content, 'uartId') and hasattr(content, 'uId'):
            return f"{content.uartId}_{content.uId}"
        return None

    @classmethod
    async def create_device_with_points(cls, db: Session, model: DeviceCreateUpdateModel, type_counter: int):
        """创建设备及关联测点"""
        # 生成设备编码
        new_code = type_counter + 1
        
        # 处理设备内容
        device_data, protocol_data = cls.prepare_device_content(model)
        device_data["code"] = str(new_code)
        
        # 创建设备记录
        new_device = await DeviceDao.add_device_dao(db, device_data)
        protocol_data["related_id"] = new_device.id
        await DeviceDao.add_protocol_dao(db, protocol_data)
        
        return {"name": model.name, "code": new_code}
    

    @classmethod
    async def get_device_count_by_type(cls, db: Session, device_type: int):
        """根据设备类型获取设备数量"""
        return await DeviceDao.count_devices_by_type(db, device_type)
    
    
    @classmethod
    async def get_dsp_devices(cls, db: Session):
        """根据dsp设备类型获取设备信息，按CAN口分组"""
        # 获取CAN口0和CAN口1的配置id
        can_port_ids = PortDao.get_can_port_ids(db)
        can_config_map = {port['id']: [] for port in can_port_ids}

        # 获取dsp设备
        dsp_devices = DeviceDao.get_devices_by_type(db, DeviceTypeMap.PCS)

        for device in dsp_devices:
            protocol = await DeviceDao.get_protocol_by_device_id(db, device.id)
            if protocol:
                content = json.loads(protocol.content)
                # 检查协议内容中是否存在canId字段
                can_id = content.get('canId')
                if can_id in can_config_map:
                    can_config_map[can_id].append(device.code)

        return can_config_map
    

    @classmethod
    async def get_stack_control_devices(cls, db: Session):
        """获取堆控设备信息"""
        # 获取堆控设备
        stack_control_devices = DeviceDao.get_devices_by_type(db, DeviceTypeMap.STACK_BMS)

        # 创建设备信息列表
        device_info_list = []

        for device in stack_control_devices:
            protocol = await DeviceDao.get_protocol_by_device_id(db, device.id)
            if protocol:
                content = json.loads(protocol.content)
                device_info = {
                    "device_id": device.id,
                    "device_code": device.code,
                    "protocol_content": content
                }
                device_info_list.append(device_info)

        return device_info_list

    @classmethod
    async def judge_alarm_exist(cls, db: Session, device_code: int):
        """
        存在告警:dsp模型中配置测点类型为告警的点有任一一个值为1
        """
        device_info = DeviceDao.get_device_by_dev_type_and_dev_code(db, DeviceTypeMap.PCS, device_code)
        if device_info and device_info.template_id:
            points = PointDao.get_point_list_by_device_template_and_group_type(db, device_info.template_id, DeviceTypeMap.PCS, DeviceGroupMap.FAULT)
            for point in points:
                rtdb_t = web_device_type_to_rtdb(DeviceTypeMap.PCS)
                if get_rtdb_point_value(rtdb_t, int(device_code) - 1, point.code) not in [0, 0.0, None, '']:
                    return True
        return False
    
    @classmethod
    async def get_radiator_devices_info(cls, db: Session):
        """获取液冷机设备信息"""
        radiator_devices = DeviceDao.get_devices_by_type(db, DeviceTypeMap.AC_LIQUID)
        return radiator_devices[0] if radiator_devices else None

    @classmethod
    async def get_metering_devices_info(cls, db: Session):
        """获取电表设备信息"""
        metering_devices = DeviceDao.get_devices_by_type(db, DeviceTypeMap.METER)
        if metering_devices:
            return [
                {"device_id": device.id, "device_code": device.code}
                for device in metering_devices
            ]
        return []
    
    @classmethod
    def get_devices_by_type(cls, db: Session, device_type: int):
        """根据设备类型查询设备列表"""
        device_list = DeviceDao.get_devices_by_type(db, device_type)
        device_info_list = []
        for device in device_list:
            device_info_list.append({
                "device_id": device.id,
                "device_code": device.code,
                "device_name": device.name,
            })
        return device_info_list
    

    @classmethod
    def get_pcs_soft_version(cls, db: Session):
        """获取PCS软件版本"""
        pcs_devices = DeviceDao.get_all_pcs_soft_versions(db)
        pcs_soft_version_list = []
        for pcs_device in pcs_devices:
            version_info = {
                "id": pcs_device.id,
                "name": pcs_device.name,
                "cpld_version": pcs_device.cpld_version,
                "dsp_cpu1_version": pcs_device.dsp_cpu1_version,
                "dsp_cpu2_version": pcs_device.dsp_cpu2_version,
                "pcs_version": pcs_device.pcs_version
            }
            pcs_soft_version_list.append(version_info)
        return pcs_soft_version_list
