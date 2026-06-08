from sqlalchemy import Column, Integer, String, Float, Text
from config.database import Base

class BasePcsConfig(Base):
    __abstract__ = True

    id = Column(Integer, primary_key=True, autoincrement=True)
    point_id = Column(Integer)
    can_id = Column(String, nullable=False)
    param_name = Column(String, nullable=False)
    param_ename = Column(String, nullable=False)
    data_type = Column(String, nullable=False)
    coefficient = Column(Float, nullable=False, default=1.0)
    default_value = Column(String, nullable=False)
    current_value = Column(String, nullable=False)
    value_config = Column(Text, nullable=False)
    ui_type = Column(String, nullable=False)
    description = Column(String)
    sort_order = Column(Integer, default=0)
    created_at = Column(String, default="datetime('now')")
    updated_at = Column(String, default="datetime('now')")


class PcsProtectConfig(BasePcsConfig):
        __tablename__ = "pcs_protect_config"


class PcsSysConfig(BasePcsConfig):
        __tablename__ = "pcs_sys_config"


class PcsRunConfig(BasePcsConfig):
        __tablename__ = "pcs_run_config"


class LogicAdvanceConfig(Base):
        __tablename__ = "logic_advance_config"

        id = Column(Integer, primary_key=True, autoincrement=True)
        type = Column(Integer, nullable=False)
        name = Column(String(100), nullable=False, unique=True)
        content = Column(String(1200), nullable=False)
