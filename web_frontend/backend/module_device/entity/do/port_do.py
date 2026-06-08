from sqlalchemy import Column, Integer, String
from config.database import Base


class Port(Base):
    """
    端口表
    """
    __tablename__ = 'ems_device_config'
    
    id = Column(Integer, primary_key=True, comment='ID')
    type = Column(Integer, comment='类型')
    name = Column(String, comment='名称')
    content = Column(String, comment='内容')