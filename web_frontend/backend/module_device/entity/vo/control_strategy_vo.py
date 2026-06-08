from pydantic import BaseModel, field_validator
from typing import Union, List


class UpdateConfigModel(BaseModel):
    point_id: int
    new_value: float

    @field_validator('new_value', mode='before')
    def convert_hex_to_float(cls, v):
        if isinstance(v, str) and v.startswith('0x'):
            # 将十六进制字符串转为十进制整数，再转为 float
            return float(int(v, 16))
        return float(v)  # 其他情况直接转 float


class CanDeviceModel(BaseModel):
    can_channel: int
    can_count: int
    device_code: List[int]


class UpdateRequest(BaseModel):
    type: str
    update_data: List[UpdateConfigModel]
    update_device_list: List[CanDeviceModel] = []


class PcsItem(BaseModel):
    pcs_id: int  # 表示编号，第几个PCS，从1开始
    action: str  # 'on' | 'off' | 'reset'