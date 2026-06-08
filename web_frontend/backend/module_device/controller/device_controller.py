from fastapi import APIRouter, Depends, Request, UploadFile, Query, File, Body
from pydantic_validation_decorator import ValidateFields
from sqlalchemy.orm import Session
from config.get_db import get_ems_db
from typing import List

from module_device.entity.vo.device_vo import *
from module_device.entity.vo.template_vo import TemplatePointModel
from module_device.service.device_service import DeviceService
from module_device.service.excel_service import ExcelService
from exceptions.exception import ServiceException
from utils.log_util import logger
from utils.common_util import bytes2file_response
from utils.page_util import PageResponseModel
from utils.response_util import ResponseUtil


deviceController = APIRouter(prefix='/device')


@deviceController.get(
    '/list', response_model=PageResponseModel
)
async def get_device_list(
    request: Request,
    device_page_query: DevicePageQueryModel = Depends(DevicePageQueryModel.as_query),
    query_db: Session = Depends(get_ems_db),
):
    """
    获取设备分页列表
    """
    # 获取分页数据
    device_page_query_result = await DeviceService.get_device_list_services(query_db, device_page_query, is_page=True)
    logger.info('获取成功')

    return ResponseUtil.success(model_content=device_page_query_result)


@deviceController.get('/listAll')
async def get_device_list(
    request: Request,
    name: str = Query(None, alias='name'),
    query_db: Session = Depends(get_ems_db),
):
    """
    获取所有设备列表
    """
    device_result = await DeviceService.get_all_device_services(query_db, name)
    logger.info('获取成功')

    return ResponseUtil.success(rows=device_result)


@deviceController.post('')
async def add_system_device(
    request: Request,
    add_device: dict,
    query_db: Session = Depends(get_ems_db),
):
    """
    添加新设备
    """
    # 通过工厂模式动态生成模型
    device_model = ProtocolFactory.create_protocol_model(add_device)
    add_device_result = await DeviceService.add_device_services(query_db, device_model)
    logger.info(add_device_result.message)
    return ResponseUtil.success(msg=add_device_result.message)

@deviceController.post('/batch')
async def batch_add_system_devices(
    request: Request,
    devices: List[ComprehensiveDeviceModel] = Body(...),
    query_db: Session = Depends(get_ems_db),
):
    """
    批量新增设备接口
    """
    # 将devices列表转换为字典列表
    devices_dicts = [device.model_dump(by_alias=True) for device in devices]
    result = await DeviceService.batch_add_devices_services(query_db, devices_dicts)
    return ResponseUtil.success(data=result, msg=f"Successfully added {len(result)} device(s)")

@deviceController.put('')
@ValidateFields(validate_model='edit_device')
async def edit_system_device(
    request: Request,
    edit_device: dict,
    query_db: Session = Depends(get_ems_db),
):
    """
    编辑设备信息
    """
    # 动态生成模型
    device_model = ProtocolFactory.create_protocol_model(edit_device)
    edit_device_result = await DeviceService.edit_device_services(query_db, device_model)
    logger.info(edit_device_result.message)
    return ResponseUtil.success(msg=edit_device_result.message)


@deviceController.delete('/{device_ids}')
async def delete_system_device(request: Request, device_ids: str, query_db: Session = Depends(get_ems_db)):
    """
    删除设备
    """
    delete_device = DeleteDeviceModel(deviceIds=device_ids)
    delete_device_result = await DeviceService.delete_device_services(query_db, delete_device)
    logger.info(delete_device_result.message)

    return ResponseUtil.success(msg=delete_device_result.message)


@deviceController.post('/import-points')
async def batch_import_point(
    request: Request,
    file: UploadFile = File(...),
    device_id: int = Query(alias='deviceId'),
    query_db: Session = Depends(get_ems_db),
):
    """
    批量导入设备测点
    """
    device_info = await DeviceService.device_detail_services(query_db, device_id)
    if device_info.template_id is None:
        raise ServiceException(message='Template ID is not assigned. Please assign template in device maintenance first.')
    dev_dict = {
        "protocol_type": device_info.protocol_type,
        "dev_type": device_info.type,
    }
    batch_import_result = await ExcelService.batch_import_point_services(query_db, file, device_info.template_id, dev_dict)
    logger.info(batch_import_result.message)

    return ResponseUtil.success(msg=batch_import_result.message)


@deviceController.post('/import-points-file')
async def export_point_template(request: Request, query_db: Session = Depends(get_ems_db)):
    """
    导出测点导入模板文件
    """
    template_import_template_result = await ExcelService.get_point_import_template_services()

    return ResponseUtil.streaming(data=bytes2file_response(template_import_template_result))


@deviceController.get('/points/{device_id}')
async def get_device_points(
    request: Request,
    device_id: int,
    query_db: Session = Depends(get_ems_db),
):
    """
    获取设备测点列表
    """
    point_list = await DeviceService.get_device_points_services(query_db, device_id)
    return ResponseUtil.success(rows=point_list)


@deviceController.get('/points-with-status/{device_id}')
async def get_device_points(
    request: Request,
    device_id: int,
    query_db: Session = Depends(get_ems_db),
):
    """
    获取带状态的设备测点列表
    """
    point_list = await DeviceService.get_device_points_with_status_services(query_db, device_id)
    return ResponseUtil.success(rows=point_list)


@deviceController.post('/export/')
async def export_device_list(
    request: Request,
    device_id: int,
    lang: str = Query('zh', alias='lang'),
    query_db: Session = Depends(get_ems_db),
):
    """
    导出设备测点列表
    """
    # 获取全量数据
    point_list = await DeviceService.get_device_points_services(query_db, device_id)
    if point_list:
        device_export_result =  await ExcelService.export_point_list_services(point_list, lang=lang)
    else:
        device_export_result =  await ExcelService.get_point_import_template_services()
    logger.info('导出成功')
    return ResponseUtil.streaming(data=bytes2file_response(device_export_result))


@deviceController.post('/point')
@ValidateFields(validate_model='add_device_point')
async def add_device_point(
    request: Request,
    add_point: TemplatePointModel,
    query_db: Session = Depends(get_ems_db),
):
    """
    添加设备测点
    """
    add_point_result = await DeviceService.add_device_point_services(query_db, add_point)
    logger.info(add_point_result.message)
    return ResponseUtil.success(data=add_point_result.result)


@deviceController.put('/point')
@ValidateFields(validate_model='edit_device_point')
async def edit_device_point(
    request: Request,
    edit_point: TemplatePointModel,
    query_db: Session = Depends(get_ems_db),
):
    """
    编辑设备测点
    """
    edit_point_result = await DeviceService.edit_device_point_services(query_db, edit_point)
    logger.info(edit_point_result.message)
    return ResponseUtil.success(msg=edit_point_result.message)


@deviceController.delete('/point/{point_ids}')
async def delete_device_point(request: Request, point_ids: str, query_db: Session = Depends(get_ems_db)):
    """
    删除设备测点
    """
    delete_point_result = await DeviceService.delete_device_point_services(query_db, point_ids)
    logger.info(delete_point_result.message)
    return ResponseUtil.success(msg=delete_point_result.message)


@deviceController.get('/dsp-devices')
async def get_dsp_devices(
    request: Request,
    query_db: Session = Depends(get_ems_db),
):
    """
    获取设备类型数量
    """

    device_dict = await DeviceService.get_dsp_devices(query_db)
    return ResponseUtil.success(data=device_dict)


@deviceController.get('/devices-by-type/{device_type}')
def get_devices_by_type(
    request: Request,
    device_type: int,
    query_db: Session = Depends(get_ems_db),
):
    """
    根据设备类型查询设备列表
    """
    devices = DeviceService.get_devices_by_type(query_db, device_type)
    return ResponseUtil.success(data=devices)


@deviceController.get('/pcs-soft-version')
def get_pcs_soft_version(
    request: Request,
    query_db: Session = Depends(get_ems_db),
):
    """
    获取PCS软件版本
    """
    devices = DeviceService.get_pcs_soft_version(query_db)
    return ResponseUtil.success(data=devices)
