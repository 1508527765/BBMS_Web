from sqlalchemy import delete, select, update, func
from sqlalchemy.orm import Session
from typing import List
from module_device.entity.do.template_do import Template
from module_device.entity.vo.template_vo import TemplateCreateUpdateModel, DeleteTemplateModel, TemplatePageQueryModel
from module_device.entity.do.template_point_do import TemplatePoint
from utils.page_util import PageUtil


class TemplateDao:
    """
    模板管理模块数据库操作层
    """

    @classmethod
    async def get_template_detail_by_id(cls, db: Session, template_id: int):
        """
        根据模板id获取模板详细信息

        :param db: orm对象
        :param template_id: 模板id
        :return: 模板信息对象
        """
        template_info = (db.execute(select(Template).where(Template.id == template_id))).scalars().first()

        return template_info


    @classmethod
    async def get_template_list(cls, db: Session, query_object: TemplatePageQueryModel, is_page: bool = False):
        """
        根据查询参数获取模板列表信息

        :param db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 模板列表信息对象
        """
        query = (
            select(Template)
            .where(
                Template.name.like(f'%{query_object.name}%') if query_object.name else True,
                Template.type == query_object.type if query_object.type is not None else True,
                Template.protocol_type == query_object.protocol_type if query_object.protocol_type is not None else True,
                Template.dev_type == query_object.dev_type if query_object.dev_type is not None else True,
            )
            .order_by(Template.id)
            .distinct()
        )
        template_list = await PageUtil.paginate(db, query, query_object.page_num, query_object.page_size, is_page)

        return template_list
    
    @classmethod
    async def get_template_all(cls, db: Session, template_type: int, protocol_type: str = None, dev_type: int = None):
        """
        根据协议类型、设备类型获取模板列表（创建设备时选用）

        :param db: orm对象
        :param protocol_type: 协议类型
        :param dev_type: 设备类型；与库中 dev_type 一致，或为 NULL 的历史模板
        :return: 模板列表信息对象
        """
        query = select(Template).order_by(Template.id).distinct()
        if protocol_type is not None:
            query = query.where(Template.protocol_type == protocol_type, Template.type == template_type)
        if dev_type is not None:
            from sqlalchemy import or_
            query = query.where(or_(Template.dev_type == dev_type, Template.dev_type.is_(None)))
        return await PageUtil.paginate(db, query, 1, 1000, False)

    @classmethod
    async def add_template_dao(cls, db: Session, template: dict):
        """
        新增模板数据库操作

        :param db: orm对象
        :param template: 模板对象
        :return:
        """
        db_template = Template(**template)
        db.add(db_template)
        db.flush()

        return db_template


    @classmethod
    async def edit_template_dao(cls, db: Session, template: dict):
        """
        编辑模板数据库操作

        :param db: orm对象
        :param template: 需要更新的模板字典
        :return:
        """
        db.execute(update(Template), [template])

    @classmethod
    async def delete_template_dao(cls, db: Session, template_ids: List[int]):
        """
        删除模板数据库操作

        :param db: orm对象
        :param template_ids: 模板id列表
        :return:
        """
        db.execute(delete(Template).where(Template.id.in_(template_ids)))

    @classmethod
    async def delete_all_templates(cls, db: Session, template_type: int):
        """
        删除某一类型的所有模板
        """
        db.execute(delete(Template).where(Template.type == template_type))

    @classmethod
    async def batch_add_templates(cls, db: Session, templates: List[dict]):
        """
        批量添加模板数据库操作

        :param db: orm对象
        :param templates: 模板字典列表
        :return:
        """
        db_templates = [Template(**template) for template in templates]
        db.bulk_save_objects(db_templates)
        db.flush()

        return db_templates


    @classmethod
    async def get_template_by_name(cls, db: Session, name: str):
        """
        根据模板名称获取模板信息
        """
        return db.execute(select(Template).where(Template.name == name)).scalars().first()  

    @classmethod
    async def get_template_point_detail_by_id(cls, db: Session, id: int):
        """
        根据测点id获取测点详细信息

        :param db: orm对象
        :param id: 测点id
        :return: 测点信息对象
        """
        point_info = (db.execute(select(TemplatePoint).where(TemplatePoint.id == id))).scalars().first()

        return point_info