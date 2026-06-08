from pydantic import BaseModel, ConfigDict, Field
from pydantic.alias_generators import to_camel
from typing import Optional, List


class TopologyCreateUpdateModel(BaseModel):
    
    model_config = ConfigDict(alias_generator=to_camel)

    id: Optional[int] = None
    parent_id: Optional[int] = None  # 根节点时为 None
    menu_tree: int  # 0: 虚拟节点, 1: 设备节点
    name: str = ''
    dev_id: Optional[int] = None  # 设备节点时必填
    virtual_dev_type: Optional[int] = None  # 虚拟节点时必填，对应 dev_type_e


class TopologyModel(BaseModel):
    model_config = ConfigDict(alias_generator=to_camel)

    id: int
    name: str
    parent_id: int
    menu_tree: int
    dev_id: Optional[int] = None
    virtual_dev_type: Optional[int] = None
