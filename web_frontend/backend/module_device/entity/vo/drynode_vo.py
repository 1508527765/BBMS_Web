from pydantic import BaseModel, ConfigDict, Field
from pydantic.alias_generators import to_camel
from typing import Optional, List

class DryNodeListModel(BaseModel):
    id: int
    seq: int
    type: int
    cabinetCode: int
    strategySeq: int
    remark: str

class DryNodeLogicListModel(BaseModel):
    id: int
    strategySeq: int
    strategyName: str

class DryNodeModel(BaseModel):
    """
    干节点表对应pydantic模型
    """

    model_config = ConfigDict(alias_generator=to_camel, from_attributes=True)

    id: Optional[int] = None
    type: int = 0
    seq: Optional[int] = None
    cabinet_code: int = 0
    strategy_seq: int = 0
    remark: Optional[str] = None

