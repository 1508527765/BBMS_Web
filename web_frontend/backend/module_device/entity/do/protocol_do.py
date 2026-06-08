from sqlalchemy import Column, Integer, String
from config.database import Base


class Protocol(Base):
    """
    协议表
    """
    __tablename__ = 'protocol'

    id = Column(Integer, primary_key=True, autoincrement=True, comment='主键标识', )
    related_id = Column(Integer, nullable=False, comment='关联设备ID')
    template_type = Column(Integer, nullable=False, comment='模板类型')
    type = Column(Integer, nullable=False, comment='协议类型')
    name = Column(String(100), nullable=False, comment='设备名称')
    content = Column(String(1200), comment='协议内容')
    remark = Column(String(1200), comment='备注')