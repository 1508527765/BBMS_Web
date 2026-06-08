from sqlalchemy import Column, Integer, SmallInteger, String
from config.database import Base


class DiDoSet(Base):
    """
    DIDO逻辑设置表
    """
    __tablename__ = 'di_do_set'

    id = Column(Integer, primary_key=True, autoincrement=True, comment='主键标识')
    seq = Column(SmallInteger, nullable=False, comment='DIDO序列号')
    type = Column(SmallInteger, nullable=False, comment='DIDO类型: 0DI，1DO')
    cabinet_code = Column(SmallInteger, nullable=False, comment='柜子编号-- 预留用')
    strategy_seq = Column(SmallInteger, nullable=False, comment='策略序号-- 从数据表中获取')
    remark = Column(String(1200), comment='备注')


class DiLogicDict(Base):
    """
    DI逻辑字典表
    """
    __tablename__ = 'di_logic_dict'

    id = Column(Integer, primary_key=True, autoincrement=True, comment='主键标识，表示DI序号')
    strategy_seq = Column(SmallInteger, nullable=False, unique=True, comment='策略序号，确保唯一')
    strategy_name = Column(String(1200), comment='策略名称')
    strategy_ename = Column(String(1200), comment='策略英文名称')


class DoLogicDict(Base):
    """
    DO逻辑字典表
    """
    __tablename__ = 'do_logic_dict'

    id = Column(Integer, primary_key=True, autoincrement=True, comment='主键标识，表示DO序号')
    strategy_seq = Column(SmallInteger, nullable=False, unique=True, comment='策略序号，确保唯一')
    strategy_name = Column(String(1200), comment='策略名称')
    strategy_ename = Column(String(1200), comment='策略英文名称')