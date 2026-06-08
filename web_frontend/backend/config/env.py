import argparse
import os
import sys
from dotenv import load_dotenv
from functools import lru_cache
from pydantic_settings import BaseSettings
from typing import Literal


class AppSettings(BaseSettings):
    """
    ????
    """

    APP_ENV: str = 'prod'
    APP_NAME: str = 'RuoYi-FasAPI'
    APP_ROOT_PATH: str = '/prod-api'
    APP_HOST: str = '0.0.0.0'
    APP_PORT: int = 9101
    APP_VERSION: str = '1.0.0'
    APP_RELOAD: bool = True
    # APP_IP_LOCATION_QUERY: bool = True
    APP_SAME_TIME_LOGIN: bool = True
    
    class Config:
        case_sensitive = True


class JwtSettings(BaseSettings):
    """
    Jwt??
    """

    jwt_secret_key: str = 'b01c66dc2c58dc6a0aabfe2144256be36226de378bf87f72c0c795dda67f4d55'
    jwt_algorithm: str = 'HS256'
    jwt_expire_minutes: int = 1440
    jwt_redis_expire_minutes: int = 30
    
    class Config:
        case_sensitive = True


class DataBaseSettings(BaseSettings):
    """
    ?????    """

    db_type: Literal['sqlite'] = 'sqlite'
    db_database: str = 'web.db'
    db_echo: bool = True
    db_max_overflow: int = 10
    db_pool_size: int = 50
    db_pool_recycle: int = 3600
    db_pool_timeout: int = 30
    # 与部署一致：默认使用 /opt/bbms/db/（可被 .env.* 或环境变量覆盖）
    db_url_web: str = 'sqlite:////opt/bbms/db/web.db'
    db_url_scu: str = 'sqlite:////opt/bbms/db/sqlite3.db'
    db_url_business: str = 'sqlite:////opt/bbms/db/business.db'
    
    class Config:
        # 兼容 DB_URL_WEB / db_url_web 两种写法，避免部署时大小写导致配置失效
        case_sensitive = False


class UploadSettings:

    """
    ????
    """

    UPLOAD_PREFIX = '/profile'
    UPLOAD_PATH = 'vf_admin/upload_path'
    UPLOAD_MACHINE = 'A'
    DEFAULT_ALLOWED_EXTENSION = [
        # ??
        'bmp',
        'gif',
        'jpg',
        'jpeg',
        'png',
        # word excel powerpoint
        'doc',
        'docx',
        'xls',
        'xlsx',
        'ppt',
        'pptx',
        'html',
        'htm',
        'txt',
        # ????
        'rar',
        'zip',
        'gz',
        'bz2',
        # ????
        'mp4',
        'avi',
        'rmvb',
        # pdf
        'pdf',
    ]
    DOWNLOAD_PATH = 'vf_admin/download_path'

    def __init__(self):
        if not os.path.exists(self.UPLOAD_PATH):
            os.makedirs(self.UPLOAD_PATH)
        if not os.path.exists(self.DOWNLOAD_PATH):
            os.makedirs(self.DOWNLOAD_PATH)


class CachePathConfig:
    """
    ??????
    """

    PATH = os.path.join(os.path.abspath(os.getcwd()), 'caches')
    PATHSTR = 'caches'


class GetConfig:
    """
    ????
    """

    def __init__(self):
        self.parse_cli_args()

    @lru_cache()
    def get_app_config(self):
        """
        ??????
        """
        # ?????????
        return AppSettings()

    @lru_cache()
    def get_jwt_config(self):
        """
        ??Jwt??
        """
        # ???Jwt????
        return JwtSettings()

    @lru_cache()
    def get_database_config(self):
        """
        ???????        """
        # ??????????
        return DataBaseSettings()

    @lru_cache()
    def get_upload_config(self):
        """
        ???????        """
        # ??????
        return UploadSettings()

    @staticmethod
    def parse_cli_args():
        """
        ???????        """
        if 'uvicorn' in sys.argv[0]:
            # uvicorn 场景下，不主动覆盖已有 APP_ENV
            if not os.environ.get('APP_ENV', '').strip():
                os.environ['APP_ENV'] = 'dev'
        else:
            parser = argparse.ArgumentParser(description='cli')
            parser.add_argument('--env', type=str, default='', help='????')
            args, _ = parser.parse_known_args()
            # 显式传参优先；未传时仅在未设置 APP_ENV 时才回退到 dev
            if args.env and str(args.env).strip():
                os.environ['APP_ENV'] = str(args.env).strip()
            elif not os.environ.get('APP_ENV', '').strip():
                os.environ['APP_ENV'] = 'dev'
        # ??????
        run_env = os.environ.get('APP_ENV', 'dev').strip() or 'dev'
        env_file = f'.env.{run_env}'
        if not os.path.exists(env_file):
            env_file = '.env.dev'
        # ????
        load_dotenv(env_file)


# ????????
get_config = GetConfig()
# ????
AppConfig = get_config.get_app_config()
# Jwt??
JwtConfig = get_config.get_jwt_config()
# ?????
DataBaseConfig = get_config.get_database_config()
# ????
UploadConfig = get_config.get_upload_config()
