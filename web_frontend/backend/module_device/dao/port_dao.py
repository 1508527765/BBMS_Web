import json

from sqlalchemy import delete, select, update
from sqlalchemy.orm import Session
from typing import List
from module_device.entity.do.port_do import Port
from module_device.entity.vo.port_vo import PortPageQueryModel
from utils.page_util import PageUtil


class PortDao:
    """
    端口管理模块数据库操作层
    """

    @classmethod
    async def get_port_detail_by_id(cls, db: Session, port_id: int):
        """
        根据端口id获取端口详细信息

        :param db: orm对象
        :param port_id: 端口id
        :return: 端口信息对象
        """
        port_info = (db.execute(select(Port).where(Port.id == port_id))).scalars().first()

        return port_info


    @classmethod
    async def get_port_list(cls, db: Session, query_object: PortPageQueryModel, is_page: bool = False):
        """
        根据查询参数获取端口列表信息

        :param db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 端口列表信息对象
        """
        query = (
            select(Port)
            .where(
                Port.type == query_object.type if query_object.type is not None else True,
            )
            .order_by(Port.id)
            .distinct()
        )
        port_list = await PageUtil.paginate(db, query, query_object.page_num, query_object.page_size, is_page)

        return port_list

    @classmethod
    async def add_port_dao(cls, db: Session, port: dict):
        """
        新增端口数据库操作
        """
        db_port = Port(**port)
        db.add(db_port)
        db.flush()
        return db_port

    @classmethod
    async def edit_port_dao(cls, db: Session, port: dict):
        """
        编辑端口数据库操作
        """
        db.execute(update(Port).where(Port.id == port["id"]).values(port))

    @classmethod
    async def delete_port_dao  (cls, db: Session, port_ids: List[int]):
        """
        删除端口数据库操作

        :param db: orm对象
        :param port_ids: 端口id列表
        :return:
        """
        db.execute(delete(Port).where(Port.id.in_(port_ids)))


    @classmethod
    def get_can_port_ids(cls, db: Session):
        """
        获取所有can配置，找到canId为0和1的两个通道的id并返回其id和名称

        :param db: orm对象
        :return: 包含canId为0和1的端口id和名称的列表
        """
        can_ports = db.execute(select(Port).where(Port.type == 7)).scalars().all()
        result = []
        for port in can_ports:
            content = json.loads(port.content)
            if 'canId' in content and content['canId'] in [0, 1]:
                result.append({"id": content['canId'], "name": port.name})
        return result
