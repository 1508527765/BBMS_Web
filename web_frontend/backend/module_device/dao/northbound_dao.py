from sqlalchemy import delete, select
from sqlalchemy.orm import Session
from typing import List
from module_device.entity.do.north_config_do import NorthConfig
from module_device.entity.vo.device_vo import NorthConfigPageQueryModel
from utils.page_util import PageUtil


class NorthboundDao:
    """
    设备管理模块数据库操作层
    """

    @classmethod
    async def get_northbound_config_detail_by_id(cls, db: Session, config_id: int):
        """
        根据北向配置id获取北向配置详细信息

        :param db: orm对象
        :param config_id: 北向配置id
        :return: 北向配置信息对象
        """
        config_info = (db.execute(select(NorthConfig).where(NorthConfig.id == config_id))).scalars().first()

        return config_info

    @classmethod
    async def get_northbound_config_list(cls, db: Session, query_object: NorthConfigPageQueryModel, is_page: bool = False):
        """
        根据查询参数获取北向配置列表信息

        :param db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 北向配置列表信息对象
        """
        query = (
            select(NorthConfig)
            .where(
                NorthConfig.name.like(f'%{query_object.name}%') if query_object.name else True,
                NorthConfig.protocol_type == query_object.protocol_type if query_object.protocol_type is not None else True,
                NorthConfig.template_id == query_object.template_id if query_object.template_id is not None else True,
            )
            .order_by(NorthConfig.id)
            .distinct()
        )
        config_list = await PageUtil.paginate(db, query, query_object.page_num, query_object.page_size, is_page)

        return config_list
    
    @classmethod    
    async def get_all_northbound_device(cls, db: Session, name: str = None):
        """
        获取所有北向配置信息
        """
        query = select(NorthConfig).where(NorthConfig.name.like(f'%{name}%') if name else True)
        config_list = await PageUtil.paginate(db, query, 1, 10000, False)

        return config_list

    @classmethod
    async def add_northbound_config_dao(cls, db: Session, config: dict):
        """
        新增北向配置数据库操作

        :param db: orm对象
        :param config: 北向配置对象
        :return:
        """
        db_config = NorthConfig(**config)
        db.add(db_config)
        db.flush()

        return db_config

    @classmethod
    async def delete_northbound_config_dao(cls, db: Session, config_ids: List[int]):
        """
        删除北向配置数据库操作

        :param db: orm对象
        :param config_ids: 北向配置id列表
        :return:
        """
        db.execute(delete(NorthConfig).where(NorthConfig.id.in_(config_ids)))

    @classmethod
    async def get_northbound_config_by_id(cls, db: Session, config_id: int):
        """
        根据北向配置 id 查询北向配置表中的记录
        """
        config_info = db.execute(select(NorthConfig).where(NorthConfig.id == config_id)).scalars().first()
        return config_info
    
    @classmethod
    async def get_northbound_configs_by_ids(cls, db: Session, config_ids: list):
        """
        根据北向配置 ID 列表查询北向配置表中的记录
        """
        return db.execute(
            select(NorthConfig).where(NorthConfig.id.in_(config_ids))
        ).scalars().all()
