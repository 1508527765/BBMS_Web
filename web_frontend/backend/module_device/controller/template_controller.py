from typing import Optional
from fastapi import APIRouter, Depends, Request, UploadFile, Query, File
from pydantic_validation_decorator import ValidateFields
from sqlalchemy.orm import Session
from config.get_db import get_ems_db
from module_device.entity.vo.template_vo import DeleteTemplateModel, TemplatePageQueryModel, TemplateCreateUpdateModel
from module_device.service.template_service import TemplateService
from module_device.service.excel_service import ExcelService
from module_device.entity.do.template_point_do import TemplatePoint
from module_device.entity.vo.template_vo import TemplatePointModel
from utils.common_util import bytes2file_response
from utils.log_util import logger
from utils.page_util import PageResponseModel
from utils.response_util import ResponseUtil

templateController = APIRouter(prefix='/template')


class TemplateFactory:
    @staticmethod
    def create_point_model(point_data: dict) -> TemplatePoint:
        # 处理point_data
        return TemplatePoint(**point_data)


@templateController.get(
    '/list', response_model=PageResponseModel
)
async def get_template_list(
        request: Request,
        template_page_query: TemplatePageQueryModel = Depends(TemplatePageQueryModel.as_query),
        query_db: Session = Depends(get_ems_db),
):
    """
    获取模板列表
    """
    template_page_query_result = await TemplateService.get_template_list_services(query_db, template_page_query,
                                                                                  is_page=True)
    logger.info('获取成功')

    return ResponseUtil.success(model_content=template_page_query_result)


@templateController.get(
    '/all', response_model=PageResponseModel
)
async def get_template_all(
        request: Request,
        protocol_type: int,
        dev_type: Optional[int] = Query(default=None, alias='devType'),
        query_db: Session = Depends(get_ems_db),
):
    """
    获取设备配置模板（可按协议类型、设备类型筛选，供创建设备时单选）
    """
    template_all_result = await TemplateService.get_template_all_services(
        query_db, template_type=0, protocol_type=protocol_type, dev_type=dev_type
    )
    for template in template_all_result:
        has_points = await TemplateService.has_template_points_services(query_db, template['id'])
        template['hasPoints'] = has_points
    logger.info('获取成功')

    return ResponseUtil.success(rows=template_all_result)


@templateController.post('')
@ValidateFields(validate_model='add_template')
async def add_system_template(
        request: Request,
        add_template: TemplateCreateUpdateModel,
        query_db: Session = Depends(get_ems_db),
):
    """
    添加系统模板
    """
    add_template_result = await TemplateService.add_template_services(query_db, add_template)
    logger.info(add_template_result.message)

    return ResponseUtil.success(msg=add_template_result.message)


@templateController.put('')
@ValidateFields(validate_model='edit_template')
async def edit_system_template(
        request: Request,
        edit_template: TemplateCreateUpdateModel,
        query_db: Session = Depends(get_ems_db),
):
    """
    编辑系统模板
    """
    edit_template_result = await TemplateService.edit_template_services(query_db, edit_template)
    logger.info(edit_template_result.message)

    return ResponseUtil.success(msg=edit_template_result.message)


@templateController.delete('/{template_ids}')
async def delete_system_template(request: Request, template_ids: str, query_db: Session = Depends(get_ems_db)):
    """
    删除系统模板
    """
    delete_template = DeleteTemplateModel(templateIds=template_ids)
    delete_template_result = await TemplateService.delete_template_services(query_db, delete_template)
    logger.info(delete_template_result.message)

    return ResponseUtil.success(msg=delete_template_result.message)


@templateController.post('/import-templates')
async def import_template(
        request: Request,
        file: UploadFile = File(...),
        template_type: str = Query(alias='templateType'),
        query_db: Session = Depends(get_ems_db),
):
    """
    导入系统模板
    """
    template_type = 0 if template_type == "device" else 1
    batch_import_result = await ExcelService.import_template_services(query_db, file, template_type)
    logger.info(batch_import_result.message)
    return ResponseUtil.success(msg=batch_import_result.message)


@templateController.post('/import-points')
async def batch_import_point(
        request: Request,
        file: UploadFile = File(...),
        template_id: int = Query(alias='templateId'),
        query_db: Session = Depends(get_ems_db),
):
    """
    批量导入测点
    """
    batch_import_result = await ExcelService.batch_import_point_services(query_db, file, template_id)
    logger.info(batch_import_result.message)

    return ResponseUtil.success(msg=batch_import_result.message)


@templateController.post('/import-points-file')
async def export_template_point_template(request: Request, query_db: Session = Depends(get_ems_db)):
    """
    导出测点导入文件
    """
    template_import_template_result = await ExcelService.get_point_import_template_services()

    return ResponseUtil.streaming(data=bytes2file_response(template_import_template_result))


@templateController.get('/points/{template_id}')
async def get_template_points(
        request: Request,
        template_id: int,
        query_db: Session = Depends(get_ems_db),
):
    """
    获取模板测点
    """
    point_list = await TemplateService.get_template_points_services(query_db, template_id)
    return ResponseUtil.success(rows=point_list)


@templateController.post('/export/')
async def export_template_list(
        request: Request,
        template_id: int,
        lang: str = Query('zh', alias='lang'),
        query_db: Session = Depends(get_ems_db),
):
    """
    导出模板测点列表
    """
    point_list = await TemplateService.get_template_points_services(query_db, template_id)
    if point_list:
        template_export_result = await ExcelService.export_point_list_services(point_list, lang=lang)
    else:
        template_export_result = await ExcelService.get_point_import_template_services()
    logger.info('导出成功')
    return ResponseUtil.streaming(data=bytes2file_response(template_export_result))


@templateController.post('/point')
@ValidateFields(validate_model='add_template_point')
async def add_template_point(
        request: Request,
        add_point: TemplatePointModel,
        query_db: Session = Depends(get_ems_db),
):
    """
    添加模板测点
    """
    add_point_result = await TemplateService.add_template_point_services(query_db, add_point)
    logger.info(add_point_result.message)
    return ResponseUtil.success(data=add_point_result.result)


@templateController.put('/point')
@ValidateFields(validate_model='edit_template_point')
async def edit_template_point(
        request: Request,
        edit_point: TemplatePointModel,
        query_db: Session = Depends(get_ems_db),
):
    """
    编辑模板测点
    """
    edit_point_result = await TemplateService.edit_template_point_services(query_db, edit_point)
    logger.info(edit_point_result.message)
    return ResponseUtil.success(msg=edit_point_result.message)


@templateController.delete('/point/{point_ids}')
async def delete_template_point(request: Request, point_ids: str, query_db: Session = Depends(get_ems_db)):
    """
    删除模板测点
    """
    delete_point_result = await TemplateService.delete_template_point_services(query_db, point_ids)
    logger.info(delete_point_result.message)
    return ResponseUtil.success(msg=delete_point_result.message)
