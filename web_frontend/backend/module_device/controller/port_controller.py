from fastapi import APIRouter, Depends, Request
from pydantic_validation_decorator import ValidateFields
from sqlalchemy.orm import Session
from config.get_db import get_ems_db
from module_device.entity.vo.port_vo import *
from module_device.service.port_service import PortService
from utils.log_util import logger
from utils.page_util import PageResponseModel
from utils.response_util import ResponseUtil


portController = APIRouter(prefix='/system/port')


class PortFactory:
    @staticmethod
    def create_port_model(port_data: dict) -> PortModel:
        port_type_map = {
            0: SerialPortModel,
            1: NetPortModel,
            4: WifiPortModel,
            5: FourGPortModel,
            6: TimeSyncPortModel,
            7: CanPortModel
        }
        port_model_class = port_type_map.get(port_data.get('type'), PortModel)
        return port_model_class(**port_data)
    

@portController.get(
    '/list', response_model=PageResponseModel
)
async def get_port_list(
    request: Request,
    port_page_query: PortPageQueryModel = Depends(PortPageQueryModel.as_query),
    query_db: Session = Depends(get_ems_db),
):
    # 获取分页数据
    port_page_query_result = await PortService.get_port_list_services(query_db, port_page_query, is_page=True)
    logger.info('获取成功')

    return ResponseUtil.success(model_content=port_page_query_result)


@portController.post('')
async def add_system_port(
    request: Request,
    add_port: dict,
    query_db: Session = Depends(get_ems_db),
):
    # 通过工厂模式动态生成模型
    port_model = PortFactory.create_port_model(add_port)
    add_port_result = await PortService.add_port_services(query_db, port_model)
    logger.info(add_port_result.message)
    return ResponseUtil.success(msg=add_port_result.message)


@portController.put('')
@ValidateFields(validate_model='edit_port')
async def edit_system_port(
    request: Request,
    edit_port: dict,
    query_db: Session = Depends(get_ems_db),
):
    # 动态生成模型
    port_model = PortFactory.create_port_model(edit_port)
    edit_port_result = await PortService.edit_port_services(query_db, port_model)
    logger.info(edit_port_result.message)

    return ResponseUtil.success(msg=edit_port_result.message)


@portController.delete('/{port_ids}')
async def delete_system_port(request: Request, port_ids: str, query_db: Session = Depends(get_ems_db)):
    delete_port = DeletePortModel(portIds=port_ids)
    delete_port_result = await PortService.delete_port_services(query_db, delete_port)
    logger.info(delete_port_result.message)

    return ResponseUtil.success(msg=delete_port_result.message)


