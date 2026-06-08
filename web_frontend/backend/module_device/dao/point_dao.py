from sqlalchemy import select, update
from sqlalchemy.orm import Session
from module_device.entity.do.point_do import Point
from module_device.entity.do.template_do import Template
from module_device.entity.do.template_point_do import TemplatePoint
from typing import List, Union
from utils.page_util import PageUtil


class PointDao:
    """
    测点管理模块数据库操作层
    """

    @classmethod
    async def get_point_list_by_template_id(cls, db: Session, template_id: int):
        """
        根据模板ID获取测点列表信息

        :param db: orm对象
        :param template_id: 模板ID
        :return: 设备列表信息对象
        :param dev_type: 设备类型
        """
        query = (
            select(TemplatePoint)
            .where(
                TemplatePoint.template_id == template_id,
            )
            .order_by(TemplatePoint.id)
            .distinct()
        )
        point_list = await PageUtil.paginate(db, query, 1, 1000, False)  #todo: 先不考虑分页

        return point_list
    
    @classmethod
    async def get_point_list_by_device_template(cls, db: Session, template_id: int, dev_type: int):
        """
        根据模板ID和设备类型获取测点列表信息

        :param db: orm对象
        :param template_id: 模板ID
        :param dev_type: 设备类型
        :return: 设备列表信息对象
        """
        query = (
            select(Point)
            .where(
                Point.template_id == template_id,
                Point.dev_type == dev_type,
            )
            .order_by(Point.id)
            .distinct()
        )
        point_list = await PageUtil.paginate(db, query, 1, 1000, False)  #todo: 先不考虑分页

        return point_list

    @classmethod
    def check_point_exists(cls, db: Session, template_id: int, dev_type: int) -> bool:
        """判断 point 表中是否存在 template_id 和 dev_type 对应的点"""
        return db.query(Point).filter_by(template_id=template_id, dev_type=dev_type).first() is not None
    
    @classmethod
    async def check_template_point_exists(cls, db: Session, template_id: int) -> bool:
        """
        判断 template_point 表中是否存在 template_id 对应的点
        """
        result = db.execute(
            select(TemplatePoint.id).where(TemplatePoint.template_id == template_id).limit(1)
        )
        return result.first() is not None
    
    @classmethod
    async def add_device_points_batch_dao(cls, query_db: Session, points: List[dict]):
        """
        批量插入设备点位
        """
        query_db.bulk_insert_mappings(Point, points)

    @classmethod
    async def add_template_point_dao(cls, db: Session, point: dict):
        """
        新增测点数据库操作

        :param db: orm对象
        :param point: 测点对象
        :return:
        """
        db_point = TemplatePoint(**point)
        db.add(db_point)
        db.flush()

        return db_point
    
    @classmethod
    async def add_point_dao(cls, db: Session, point: dict):
        """
        新增测点数据库操作

        :param db: orm对象
        :param point: 测点对象
        :return:
        """
        db_point = Point(**point)
        db.add(db_point)
        db.flush()

        return db_point
    
    @classmethod
    async def edit_template_point_dao(cls, db: Session, point: dict):
        """
        编辑模板测点信息数据库操作
        """
        db.execute(update(TemplatePoint).where(TemplatePoint.id == point['id']), point)
        db.commit()

    @classmethod
    async def edit_point_dao(cls, db: Session, point: dict):
        """
        编辑设备测点信息数据库操作
        """
        db.execute(update(Point).where(Point.id == point['id']), point)
        db.commit()

    @classmethod
    async def delete_template_point_by_id(cls, db: Session, point_id: Union[int, List[int]]):
        """
        根据测点ID删除TemplatePoint表测点
        """
        if isinstance(point_id, int):
            db.query(TemplatePoint).filter(TemplatePoint.id == point_id).delete()
        elif isinstance(point_id, list):
            db.query(TemplatePoint).filter(TemplatePoint.id.in_(point_id)).delete()

    @classmethod
    async def delete_point_by_id(cls, db: Session, point_id: Union[int, List[int]]):
        """
        根据测点ID删除Point表测点
        """
        if isinstance(point_id, int):
            db.query(Point).filter(Point.id == point_id).delete()
        elif isinstance(point_id, list):
            db.query(Point).filter(Point.id.in_(point_id)).delete()

    @classmethod
    async def delete_template_points_by_template_id(cls, db: Session, template_id: int):
        """
        根据模板ID删除TemplatePoint表测点
        """
        db.query(TemplatePoint).filter(TemplatePoint.template_id == template_id).delete()

    @classmethod
    async def sync_template_points_protocol_from_template(cls, db: Session, template_id: int) -> int:
        """
        将 template_point.protocol_type 统一为所属 template.protocol_type。
        返回受影响的行数（无模板时返回 0）。
        """
        row = db.execute(select(Template.protocol_type).where(Template.id == template_id)).first()
        if not row:
            return 0
        proto = int(row[0])
        res = db.execute(
            update(TemplatePoint)
            .where(TemplatePoint.template_id == template_id)
            .values(protocol_type=proto)
        )
        db.flush()
        return getattr(res, "rowcount", 0) or 0
    
    @classmethod
    async def delete_points_by_template_id_and_dev_type(cls, db: Session, template_id: int, dev_type: int):
        """
        根据模板ID和设备id删除Point表测点
        """
        db.query(Point).filter(Point.template_id == template_id, Point.dev_type == dev_type).delete()

    @classmethod
    async def get_device_point_detail_by_id(cls, db: Session, id: int):
        """
        根据测点id获取测点详细信息
        """
        point_info = (db.execute(select(Point).where(Point.id == id))).scalars().first()

        return point_info

    # @classmethod
    # async def get_device_points_by_dev_type_and_dev_code(cls, db: Session, dev_type: int, dev_code: int):
    #     """
    #     根据设备 id 查询协议表中的记录
    #     """
    #     point_info = db.execute(select(Point).where(Point.dev_type == dev_type, Point.code == dev_code)).scalars().all()
    #     return point_info

    @classmethod
    def get_point_names_by_dev_type_and_template_id(cls, db: Session, dev_type: int, template_id: int):
        point_list = db.query(Point).filter_by(dev_type=dev_type, template_id=template_id).order_by(Point.id).all()
        return {point.code: (point.name, point.ename) for point in point_list}
    
    @classmethod
    def get_point_list_by_device_template_and_group_type(cls, db: Session, template_id: int, dev_type: int, group_type: int):
        """
        获取测点列表信息，根据模板ID、设备类型、测点类型获取测点列表信息

        :param db: orm对象
        :param template_id: 模板ID
        :param dev_type: 设备类型
        :param group_type: 测点类型
        :return: 设备列表信息对象
        """
        query = (
            select(Point)
            .where(
                Point.template_id == template_id,
                Point.dev_type == dev_type,
                Point.group_type == group_type,
            )
            .order_by(Point.id)
            .distinct()
        )
        point_list = db.execute(query).scalars().all()

        return point_list