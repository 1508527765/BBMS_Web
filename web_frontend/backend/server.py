import os
from contextlib import asynccontextmanager
from fastapi import FastAPI
from config.env import AppConfig
from exceptions.handle import handle_exception
from middlewares.handle import handle_middleware
from module_admin.controller.common_controller import commonController
from module_admin.controller.dict_controller import dictController
from module_admin.controller.log_controller import logController
from module_admin.controller.login_controller import loginController
from module_admin.controller.menu_controller import menuController
from module_admin.controller.online_controller import onlineController
from module_admin.controller.role_controller import roleController
from module_admin.controller.server_controller import serverController
from module_admin.controller.user_controller import userController
from module_device.controller.template_controller import templateController
from module_device.controller.device_controller import deviceController
from module_device.controller.overview_controller import overviewController
from module_device.controller.northbound_controller import northboundController
from module_device.controller.port_controller import portController
from module_device.controller.topology_controller import topologyController
from module_device.controller.drynode_controller import drynodeController
from module_device.controller.history_controller import historyController
from module_device.controller.rtdb_controller import rtdbController
from module_device.controller.control_strategy_controller import controlStrategyController
from module_device.controller.battery_config_controller import batteryConfigController
from module_device.service.rtdb_service import init_rtdb, init_web_sign
from module_device.service.scu_version_service import init_web_version
from module_device.controller.localupdate_controller import localUpdateController
from sub_applications.handle import handle_sub_applications
from utils.log_util import logger


# 生命周期事件
def _env_truthy(name: str) -> bool:
    return os.getenv(name, "").strip().lower() in ("1", "true", "yes", "on")


@asynccontextmanager
async def lifespan(app: FastAPI):
    logger.info(f'{AppConfig.APP_NAME} Starting...')
    # C 进程异常退出时仍可启动后端联调（模板/拓扑/电池配置等非实时接口）；实时库接口依赖 ordin_bbms_c + SHM
    if _env_truthy("BBMS_SKIP_RTDB_INIT"):
        logger.warning(
            "已启用 BBMS_SKIP_RTDB_INIT：跳过 init_rtdb/init_web_sign（不写库、不改业务规则，仅开发联调）"
        )
    else:
        init_rtdb()
        init_web_sign()
    init_web_version()
    logger.info(f'{AppConfig.APP_NAME} Started successfully')
    yield


# 初始化FastAPI对象
app = FastAPI(
    title=AppConfig.APP_NAME,
    description=f'{AppConfig.APP_NAME} API Documentation',
    version=AppConfig.APP_VERSION,
    lifespan=lifespan,
)

# 挂载子应用
handle_sub_applications(app)
# 加载中间件处理方法
handle_middleware(app)
# 加载全局异常处理方法
handle_exception(app)

# 加载路由列表
controller_list = [
    {'router': loginController, 'tags': ['Login Module']},
    {'router': userController, 'tags': ['System Management - User Management']},
    {'router': roleController, 'tags': ['System Management - Role Management']},
    {'router': menuController, 'tags': ['System Management - Menu Management']},
    {'router': dictController, 'tags': ['System Management - Dictionary Management']},
    {'router': logController, 'tags': ['System Management - Log Management']},
    {'router': onlineController, 'tags': ['System Monitoring - Online Users']},
    {'router': serverController, 'tags': ['System Monitoring - Menu Management']},
    {'router': commonController, 'tags': ['Common Module']},
    {'router': templateController, 'tags': ['Template Management']},
    {'router': deviceController, 'tags': ['Device Management']},
    {'router': portController, 'tags': ['Port Management']},
    {'router': topologyController, 'tags': ['Device Topology']},
    {'router': drynodeController, 'tags': ['Dry Node Management']},
    {'router': northboundController, 'tags': ['Northbound Configuration Management']},
    {'router': rtdbController, 'tags': ['Time Series Database']},
    {'router': localUpdateController, 'tags': ['Local Maintenance Management']},
    {'router': controlStrategyController, 'tags': ['Control Strategy Management']},
    {'router': batteryConfigController, 'tags': ['Battery Configuration Management']},
    {'router': historyController, 'tags': ['History Data Management']},
    {'router': overviewController, 'tags': ['Overview Management']},
]

for controller in controller_list:
    app.include_router(router=controller.get('router'), tags=controller.get('tags'))
