import os

from fastapi import APIRouter, Depends, Request, UploadFile, File
from pydantic import BaseModel, validator
from sqlalchemy.orm import Session
from typing import Optional, List
from config.get_db import get_ems_db

from module_device.service.rtdb_service import write_web_sign, get_rtdb_point_value, set_rtdb_point_value
from module_device.constant.web_sign_constant import *
from utils.log_util import logger
from utils.response_util import ResponseUtil

localUpdateController = APIRouter(prefix='/localupdate')

UPLOAD_FOLDER = '/opt/'


class DSPUpgradeParams(BaseModel):
    mode: str
    channel: List[int]
    cpu: int
    channel1_count: int
    channel2_count: int
    channel1_dsps: list
    channel2_dsps: list

    @validator('mode')
    def validate_mode(cls, v):
        if v not in ['broadcast', 'sequential']:
            raise ValueError("Invalid upgrade mode")
        return v
    
    @validator('channel', each_item=True)
    def validate_channel(cls, v):
        if v not in [1, 2]:
            raise ValueError("Invalid channel")
        return v

class DSPUpgradeProgressParams(BaseModel):
    mode: str
    channel: List[int]
    dsp_id: Optional[int] = None

    @validator('mode')
    def validate_mode(cls, v):
        if v not in ['broadcast', 'sequential']:
            raise ValueError("Invalid upgrade mode")
        return v

    @validator('channel', each_item=True)
    def validate_channel(cls, v):
        if v not in [1, 2]:
            raise ValueError("Invalid channel")
        return v

    @validator('dsp_id', always=True)
    def validate_dsp_id(cls, v, values):
        mode = values.get('mode')
        if mode == 'sequential' and (v is None or not (1 <= v <= 16)):
            raise ValueError("DSP ID在sequential模式下必须在1到16之间")
        return v

@localUpdateController.post('/upload/local')
async def upload_local_upgrade_package(
    request: Request,
    file: UploadFile = File(...),
    query_db: Session = Depends(get_ems_db),
):
    """
    接收本机升级包并存放到指定路径
    """
    # 确定存储路径
    save_path = UPLOAD_FOLDER + 'firmware/'
    
    # 确保存储路径存在
    os.makedirs(save_path, exist_ok=True)

    # 删除这个路径下的所有deb文件
    for existing_file in os.listdir(save_path):
        if existing_file.endswith('.deb'):
            os.remove(os.path.join(save_path, existing_file))
    
    file_location = os.path.join(save_path, file.filename)

    # 保存文件
    with open(file_location, "wb") as buffer:
        buffer.write(await file.read())

    logger.info(f"本机升级包已存储到: {file_location}")
    return ResponseUtil.success(msg="Local file uploaded successfully")


@localUpdateController.post('/upload/dsp')
async def upload_dsp_upgrade_package(
    request: Request,
    file: UploadFile = File(...),
    cpu: int = 1
):
    """
    接收DSP升级包并存放到指定路径
    """
    # 确定存储路径
    if cpu == 1:
        save_path = os.path.join(UPLOAD_FOLDER, 'dsp', 'cpu1')
    elif cpu == 2:
        save_path = os.path.join(UPLOAD_FOLDER, 'dsp', 'cpu2')
    else:
        return ResponseUtil.error(msg="Invalid CPU parameter")

    # 确保存储路径存在
    os.makedirs(save_path, exist_ok=True)
    
    file_location = os.path.join(save_path, file.filename)

    # 保存文件
    with open(file_location, "wb") as buffer:
        buffer.write(await file.read())

    logger.info(f"DSP升级包已存储到: {file_location}")
    return ResponseUtil.success(msg="DSP file uploaded successfully")

@localUpdateController.post('/confirm/local')
async def confirm_local_upgrade(request: Request):
    """
    确认本机升级并调用RTDB接口
    """
    # 调用RTDB接口进行本机升级确认
    write_web_sign(SignConstant.kSign_SysUpdate, 1)  # 使用writeWebSign方法写入标识通知
    set_rtdb_point_value(0, 0, PointConstant.SCU_UPDATE_PROGRESS, 0)
    logger.info("本机升级确认，调用RTDB接口")
    return ResponseUtil.success(msg="Local upgrade confirmed successfully")

@localUpdateController.post('/reboot/local')
async def reboot_local(request: Request):
    """
    重启本机
    """
    # 调用RTDB接口进行本机升级确认
    # signType为kSign_SysUpdate，对应位置写1
    write_web_sign(SignConstant.kSign_SysReboot, 1)  # 使用writeWebSign方法写入标识通知
    logger.info("本机重启，调用RTDB接口")
    return ResponseUtil.success(msg="Local reboot successful")

@localUpdateController.post('/confirm/dsp')
async def confirm_dsp_upgrade(
    request: Request,
    params: DSPUpgradeParams,
):
    """
    确认DSP升级并调用RTDB接口
    """
    if params.mode not in ['broadcast', 'sequential']:
        return ResponseUtil.error(msg="Invalid upgrade mode")
    
    cpu = params.cpu

    # 生成通道1和通道2的掩码
    selected_dsp_bits_channel1 = 0
    selected_dsp_bits_channel2 = 0

    for dsp in params.channel1_dsps:
        bit_index = dsp - 1
        selected_dsp_bits_channel1 |= (1 << bit_index)

    for dsp in params.channel2_dsps:
        bit_index = dsp - (params.channel1_count + 1)
        selected_dsp_bits_channel2 |= (1 << bit_index)

    # 根据CPU设置bit8和bit9
    if cpu == 1:
        selected_dsp_bits_channel1 |= (1 << 8)
        selected_dsp_bits_channel2 |= (1 << 8)
    elif cpu == 2:
        selected_dsp_bits_channel1 |= (1 << 9)
        selected_dsp_bits_channel2 |= (1 << 9)

    # 写入标识通知
    for channel in params.channel:
        if channel == 1:
            write_web_sign(SignConstant.kSign_DspSelectCanI, selected_dsp_bits_channel1)
        elif channel == 2:
            write_web_sign(SignConstant.kSign_DspSelectCanII, selected_dsp_bits_channel2)

    # 清空所有DSP通道升级进度
    set_rtdb_point_value(0, 0, PointConstant.DSP_CHANNEL_1_UPDATE_PROGRESS, 0)
    set_rtdb_point_value(0, 0, PointConstant.DSP_CHANNEL_2_UPDATE_PROGRESS, 0)
    # 清空所有DSP升级进度点位
    for i in range(16):
        set_rtdb_point_value(0, 0, PointConstant.DSP_UPDATE_PROGRESS_START + i, 0)

    return ResponseUtil.success(msg=f"DSP upgrade confirmed successfully")

@localUpdateController.get('/progress/local')
async def get_local_upgrade_progress(request: Request):
    """
    获取本机升级进度
    """
    # 模拟获取进度
    progress = get_rtdb_point_value(0, 0, PointConstant.SCU_UPDATE_PROGRESS)
    # 读到结果为100时，写进度为0
    set_rtdb_point_value(0, 0, PointConstant.SCU_UPDATE_PROGRESS, 0)
    logger.info(f"获取本机升级进度: {progress}%")
    return ResponseUtil.success(data={"progress": progress})

@localUpdateController.post('/progress/dsp')
async def get_dsp_upgrade_progress(
    request: Request,
    params: DSPUpgradeProgressParams,
):
    """
    获取DSP升级进度
    """
    progress = {}
    if params.mode == 'broadcast':
        for channel in params.channel:
            if channel == 1:
                progress[channel] = get_rtdb_point_value(0, 0, PointConstant.DSP_CHANNEL_1_UPDATE_PROGRESS)
            elif channel == 2:
                progress[channel] = get_rtdb_point_value(0, 0, PointConstant.DSP_CHANNEL_2_UPDATE_PROGRESS)
    else:
        for channel in params.channel:
            progress[channel] = get_rtdb_point_value(0, 0, PointConstant.DSP_UPDATE_PROGRESS_START + params.dsp_id - 1)

    return ResponseUtil.success(data= progress)

@localUpdateController.get('/dsp/addressing')
async def set_dsp_address(request: Request, channel: str):
    """
    DSP通道编址设置
    :param channel: 通道号列表,只能包含1或2
    """
    channel = [int(ch) for ch in channel.split(',')]
    for ch in channel:
        if ch not in [1, 2]:
            return ResponseUtil.error(msg="Invalid channel number, must be 1 or 2")

    # 先将对应通道的编址状态置为0
    sign_value = 0
    for ch in channel:
        if ch == 1:
            set_rtdb_point_value(0, 0, PointConstant.DSP_AUTO_SET_ADDR_RESULT_1, 0)  # 清空can1编址状态
            sign_value |= 0x01
        else:
            set_rtdb_point_value(0, 0, PointConstant.DSP_AUTO_SET_ADDR_RESULT_2, 0)  # 清空can2编址状态
            sign_value |= 0x100

    # 写入标识通知
    write_web_sign(SignConstant.kSign_DspAutoSetAddr, sign_value)
    return ResponseUtil.success(msg="DSP channel addressing command sent")

@localUpdateController.get('/dsp/addressing/result')
async def get_dsp_address_result(request: Request, channel: str):
    """
    获取DSP通道编址结果
    :param channel: 通道号列表,只能包含1或2
    """
    channel = [int(ch) for ch in channel.split(',')]
    success_channels = []
    failed_channels = []

    for ch in channel:
        if ch == 1:
            result = get_rtdb_point_value(0, 0, PointConstant.DSP_AUTO_SET_ADDR_RESULT_1)
            if result == 1:
                success_channels.append(ch)
            elif result == -1:
                failed_channels.append(ch)
        else:
            result = get_rtdb_point_value(0, 0, PointConstant.DSP_AUTO_SET_ADDR_RESULT_2)
            if result == 1:
                success_channels.append(ch)
            elif result == -1:
                failed_channels.append(ch)

    return ResponseUtil.success(data={
        "success_channels": success_channels,
        "failed_channels": failed_channels,
        "result": 1 if len(success_channels) == len(channel) else 0
    })