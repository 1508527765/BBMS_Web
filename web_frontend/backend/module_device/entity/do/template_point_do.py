from pydantic import ConfigDict
from pydantic.alias_generators import to_camel

from sqlalchemy import Column, Integer, String, Float, SmallInteger
from config.database import Base


class TemplatePoint(Base):
    __tablename__ = 'template_point'

    model_config = ConfigDict(alias_generator=to_camel, from_attributes=True)


    id = Column(Integer, primary_key=True, autoincrement=True)
    template_id = Column(Integer, nullable=False)
    protocol_type = Column(Integer, nullable=False)
    dev_type = Column(Integer, nullable=False)
    group_type = Column(Integer, nullable=False)
    code = Column(Integer, nullable=False)
    name = Column(String(100), nullable=False)
    ename = Column(String(100), nullable=False)
    data_type = Column(Integer, nullable=False)
    attribute = Column(Integer, nullable=False)
    func = Column(Integer, nullable=False)
    reg_addr = Column(Integer, nullable=False)
    bit_pos = Column(Integer)
    bit_num = Column(Integer)
    endian = Column(Integer, nullable=False)
    precision = Column(Integer)
    ratio = Column(Float, nullable=False)
    offset = Column(Float, nullable=False)
    is_persisted = Column(SmallInteger, nullable=False)
    storage_interval = Column(Integer, nullable=False)
    mutate_bound = Column(Float)
    default_value = Column(Float)
    min_value = Column(Float)
    max_value = Column(Float)
    unit = Column(String(20))
    is_show = Column(SmallInteger, nullable=False)

    # def __repr__(self):
    #     return f"<PointModel(id={self.id}, name={self.name}, code={self.code})>"
