from fastapi import APIRouter
from pydantic import BaseModel

from module_device.service.rtdb_service import (
    init_rtdb,
    init_web_sign,
    get_rtdb_point_value,
    set_rtdb_point_value,
    read_web_sign,
    write_web_sign
)

rtdbController = APIRouter(prefix='/rtdb')

class SetPointValueRequest(BaseModel):
    dev_type: int
    dev_id: int
    point_id: int
    value: float


@rtdbController.get("/init_rtdb")
async def api_init_rtdb():
    """初始化RTDB实时库"""
    return {"status": init_rtdb()}

@rtdbController.get("/init_web_sign") 
async def api_init_web_sign():
    """初始化Web配置通知组件"""
    return {"status": init_web_sign()}

@rtdbController.get("/get_point_value")
async def api_get_rtdb_point_value(dev_type: int, dev_id: int, point_id: int):
    """获取RTDB测点值
    
    Args:
        dev_type: 设备类型
        dev_id: 设备序号(从0开始)
        point_id: 设备点号(从0开始)
    """
    value = get_rtdb_point_value(dev_type, dev_id, point_id)
    return {
        "dev_type": dev_type,
        "dev_id": dev_id, 
        "point_id": point_id,
        "value": value
    }

@rtdbController.post("/set_point_value")
async def api_set_rtdb_point_value(request: SetPointValueRequest):
    """写入RTDB测点值

    Args:
        dev_type: 设备类型
        dev_id: 设备序号(从0开始) 
        point_id: 设备点号(从0开始)
        value: 写入的值"""
    set_rtdb_point_value(request.dev_type, request.dev_id, request.point_id, request.value)
    return {"status": "success"}

@rtdbController.get("/read_web_sign")
async def api_read_web_sign(sign_type: int):
    """读取Web配置通知信号"""
    return {
        "sign_type": sign_type,
        "value": read_web_sign(sign_type)
    }

@rtdbController.post("/write_web_sign")
async def api_write_web_sign(sign_type: int, value: int):
    """写入Web配置通知信号"""
    write_web_sign(sign_type, value)
    return {"status": "success"}
