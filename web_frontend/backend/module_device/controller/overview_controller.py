from fastapi import APIRouter, Depends, Request, Body
from typing import List
from sqlalchemy.orm import Session
from config.get_db import get_ems_db

from module_device.constant.device_constant import RtdbDevType
from module_device.service.device_service import DeviceService
from module_device.entity.vo.device_vo import DspControlModel
from module_device.service.rtdb_service import get_rtdb_point_value, set_rtdb_point_value
from module_device.service.overview_service import build_bbms_overview_payload
from utils.response_util import ResponseUtil


overviewController = APIRouter(prefix='/overview')


@overviewController.get('/combined-info')
async def get_combined_info(
    request: Request,
    query_db: Session = Depends(get_ems_db),
):
    """
    获取拓扑信息、电站信息和储能统计信息
    """
    # 保留两位小数，直接舍弃第三位及之后的数值
    def truncate_to_two_decimals(value):
        return int(value * 100) / 100

    # 获取拓扑信息
    ret = {
        "dsp": {},
        "radiator": {},
        "metering": [],
        "transformer": {}
    }
    device_dict = await DeviceService.get_dsp_devices(query_db)
    dsp_info = {}
    for channel, device_codes in device_dict.items():
        dsp_info[str(channel + 1)] = []
        for device_code in device_codes:
            device_info = {}
            device_index = device_code - 1
            device_info['deviceId'] = device_code
            device_info['voltage'] = get_rtdb_point_value(RtdbDevType.DSP, device_index, 161)
            device_info['current'] = get_rtdb_point_value(RtdbDevType.DSP, device_index, 162)
            device_info['activePower'] = get_rtdb_point_value(RtdbDevType.DSP, device_index, 163)
            device_info['reactivePower'] = get_rtdb_point_value(RtdbDevType.DSP, device_index, 164)
            device_info['statusDisplay'] = get_rtdb_point_value(RtdbDevType.DSP, device_index, 217)
            device_info['powerFactor'] = get_rtdb_point_value(RtdbDevType.DSP, device_index, 153)
            modulation_index = get_rtdb_point_value(RtdbDevType.DSP, device_index, 213)
            device_info['modulationIndex'] = int(modulation_index * 100) / 100
            # 开关机状态/运行状态
            device_info['powerSwitch'] = get_rtdb_point_value(RtdbDevType.DSP, device_index, 272)
            device_info['runSwitch'] = get_rtdb_point_value(RtdbDevType.DSP, device_index, 273)
            # 使用硬编码将runSwitch=3的情况改为5, 方便前端进行展示
            if device_info['runSwitch'] == 3:
                device_info['runSwitch'] = 5
            # 告警状态
            device_info['alarmStatus'] = await DeviceService.judge_alarm_exist(query_db, device_code)

            dsp_info[str(channel + 1)].append(device_info)
    ret['dsp'] = dsp_info

    # 液冷机
    radiator_device = await DeviceService.get_radiator_devices_info(query_db)
    radiator_info = {}
    if radiator_device:
        device_index = radiator_device.code - 1
        radiator_info['runStatus'] = get_rtdb_point_value(RtdbDevType.AIR_CONDITIONER_LIQUID_COOLING_UNIT, device_index, 60)
        radiator_info['supplyLiquidTemp'] = get_rtdb_point_value(RtdbDevType.AIR_CONDITIONER_LIQUID_COOLING_UNIT, device_index, 61)
        radiator_info['supplyFlow'] = get_rtdb_point_value(RtdbDevType.AIR_CONDITIONER_LIQUID_COOLING_UNIT, device_index, 62)
        radiator_info['supplyPressure'] = get_rtdb_point_value(RtdbDevType.AIR_CONDITIONER_LIQUID_COOLING_UNIT, device_index, 64)
       
        radiator_info['supplyLiquidTemp'] = truncate_to_two_decimals(radiator_info['supplyLiquidTemp'])
        radiator_info['supplyFlow'] = truncate_to_two_decimals(radiator_info['supplyFlow'])
        radiator_info['supplyPressure'] = truncate_to_two_decimals(radiator_info['supplyPressure'])
        
        ret['radiator'] = radiator_info

    metering_devices = await DeviceService.get_metering_devices_info(query_db)
    metering_info = []
    if metering_devices:
        for device in metering_devices:
            device_index = int(device['device_code']) - 1
            # 正向有功电度和反向有功电度
            metering_info.append({
                'device_id': device['device_id'],
                'device_code': device['device_code'],
                'forwardActiveEnergy': get_rtdb_point_value(RtdbDevType.ELECTRIC_METER_DEVICE, device_index, 29),
                'reverseActiveEnergy': get_rtdb_point_value(RtdbDevType.ELECTRIC_METER_DEVICE, device_index, 30)
            })
    ret['metering'] = metering_info

    # 主页上变压器展示两个信息
    transformer_info = {
        "status": get_rtdb_point_value(RtdbDevType.TRANSFORMER, 0, 5),
        "temperature": get_rtdb_point_value(RtdbDevType.TRANSFORMER, 0, 61)
    }
    transformer_info['temperature'] = truncate_to_two_decimals(transformer_info['temperature'])
    ret['transformer'] = transformer_info

    # 获取电站信息
    stack_list = await DeviceService.get_stack_control_devices(query_db)
    total_active_power = 0
    total_reactive_power = 0
    pcs_count = sum(len(devices) for devices in device_dict.values())
    battery_stack_count = len(stack_list)

    for channel, device_codes in device_dict.items():
        for device_code in device_codes:
            device_index = device_code - 1
            total_active_power += get_rtdb_point_value(RtdbDevType.DSP, device_index, 163)
            total_reactive_power += get_rtdb_point_value(RtdbDevType.DSP, device_index, 164)

    station_info = {
        "total_active_power": total_active_power,
        "total_reactive_power": total_reactive_power,
        "pcs_count": pcs_count,
        "battery_stack_count": battery_stack_count
    }

    # 获取储能统计信息
    total_charge_today = 0
    total_discharge_today = 0
    total_charge_accumulated = 0
    total_discharge_accumulated = 0

    for device_info in stack_list:
        device_code = device_info['device_code']
        device_index = device_code - 1
        total_charge_today += get_rtdb_point_value(RtdbDevType.BBMS, device_index, 8)
        total_discharge_today += get_rtdb_point_value(RtdbDevType.BBMS, device_index, 9)
        total_charge_accumulated += get_rtdb_point_value(RtdbDevType.BBMS, device_index, 10)
        total_discharge_accumulated += get_rtdb_point_value(RtdbDevType.BBMS, device_index, 11)

    energy_storage_info = {
        "total_charge_today": truncate_to_two_decimals(total_charge_today),
        "total_discharge_today": truncate_to_two_decimals(total_discharge_today),
        "total_charge_accumulated": truncate_to_two_decimals(total_charge_accumulated),
        "total_discharge_accumulated": truncate_to_two_decimals(total_discharge_accumulated)
    }

    # 系统概览页：与设备监控 BBMS 堆控点表同步
    bbms_overview = await build_bbms_overview_payload(query_db)

    # 合并所有信息（保留旧字段供兼容；前端概览页使用 runStatus 等）
    combined_info = {
        "topology_info": ret,
        "station_info": station_info,
        "energy_storage_info": energy_storage_info,
        **bbms_overview,
    }

    return ResponseUtil.success(data=combined_info)


@overviewController.post('/dsp-state')
async def dsp_state(
    request: Request,
    dsp: DspControlModel,
):
    """
    控制多个DSP
    :param dsp_list: 包含多个DSP控制信息的列表，每个元素为字典，包含dsp_id, power_switch, run_switch
    """
    dsp_id = dsp.dsp_id
    power_switch = dsp.power_switch
    run_switch = dsp.run_switch

    # 设置开关机状态，点号:272
    set_rtdb_point_value(RtdbDevType.DSP, dsp_id - 1, 272, power_switch)

    # 当 run_switch == 5 时，先取 522 点位实时值
    if run_switch == 5:
        current_522_value = get_rtdb_point_value(RtdbDevType.DSP, dsp_id - 1, 522)
        if current_522_value == 1:
            send_run_switch = 3
        else:
            send_run_switch = 5
    else:
        send_run_switch = run_switch

    # 设置运行状态，点号:273
    set_rtdb_point_value(RtdbDevType.DSP, dsp_id - 1, 273, send_run_switch)

    return ResponseUtil.success(msg="DSP control set success!")

