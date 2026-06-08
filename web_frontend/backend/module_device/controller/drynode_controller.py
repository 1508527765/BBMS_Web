from fastapi import APIRouter, Depends, Request, Query, File, UploadFile
from sqlalchemy.orm import Session
from config.get_db import get_ems_db
from module_device.constant.web_sign_constant import SignConstant
from module_device.service.drynode_service import DryNodeService
from module_device.service.excel_service import ExcelService
from module_device.service.rtdb_service import write_web_sign
from module_device.entity.vo.drynode_vo import DryNodeModel
from utils.common_util import bytes2file_response
from utils.log_util import logger
from utils.response_util import ResponseUtil

drynodeController = APIRouter(prefix='/drynode')

@drynodeController.get('/list')
def get_dry_node_list(
    request: Request,
    query_db: Session = Depends(get_ems_db),
):
    # 获取干节点结构列表
    dry_node_data = DryNodeService.get_dry_node_list_services(query_db)
    logger.info('获取干节点列表成功')

    return ResponseUtil.success(rows=dry_node_data)


@drynodeController.get('/logicList')
def get_dry_node_logic_list(
    request: Request,
    type: str = Query(..., description='类型: DI, DO'),
    query_db: Session = Depends(get_ems_db),
):
    # 获取逻辑字典
    logic_dict_data = DryNodeService.get_dry_node_logic_dict_services(query_db, type)
    logger.info('获取逻辑字典成功')

    return ResponseUtil.success(rows=logic_dict_data)


@drynodeController.post('')
def add_dry_node(
    request: Request,
    dry_node_data: DryNodeModel,
    query_db: Session = Depends(get_ems_db),
):
    add_dry_node_result = DryNodeService.add_dry_node_services(query_db, dry_node_data)
    write_web_sign(SignConstant.kSign_DiDoLogicConfig, 1)
    logger.info(add_dry_node_result.message)

    return ResponseUtil.success(msg=add_dry_node_result.message)


@drynodeController.put('')
def edit_dry_node(
    request: Request,
    dry_node_data: DryNodeModel,
    query_db: Session = Depends(get_ems_db),
):
    edit_dry_node_result = DryNodeService.edit_dry_node_services(query_db, dry_node_data)
    write_web_sign(SignConstant.kSign_DiDoLogicConfig, 1)
    logger.info(edit_dry_node_result.message)

    return ResponseUtil.success(msg=edit_dry_node_result.message)


@drynodeController.delete('/{dry_node_ids}')
def delete_dry_node(request: Request, dry_node_ids: str, query_db: Session = Depends(get_ems_db)):
    dry_node_id_list = [dry_node_id for dry_node_id in dry_node_ids.split(',') if dry_node_id]
    delete_dry_node_result = DryNodeService.delete_dry_node_services(query_db, dry_node_id_list)
    write_web_sign(SignConstant.kSign_DiDoLogicConfig, 1)
    logger.info(delete_dry_node_result.message)

    return ResponseUtil.success(msg=delete_dry_node_result.message)


@drynodeController.post('/export-all-drynodes')
async def export_all_drynodes(request: Request, query_db: Session = Depends(get_ems_db)):
    drynode_export_result = await ExcelService.export_drynode_data_services(query_db)
    return ResponseUtil.streaming(data=bytes2file_response(drynode_export_result))


@drynodeController.post('/import-drynodes-file')
async def export_drynode_template(request: Request, query_db: Session = Depends(get_ems_db)):
    drynode_import_template_result = await ExcelService.get_drynode_import_template_services()

    return ResponseUtil.streaming(data=bytes2file_response(drynode_import_template_result))


@drynodeController.post('/import-drynodes')
async def batch_import_drynode(
    request: Request,
    file: UploadFile = File(...),
    query_db: Session = Depends(get_ems_db),
):
    batch_import_result = await ExcelService.import_topology_data_services(query_db, file)
    logger.info(batch_import_result.message)

    return ResponseUtil.success(msg=batch_import_result.message)