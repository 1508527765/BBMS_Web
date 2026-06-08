import json
from sqlalchemy.orm import Session
from exceptions.exception import ServiceException
from module_device.dao.port_dao import PortDao
from module_device.entity.vo.port_vo import DeletePortModel, PortModel, PortPageQueryModel, CrudResponseModel
from utils.common_util import CamelCaseUtil


class PortService:
    """
    端口管理模块服务层
    """

    @staticmethod
    def prepare_port_content(port_object: PortModel) -> dict:
        """
        将 port_object 的字段处理为 content 格式
        """
        # 提取除 id、type、name 以外的字段
        exclude_fields = {'id', 'type', 'name'}
        content_data = {
            key: value
            for key, value in port_object.model_dump().items()
            if key not in exclude_fields and value is not None
        }
        return {
            "id": port_object.id,
            "type": port_object.type,
            "name": port_object.name,
            "content": json.dumps(content_data)
        }

    @classmethod
    async def get_port_list_services(
        cls, query_db: Session, query_object: PortPageQueryModel, is_page: bool = True
    ):
        """
        获取端口列表信息service

        :param query_db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 端口列表信息对象
        """
        port_list_result = await PortDao.get_port_list(query_db, query_object, is_page)

        for item in port_list_result.rows:
            content_json = json.loads(item.get("content", "{}"))
            item.update(content_json)
            del item["content"]

        return port_list_result

        
    @classmethod
    async def add_port_services(cls, query_db: Session, port_object: PortModel):
        """
        新增端口信息service
        """
        try:
            # 处理 content 字段
            port_data = cls.prepare_port_content(port_object)
            await PortDao.add_port_dao(query_db, port_data)
            query_db.commit()
            return CrudResponseModel(is_success=True, message='Added successfully')
        except Exception as e:
            query_db.rollback()
            raise e


    @classmethod
    async def edit_port_services(cls, query_db: Session, port_object: PortModel):
        """
        编辑端口信息service
        """
        port_info = await cls.port_detail_services(query_db, port_object.id)
        if port_info.id:
            try:
                # 处理 content 字段
                port_data = cls.prepare_port_content(port_object)
                await PortDao.edit_port_dao(query_db, port_data)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Updated successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Port does not exist')


    @classmethod
    async def delete_port_services(cls, query_db: Session, port_object: DeletePortModel):
        """
        删除端口信息service

        :param query_db: orm对象
        :param page_object: 删除端口对象
        :return: 删除端口校验结果
        """
        if port_object.port_ids:
            port_id_list = port_object.port_ids.split(',')
            try:
                await PortDao.delete_port_dao(query_db, port_id_list)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Deleted successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='The port id is empty')

    @classmethod
    async def port_detail_services(cls, query_db: Session, port_id: int):
        """
        获取端口详细信息service

        :param query_db: orm对象
        :param port_id: 端口id
        :return: 端口id对应的信息
        """
        port = await PortDao.get_port_detail_by_id(query_db, port_id=port_id)
        if port:
            result = PortModel(**CamelCaseUtil.transform_result(port))
        else:
            result = PortModel(**dict())

        return result
