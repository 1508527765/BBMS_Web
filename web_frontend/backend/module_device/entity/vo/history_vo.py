from pydantic import BaseModel, ConfigDict, Field
from pydantic.alias_generators import to_camel
from typing import Optional
from datetime import datetime


class DataQueryModel(BaseModel):
    """
    当天数据查询模型，查询条件：设备类型、设备序号，查询起始结束时间
    """

    # model_config = ConfigDict(alias_generator=to_camel, from_attributes=True)

    dev_type: int
    dev_code: int
    dev_name: str = None
    date: str
    start_time: int
    end_time: int
    page_num: int = 1
    page_size: int = 30


class EventQueryModel(BaseModel):
    """
    事件查询模型，查询条件：设备类型、设备序号，查询起始结束时间
    """

    # model_config = ConfigDict(alias_generator=to_camel, from_attributes=True)
    dev_type: Optional[int] = None
    dev_code: Optional[int] = None
    start_time: int
    end_time: int
    page_num: int = 1
    page_size: int = 30
    type: str
