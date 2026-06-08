from sqlalchemy import Column, Integer, Float, TIMESTAMP
from config.database import Base


class TimedDataStorage(Base):
    """
    设备时序数据存储表
    """
    __tablename__ = 'timed_data_storage'

    id = Column(Integer, primary_key=True, autoincrement=True, comment='主键标识')
    dev_type = Column(Integer, nullable=False, comment='设备类型')
    dev_id = Column(Integer, nullable=False, comment='设备Id')
    point_id = Column(Integer, nullable=False, comment='点位Id')
    storage_interval = Column(Integer, nullable=False, comment='存储周期')
    timed_value = Column(Float, nullable=False, comment='值')
    created_time = Column(TIMESTAMP, nullable=False, comment='备份时间', server_default='CURRENT_TIMESTAMP')


class FaultsRecord(Base):
    """
    故障数据记录表
    """
    __tablename__ = 'faults_record'

    id = Column(Integer, primary_key=True, autoincrement=True, comment='主键标识')
    dev_type = Column(Integer, nullable=False, comment='设备类型')
    dev_id = Column(Integer, nullable=False, comment='设备Id')
    point_id = Column(Integer, nullable=False, comment='点位Id')
    faults_value = Column(Integer, nullable=False, comment='故障值')
    created_time = Column(TIMESTAMP, nullable=False, comment='备份时间', server_default='CURRENT_TIMESTAMP')


class EventsRecord(Base):
    """
    事件数据记录表
    """
    __tablename__ = 'events_record'

    id = Column(Integer, primary_key=True, autoincrement=True, comment='主键标识')
    dev_type = Column(Integer, nullable=False, comment='设备类型')
    dev_id = Column(Integer, nullable=False, comment='设备Id')
    point_id = Column(Integer, nullable=False, comment='点位Id')
    faults_value = Column(Integer, nullable=False, comment='故障值')
    created_time = Column(TIMESTAMP, nullable=False, comment='备份时间', server_default='CURRENT_TIMESTAMP')
