from sqlalchemy import Column, Integer, String, SmallInteger, UniqueConstraint
from config.database import Base


class Device(Base):
    """
    设备表
    """
    __tablename__ = 'device'

    id = Column(Integer, primary_key=True, autoincrement=True, comment='主键标识', )
    sn = Column(String(50), nullable=False, comment='设备sn')
    type = Column(Integer, nullable=False, comment='设备类型')
    code = Column(Integer, nullable=False, comment='设备编号')
    name = Column(String(100), nullable=False, comment='设备名称')
    protocol_type = Column(Integer, nullable=False, comment='协议类型')
    template_id = Column(Integer, nullable=False, comment='模板ID')
    sort_order = Column(Integer, comment='序号')
    remark = Column(String(1200), comment='备注')
    status = Column(SmallInteger, comment='状态')

    # todo: UNIQUE(name), -- 设备名称唯一
    # todo: UNIQUE(type, code) -- 设备类型和设备编号唯一

    # __table_args__ = (UniqueConstraint('dict_type', name='uq_sys_dict_type_dict_type'),)
