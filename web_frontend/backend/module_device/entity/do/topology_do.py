from sqlalchemy import Column, Integer, String, Text
from config.database import Base


class Topology(Base):
    """
    拓扑结构表
    """
    __tablename__ = 'topology'

    id = Column(Integer, primary_key=True, autoincrement=True, comment='主键标识')
    parent_id = Column(Integer, comment='父Id')
    menu_tree = Column(Integer, nullable=False, comment='类型:0-虚拟节点，1-设备节点')
    name = Column(String(100), comment='层级名称')
    dev_id = Column(Integer, comment='设备Id')
    virtual_dev_type = Column(Integer, comment='虚拟节点设备类型(dev_type_e)，menu_tree=0 时使用')
    sort_order = Column(Integer, nullable=False, comment='序号')
    remark = Column(Text, comment='备注')