from sqlalchemy.orm import Session
from typing import List
from exceptions.exception import ServiceException
from module_device.dao.drynode_dao import DryNodeDao
from module_device.entity.vo.drynode_vo import DryNodeModel
from utils.common_util import CamelCaseUtil
from module_admin.entity.vo.common_vo import CrudResponseModel


class DryNodeService:
    """
    拓扑管理模块服务层
    """

    @classmethod
    def get_dry_node_list_services(cls, query_db: Session):
        """
        获取干节点列表
        """
        dry_node_data = DryNodeDao.get_dry_node_list_dao(query_db)

        di_strategy_name = DryNodeDao.get_dry_node_logic_dict_dao(query_db, 'DI')
        do_strategy_name = DryNodeDao.get_dry_node_logic_dict_dao(query_db, 'DO')
        
        di_strategy_name_dict = {strat.strategy_seq: strat.strategy_name for strat in di_strategy_name}
        di_strategy_ename_dict = {strat.strategy_seq: strat.strategy_ename for strat in di_strategy_name}
        do_strategy_name_dict = {strat.strategy_seq: strat.strategy_name for strat in do_strategy_name}
        do_strategy_ename_dict = {strat.strategy_seq: strat.strategy_ename for strat in do_strategy_name}
        
        di_nodes = []
        do_nodes = []
        
        for item in dry_node_data:            
            node_dict = {
                'id': item.id,
                'seq': item.seq,
                'type': str(item.type),
                'cabinetCode': item.cabinet_code,
                'strategySeq': item.strategy_seq,
                'strategyName': di_strategy_name_dict.get(item.strategy_seq, '') if item.type == 0 else do_strategy_name_dict.get(item.strategy_seq, ''),
                'strategyEname': di_strategy_ename_dict.get(item.strategy_seq, '') if item.type == 0 else do_strategy_ename_dict.get(item.strategy_seq, ''),
                'remark': item.remark
            }
            
            if item.type == 0:
                di_nodes.append(node_dict)
            elif item.type == 1:
                do_nodes.append(node_dict)
        
        return {'DI': di_nodes, 'DO': do_nodes}

    @classmethod
    def get_dry_node_logic_dict_services(cls, query_db: Session, type: str):
        """
        获取干节点逻辑字典
        """
        logic_dict_data = DryNodeDao.get_dry_node_logic_dict_dao(query_db, type)
        logic_dict_list = []
        for item in logic_dict_data:
            logic_dict_list.append({
                'id': item.id,
                'strategySeq': item.strategy_seq,
                'strategyName': item.strategy_name,
                'strategyEname': item.strategy_ename
            })
        return logic_dict_list
        
    @classmethod
    def add_dry_node_services(cls, query_db: Session, dry_node_data: DryNodeModel):
        """
        增加干节点
        """
        try:
            # 查询当前有多少个干节点
            current_dry_nodes = DryNodeDao.get_dry_node_count_by_type(query_db, dry_node_data.type)
            dry_node_data.seq = current_dry_nodes + 1  # 设置seq为点数+1 
            DryNodeDao.add_dry_node_dao(query_db, dry_node_data)
            query_db.commit()
            return CrudResponseModel(is_success=True, code=200, message='Dry node added successfully')
        except Exception as e:
            query_db.rollback()
            raise e
        
    @classmethod
    def edit_dry_node_services(cls, query_db: Session, dry_node_data: DryNodeModel):
        """
        编辑干节点
        """
        try:
            drynode_id = dry_node_data.id
            if not drynode_id:
                raise ServiceException(message='Dry node does not exist')
            drynode = cls.drynode_detail_services(query_db, drynode_id)
            if not drynode:
                raise ServiceException(message='Dry node does not exist')
            
            DryNodeDao.edit_dry_node_dao(query_db, dry_node_data)
            query_db.commit()
            return CrudResponseModel(is_success=True, code=200, message='Dry node updated successfully')
        except Exception as e:
            query_db.rollback()
            raise e

    @classmethod
    def delete_dry_node_services(cls, query_db: Session, delete_list: List[int]):
        """
        删除干节点
        """
        try:
            DryNodeDao.delete_dry_node_dao(query_db, delete_list)
            query_db.commit()
            return CrudResponseModel(is_success=True, code=200, message='Dry node deleted successfully')
        except Exception as e:
            query_db.rollback()
            raise e

    @classmethod
    def drynode_detail_services(cls, query_db: Session, drynode_id: int):
        """
        获取干节点详细信息service

        :param query_db: orm对象
        :param drynode_id: 干节点id
        :return: 干节点id对应的信息
        """
        drynode = DryNodeDao.get_dry_node_by_id(query_db, drynode_id)
        if drynode:
            result = DryNodeModel(**CamelCaseUtil.transform_result(drynode))
        else:
            result = DryNodeModel(**dict())

        return result
