import ctypes
import math
import os
import sys
from enum import IntEnum

# 定义枚举类型
class RtdbType(IntEnum):
    Rtdb_ShMem = 0  # 实时库类型 - 共享内存

class SignShareType(IntEnum):
    Sign_ShMem = 0  # 通知组件类型 - 共享内存

class ShmCreateType(IntEnum):
    Shm_Create = 0  # 创建共享内存

# 加载动态库（实时场景必须存在，缺失时直接抛错避免返回“模拟值”）
rtdb = None
if sys.platform != "win32":
    LIB_PATH = "/usr/lib/aarch64-linux-gnu/libcrtdb.so"
    try:
        rtdb = ctypes.CDLL(LIB_PATH)
        rtdb.initRtdb.restype = ctypes.c_uint8
        rtdb.getRtdbPointValue.restype = ctypes.c_double
        rtdb.readWebSign.restype = ctypes.c_uint16
        rtdb.initWebSign.restype = ctypes.c_uint8
        rtdb.setRtdbPointValue.restype = None
        rtdb.initRtdb.argtypes = [ctypes.c_int, ctypes.c_void_p, ctypes.c_int]
        rtdb.getRtdbPointValue.argtypes = [ctypes.c_int, ctypes.c_uint16, ctypes.c_uint16, ctypes.c_uint16]
        rtdb.setRtdbPointValue.argtypes = [ctypes.c_int, ctypes.c_uint16, ctypes.c_uint16, ctypes.c_uint16, ctypes.c_double]
        rtdb.initWebSign.argtypes = [ctypes.c_int, ctypes.c_int]
        rtdb.readWebSign.argtypes = [ctypes.c_int, ctypes.c_int]
        rtdb.writeWebSign.argtypes = [ctypes.c_int, ctypes.c_int, ctypes.c_uint16]
    except (OSError, FileNotFoundError):
        rtdb = None


def _require_rtdb():
    if rtdb is None:
        raise RuntimeError("RTDB library unavailable: /usr/lib/aarch64-linux-gnu/libcrtdb.so")


def init_rtdb():
    """ 
    初始化RTDB
    :return: 0-成功 1-失败
    """
    _require_rtdb()
    rtdb.initRtdb(RtdbType.Rtdb_ShMem, 0, ShmCreateType.Shm_Create)
    print("Real-time database initialization succeeded")

def init_web_sign():
    """ 
    初始化Web配置通知组件
    :return: 0-成功 1-失败
    """
    _require_rtdb()
    rtdb.initWebSign(SignShareType.Sign_ShMem, ShmCreateType.Shm_Create)
    print("Web configuration notification component initialization succeeded")

def sanitize_rtdb_value(value):
    """NaN/Inf 无法 JSON 序列化，统一转为 None 供前端显示为 --。"""
    if isinstance(value, float) and (math.isnan(value) or math.isinf(value)):
        return None
    return value


def get_rtdb_point_value(dev_type: int, dev_id: int, point_id: int) -> float:
    """ 
    获取RTDB测点值
    :param dev_type: 设备类型
    :param dev_id: 设备序号(从0开始)
    :param point_id: 测点号(从0开始)
    :return: 测点值
    """
    _require_rtdb()
    raw = rtdb.getRtdbPointValue(RtdbType.Rtdb_ShMem, dev_type, dev_id, point_id)
    return sanitize_rtdb_value(raw)

def set_rtdb_point_value(dev_type: int, dev_id: int, point_id: int, value: float):
    """
    设置RTDB测点值
    :param dev_type: 设备类型
    :param dev_id: 设备序号(从0开始) 
    :param point_id: 测点号(从0开始)
    :param value: 要设置的值
    """
    _require_rtdb()
    rtdb.setRtdbPointValue(RtdbType.Rtdb_ShMem, dev_type, dev_id, point_id, value)

def read_web_sign(sign_type: int) -> int:
    """ 
    读取Web配置通知值
    :param sign_type: 通知类型
    :return: 通知值
    """
    _require_rtdb()
    return rtdb.readWebSign(SignShareType.Sign_ShMem, sign_type)

def write_web_sign(sign_type: int, value: int):
    """
    写入Web配置通知值
    :param sign_type: 通知类型
    :param value: 通知值
    """
    _require_rtdb()
    rtdb.writeWebSign(SignShareType.Sign_ShMem, sign_type, value)
