from fastapi import APIRouter, Depends, Request, File, UploadFile
from sqlalchemy.orm import Session
from config.get_db import get_ems_db
from module_device.service.topology_service import TopologyService
from module_device.entity.vo.topology_vo import TopologyCreateUpdateModel
from module_device.service.excel_service import ExcelService
from utils.common_util import bytes2file_response
from utils.log_util import logger
from utils.response_util import ResponseUtil

topologyController = APIRouter(prefix='/topology')

@topologyController.get('/list')
async def get_topology_list(
    request: Request,
    query_db: Session = Depends(get_ems_db),
):
    # 获取拓扑结构列表
    topology_data = await TopologyService.get_topology_list_services(query_db)
    logger.info('获取拓扑结构列表成功')

    return ResponseUtil.success(data=topology_data)

@topologyController.post('')
async def add_topology(
    request: Request,
    topology_data: TopologyCreateUpdateModel,
    query_db: Session = Depends(get_ems_db),
):
    # if topology_data.menu_tree == 1 and topology_data.dev_id is None:
    #     return ResponseUtil.error(msg="设备节点必须选择设备")
    add_topology_result = await TopologyService.add_topology_services(query_db, topology_data)
    logger.info(add_topology_result.message)

    return ResponseUtil.success(msg=add_topology_result.message)

@topologyController.put('')
async def edit_topology(
    request: Request,
    topology_data: TopologyCreateUpdateModel,
    query_db: Session = Depends(get_ems_db),
):
    edit_topology_result = await TopologyService.edit_topology_services(query_db, topology_data)
    logger.info(edit_topology_result.message)

    return ResponseUtil.success(msg=edit_topology_result.message)

@topologyController.delete('/{topology_ids}')
async def delete_topology(request: Request, topology_ids: str, query_db: Session = Depends(get_ems_db)):
    delete_topology_result = await TopologyService.delete_topology_services(query_db, topology_ids)
    logger.info(delete_topology_result.message)

    return ResponseUtil.success(msg=delete_topology_result.message)


@topologyController.post('/export-all-topologies')
async def export_all_topologies(request: Request, query_db: Session = Depends(get_ems_db)):
    topology_export_result = await ExcelService.export_topology_data_services(query_db)
    return ResponseUtil.streaming(data=bytes2file_response(topology_export_result))


@topologyController.post('/import-topologies-file')
async def export_topology_template(request: Request, query_db: Session = Depends(get_ems_db)):
    topology_import_template_result = await ExcelService.get_topology_import_template_services()

    return ResponseUtil.streaming(data=bytes2file_response(topology_import_template_result))


@topologyController.post('/import-topologies')
async def batch_import_topology(
    request: Request,
    file: UploadFile = File(...),
    query_db: Session = Depends(get_ems_db),
):
    batch_import_result = await ExcelService.batch_import_topology_services(query_db, file)
    logger.info(batch_import_result.message)

    return ResponseUtil.success(msg=batch_import_result.message)