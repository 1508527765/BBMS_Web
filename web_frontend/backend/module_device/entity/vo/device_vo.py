from pydantic import BaseModel, Field, ConfigDict
from pydantic.alias_generators import to_camel
from typing import Optional, Any
from module_device.annotation.pydantic_annotation import as_query


class DeviceModel(BaseModel):
    """
    设备模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    id: Optional[int] = Field(default=None, description='ID')
    sn: str = Field(default="0", description='设备序列号')
    type: int
    code: int
    name: str
    protocol_type: int
    template_id: Optional[int] = None
    sort_order: Optional[int]
    remark: Optional[str]
    status: Optional[int]


class ComprehensiveDeviceModel(BaseModel):
    """
    综合设备模型，包含所有字段
    """

    model_config = ConfigDict(alias_generator=to_camel)

    # DeviceCreateUpdateModel fields
    id: Optional[int] = Field(default=None, description='ID')
    sn: str = Field(default="0", description='设备序列号')
    type: int
    code: int = 0
    name: str
    protocol_type: int
    template_id: Optional[int] = None
    sort_order: Optional[int] = 0
    remark: Optional[str] = ""
    status: Optional[int] = 1

    # ModbusRtuDeviceModel fields
    uartId: Optional[int] = None
    address: Optional[str] = None
    baud: Optional[int] = None
    data: Optional[int] = None
    parity: Optional[int] = None
    stop: Optional[int] = None

    # CanDeviceModel fields
    canId: Optional[int] = None

    # ModbusTcpDeviceModel fields
    ip: Optional[str] = None
    port: Optional[int] = None

    # Common fields for ModbusRtuDeviceModel, CanDeviceModel, ModbusTcpDeviceModel
    timeout: Optional[int] = None
    uId: Optional[int] = None


class DeviceCreateUpdateModel(BaseModel):
    """
    设备创建更新模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    id: Optional[int] = Field(default=None, description='ID')
    sn: str = Field(default="0", description='设备序列号')
    type: int
    code: int = 0
    name: str
    protocol_type: int
    template_id: Optional[int] = None
    sort_order: Optional[int] = 0
    remark: Optional[str] = ""
    status: Optional[int] = 1


class ModbusRtuDeviceModel(DeviceCreateUpdateModel):
    """
    ModbusRtu设备模型
    """
    uartId: Optional[int]
    address: Optional[str]
    baud: Optional[int]
    data: Optional[int]
    parity: Optional[int]
    stop: Optional[int]

    timeout: Optional[int]
    uId: Optional[int]


class CanDeviceModel(DeviceCreateUpdateModel):
    """
    CAN设备模型
    """
    canId: Optional[int]
    baud: Optional[int]

    timeout: Optional[int]
    uId: Optional[int]


class ModbusTcpDeviceModel(DeviceCreateUpdateModel):
    """
    ModbusTcp设备模型
    """
    ip: Optional[str]
    port: Optional[int]
    
    timeout: Optional[int]
    uId: Optional[int]


class DeleteDeviceModel(BaseModel):
    """
    删除设备模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    device_ids: str = Field(description='需要刪除的设备ID')


@as_query
class DevicePageQueryModel(BaseModel):
    """
    设备分页查询模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    page_num: int = Field(default=1, description='当前页数')
    page_size: int = Field(default=10, description='每页显示数量')

    code: Optional[int] = Field(default=None, description='设备编号')
    type: Optional[int] = Field(default=None, description='设备类型')
    name: Optional[str] = Field(default=None, description='设备名称')
    protocol_type: Optional[int] = Field(default=None, description='协议类型')


@as_query
class NorthConfigPageQueryModel(BaseModel):
    """
    北向配置分页查询模型
    """

    model_config = ConfigDict(alias_generator=to_camel)

    page_num: int = Field(default=1, description='当前页数')
    page_size: int = Field(default=10, description='每页显示数量')

    name: Optional[str] = Field(default=None, description='配置名称')
    protocol_type: Optional[int] = Field(default=None, description='协议类型')
    template_id: Optional[int] = Field(default=None, description='模板ID')


class CrudResponseModel(BaseModel):
    """
    操作响应模型
    """

    is_success: bool = Field(description='操作是否成功')
    message: str = Field(description='响应信息')
    result: Optional[Any] = Field(default=None, description='响应结果')


class ProtocolFactory:
    @staticmethod
    def create_protocol_model(protocol_data: dict) -> DeviceCreateUpdateModel:
        from module_device.constant.device_constant import ProtocolTypeMap

        protocol_type = protocol_data.get('protocolType')
        if protocol_type in (
            ProtocolTypeMap.MODBUS_TCP_MASTER,
            ProtocolTypeMap.RBMS_TCP_PRIVATE,
            ProtocolTypeMap.MODBUS_MULTI_ID_TCP,
        ):
            return ModbusTcpDeviceModel(**protocol_data)
        if protocol_type == ProtocolTypeMap.MODBUS_RTU_MASTER:
            return ModbusRtuDeviceModel(**protocol_data)
        if protocol_type in (
            ProtocolTypeMap.RBMS_CAN_PRIVATE,
            ProtocolTypeMap.TMS_PRIVATE_CAN,
        ):
            return CanDeviceModel(**protocol_data)
        return DeviceCreateUpdateModel(**protocol_data)


class DspControlModel(BaseModel):

    model_config = ConfigDict(alias_generator=to_camel)
    
    dsp_id: int
    power_switch: int
    run_switch: int