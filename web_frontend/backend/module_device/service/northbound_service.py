import json

from sqlalchemy.orm import Session
from exceptions.exception import ServiceException
from module_device.entity.vo.device_vo import *
from module_device.dao.northbound_dao import NorthboundDao
from module_device.dao.point_dao import PointDao
from module_device.service.template_service import TemplateService
from module_device.entity.vo.device_vo import DeleteDeviceModel, DeviceCreateUpdateModel, DeviceModel, \
    NorthConfigPageQueryModel, CrudResponseModel
from utils.common_util import CamelCaseUtil


class NorthboundService:
    """
    设备管理模块服务层
    """

    @staticmethod
    def prepare_device_content(config_object: DeviceCreateUpdateModel) -> dict:
        """
        将 config_object 的字段处理为 content 格式
        """
        content_fields = {'address', 'baud', 'data', 'ip', 'netId', 'parity', 'port', 'stop', 'timeout', 'uId', 'uartId', 'canId'}
        content_data = {
            key: value
            for key, value in config_object.model_dump().items()
            if key in content_fields and value is not None
        }

        device_dict = {
            "id": config_object.id,
            "sn": config_object.sn,
            "type": config_object.type,
            "code": config_object.code,
            "name": config_object.name,
            "protocol_type": config_object.protocol_type,
            "template_id": config_object.template_id,
            "sort_order": config_object.sort_order,
            "remark": config_object.remark,
            "status": config_object.status,
        }
        
        protocol_dict = {
            # "id": config_object.protocol_type,
            "related_id": config_object.id,
            "template_type": config_object.template_id,
            "type": config_object.protocol_type,
            "name": config_object.name,  #todo:其实要加上code
            "content": json.dumps(content_data),
            # "remark": config_object.protocol_remark,
        }

        return device_dict, protocol_dict

    @classmethod
    async def get_northbound_config_list_services(
            cls, query_db: Session, query_object: NorthConfigPageQueryModel, is_page: bool = True
    ):
        """
        获取北向配置列表信息service

        :param query_db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 北向配置列表信息对象
        """
        config_list_result = await NorthboundDao.get_northbound_config_list(query_db, query_object, is_page)

        return config_list_result
    

    @classmethod
    async def get_all_northbound_config_services(cls, query_db: Session, name: str = None):
        """
        获取所有北向配置信息service
        """
        config_result = await NorthboundDao.get_all_northbound_config(query_db, name)
        return config_result

    @classmethod
    async def add_northbound_config_services(cls, query_db: Session, config_object: DeviceCreateUpdateModel):
        """
        新增北向配置信息service
        """
        try:
            # 查询当前已有的同类型设备数量
            existing_count = await NorthboundDao.count_northbound_configs_by_type(query_db, config_object.type)
            new_code = existing_count + 1  # 计算新设备的 code

            # 处理 content 字段
            device_data, protocol_data = cls.prepare_device_content(config_object)
            device_data["code"] = str(new_code)  # 设置新设备的 code

            # 新增设备
            new_device = await NorthboundDao.add_northbound_device_dao(query_db, device_data)
            protocol_data["related_id"] = new_device.id
            await NorthboundDao.add_northbound_protocol_dao(query_db, protocol_data)

            # 判断是否需要copy测点
            if not await cls.judge_if_points_exist(query_db, config_object.template_id, config_object.type):
                # 获取模板点位
                template_points = await TemplateService.get_template_points_services(query_db, config_object.template_id)
                
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
            return CrudResponseModel(is_success=True, message=f'新增成功，设备编号: {new_code}')
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
    async def edit_northbound_device_services(cls, query_db: Session, config_object: DeviceCreateUpdateModel):
        """
        编辑设备信息 service，同时更新 protocol 表的相关信息
        """
        device_info = await cls.northbound_device_detail_services(query_db, config_object.id)
        if device_info.id:
            try:
                # 处理设备数据
                device_data, protocol_data = cls.prepare_device_content(config_object)

                # 更新设备表
                await NorthboundDao.edit_northbound_device_dao(query_db, device_data)

                # 检查 protocol 是否存在
                protocol_info = await NorthboundDao.get_northbound_protocol_by_device_id(query_db, config_object.id)
                if protocol_info:
                    # 更新 protocol 表
                    await NorthboundDao.edit_northbound_protocol_dao(query_db, protocol_data)
                else:
                    # 若不存在，则插入新的 protocol 记录
                    protocol_data["related_id"] = config_object.id
                    await NorthboundDao.add_northbound_protocol_dao(query_db, protocol_data)

                query_db.commit()
                return CrudResponseModel(is_success=True, message='Updated successfully')

            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Device does not exist')

    @classmethod
    async def delete_northbound_device_services(cls, query_db: Session, config_object: DeleteDeviceModel):
        """
        删除设备信息 service，同时删除 protocol 表的相关信息，并重新编号 code
        """
        if not config_object.device_ids:
            raise ServiceException(message='Device ID is empty')

        device_id_list = config_object.device_ids.split(',')

        try:
            # 查询要删除的设备信息
            devices_to_delete = await NorthboundDao.get_northbound_devices_by_ids(query_db, device_id_list)
            if not devices_to_delete:
                raise ServiceException(message="Device does not exist")

            # 先删除 protocol 表中对应的 related_id 记录
            await NorthboundDao.delete_northbound_protocol_dao(query_db, device_id_list)

            # 删除设备表中的设备数据
            await NorthboundDao.delete_northbound_device_dao(query_db, device_id_list)

            query_db.commit()
            return CrudResponseModel(is_success=True, message='Deleted successfully')

        except Exception as e:
            query_db.rollback()
            raise e

    @classmethod
    async def northbound_device_detail_services(cls, query_db: Session, device_id: int):
        """
        获取设备详细信息service

        :param query_db: orm对象
        :param device_id: 设备id
        :return: 设备id对应的信息
        """
        device = await NorthboundDao.get_northbound_device_detail_by_id(query_db, device_id=device_id)
        if device:
            result = DeviceCreateUpdateModel(**CamelCaseUtil.transform_result(device))
        else:
            result = DeviceCreateUpdateModel(**dict())

        return result
