from sqlalchemy import select, update, delete
from sqlalchemy.orm import Session
from module_device.entity.do.drynode_do import DiLogicDict, DoLogicDict, DiDoSet
from module_device.entity.vo.drynode_vo import DryNodeModel
from typing import List
from utils.page_util import PageUtil
from utils.common_util import CamelCaseUtil, export_list2excel


class DryNodeDao:
    """
    干节点管理模块数据库操作层
    """

    @classmethod
    def get_dry_node_list_dao(cls, db: Session):
        """
        获取干节点列表
        """
        return db.execute(select(DiDoSet)).scalars().all()

    @classmethod
    def get_dry_node_logic_dict_dao(cls, db: Session, type: str):
        """
        获取干节点逻辑字典
        """
        if type == 'DI':
            return db.execute(select(DiLogicDict)).scalars().all()
        elif type == 'DO':
            return db.execute(select(DoLogicDict)).scalars().all()
        else:
            return []

    @classmethod
    def add_dry_node_dao(cls, db: Session, dry_node_data: DryNodeModel):
        db_dry_node = DiDoSet(**dry_node_data.model_dump())
        db.add(db_dry_node)
        db.flush()

        return db_dry_node
    
    @classmethod
    def edit_dry_node_dao(cls, db: Session, dry_node_data: DryNodeModel):
        """
        编辑干节点数据库操作

        :param db: orm对象
        :param dry_node_data: 需要更新的干节点字典
        :return:
        """
        db.query(DiDoSet).filter(DiDoSet.id == dry_node_data.id).update(
            dry_node_data.model_dump(),
            synchronize_session=False
        )
        db.commit()

    @classmethod
    def delete_dry_node_dao(cls, db: Session, delete_list: List[int]):
        """
        删除干节点数据库操作

        :param db: orm对象
        :param dry_node: 干节点对象
        :return:
        """
        db.execute(delete(DiDoSet).where(DiDoSet.id.in_(delete_list)))

    @classmethod
    def dry_node_detail_services(cls, query_db: Session, dry_node_id: int):
        """
        获取干节点详细信息service

        :param query_db: orm对象
        :param dry_node_id: 干节点id
        :return: 干节点id对应的信息
        """
        dry_node = DryNodeDao.get_dry_node_detail_by_id(query_db, dry_node_id=dry_node_id)
        if dry_node:
            result = DryNodeModel(**CamelCaseUtil.transform_result(dry_node))
        else:
            result = DryNodeModel(**dict())

        return result
    
    @classmethod
    def get_dry_node_count_by_type(cls, db: Session, node_type: int) -> int:
        """
        根据类型获取有多少个当前类型节点

        :param db: orm对象
        :param node_type: 节点类型
        :return: 当前类型节点的数量
        """
        count = db.query(DiDoSet).filter(DiDoSet.type == node_type).count()
        return count
    
    @classmethod
    def get_dry_node_by_id(cls, db: Session, dry_node_id: int):
        """
        根据id获取干节点
        """
        return db.query(DiDoSet).filter(DiDoSet.id == dry_node_id).first()
    
    @classmethod
    def delete_all_drynode_data(cls, db: Session):
        """
        删除所有干节点数据
        """
        db.execute(delete(DiDoSet))

    @classmethod
    def get_all_drynode_data(cls, db: Session):
        """
        获取所有干节点数据
        """
        return db.execute(select(DiDoSet).order_by(DiDoSet.type, DiDoSet.seq)).scalars().all()

