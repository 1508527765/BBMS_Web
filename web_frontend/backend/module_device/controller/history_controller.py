import os
import time
import shutil
from datetime import datetime

from fastapi import APIRouter, Depends, Request, Form
from fastapi.responses import FileResponse, StreamingResponse
from sqlalchemy.orm import Session
from config.get_db import get_business_db
from module_device.constant.web_sign_constant import *
from module_device.service.history_service import HistoryService
from module_device.service.rtdb_service import read_web_sign, write_web_sign
from module_device.entity.vo.history_vo import *
from utils.log_util import logger
from utils.response_util import ResponseUtil
from utils.common_util import bytes2file_response

historyController = APIRouter(prefix='/history')


STORAGE_PATH = '/opt/bbms/storage'
# STORAGE_PATH = 'E:/ems/storage'
STORAGE_PATH_TEMP = f'{STORAGE_PATH}/zip'

@historyController.post('/timed-data')
async def get_timed_data(
    request: Request,
    query: DataQueryModel,
    query_db_business: Session = Depends(get_business_db)
):
    timed_data_result = await HistoryService.get_timed_data_list_services(query_db_business, query)

    logger.info('获取存储数据成功')

    return ResponseUtil.success(model_content=timed_data_result)


@historyController.post('/export-timed-data')
async def export_timed_data(
    request: Request,
    query: DataQueryModel = Form()
):
    # 创建临时目录
    os.makedirs(STORAGE_PATH_TEMP, exist_ok=True)
    
    folder_path = os.path.join(STORAGE_PATH, str(query.date))
    zip_file_path = os.path.join(STORAGE_PATH_TEMP, f"{query.date}.zip")

    # 处理当日数据导出
    today = datetime.now().strftime('%Y-%m-%d')
    if str(query.date) == today:
        # 删除已存在的zip文件
        if os.path.exists(zip_file_path):
            os.remove(zip_file_path)
            
        # 通知C程序生成数据并等待完成
        write_web_sign(SignConstant.kSign_ExportCurDayLog, 1)
        
        for _ in range(25):  # 最多等待25秒
            if read_web_sign(SignConstant.kSign_ExportCurDayLog) == 100:
                write_web_sign(SignConstant.kSign_ExportCurDayLog, 0)
                break
            time.sleep(1)
        else:  # 超时处理
            return ResponseUtil.failure(msg="Export data timeout")

    # 检查数据目录是否存在
    if not os.path.isdir(folder_path):
        return ResponseUtil.failure(msg="Export error: Data file not found")

    # 生成zip文件(如果不存在)
    if not os.path.exists(zip_file_path):
        base_name = os.path.splitext(zip_file_path)[0]
        zip_file_path = shutil.make_archive(base_name, 'zip', folder_path)

    # 最终检查并返回
    if not os.path.exists(zip_file_path):
        return ResponseUtil.failure(msg="Export error: Failed to generate zip file")

    return FileResponse(
        zip_file_path,
        media_type="application/zip", 
        filename=f"{query.date}.zip"
    )


@historyController.post('/event-data')
async def get_event_data(
    request: Request,
    query: EventQueryModel,
    query_db_business: Session = Depends(get_business_db),
):
    event_data_result = await HistoryService.get_event_data_list_services(query_db_business, query)

    logger.info('获取事件/故障数据成功')

    return ResponseUtil.success(model_content=event_data_result)


@historyController.post('/export-event-data')
async def export_event_data(
    request: Request,
    query: EventQueryModel = Form(),
    query_db_business: Session = Depends(get_business_db),
):
    event_data_result = await HistoryService.export_event_data_list_services(query_db_business, query)

    logger.info('导出成功')

    return ResponseUtil.streaming(data=bytes2file_response(event_data_result))