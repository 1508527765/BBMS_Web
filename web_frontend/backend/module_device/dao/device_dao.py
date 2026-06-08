import json
from typing import Set, Tuple


from sqlalchemy import delete, select, func, update
from sqlalchemy.orm import Session
from typing import List
from module_device.entity.do.device_do import Device
from module_device.entity.do.protocol_do import Protocol
from module_device.entity.vo.device_vo import DevicePageQueryModel
from module_device.entity.do.pcs_soft_version_do import PcsSoftVersion
from utils.page_util import PageUtil


class DeviceDao:
    """
    设备管理模块数据库操作层
    """

    @classmethod
    async def get_device_detail_by_id(cls, db: Session, device_id: int):
        """
        根据设备id获取设备详细信息

        :param db: orm对象
        :param device_id: 设备id
        :return: 设备信息对象
        """
        device_info = (db.execute(select(Device).where(Device.id == device_id))).scalars().first()

        return device_info

    @classmethod
    async def get_device_list(cls, db: Session, query_object: DevicePageQueryModel, is_page: bool = False):
        """
        根据查询参数获取设备列表信息

        :param db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 设备列表信息对象
        """
        query = (
            select(Device)
            .where(
                Device.code == query_object.code if query_object.code else True,
                Device.type == query_object.type if query_object.type is not None else True,
                Device.name.like(f'%{query_object.name}%') if query_object.name else True,
                Device.protocol_type == query_object.protocol_type if query_object.protocol_type is not None else True,
            )
            .order_by(Device.id)
            .distinct()
        )
        device_list = await PageUtil.paginate(db, query, query_object.page_num, query_object.page_size, is_page)

        for device in device_list.rows:
            protocol_query = select(Protocol).where(Protocol.related_id == device.get("id"))
            protocol_info = db.execute(protocol_query).scalars().first()

            if protocol_info:
                content_json = json.loads(protocol_info.content or "{}")
                device.update(content_json)

        return device_list
    
    @classmethod    
    async def get_all_device(cls, db: Session, name: str = None):
        """
        获取所有设备信息
        """
        query = select(Device).where(Device.name.like(f'%{name}%') if name else True)
        device_list = await PageUtil.paginate(db, query, 1, 10000, False)

        return device_list

    @classmethod
    async def add_device_dao(cls, db: Session, device: dict):
        """
        新增设备数据库操作

        :param db: orm对象
        :param device: 设备对象
        :return:
        """
        db_device = Device(**device)
        db.add(db_device)
        db.flush()

        return db_device

    @classmethod
    async def add_protocol_dao(cls, db: Session, protocol: dict):
        """
        新增设备数据库操作

        :param db: orm对象
        :param protocol: 协议对象
        :return:
        """
        db_protocol = Protocol(**protocol)
        db.add(db_protocol)
        db.flush()

        return db_protocol

    @classmethod
    async def edit_device_dao(cls, db: Session, device: dict):
        """
        编辑设备数据库操作

        :param db: orm对象
        :param device: 需要更新的设备字典
        :return:
        """
        db.execute(update(Device), [device])

    @classmethod
    async def delete_device_dao(cls, db: Session, device_ids: List[int]):
        """
        删除设备数据库操作

        :param db: orm对象
        :param device_ids: 设备id列表
        :return:
        """
        db.execute(delete(Device).where(Device.id.in_(device_ids)))

    @classmethod
    async def get_protocol_by_device_id(cls, db: Session, device_id: int):
        """
        根据设备 id 查询协议表中的记录
        """
        protocol_info = db.execute(select(Protocol).where(Protocol.related_id == device_id)).scalars().first()
        return protocol_info
    
    @classmethod
    async def get_devices_by_ids(cls, db: Session, device_ids: list):
        """
        根据设备 ID 列表查询设备
        """
        return db.execute(
            select(Device).where(Device.id.in_(device_ids))
        ).scalars().all()

    @classmethod
    async def edit_protocol_dao(cls, db: Session, protocol: dict):
        """
        编辑协议表的数据库操作
        """
        db.execute(update(Protocol).where(Protocol.related_id == protocol["related_id"]).values(protocol))

    @classmethod
    async def delete_protocol_dao(cls, db: Session, device_ids: List[int]):
        """
        删除协议表中相关设备的记录
        """
        db.execute(delete(Protocol).where(Protocol.related_id.in_(device_ids)))

    @classmethod
    async def count_devices_by_type(cls, db: Session, device_type: int) -> int:
        """
        统计同类型设备的数量
        """
        count = (
            db.execute(
                select(func.count('*'))
                .select_from(Device)
                .where(Device.type == device_type)
                .limit(1)
            )
        ).scalar()

        return count
    
    @classmethod
    async def batch_add_devices(cls, db: Session, devices: list):
        result = await db.execute(
            Device.__table__.insert().returning(Device.id),
            devices
        )
        return [row.id for row in result]
    
    @classmethod
    async def batch_add_protocols(cls, db: Session, protocols: list):
        result = await db.execute(
            Protocol.__table__.insert().returning(Protocol.id),
            protocols
        )
        return [row.id for row in result]
    
    @classmethod
    async def delete_all_devices(cls, db: Session):
        db.execute(delete(Device))

    @classmethod
    async def delete_all_protocols(cls, db: Session):
        db.execute(delete(Protocol))        

    @classmethod
    async def get_device_all(cls, db: Session):
        """
        获取所有设备信息，不带任何查询条件，仅用于设备导出
        """
        return db.execute(select(Device).order_by(Device.protocol_type, Device.type)).scalars().all()
    
    @classmethod
    async def get_protocol_all(cls, db: Session):
        return db.execute(select(Protocol).order_by(Protocol.related_id)).scalars().all()

    @classmethod
    def check_device_exists(cls, db: Session, template_id: int, dev_type: int) -> bool:
        """判断 point 表中是否存在 template_id 和 dev_type 对应的点"""
        return db.query(Device).filter_by(template_id=template_id, type=dev_type).first() is not None
    
    @classmethod
    def check_device_exists_by_name(cls, db: Session, name: str) -> bool:
        """判断是否有同名的点"""
        return db.query(Device).filter_by(name=name).first() is not None
    
    @classmethod
    def get_devices_by_type(cls, db: Session, type: int):
        return db.query(Device).filter_by(type=type).all()
    
    @classmethod
    def get_devices_by_type_and_protocol(cls, db: Session, type: int, protocol_type: int):
        return db.query(Device).filter_by(type=type, protocol_type=protocol_type).all()
    
    @classmethod
    def get_device_by_dev_type_and_dev_code(cls, db: Session, dev_type: int, dev_code: str):
        return db.query(Device).filter_by(type=dev_type, code=dev_code).first()
    
    @classmethod
    def get_devices_by_dev_pairs(cls, db: Session, dev_pairs: Set[Tuple[int, str]]):
        if not dev_pairs:   
            return []

        all_devices = db.query(Device).all()
        return [d for d in all_devices if (d.type, d.code) in dev_pairs]
    
    @classmethod
    def get_all_pcs_soft_versions(cls, db: Session):
        return db.query(PcsSoftVersion).all()