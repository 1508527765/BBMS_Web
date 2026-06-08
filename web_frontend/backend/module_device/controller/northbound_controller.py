from fastapi import APIRouter, Depends, Request, UploadFile, Query, File
from pydantic_validation_decorator import ValidateFields
from sqlalchemy.orm import Session
from config.get_db import get_ems_db

from module_device.entity.vo.device_vo import *
from module_device.service.excel_service import ExcelService
from module_device.service.northbound_service import NorthboundService
from utils.log_util import logger
from utils.common_util import bytes2file_response
from utils.page_util import PageResponseModel
from utils.response_util import ResponseUtil


northboundController = APIRouter(prefix='/northbound')


@northboundController.get(
    '/list', response_model=PageResponseModel
)
async def get_northbound_list(
    request: Request,
    config_page_query: NorthConfigPageQueryModel = Depends(NorthConfigPageQueryModel.as_query),
    query_db: Session = Depends(get_ems_db),
):
    # 获取分页数据
    config_page_query_result = await NorthboundService.get_northbound_config_list_services(query_db, config_page_query, is_page=True)
    logger.info('获取成功')

    return ResponseUtil.success(model_content=config_page_query_result)


@northboundController.get('/listAll')
async def get_northbound_list_all(
    request: Request,
    name: str = Query(None, alias='name'),
    query_db: Session = Depends(get_ems_db),
):
    config_result = await NorthboundService.get_all_northbound_config_services(query_db, name)
    logger.info('获取成功')

    return ResponseUtil.success(rows=config_result)


@northboundController.post('')
async def add_northbound_config(
    request: Request,
    add_config: dict,
    query_db: Session = Depends(get_ems_db),
):
    # 通过工厂模式动态生成模型
    config_model = ProtocolFactory.create_protocol_model(add_config)
    add_config_result = await NorthboundService.add_northbound_config_services(query_db, config_model)
    logger.info(add_config_result.message)
    return ResponseUtil.success(msg=add_config_result.message)


@northboundController.put('')
@ValidateFields(validate_model='edit_device')
async def edit_northbound_config(
    request: Request,
    edit_config: dict,
    query_db: Session = Depends(get_ems_db),
):
    # 动态生成模型
    config_model = ProtocolFactory.create_protocol_model(edit_config)
    edit_config_result = await NorthboundService.edit_northbound_config_services(query_db, config_model)
    logger.info(edit_config_result.message)
    return ResponseUtil.success(msg=edit_config_result.message)


@northboundController.delete('/{device_ids}')
async def delete_northbound_device(request: Request, device_ids: str, query_db: Session = Depends(get_ems_db)):
    delete_device = DeleteDeviceModel(deviceIds=device_ids)
    delete_device_result = await NorthboundService.delete_northbound_device_services(query_db, delete_device)
    logger.info(delete_device_result.message)

    return ResponseUtil.success(msg=delete_device_result.message)



