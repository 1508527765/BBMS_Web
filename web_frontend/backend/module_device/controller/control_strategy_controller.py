from fastapi import APIRouter, Depends, Request, UploadFile, Query, File
from sqlalchemy.orm import Session

from typing import List

from config.get_db import get_ems_db
from utils.response_util import ResponseUtil
from utils.log_util import logger
from module_device.service.device_service import DeviceService
from module_device.service.control_strategy_service import ControlStrategyService
from module_device.service.excel_service import ExcelService
from module_device.service.rtdb_service import write_web_sign, read_web_sign
from module_device.entity.vo.control_strategy_vo import UpdateRequest, PcsItem, UpdateConfigModel
from module_device.constant.web_sign_constant import *
from utils.common_util import bytes2file_response

controlStrategyController = APIRouter(prefix='/control-strategy')


@controlStrategyController.get('/list')
async def get_protect_config_list(request: Request, type:str, device_code:int, db: Session = Depends(get_ems_db)):
    data = await ControlStrategyService.get_protect_config_list(type, device_code, db)
    logger.info("获取保护参数配置列表成功")
    return ResponseUtil.success(data=data)

@controlStrategyController.post('/update')
async def update_protect_config(
    request: Request,
    data: UpdateRequest,
    db: Session = Depends(get_ems_db),
):
    strategy_type = data.type
    update_data = data.update_data
    if not update_data:
        # 从数据库中查找对应类型的所有默认设置列表
        default_config_list = await ControlStrategyService.get_default_config_list(db, strategy_type)
        # 转换为List[UpdateConfigModel]的形式
        update_data = [
            UpdateConfigModel(point_id=config.point_id, new_value=config.default_value)
            for config in default_config_list
        ]

    update_device_list = data.update_device_list
    if strategy_type == "sys":
        can1_signal = SignConstant.kSign_DspSysCfgCanI
        can2_signal = SignConstant.kSign_DspSysCfgCanII
    elif strategy_type == "protect":
        can1_signal = SignConstant.kSign_DspProCfgCanI
        can2_signal = SignConstant.kSign_DspProCfgCanII
    elif strategy_type == "run":
        can1_signal = SignConstant.kSign_DspRunCfgCanI
        can2_signal = SignConstant.kSign_DspRunCfgCanII
    else:
        return ResponseUtil.failure(msg="Invalid strategy type")

    device_group_1 = []  # 通道1设备编号列表
    device_group_2 = []  # 通道2设备编号列表
    can_count_1 = 0      # 通道1设备总数
    result_message = ""

    for can_device_data in update_device_list:
        can_channel = can_device_data.can_channel
        device_code = can_device_data.device_code
        can_count = can_device_data.can_count  # 每个通道的DSP总数

        result = await ControlStrategyService.update_protect_config_value(db, device_code, update_data)
        logger.info(result.message)
        result_message = result.message  # 最后设备的返回信息

        if can_channel == 0:
            device_group_1.extend(device_code)
            can_count_1 = can_count  # 任意通道1设备都能提供通道1的总数量
        elif can_channel == 1:
            device_group_2.extend(device_code)

    # 通道1：bit = device_code - 1
    if device_group_1:
        selected_bits_1 = 0
        for code in device_group_1:
            bit_index = code - 1
            selected_bits_1 |= (1 << bit_index)
        write_web_sign(can1_signal, selected_bits_1)

    # 通道2：bit = device_code - (can_count_1 + 1)
    if device_group_2:
        selected_bits_2 = 0
        for code in device_group_2:
            bit_index = code - (can_count_1 + 1)
            if bit_index >= 0:
                selected_bits_2 |= (1 << bit_index)
            else:
                logger.warning(f"设备编号 {code} 小于 can_count+1，忽略")
        write_web_sign(can2_signal, selected_bits_2)

    return ResponseUtil.success(msg=result_message)


@controlStrategyController.get('/advanced/list-configs')
async def get_all_advanced_configs(request: Request, db: Session = Depends(get_ems_db)):
    """
    查询所有高级配置并展示
    """
    try:
        advanced_configs = await ControlStrategyService.get_advanced_config_list(db)
        return ResponseUtil.success(data=advanced_configs)
    except Exception as e:
        logger.error(f"获取高级配置失败: {str(e)}")
        return ResponseUtil.failure(msg="Failed to retrieve advanced configuration")

@controlStrategyController.post('/advanced/save-configs')
async def save_advanced_config(request: Request, config_data: dict, db: Session = Depends(get_ems_db)):
    """
    保存下发高级配置
    """
    try:
        success = await ControlStrategyService.save_advanced_config(db, config_data)
        if success:
            # 保存成功后通知c
            write_web_sign(SignConstant.kSign_DspAdvanceCfg, 1)
            return ResponseUtil.success(msg="Advanced configuration saved successfully")
        else:
            return ResponseUtil.failure(msg="Failed to save advanced configuration")
    except Exception as e:
        logger.error(f"保存高级配置失败: {str(e)}")
        return ResponseUtil.failure(msg="Failed to save advanced configuration")

@controlStrategyController.get("/pcs/list-with-status")
async def get_pcs_list(request: Request, query_db: Session = Depends(get_ems_db)):
    """
    获取PCS配置
    """
    device_dict = await DeviceService.get_dsp_devices(query_db)
    on_flag = read_web_sign(SignConstant.kSign_DspOnSwitch)
    off_flag = read_web_sign(SignConstant.kSign_DspOffSwitch)
    reset_flag = read_web_sign(SignConstant.kSign_DspResetReboot)

    pcs_list = []
    for channel in device_dict:
        for dsp in device_dict[channel]:
            dsp_id = int(dsp)
            # 还原为每台dsp设备的状态
            status = {
                "dsp_id": dsp_id,
                "on": bool(on_flag & (1 << (dsp_id - 1))),
                "off": bool(off_flag & (1 << (dsp_id - 1))),
                "reset": bool(reset_flag & (1 << (dsp_id - 1)))
            }
            pcs_list.append(status)

    return ResponseUtil.success(data=pcs_list)

@controlStrategyController.post("/pcs/save-configs")
async def save_pcs_config(config_list: List[PcsItem]):
    # 初始化3个标志位
    on_flag = 0
    off_flag = 0
    reset_flag = 0

    # 遍历配置项并设置对应位
    for item in config_list:
        index = item.pcs_id - 1
        if item.action == "on":
            on_flag |= (1 << index)
        elif item.action == "off":
            off_flag |= (1 << index)
        elif item.action == "reset":
            reset_flag |= (1 << index)

    write_web_sign(SignConstant.kSign_DspOnSwitch, on_flag)
    write_web_sign(SignConstant.kSign_DspOffSwitch, off_flag)
    write_web_sign(SignConstant.kSign_DspResetReboot, reset_flag)

    return ResponseUtil.success(msg="Configuration successful")


@controlStrategyController.post("/import-configs")
async def import_control_strategy_configs(
    request: Request,
    file: UploadFile = File(...),
    setting_type: str = Query(alias='settingType'),
    lang: str = Query(alias='lang', default='zh'),
    query_db: Session = Depends(get_ems_db),
):
    import_result = await ExcelService.import_control_strategy_services(query_db, file, setting_type, lang)
    logger.info(import_result.message)

    return ResponseUtil.success(msg=import_result.message)

        
@controlStrategyController.post('/export-configs/{setting_type}/{lang}')
async def export_control_strategy_configs(
    request: Request,
    setting_type: str,
    lang: str,
    query_db: Session = Depends(get_ems_db),
):
    """
    导出控制策略列表
    """
    # 获取全量数据
    point_list = await ControlStrategyService.get_protect_config_list(setting_type, 1, query_db)  # 获取第一台设备的数据
    if point_list:
        control_strategy_export_result =  await ExcelService.export_control_strategy_services(point_list, lang)
    else:
        control_strategy_export_result =  await ExcelService.get_control_strategy_import_template_services()
    logger.info('导出成功')
    return ResponseUtil.streaming(data=bytes2file_response(control_strategy_export_result))

@controlStrategyController.post('/import-configs-file')
async def download_import_template(request: Request):
    """
    下载控制策略导入模板
    """
    try:
        template_data = await ExcelService.get_control_strategy_import_template_services()
        logger.info('模板下载成功')
        return ResponseUtil.streaming(data=bytes2file_response(template_data))
    except Exception as e:
        logger.error(f"下载模板失败: {str(e)}")
        return ResponseUtil.failure(msg="Failed to download template")
