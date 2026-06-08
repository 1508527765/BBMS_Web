import jwt
import random
import uuid
from datetime import datetime, timedelta, timezone
from fastapi import Depends, Form, Request
from fastapi.security import OAuth2PasswordBearer, OAuth2PasswordRequestForm
from jwt.exceptions import InvalidTokenError
# from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.orm import Session
from typing import Dict, List, Optional, Union
from config.constant import CommonConstant, MenuConstant
from config.env import AppConfig, JwtConfig
from config.get_db import get_db
from exceptions.exception import LoginException, AuthException, ServiceException
from module_admin.dao.login_dao import login_by_account
from module_admin.dao.user_dao import UserDao
from module_admin.entity.do.menu_do import SysMenu
from module_admin.entity.vo.common_vo import CrudResponseModel
from module_admin.entity.vo.login_vo import MenuTreeModel, MetaModel, RouterModel, SmsCode, UserLogin, UserRegister
from module_admin.entity.vo.user_vo import AddUserModel, CurrentUserModel, ResetUserModel, TokenData, UserInfoModel
from module_admin.service.user_service import UserService
from utils.common_util import CamelCaseUtil
from utils.log_util import logger
from utils.message_util import message_service
from utils.pwd_util import PwdUtil

# bcrypt 最多 72 字节，登录前将密码截断避免 passlib 报错
def _truncate_password(s: str, max_bytes: int = 72) -> str:
    if not s or len(s.encode('utf-8')) <= max_bytes:
        return s
    return s.encode('utf-8')[:max_bytes].decode('utf-8', errors='ignore')

oauth2_scheme = OAuth2PasswordBearer(tokenUrl='login')

MENU_NAME_MAP ={
    "系统概览": "Dashboard",
    "设备监控": "Device Monitor",
    "设备维护": "Device Maintenance",
    "设备管理": "Device Management",
    "设备拓扑": "Device Topology",
    "干节点管理": "Dry Node Management",
    "北向配置": "Northbound Config",
    "系统配置": "System Configuration",
    "系统参数": "BMS Parameters",
    "电池配置": "Battery Configuration",
    "告警配置": "Alarm Configuration",
    "DI/DO配置": "DI/DO Configuration",
    "保护设置": "Protection Settings",
    "运行设置": "Operation Settings",
    "系统设置": "System Settings",
    "高级设置": "Advanced Settings",
    "历史数据": "History Data",
    "模板管理": "Template Management",
    "模板配置": "Template Management",
    "设备模板": "Device Templates",
    "北向模板": "Northbound Templates",
    "数据查询": "Data Query",
    "事件记录": "Event Logs",
    "告警记录": "Alarm Logs",
    "故障告警": "Fault Alarms",
    "本机配置": "Local Config",
    "端口配置": "Port Config",
    "对时配置": "Time Sync",
    "本机维护": "Local Maintenance",
    "用户管理": "User Management",
    "角色管理": "Role Management",
    "菜单管理": "Menu Management",
    "站点配置": "Site Config",
    "关于本机": "About Device",
    "系统管理": "System Admin",
    "部门管理": "Department",
    "岗位管理": "Position",
    "字典管理": "Dictionary",
    "参数设置": "Parameters",
    "通知公告": "Notices",
    "日志管理": "Log Management",
    "操作日志": "Operation Log",
    "登录日志": "Login Log",
    "系统监控": "System Monitor",
    "在线用户": "Online Users",
    "定时任务": "Scheduled Tasks",
    "服务监控": "Service Monitor",
    "缓存监控": "Cache Monitor",
    "缓存列表": "Cache List",
    "系统工具": "System Tools",
    "表单构建": "Form Builder",
    "代码生成": "Code Generator",
    "系统接口": "System API"
}


class CustomOAuth2PasswordRequestForm(OAuth2PasswordRequestForm):
    """
    自定义OAuth2PasswordRequestForm类，增加验证码及会话编号参数
    """

    def __init__(
        self,
        grant_type: str = Form(default=None, regex='password'),
        username: str = Form(),
        password: str = Form(),
        scope: str = Form(default=''),
        client_id: Optional[str] = Form(default=None),
        client_secret: Optional[str] = Form(default=None),
        code: Optional[str] = Form(default=''),
        uuid: Optional[str] = Form(default=''),
        login_info: Optional[Dict[str, str]] = Form(default=None),
    ):
        super().__init__(
            grant_type=grant_type,
            username=username,
            password=password,
            scope=scope,
            client_id=client_id,
            client_secret=client_secret,
        )
        self.code = code
        self.uuid = uuid
        self.login_info = login_info


class LoginService:
    """
    登录模块服务层
    """

    @classmethod
    async def authenticate_user(cls, request: Request, query_db: Session, login_user: UserLogin):
        """
        根据用户名密码校验用户登录

        :param request: Request对象
        :param query_db: orm对象
        :param login_user: 登录用户对象
        :return: 校验结果
        """
        await cls.__check_login_ip(request)
        # 判断请求是否来自于api文档，如果是返回指定格式的结果，用于修复api文档认证成功后token显示undefined的bug
        request_from_swagger = (
            request.headers.get('referer').endswith('docs') if request.headers.get('referer') else False
        )
        request_from_redoc = (
            request.headers.get('referer').endswith('redoc') if request.headers.get('referer') else False
        )
        # 判断是否开启验证码，开启则验证，否则不验证（dev模式下来自API文档的登录请求不检验）
        if not login_user.captcha_enabled or (
            (request_from_swagger or request_from_redoc) and AppConfig.app_env == 'dev'
        ):
            pass
        else:
            await cls.__check_login_captcha(request, login_user)
        user = login_by_account(query_db, login_user.user_name)
        if not user:
            logger.warning('用户不存在')
            raise LoginException(data='', message='User not found')
        pwd = _truncate_password(login_user.password)
        if not PwdUtil.verify_password(pwd, user[0].password):
            logger.warning('密码错误')
            raise LoginException(data='', message='Password incorrect')
        if user[0].status == '1':
            logger.warning('用户已停用')
            raise LoginException(data='', message='User has been disabled')
        return user

    @classmethod
    async def __check_login_ip(cls, request: Request):
        """
        校验用户登录ip是否在黑名单内

        :param request: Request对象
        :return: 校验结果
        """
        return True

    @classmethod
    async def __check_login_captcha(cls, request: Request, login_user: UserLogin):
        """
        校验用户登录验证码

        :param request: Request对象
        :param login_user: 登录用户对象
        :return: 校验结果
        """
        return True

    @classmethod
    async def create_access_token(cls, data: dict, expires_delta: Union[timedelta, None] = None):
        """
        根据登录信息创建当前用户token

        :param data: 登录信息
        :param expires_delta: token有效期
        :return: token
        """
        to_encode = data.copy()
        if expires_delta:
            expire = datetime.now(timezone.utc) + expires_delta
        else:
            expire = datetime.now(timezone.utc) + timedelta(minutes=30)
        to_encode.update({'exp': expire})
        encoded_jwt = jwt.encode(to_encode, JwtConfig.jwt_secret_key, algorithm=JwtConfig.jwt_algorithm)
        return encoded_jwt

    @classmethod
    async def get_current_user(
        cls, request: Request = Request, token: str = Depends(oauth2_scheme), query_db: Session = Depends(get_db)
    ):
        """
        根据token获取当前用户信息

        :param request: Request对象
        :param token: 用户token
        :param query_db: orm对象
        :return: 当前用户信息对象
        :raise: 令牌异常AuthException
        """
        # if token[:6] != 'Bearer':
        #     logger.warning("用户token不合法")
        #     raise AuthException(data="", message="用户token不合法")
        try:
            if token.startswith('Bearer'):
                token = token.split(' ')[1]
            payload = jwt.decode(token, JwtConfig.jwt_secret_key, algorithms=[JwtConfig.jwt_algorithm])
            user_id: str = payload.get('user_id')
            session_id: str = payload.get('session_id')
            if not user_id:
                logger.warning('用户token不合法')
                raise AuthException(data='', message='Invalid user token')
            token_data = TokenData(user_id=int(user_id))
        except InvalidTokenError:
            logger.warning('用户token已失效，请重新登录')
            raise AuthException(data='', message='User token has expired, please login again')
        query_user = await UserDao.get_user_by_id(query_db, user_id=token_data.user_id)
        if query_user.get('user_basic_info') is None:
            logger.warning('用户token不合法')
            raise AuthException(data='', message='Invalid user token')

        role_id_list = [item.role_id for item in query_user.get('user_role_info')]
        if 1 in role_id_list:
            permissions = ['*:*:*']
        else:
            permissions = [row.perms for row in query_user.get('user_menu_info')]
        post_ids = ','.join([str(row.post_id) for row in query_user.get('user_post_info')])
        role_ids = ','.join([str(row.role_id) for row in query_user.get('user_role_info')])
        roles = [row.role_key for row in query_user.get('user_role_info')]

        current_user = CurrentUserModel(
            permissions=permissions,
            roles=roles,
            user=UserInfoModel(
                **CamelCaseUtil.transform_result(query_user.get('user_basic_info')),
                postIds=post_ids,
                roleIds=role_ids,
                dept=CamelCaseUtil.transform_result(query_user.get('user_dept_info')),
                role=CamelCaseUtil.transform_result(query_user.get('user_role_info')),
            ),
        )
        return current_user

    @classmethod
    async def get_current_user_routers(cls, user_id: int, lang: str = 'cn', query_db: Session = Depends(get_db)):
        """
        根据用户id获取当前用户路由信息

        :param user_id: 用户id
        :param lang: 语言
        :param query_db: orm对象
        :return: 当前用户路由信息对象
        """
        query_user = await UserDao.get_user_by_id(query_db, user_id=user_id)
        # BMS Web: 排除日志管理及其子菜单（menu_id=108 为日志管理）
        _menu_info = query_user.get('user_menu_info')
        _log_menu_id = 108
        user_router_menu = sorted(
            [
                row
                for row in _menu_info
                if row.menu_type in [MenuConstant.TYPE_DIR, MenuConstant.TYPE_MENU]
                and row.menu_id != _log_menu_id
                and getattr(row, 'parent_id', 0) != _log_menu_id
            ],
            key=lambda x: x.order_num,
        )
        menus = cls.__generate_menus(0, user_router_menu)
        user_router = cls.__generate_user_router_menu(menus, lang)
        return [router.model_dump(exclude_unset=True, by_alias=True) for router in user_router]

    @classmethod
    def __generate_menus(cls, pid: int, permission_list: List[SysMenu]):
        """
        工具方法：根据菜单信息生成菜单信息树形嵌套数据

        :param pid: 菜单id
        :param permission_list: 菜单列表信息
        :return: 菜单信息树形嵌套数据
        """
        menu_list: List[MenuTreeModel] = []
        for permission in permission_list:
            if permission.parent_id == pid:
                children = cls.__generate_menus(permission.menu_id, permission_list)
                menu_list_data = MenuTreeModel(**CamelCaseUtil.transform_result(permission))
                if children:
                    menu_list_data.children = children
                menu_list.append(menu_list_data)

        return menu_list

    @classmethod
    def __get_menu_title(cls, permission: MenuTreeModel, lang: str = 'cn') -> str:
        """
        获取菜单在不同语言下的显示标题。

        - 中文(lang='cn')：直接使用数据库中的 menu_name
        - 英文(lang='en')：
          1. 优先使用 MENU_NAME_MAP 中的中英映射
          2. 若未匹配到，则根据组件路径/路由路径对关键菜单进行英文兜底
          3. 仍未命中时，回退为原始 menu_name
        """
        if lang == 'cn':
            return permission.menu_name

        # 一级「系统配置」目录（M）与「系统参数」子页（C）英文区分
        if permission.menu_name == '系统配置' and getattr(permission, 'menu_type', '') == 'M':
            return 'System Configuration'
        if permission.menu_name == '系统参数':
            return MENU_NAME_MAP.get('系统参数', 'BMS Parameters')

        # 1. 先按中文名做标准映射
        if permission.menu_name in MENU_NAME_MAP:
            return MENU_NAME_MAP[permission.menu_name]

        # 2. 再按组件 / 路径做关键菜单兜底（避免数据库里名字有微调时映射失效）
        component = getattr(permission, 'component', '') or ''
        path = getattr(permission, 'path', '') or ''

        # 系统配置子页（仅用 component 区分，避免与历史数据等子 path 同名冲突）
        if component.endswith('battery/config/system/index'):
            return 'BMS Parameters'
        if component.endswith('battery/config/battery/index'):
            return 'Battery Configuration'
        if component.endswith('battery/config/alarm/index'):
            return 'Alarm Configuration'
        if component.endswith('battery/config/dido/index'):
            return 'DI/DO Configuration'

        # 电池配置（旧单页 / 兼容）
        if component.endswith('battery/config/index') or path == 'battery-config':
            return 'Battery Configuration'

        # 历史数据 - 数据查询
        if component.endswith('history/timed/index') or path in ('timed', 'query'):
            return 'Data Query'

        # 历史数据 - 事件记录
        if component.endswith('history/event/index') or path == 'event':
            return 'Event Logs'

        # 历史数据 - 告警记录（原故障告警/faults/alarm）
        if component.endswith('history/alarm/index') or path in ('faults', 'alarm'):
            return 'Alarm Logs'

        # 3. 其他菜单保持原样
        return permission.menu_name

    @classmethod
    def __generate_user_router_menu(cls, permission_list: List[MenuTreeModel], lang: str = 'cn'):
        """
        工具方法：根据菜单树信息生成路由信息树形嵌套数据

        :param permission_list: 菜单树列表信息
        :return: 路由信息树形嵌套数据
        """
        router_list: List[RouterModel] = []
        for permission in permission_list:
            router = RouterModel(
                hidden=True if permission.visible == '1' else False,
                name=RouterUtil.get_router_name(permission),
                path=RouterUtil.get_router_path(permission),
                component=RouterUtil.get_component(permission),
                query=permission.query,
                meta=MetaModel(
                    title=cls.__get_menu_title(permission, lang),
                    icon=permission.icon,
                    noCache=True if permission.is_cache == 1 else False,
                    link=permission.path if RouterUtil.is_http(permission.path) else None,
                ),
            )
            c_menus = permission.children
            if c_menus and permission.menu_type == MenuConstant.TYPE_DIR:
                router.always_show = True
                router.redirect = 'noRedirect'
                router.children = cls.__generate_user_router_menu(c_menus, lang)
            elif RouterUtil.is_menu_frame(permission):
                router.meta = None
                children_list: List[RouterModel] = []
                children = RouterModel(
                    path=permission.path,
                    component=permission.component,
                    name=RouterUtil.get_route_name(permission.route_name, permission.path),
                    meta=MetaModel(
                        title=cls.__get_menu_title(permission, lang),
                        icon=permission.icon,
                        noCache=True if permission.is_cache == 1 else False,
                        link=permission.path if RouterUtil.is_http(permission.path) else None,
                    ),
                    query=permission.query,
                )
                children_list.append(children)
                router.children = children_list
            elif permission.parent_id == 0 and RouterUtil.is_inner_link(permission):
                router.meta = MetaModel(title=cls.__get_menu_title(permission, lang), icon=permission.icon)
                router.path = '/'
                children_list: List[RouterModel] = []
                router_path = RouterUtil.inner_link_replace_each(permission.path)
                children = RouterModel(
                    path=router_path,
                    component=MenuConstant.INNER_LINK,
                    name=RouterUtil.get_route_name(permission.route_name, permission.path),
                    meta=MetaModel(
                        title=permission.menu_name if lang == 'cn' else MENU_NAME_MAP.get(permission.menu_name, permission.menu_name),
                        icon=permission.icon,
                        link=permission.path if RouterUtil.is_http(permission.path) else None,
                    ),
                )
                children_list.append(children)
                router.children = children_list

            router_list.append(router)

        # 统一后处理：基于路由的 meta.title 再做一次精确过滤和重命名
        cls.__post_process_router_menu(router_list, lang)
        return router_list

    @classmethod
    def __post_process_router_menu(cls, routers: List[RouterModel], lang: str = 'cn') -> None:
        """
        在路由层面对菜单做一次后处理，避免依赖数据库中的原始名称：

        - 删除“干节点管理”菜单
        - 将“设备拓扑”统一显示为“拓扑配置”（仅中文）
        - “设备维护”一级菜单下，仅保留“设备管理”和“拓扑配置”
        - “本机配置”一级菜单下，隐藏“角色管理 / 菜单管理 / 站点配置”等系统类页面
        """
        if not routers:
            return

        def _filter_children(
            children: List[RouterModel],
            is_device_maintenance: bool,
            is_local_config: bool,
            is_history_data: bool,
        ) -> List[RouterModel]:
            filtered: List[RouterModel] = []
            for child in children:
                meta = child.meta
                title = getattr(meta, "title", "") if meta else ""
                component = getattr(child, "component", "") or ""

                # 1) 全局删除“干节点管理”相关菜单
                if "干节点" in title:
                    continue

                # 2) “历史数据”下不允许挂“设备模板”
                if is_history_data and component.endswith("template/manage/index"):
                    continue

                # 2) 统一重命名“设备拓扑”为“拓扑配置”（中文）/“Topology Configuration”（英文）
                if meta and title in ("设备拓扑", "拓扑配置"):
                    if lang == "cn":
                        meta.title = "拓扑配置"
                    else:
                        # 其它语言统一使用英文标题
                        meta.title = "Topology Configuration"
                    title = meta.title

                # 3) 如果是“设备维护”下的二级菜单，仅保留“设备管理 / 拓扑配置”
                if is_device_maintenance:
                    # 兼容：允许添加“模板管理”（template/manage/index）
                    if title in ("设备管理", "拓扑配置"):
                        filtered.append(child)
                    elif component.endswith("template/manage/index"):
                        # 由于数据库 menu_name 可能出现编码差异，这里统一覆盖显示标题
                        if lang == "cn":
                            # 统一中文展示，兼容“模板配置/模板管理”叫法
                            child.meta.title = "模板管理"
                        else:
                            child.meta.title = "Template Management"
                        filtered.append(child)
                # 4) 如果是“本机配置”下的二级菜单，隐藏角色/菜单/站点配置
                elif is_local_config:
                    # 同时兼容中英文标题
                    if title in ("角色管理", "菜单管理", "站点配置", "Role Management", "Menu Management", "Station Config"):
                        continue
                    filtered.append(child)
                else:
                    filtered.append(child)

                # 递归处理子节点
                if getattr(child, "children", None):
                    child.children = _filter_children(
                        child.children,
                        is_device_maintenance=False,
                        is_local_config=False,
                        is_history_data=False,
                    )

            return filtered

        for router in routers:
            meta = router.meta
            title = getattr(meta, "title", "") if meta else ""
            is_device_maintenance = (title == "设备维护")
            is_local_config = (title == "本机配置")
            is_history_data = title in ("历史数据", "History Data")
            if getattr(router, "children", None):
                router.children = _filter_children(
                    router.children,
                    is_device_maintenance,
                    is_local_config,
                    is_history_data,
                )

    @classmethod
    async def register_user_services(cls, request: Request, query_db: Session, user_register: UserRegister):
        """
        用户注册services

        :param request: Request对象
        :param query_db: orm对象
        :param user_register: 注册用户对象
        :return: 注册结果
        """
        if user_register.password == user_register.confirm_password:
            add_user = AddUserModel(
                userName=user_register.username,
                nickName=user_register.username,
                password=PwdUtil.get_password_hash(user_register.password),
            )
            result = await UserService.add_user_services(query_db, add_user)
            return result
        else:
            raise ServiceException(message='Passwords do not match')

    @classmethod
    async def get_sms_code_services(cls, request: Request, query_db: Session, user: ResetUserModel):
        """
        获取短信验证码service

        :param request: Request对象
        :param query_db: orm对象
        :param user: 用户对象
        :return: 短信验证码对象
        """
        is_user = await UserDao.get_user_by_name(query_db, user.user_name)
        if is_user:
            sms_code = str(random.randint(100000, 999999))
            session_id = str(uuid.uuid4())
            # 此处模拟调用短信服务
            message_service(sms_code)

            return SmsCode(**dict(is_success=True, sms_code=sms_code, session_id=session_id, message='Retrieved successfully'))

        return SmsCode(**dict(is_success=False, sms_code='', session_id='', message='User not found'))

    @classmethod
    async def forget_user_services(cls, request: Request, query_db: Session, forget_user: ResetUserModel):
        """
        用户忘记密码services

        :param request: Request对象
        :param query_db: orm对象
        :param forget_user: 重置用户对象
        :return: 重置结果
        """
        forget_user.password = PwdUtil.get_password_hash(forget_user.password)
        forget_user.user_id = (UserDao.get_user_by_name(query_db, forget_user.user_name)).user_id
        edit_result = UserService.reset_user_services(query_db, forget_user)
        result = edit_result.dict()

        return CrudResponseModel(**result)

    @classmethod
    async def logout_services(cls, request: Request, session_id: str):
        """
        退出登录services

        :param request: Request对象
        :param session_id: 会话编号
        :return: 退出登录结果
        """
        return True


class RouterUtil:
    """
    路由处理工具类
    """

    @classmethod
    def get_router_name(cls, menu: MenuTreeModel):
        """
        获取路由名称

        :param menu: 菜单数对象
        :return: 路由名称
        """
        # 非外链并且是一级目录（类型为目录）
        if cls.is_menu_frame(menu):
            return ''

        return cls.get_route_name(menu.route_name, menu.path)

    @classmethod
    def get_route_name(cls, name: str, path: str):
        """
        获取路由名称，如没有配置路由名称则取路由地址

        :param name: 路由名称
        :param path: 路由地址
        :return: 路由名称（驼峰格式）
        """
        router_name = name if name else path
        return router_name.capitalize()

    @classmethod
    def get_router_path(cls, menu: MenuTreeModel):
        """
        获取路由地址

        :param menu: 菜单数对象
        :return: 路由地址
        """
        # 内链打开外网方式
        router_path = menu.path
        if menu.parent_id != 0 and cls.is_inner_link(menu):
            router_path = cls.inner_link_replace_each(router_path)
        # 非外链并且是一级目录（类型为目录）
        if menu.parent_id == 0 and menu.menu_type == MenuConstant.TYPE_DIR and menu.is_frame == MenuConstant.NO_FRAME:
            router_path = f'/{menu.path}'
        # 非外链并且是一级目录（类型为菜单）
        elif cls.is_menu_frame(menu):
            router_path = '/'
        return router_path

    @classmethod
    def get_component(cls, menu: MenuTreeModel):
        """
        获取组件信息

        :param menu: 菜单数对象
        :return: 组件信息
        """
        component = MenuConstant.LAYOUT
        if menu.component and not cls.is_menu_frame(menu):
            component = menu.component
        elif (menu.component is None or menu.component == '') and menu.parent_id != 0 and cls.is_inner_link(menu):
            component = MenuConstant.INNER_LINK
        elif (menu.component is None or menu.component == '') and cls.is_parent_view(menu):
            component = MenuConstant.PARENT_VIEW
        # 一级「系统配置」若误配 ParentView，整页只有 router-view 无 Layout 外壳（顶栏/AppMain）
        if (
            getattr(menu, 'parent_id', None) == 0
            and component == MenuConstant.PARENT_VIEW
            and cls._router_first_segment(menu.path) == 'system-config'
        ):
            component = MenuConstant.LAYOUT
        return component

    @staticmethod
    def _router_first_segment(path: Optional[str]) -> str:
        if not path:
            return ''
        s = str(path).strip().replace('\\', '/').lstrip('/')
        return (s.split('/')[0] or '').lower()

    @classmethod
    def is_menu_frame(cls, menu: MenuTreeModel):
        """
        判断是否为菜单内部跳转

        :param menu: 菜单数对象
        :return: 是否为菜单内部跳转
        """
        return (
            menu.parent_id == 0 and menu.menu_type == MenuConstant.TYPE_MENU and menu.is_frame == MenuConstant.NO_FRAME
        )

    @classmethod
    def is_inner_link(cls, menu: MenuTreeModel):
        """
        判断是否为内链组件

        :param menu: 菜单数对象
        :return: 是否为内链组件
        """
        return menu.is_frame == MenuConstant.NO_FRAME and cls.is_http(menu.path)

    @classmethod
    def is_parent_view(cls, menu: MenuTreeModel):
        """
        判断是否为parent_view组件

        :param menu: 菜单数对象
        :return: 是否为parent_view组件
        """
        return menu.parent_id != 0 and menu.menu_type == MenuConstant.TYPE_DIR

    @classmethod
    def is_http(cls, link: str):
        """
        判断是否为http(s)://开头

        :param link: 链接
        :return: 是否为http(s)://开头
        """
        return link.startswith(CommonConstant.HTTP) or link.startswith(CommonConstant.HTTPS)

    @classmethod
    def inner_link_replace_each(cls, path: str):
        """
        内链域名特殊字符替换

        :param path: 内链域名
        :return: 替换后的内链域名
        """
        old_values = [CommonConstant.HTTP, CommonConstant.HTTPS, CommonConstant.WWW, '.', ':']
        new_values = ['', '', '', '/', '/']
        for old, new in zip(old_values, new_values):
            path = path.replace(old, new)
        return path
