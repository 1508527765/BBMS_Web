from sqlalchemy import select, update, delete
from sqlalchemy.orm import Session

from datetime import datetime, timezone

from module_device.entity.do.history_do import TimedDataStorage, FaultsRecord, EventsRecord
from module_device.entity.vo.history_vo import DataQueryModel, EventQueryModel
from typing import List
from utils.page_util import PageUtil
from utils.common_util import CamelCaseUtil, export_list2excel


class HistoryDao:
    """
    历史数据管理模块数据库操作层
    """
    @classmethod
    async def get_timed_data_list_dao(cls, db: Session, query_object: DataQueryModel, is_page):
        """
        获取历史数据列表
        """
        start_time_dt = datetime.fromtimestamp(query_object.start_time)
        end_time_dt = datetime.fromtimestamp(query_object.end_time)
        
        query = (
            select(TimedDataStorage)
            .where(
                TimedDataStorage.dev_type == query_object.dev_type,
                TimedDataStorage.dev_id == query_object.dev_code,
                TimedDataStorage.created_time >= start_time_dt,
                TimedDataStorage.created_time <= end_time_dt,
            )
            .order_by(TimedDataStorage.created_time.asc())
        )
        timed_data_list = await PageUtil.paginate(db, query, query_object.page_num, query_object.page_size, is_page)

        return timed_data_list


    @classmethod
    async def get_event_data_list_dao(cls, db: Session, query_object: EventQueryModel, is_page: bool):
        """
        获取历史数据列表
        """
        start_time_dt = datetime.fromtimestamp(query_object.start_time)
        end_time_dt = datetime.fromtimestamp(query_object.end_time)

        query_model = EventsRecord if query_object.type == 'event' else FaultsRecord
        query = select(query_model).where(
            query_model.created_time >= start_time_dt,
            query_model.created_time <= end_time_dt
        ).order_by(query_model.created_time.desc())

        if query_object.dev_type is not None:
            query = query.where(
                query_model.dev_type == query_object.dev_type
            )
        if query_object.dev_code is not None:
            query = query.where(
                query_model.dev_id == query_object.dev_code
            )

        return await PageUtil.paginate(db, query, query_object.page_num, query_object.page_size, is_page)

