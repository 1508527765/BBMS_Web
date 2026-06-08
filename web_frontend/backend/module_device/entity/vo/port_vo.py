from pydantic import BaseModel, Field, ConfigDict
from pydantic.alias_generators import to_camel
from module_device.annotation.pydantic_annotation import as_query
from typing import Optional, Any


class PortModel(BaseModel):
    """
    端口模型
    """

    id: Optional[int] = Field(default=None, description='ID')
    type: int
    name: str


class SerialPortModel(PortModel):
    """
    串口模型
    """
    uartId: Optional[int]
    address: Optional[str]
    baud: Optional[int]
    data: Optional[int]
    parity: Optional[int]
    stop: Optional[int]


class NetPortModel(PortModel):
    """
    网络端口模型
    """
    netId: Optional[int]
    isDHCP: Optional[int]
    ip: Optional[str]
    mask: Optional[str]
    gateway: Optional[str]
    address: Optional[str]


class WifiPortModel(PortModel):
    """
    无线端口模型
    """
    wifiName: Optional[str]
    wifiPassword: Optional[str]
    enable: Optional[int]


class FourGPortModel(PortModel):
    """
    4G端口模型
    """
    cmdContent: Optional[str]
    enable: Optional[int]


class TimeSyncPortModel(PortModel):
    """
    时间同步端口模型
    """
    isNtp: Optional[int]
    address: Optional[str]
    port: Optional[int]
    manualTime: Optional[str]


class CanPortModel(PortModel):
    """
    CAN端口模型
    """
    canId: Optional[int]
    address: Optional[str]
    baud: Optional[int]


class DeletePortModel(BaseModel):
    """
    删除端口模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    port_ids: str = Field(description='需要删除的端口ID')

@as_query
class PortPageQueryModel(BaseModel):
    """
    端口分页查询模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    page_num: int = Field(default=1, description='当前页数')
    page_size: int = Field(default=10, description='每页显示数量')
    type: Optional[int] = Field(default=None, description='端口类型')

class CrudResponseModel(BaseModel):
    """
    操作响应模型
    """

    is_success: bool = Field(description='操作是否成功')
    message: str = Field(description='响应信息')
    result: Optional[Any] = Field(default=None, description='响应结果')
