# from config.database import Base
from config.env import DataBaseConfig
from utils.log_util import logger
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from sqlalchemy.pool import NullPool

dbc = DataBaseConfig

# 使用 sqlite 数据库
DATABASE_URL = DataBaseConfig.db_url_web
DATABASE_URL_EMS = DataBaseConfig.db_url_scu
DATABASE_URL_BUSINESS = DataBaseConfig.db_url_business

# 创建 Engine，禁用连接池
engine = create_engine(
    DATABASE_URL,
    poolclass=NullPool,
    connect_args={"check_same_thread": False},  # SQLite 多线程访问必须加
)
engine_ems = create_engine(
    DATABASE_URL_EMS,
    poolclass=NullPool,
    connect_args={"check_same_thread": False},
)
engine_business = create_engine(
    DATABASE_URL_BUSINESS,
    poolclass=NullPool,
    connect_args={"check_same_thread": False},
)

# 普通 SessionLocal（非 scoped_session）
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
EmsSessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine_ems)
BusinessSessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine_business)

# 每次依赖注入创建全新 Session
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

def get_ems_db():
    db = EmsSessionLocal()
    try:
        yield db
    finally:
        db.close()

def get_business_db():
    db = BusinessSessionLocal()
    try:
        yield db
    finally:
        db.close()

# def init_create_table():
#     """
#     应用启动时初始化数据库连接
#     """
#     logger.info('初始化数据库连接...')
#     with engine.begin() as conn:
#         Base.metadata.create_all(bind=conn)
#     logger.info('数据库连接成功')
