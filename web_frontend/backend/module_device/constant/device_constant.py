# -*- coding: utf-8 -*-
"""
设备与协议常量（与 C kit_data.h 一致）：
- DeviceTypeMap / RtdbDevType：dev_type_e，库 device.type 直接存 C 编号
- ProtocolTypeMap：protocol_type_master_e，库 protocol_type 直接存 C 编号
"""


# 本机系统设备（库 id=1，与 RTDB SCU 等约定一致）
LOCAL_SYSTEM_DEVICE_ID = 1
LOCAL_SYSTEM_DEVICE_NAME = "Local system"


class ProtocolTypeMap:
    """
    主站通讯协议类型：与 C protocol_type_master_e（kit_data.h）一致。
    0..7 可写入库；8=kProto_Master_End 为枚举结束标记，不可作为设备协议。
    """

    LOCAL_INFO = 0  # kProto_Master_Info_Master
    BBMS_PRIVATE = 1  # kProto_BbmsPrivate_Master
    MODBUS_TCP_MASTER = 2  # kProto_ModbusTCP_Master
    MODBUS_RTU_MASTER = 3  # kProto_ModbusRTU_Master
    RBMS_TCP_PRIVATE = 4  # kProto_RbmsPrivate_TCP_Master
    RBMS_CAN_PRIVATE = 5  # kProto_RbmsPrivate_Can_Master
    MODBUS_MULTI_ID_TCP = 6  # kProto_ModbusTCPMulti_Master
    TMS_PRIVATE_CAN = 7  # kProto_TMSPrivateCan_Master
    MASTER_END = 8  # kProto_Master_End，禁止写入 device/template


class DeviceGroupMap:
    TELECONTROL_SIGNAL = 0
    TELEMETRY_SIGNAL = 1
    REMOTE_CONTROL = 2
    REMOTE_ADJUSTMENT = 3
    SETTING = 4
    TELEPULSE = 5
    FAULT = 6
    EVENT = 7


class RtdbDevType:
    """RTDB / C 侧 dev_type_e（kit_data.h）"""

    SCU = 0  # kDev_Type_Con
    BBMS = 1  # kDev_Type_BBMS
    RBMS = 2  # kDev_Type_RBMS
    PCS = 3  # kDev_Type_PCS
    TRANSFORMER = 4  # kDev_Type_Transfer
    AIR_CONDITIONER_LIQUID_COOLING_UNIT = 5  # kDev_Type_Lqd
    ELECTRIC_METER_DEVICE = 6  # kDev_Type_Meter
    UPS_DEVICE = 7  # kDev_Type_UPS
    FIRE_DEVICE = 8  # kDev_Type_FireDevice
    DEHUMIDIFIER = 9  # kDev_Type_dehumidifier
    TEMP_SENSOR = 10
    HUMIDITY_SENSOR = 11
    WATER_SENSOR = 12
    SMOKE_SENSOR = 13
    OTHER_SENSOR_1 = 14
    OTHER_SENSOR_2 = 15
    OTHER_SENSOR_3 = 16
    # 兼容旧名（勿在新代码中使用）
    DSP = PCS
    STACK_CONTROL = BBMS


class DeviceTypeMap:
    """库 device.type / 前端 deviceTypes.js，与 C dev_type_e 一致"""

    LOCAL_SYSTEM = 0
    STACK_BMS = 1
    CLUSTER_BMS = 2
    PCS = 3
    TRANSFORMER = 4
    AC_LIQUID = 5
    METER = 6
    UPS = 7
    FIRE = 8
    DEHUMIDIFIER = 9
    TEMP_SENSOR = 10
    HUMIDITY_SENSOR = 11
    WATER_SENSOR = 12
    SMOKE_SENSOR = 13
    OTHER_SENSOR_1 = 14
    OTHER_SENSOR_2 = 15
    OTHER_SENSOR_3 = 16
    RESERVED_17 = 17
    RESERVED_18 = 18
    RESERVED_19 = 19
    RESERVED_20 = 20
    RESERVED_21 = 21
    RESERVED_22 = 22
    RESERVED_23 = 23
    RESERVED_24 = 24
    RESERVED_25 = 25
    RESERVED_26 = 26
    RESERVED_27 = 27
    RESERVED_28 = 28
    RESERVED_29 = 29

    SCU = LOCAL_SYSTEM


def apply_local_system_display_name(row: dict) -> None:
    """
    列表/详情 API 返回前：本机设备固定展示名为 Local system（库中可能仍为历史名 BBMS）。
    """
    if not row or not isinstance(row, dict):
        return
    rid = row.get("id")
    try:
        if rid is not None and int(rid) == LOCAL_SYSTEM_DEVICE_ID:
            row["name"] = LOCAL_SYSTEM_DEVICE_NAME
    except (TypeError, ValueError):
        pass


def web_device_type_to_rtdb(web_type: int) -> int:
    """库 device.type 已与 C dev_type_e 对齐，直接作为 RTDB 设备类型使用。"""
    return int(web_type)
