import os
import sys
import uvicorn
from server import app, AppConfig  # noqa: F401


if getattr(sys, 'frozen', False):
    # 如果是 PyInstaller 打包后的应用，修正路径
    app_dir = os.path.dirname(sys.executable)
    sys.path.append(app_dir)

if __name__ == '__main__':
    # 与 GetConfig 一致：按 APP_ENV 选 .env，避免 prod 仍被强制加载 .env.dev
    _run_env = (getattr(AppConfig, 'APP_ENV', None) or os.environ.get('APP_ENV', 'dev') or 'dev').strip()
    _env_file = f'.env.{_run_env}'
    if not os.path.exists(_env_file):
        _env_file = '.env.dev'
    uvicorn.run(
        app=app,
        host=AppConfig.APP_HOST,
        port=AppConfig.APP_PORT,
        root_path=AppConfig.APP_ROOT_PATH,
        reload=False,  # 禁用自动重载功能
        env_file=_env_file,
    )