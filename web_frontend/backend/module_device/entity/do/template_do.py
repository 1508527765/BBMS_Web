from sqlalchemy import Column, Integer, String
from config.database import Base


class Template(Base):
    """
    模板表
    """
    __tablename__ = 'template'
    
    id = Column(Integer, primary_key=True, comment='ID')
    type = Column(Integer, comment='类型')
    protocol_type = Column(Integer, comment='协议类型')
    dev_type = Column(Integer, comment='设备类型')
    name = Column(String, comment='名称')
    remark = Column(String, comment='备注')
    