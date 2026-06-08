from sqlalchemy import Column, Integer, String, SmallInteger, UniqueConstraint
from config.database import Base


class NorthConfig(Base):
    """
    北向配置表
    """
    __tablename__ = 'north_config'

    id = Column(Integer, primary_key=True, autoincrement=True, comment='主键标识', )
    name = Column(String(100), nullable=False, comment='北向配置名称')
    protocol_type = Column(Integer, nullable=False, comment='协议类型')
    config_type = Column(Integer, nullable=False, comment='配置类型')
    template_id = Column(Integer, nullable=False, comment='模板ID')
    remark = Column(String(1200), comment='备注')
