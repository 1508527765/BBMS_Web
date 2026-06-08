from sqlalchemy.orm import Session
from exceptions.exception import ServiceException
from module_device.dao.template_dao import TemplateDao
from module_device.dao.point_dao import PointDao
from module_device.entity.vo.template_vo import DeleteTemplateModel, TemplateCreateUpdateModel, TemplateModel, TemplatePageQueryModel, CrudResponseModel
from module_device.entity.vo.template_vo import TemplatePointModel
from utils.common_util import CamelCaseUtil


class TemplateService:
    """
    模板管理模块服务层
    """

    @classmethod
    async def _protocol_type_for_template(cls, query_db: Session, template_id: int) -> int:
        """与 template 表 protocol_type 保持一致（模板测点写入时统一使用）。"""
        t = await TemplateDao.get_template_detail_by_id(query_db, template_id)
        if not t:
            raise ServiceException(message="Template does not exist")
        return int(t.protocol_type)

    @classmethod
    async def get_template_list_services(
        cls, query_db: Session, query_object: TemplatePageQueryModel, is_page: bool = True
    ):
        """
        获取模板列表信息service

        :param query_db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 模板列表信息对象
        """
        template_list_result = await TemplateDao.get_template_list(query_db, query_object, is_page)

        return template_list_result

    @classmethod
    async def get_template_all_services(cls, query_db: Session, template_type: int, protocol_type: str, dev_type: int = None):
        """
        获取所有模板信息service
        """
        template_all_result = await TemplateDao.get_template_all(query_db, template_type, protocol_type, dev_type)
        return template_all_result

    @classmethod
    async def add_template_services(cls, query_db: Session, template_object: TemplateCreateUpdateModel):
        """
        新增模板信息service

        :param query_db: orm对象
        :param page_object: 新增模板对象
        :return: 新增模板校验结果
        """
        try:
            if template_object.type == 0 and template_object.dev_type is None:
                raise ServiceException(message='Device type is required for device template')
            # 查询是否有同名的模板
            existing_template = await TemplateDao.get_template_by_name(query_db, template_object.name)
            if existing_template:
                raise ServiceException(message='Template name already exists')
            
            template_data = template_object.model_dump()
            await TemplateDao.add_template_dao(query_db, template_data)
            query_db.commit()
            return CrudResponseModel(is_success=True, message='Added successfully')
        except Exception as e:
            query_db.rollback()
            raise e

    @classmethod
    async def edit_template_services(cls, query_db: Session, template_object: TemplateCreateUpdateModel):
        """
        编辑模板信息service

        :param query_db: orm对象
        :param template_object: 编辑模板对象
        :return: 编辑模板校验结果
        """
        edit_template = template_object.model_dump(exclude_unset=True)
        template_info = await cls.template_detail_services(query_db, template_object.id)
        if template_info.id:
            if template_object.type == 0 and template_object.dev_type is None:
                raise ServiceException(message='Device type is required for device template')
            try:
                await TemplateDao.edit_template_dao(query_db, edit_template)
                await PointDao.sync_template_points_protocol_from_template(query_db, template_object.id)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Updated successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Template does not exist')

    @classmethod
    async def delete_template_services(cls, query_db: Session, template_object: DeleteTemplateModel):
        """
        删除模板信息service

        :param query_db: orm对象
        :param page_object: 删除模板对象
        :return: 删除模板校验结果
        """
        if template_object.template_ids:
            template_id_list = template_object.template_ids.split(',')
            try:
                await TemplateDao.delete_template_dao(query_db, template_id_list)
                for template_id in template_id_list:
                    await PointDao.delete_template_points_by_template_id(query_db, template_id)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Deleted successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Template id(s) is empty')

    @classmethod
    async def template_detail_services(cls, query_db: Session, template_id: int):
        """
        获取模板详细信息service

        :param query_db: orm对象
        :param template_id: 模板id
        :return: 模板id对应的信息
        """
        template = await TemplateDao.get_template_detail_by_id(query_db, template_id=template_id)
        if template:
            result = TemplateModel(**CamelCaseUtil.transform_result(template))
        else:
            result = TemplateModel(**dict())

        return result
    
    @classmethod
    async def get_template_points_services(cls, query_db: Session, template_id: int):
        """
        获取模板测点信息service
        """
        points = await PointDao.get_point_list_by_template_id(query_db, template_id)
        return points
    
    @classmethod
    async def template_point_detail_services(cls, query_db: Session, point_id: int):
        """
        获取模板测点详细信息service
        """
        point = await TemplateDao.get_template_point_detail_by_id(query_db, point_id)
        if not point:
            return None
        return point

    @classmethod
    async def add_template_point_services(cls, query_db: Session, add_point: TemplatePointModel):
        """
        增加模板测点信息service
        """
        try:
            point_data = add_point.model_dump(by_alias=False)
            template_id = point_data["template_id"]
            point_data["protocol_type"] = await cls._protocol_type_for_template(query_db, template_id)
            t = await TemplateDao.get_template_detail_by_id(query_db, template_id)
            if t and t.dev_type is not None:
                point_data["dev_type"] = int(t.dev_type)
            db_point = await PointDao.add_template_point_dao(query_db, point_data)
            query_db.commit()
            return CrudResponseModel(is_success=True, message='Added successfully', result=db_point.id)
        except Exception as e:
            query_db.rollback()
            raise e

    @classmethod
    async def edit_template_point_services(cls, query_db: Session, edit_point: TemplatePointModel):
        """
        编辑模板测点信息service
        """
        edit_point = edit_point.model_dump(exclude_unset=True)
        point_info = await cls.template_point_detail_services(query_db, edit_point.get('id'))
        if point_info.id:
            try:
                edit_point["protocol_type"] = await cls._protocol_type_for_template(
                    query_db, point_info.template_id
                )
                await PointDao.edit_template_point_dao(query_db, edit_point)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Updated successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Template point does not exist')

    @classmethod
    async def delete_template_point_services(cls, query_db: Session, point_ids: str):
        """
        删除模板测点信息service
        """
        if point_ids:
            point_id_list = point_ids.split(',')
            try:
                await PointDao.delete_template_point_by_id(query_db, point_id_list)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Deleted successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Point id(s) is empty')

    @classmethod
    async def has_template_points_services(cls, query_db: Session, template_id: int) -> bool:
        """
        判断模板是否有测点service
        
        :param query_db: orm对象
        :param template_id: 模板id
        :return: 是否有测点
        """
        try:
            has_point = await PointDao.check_template_point_exists(query_db, template_id)
            return has_point
        except Exception as e:
            raise e
