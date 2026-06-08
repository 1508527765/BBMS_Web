from sqlalchemy import Column, Integer, String
from config.database import Base


class PcsSoftVersion(Base):
    __tablename__ = 'pcs_soft_version'

    id = Column(Integer, primary_key=True, autoincrement=True)
    name = Column(String(100), nullable=False)
    cpld_version = Column(String(100), nullable=False)
    dsp_cpu1_version = Column(String(100), nullable=False)
    dsp_cpu2_version = Column(String(100), nullable=False)
    pcs_version = Column(String(100), nullable=False)