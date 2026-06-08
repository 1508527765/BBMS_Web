from typing import Optional

from sqlalchemy.orm import Session
from exceptions.exception import ServiceException
from module_device.constant.device_constant import DeviceTypeMap
from module_device.dao.topology_dao import TopologyDao
from module_device.dao.device_dao import DeviceDao
from module_device.entity.vo.topology_vo import TopologyCreateUpdateModel, TopologyModel
from utils.common_util import CamelCaseUtil
from module_admin.entity.vo.common_vo import CrudResponseModel


class TopologyService:
    """
    拓扑管理模块服务层
    """

    @classmethod
    async def get_topology_list_services(cls, query_db: Session):
        """
        获取拓扑列表信息，并组织为树状结构
        """
        topology_list_result = await TopologyDao.get_topology_list(query_db)

        def _menu_tree_int(raw):
            """SQLite 等可能返回字符串 menu_tree，统一为整数再判断文件夹/设备"""
            try:
                return int(raw) if raw is not None and str(raw).strip() != "" else 0
            except (TypeError, ValueError):
                return 0

        # 创建节点映射
        node_map = {}
        for node in topology_list_result:
            mt = _menu_tree_int(node.get("menu_tree"))
            dt = node.get("device_type")
            try:
                dt_int = int(dt) if dt is not None and str(dt).strip() != "" else None
            except (TypeError, ValueError):
                dt_int = None
            dev_raw = node.get("dev_id")
            if dev_raw is not None and str(dev_raw).strip() != "":
                try:
                    dev_id_out = str(int(dev_raw))
                except (TypeError, ValueError):
                    dev_id_out = str(dev_raw)
            else:
                dev_id_out = None
            vdt_raw = node.get("virtual_dev_type")
            try:
                vdt_int = int(vdt_raw) if vdt_raw is not None and str(vdt_raw).strip() != "" else None
            except (TypeError, ValueError):
                vdt_int = None
            if mt == 0:
                if vdt_int is None:
                    name_lower = str(node.get("name") or "").lower()
                    if "rbms" in name_lower:
                        vdt_int = DeviceTypeMap.CLUSTER_BMS
                    elif "bbms" in name_lower:
                        vdt_int = DeviceTypeMap.STACK_BMS
                layout_dt = vdt_int
            else:
                layout_dt = dt_int
            node_map[str(node.get("id"))] = {
                "id": str(node.get("id")),
                "type": "virtual" if mt == 0 else "device",
                "label": node.get("name")
                if mt == 0
                else (
                    node.get("device_name")
                    if dt_int in (0, DeviceTypeMap.STACK_BMS)
                    else f"{node.get('device_code')} # {node.get('device_name')}"
                ),
                "devId": dev_id_out,
                "deviceName": node.get('device_name'),
                "deviceType": layout_dt,
                "virtualDevType": vdt_int if mt == 0 else None,
                "deviceCode": node.get('device_code'),
                "templateId": node.get('template_id') if mt == 1 else None,
                "parentId": node.get('parent_id') if node.get('parent_id') != -999 else None,
                "children": []
            }

        # 组织 Parent-Child 关系
        root_nodes = set(node_map.keys())  # 先假设所有节点都是根节点
        for node in topology_list_result:
            node_id = str(node.get('id'))
            parent_id = str(node.get('parent_id')) if node.get('parent_id') is not None else "-999"

            if parent_id != "-999" and parent_id in node_map:
                node_map[parent_id]["children"].append(node_map[node_id])
                if node_id in root_nodes:
                    root_nodes.remove(node_id)  # 如果某个节点被添加到别的 `children` 里，说明它不是根节点

        # 只保留真正的根节点，并按拓扑列表中的顺序排序
        root_nodes = sorted(
            [node_map[node_id] for node_id in root_nodes],
            key=lambda x: topology_list_result.index(next(node for node in topology_list_result if str(node.get('id')) == x['id']))
        )

        return root_nodes


    @classmethod
    async def add_topology_services(cls, query_db: Session, topology_object: TopologyCreateUpdateModel):
        """
        新增拓扑信息service

        :param query_db: orm对象
        :param topology_object: 新增拓扑对象
        :return: 新增拓扑校验结果
        """
        try:
            await cls._validate_topology_payload(query_db, topology_object, current_topology_id=None)
            await TopologyDao.add_topology_dao(query_db, topology_object)
            query_db.commit()
            return CrudResponseModel(is_success=True, message='Added successfully')
        except Exception as e:
            query_db.rollback()
            raise e

    @classmethod
    async def edit_topology_services(cls, query_db: Session, topology_object: TopologyCreateUpdateModel):
        """
        编辑拓扑信息service

        :param query_db: orm对象
        :param topology_object: 编辑拓扑对象
        :return: 编辑拓扑校验结果
        """
        edit_topology = topology_object.model_dump(exclude_unset=True)
        topology_info = await cls.topology_detail_services(query_db, topology_object.id)
        if topology_info.id:
            try:
                await cls._validate_topology_payload(query_db, topology_object, current_topology_id=topology_object.id)
                await TopologyDao.edit_topology_dao(query_db, edit_topology)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Updated successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Topology does not exist')

    @classmethod
    async def delete_topology_services(cls, query_db: Session, topology_ids: str):
        """
        删除拓扑信息service

        :param query_db: orm对象
        :param topology_ids: 删除拓扑对象
        :return: 删除拓扑校验结果
        """
        if topology_ids:
            topology_id_list = topology_ids.split(',')
            try:
                # 系统基础节点保护：本机BBMS拓扑节点不可删除
                for tid in topology_id_list:
                    topo = await TopologyDao.get_topology_detail_by_id(query_db, int(tid))
                    if not topo:
                        continue
                    if topo.menu_tree == 1 and topo.dev_id:
                        device = await DeviceDao.get_device_detail_by_id(query_db, topo.dev_id)
                        if device and (
                            int(device.type) == DeviceTypeMap.LOCAL_SYSTEM
                            or int(device.id) == 1
                            or str(device.name or '').strip().lower() == 'local system'
                        ):
                            raise ServiceException(message='Local system topology node cannot be deleted')
                await TopologyDao.delete_topology_dao(query_db, topology_id_list)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Deleted successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Topology ID input is empty')

    @classmethod
    async def topology_detail_services(cls, query_db: Session, topology_id: int):
        """
        获取拓扑详细信息service

        :param query_db: orm对象
        :param topology_id: 拓扑id
        :return: 拓扑id对应的信息
        """
        topology = await TopologyDao.get_topology_detail_by_id(query_db, topology_id=topology_id)
        if not topology:
            topology = TopologyModel(**dict())
            return topology
        return TopologyModel(**CamelCaseUtil.transform_result(topology))

    @classmethod
    async def _get_device_type_by_id(cls, query_db: Session, dev_id: int):
        if not dev_id:
            return None
        device = await DeviceDao.get_device_detail_by_id(query_db, int(dev_id))
        if not device:
            return None
        try:
            return int(device.type)
        except Exception:
            return None

    @classmethod
    def _virtual_dev_type_int(cls, topo_node) -> Optional[int]:
        if not topo_node:
            return None
        raw = getattr(topo_node, 'virtual_dev_type', None)
        if raw is None and isinstance(topo_node, dict):
            raw = topo_node.get('virtual_dev_type')
        if raw is not None and str(raw).strip() != '':
            try:
                return int(raw)
            except (TypeError, ValueError):
                pass
        name = ''
        if isinstance(topo_node, dict):
            name = str(topo_node.get('name') or '')
        else:
            name = str(getattr(topo_node, 'name', None) or '')
        name_lower = name.lower()
        if 'rbms' in name_lower or '簇' in name:
            return DeviceTypeMap.CLUSTER_BMS
        if 'bbms' in name_lower or '堆' in name:
            return DeviceTypeMap.STACK_BMS
        return None

    @classmethod
    async def _parent_is_stack_container(cls, query_db: Session, parent_node) -> bool:
        """堆控设备节点，或虚拟设备类型为堆控(1)的虚拟节点，可作为簇/辅机父节点。"""
        if not parent_node:
            return False
        try:
            menu_tree = int(parent_node.menu_tree)
        except (TypeError, ValueError):
            return False
        if menu_tree == 1:
            parent_dev_type = await cls._get_device_type_by_id(query_db, int(parent_node.dev_id or 0))
            return parent_dev_type == DeviceTypeMap.STACK_BMS
        if menu_tree == 0:
            return cls._virtual_dev_type_int(parent_node) == DeviceTypeMap.STACK_BMS
        return False

    @classmethod
    async def _validate_topology_payload(
            cls,
            query_db: Session,
            topology_object: TopologyCreateUpdateModel,
            current_topology_id: int = None
    ):
        parent_id = topology_object.parent_id
        menu_tree = int(topology_object.menu_tree)
        dev_id = topology_object.dev_id

        parent_node = None
        if parent_id is not None:
            parent_node = await TopologyDao.get_topology_detail_by_id(query_db, int(parent_id))
            if not parent_node:
                raise ServiceException(message='Parent topology node does not exist')

        if menu_tree == 1 and not dev_id:
            raise ServiceException(message='Device node must select a device')

        if menu_tree == 0:
            vdt = topology_object.virtual_dev_type
            if vdt is None or str(vdt).strip() == '':
                raise ServiceException(message='Virtual node must select virtual device type')
            try:
                vdt_int = int(vdt)
            except (TypeError, ValueError):
                raise ServiceException(message='Invalid virtual device type')
            if vdt_int == DeviceTypeMap.LOCAL_SYSTEM:
                raise ServiceException(message='Virtual node cannot use local system type')
            return

        if menu_tree != 1:
            return

        device_type = await cls._get_device_type_by_id(query_db, int(dev_id))
        if device_type is None:
            raise ServiceException(message='Selected device does not exist')

        # 本机（Local system）：唯一，且仅允许根节点
        if device_type == DeviceTypeMap.LOCAL_SYSTEM:
            if parent_id is not None:
                raise ServiceException(message='Local system must be the topology root')
            topology_list = await TopologyDao.get_topology_list(query_db)
            for node in topology_list:
                if int(node.get('menu_tree', 0)) != 1:
                    continue
                if int(node.get('device_type', -1)) != DeviceTypeMap.LOCAL_SYSTEM:
                    continue
                if current_topology_id is not None and int(node.get('id')) == int(current_topology_id):
                    continue
                raise ServiceException(message='Only one local system device node is allowed')
            return

        # 堆控：仅允许挂在本机（Local system）下
        if device_type == DeviceTypeMap.STACK_BMS:
            if not parent_node or int(parent_node.menu_tree) != 1:
                raise ServiceException(message='Stack BMS must be placed under Local system')
            parent_dev_type = await cls._get_device_type_by_id(query_db, int(parent_node.dev_id or 0))
            if parent_dev_type != DeviceTypeMap.LOCAL_SYSTEM:
                raise ServiceException(message='Stack BMS can only be placed under Local system')
            return

        # 其余设备类型：挂在堆控设备节点或虚拟堆控节点下
        if not parent_node:
            raise ServiceException(message='Device must be placed under a stack BMS or virtual stack BMS node')
        if not await cls._parent_is_stack_container(query_db, parent_node):
            raise ServiceException(message='Device can only be placed under stack BMS or virtual stack BMS')