from sqlalchemy import delete, select, update
from sqlalchemy.orm import Session
from typing import List
from module_device.constant.device_constant import (
    LOCAL_SYSTEM_DEVICE_ID,
    LOCAL_SYSTEM_DEVICE_NAME,
)
from module_device.entity.do.device_do import Device
from module_device.entity.do.topology_do import Topology
from module_device.entity.vo.topology_vo import TopologyCreateUpdateModel


class TopologyDao:
    """
    拓扑管理模块数据库操作层
    """

    @classmethod
    async def get_topology_detail_by_id(cls, db: Session, topology_id: int):
        """
        根据拓扑id获取拓扑详细信息

        :param db: orm对象
        :param topology_id: 拓扑id
        :return: 拓扑信息对象
        """
        topology_info = (db.execute(select(Topology).where(Topology.id == topology_id))).scalars().first()

        return topology_info


    @classmethod
    async def get_topology_list(cls, db: Session):
        """
        获取拓扑列表信息

        :param db: orm对象
        :return: 拓扑列表信息对象
        """
        query = select(Topology).order_by(Topology.id).distinct()
        topology_list = db.execute(query).scalars().all()

        # 获取每个topology的dev_id对应的设备信息
        topology_list_result = []
        for topology in topology_list:
            topology_dict = dict(
                id=topology.id,
                name=topology.name,
                dev_id=topology.dev_id,
                parent_id=topology.parent_id,
                menu_tree=topology.menu_tree,
                virtual_dev_type=getattr(topology, "virtual_dev_type", None),
            )
            if topology_dict.get('dev_id'):
                device_info = db.execute(
                    select(Device.code, Device.name, Device.type, Device.template_id).where(Device.id == topology.dev_id)
                ).first()
                if device_info:
                    topology_dict['device_code'] = device_info.code
                    topology_dict['device_name'] = device_info.name
                    if int(topology_dict.get('dev_id') or 0) == LOCAL_SYSTEM_DEVICE_ID:
                        topology_dict['device_name'] = LOCAL_SYSTEM_DEVICE_NAME
                    topology_dict['device_type'] = device_info.type
                    topology_dict['template_id'] = device_info.template_id
            topology_list_result.append(topology_dict)
        return topology_list_result

    @classmethod
    async def add_topology_dao(cls, db: Session, node: TopologyCreateUpdateModel):
        """
        新增拓扑数据库操作
        """
        new_node = Topology(
            parent_id=node.parent_id if node.parent_id is not None else -999,
            menu_tree=node.menu_tree,
            name=node.name,
            dev_id=node.dev_id,
            virtual_dev_type=node.virtual_dev_type,
            sort_order=0  # todo:先预设为 0，之后可根据需求排序
        )
        db.add(new_node)
        db.flush()
        return new_node

    @classmethod
    async def edit_topology_dao(cls, db: Session, node: dict):
        """
        编辑拓扑数据库操作
        """
        db.execute(update(Topology).where(Topology.id == node['id']).values(node))

    @classmethod
    async def delete_topology_dao(cls, db: Session, topology_ids: List[int]):
        """
        递归删除拓扑结构中的节点及其子孙节点
        :param db: orm对象
        :param topology_ids: 拓扑id列表
        """
        # 获取所有要删除的节点ID（包含子孙节点）
        all_ids = set(topology_ids)
        queue = topology_ids[:]  # 复制一份用于遍历

        while queue:
            current_id = queue.pop()
            result = db.execute(select(Topology.id).where(Topology.parent_id == current_id))
            children_ids = result.scalars().all()  # 获取所有 ID

            if children_ids:
                all_ids.update(children_ids)
                queue.extend(children_ids)

        # 执行删除
        db.execute(delete(Topology).where(Topology.id.in_(all_ids)))

    @classmethod
    async def get_all_topology_data(cls, db: Session):
        """
        获取所有拓扑数据, 仅用于导出
        """
        query = select(Topology).order_by(Topology.id)
        topology_data = db.execute(query).scalars().all()
        return topology_data
    
    @classmethod
    async def add_topologies_dao(cls, db: Session, nodes: List[dict]):
        """
        批量增加拓扑数据
        :param db: orm对象
        :param nodes: 拓扑数据列表，每个元素为一个包含拓扑信息的字典
        """
        new_nodes = [
            Topology(
                parent_id=node.get('parent_id', -999),
                menu_tree=node.get('menu_tree'),
                name=node.get('name'),
                dev_id=node.get('dev_id'),
                sort_order=node.get('sort_order', 0)
            )
            for node in nodes
        ]

        db.add_all(new_nodes)
        db.flush()
        return new_nodes

    @classmethod
    async def delete_all_topology_data(cls, db: Session):
        """
        删除所有拓扑数据
        """
        db.execute(delete(Topology))
    
    @classmethod
    async def check_device_in_topology(cls, db: Session, device_id: int):
        """
        检查设备是否在设备拓扑中
        """
        return db.query(Topology).filter(Topology.dev_id == device_id).count() > 0

