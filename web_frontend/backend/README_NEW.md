# WEB 系统交付文档

## 项目简介

本项目是一个基于 FastAPI 框架 + Vue2 开发的管理系统，提供系统管理和设备管理两大核心功能模块。系统采用 SQLite 数据库存储。

## 功能模块说明

### 一、系统管理模块 (module_admin)

#### 1. 登录认证模块 (Login Module)

- **功能描述**：提供用户登录、登出、Token 认证等功能
- **主要接口**：`/login`、`/logout`、`/getInfo`、`/getRouters`
- **特性**：支持 JWT Token 认证

#### 2. 用户管理 (System Management - User Management)

- **功能描述**：系统用户的增删改查、角色分配、密码重置等
- **主要功能**：
  - 用户列表查询（支持分页、条件筛选）
  - 用户新增、编辑、删除
  - 用户角色分配
  - 用户密码重置
  - 用户状态管理

#### 3. 角色管理 (System Management - Role Management)

- **功能描述**：系统角色的权限配置和管理
- **主要功能**：
  - 角色列表查询
  - 角色新增、编辑、删除
  - 角色权限分配
  - 角色数据权限配置

#### 4. 菜单管理 (System Management - Menu Management)

- **功能描述**：系统菜单和路由的配置管理
- **主要功能**：
  - 菜单树形结构查询
  - 菜单新增、编辑、删除
  - 菜单权限配置

#### 5. 服务器监控 (System Monitoring - Server Management)

- **功能描述**：服务器性能指标监控
- **主要功能**：
  - CPU、内存、磁盘使用率监控
  - 系统信息查询

### 二、设备管理模块 (module_device)

#### 1. 模板管理 (Template Management)

- **功能描述**：设备模板的配置和管理
- **主要功能**：
  - 设备模板列表查询
  - 模板新增、编辑、删除
  - 模板点位配置

#### 2. 设备管理 (Device Management)

- **功能描述**：设备的全生命周期管理
- **主要功能**：
  - 设备列表查询（支持分页、条件筛选）
  - 设备新增、编辑、删除
  - 设备状态监控
  - 设备参数配置

#### 3. 端口管理 (Port Management)

- **功能描述**：设备通信端口的配置和管理
- **主要功能**：
  - 端口列表查询
  - 端口新增、编辑、删除
  - 端口参数配置

#### 4. 设备拓扑 (Device Topology)

- **功能描述**：设备拓扑结构的可视化管理
- **主要功能**：
  - 拓扑结构查询
  - 拓扑关系配置
  - 拓扑图展示

#### 5. 干节点管理 (Dry Node Management)

- **功能描述**：干节点（DI/DO）的配置和管理
- **主要功能**：
  - 干节点列表查询
  - 干节点逻辑配置
  - 干节点状态监控

#### 6. 北向配置管理 (Northbound Configuration Management)

- **功能描述**：北向接口的配置和管理，该功能需要重新设计，此处代码为之前留下的代码逻辑
- **主要功能**：
  - 北向配置列表查询
  - 北向配置新增、编辑、删除
  - 北向接口参数配置

#### 7. 实时数据库 (Time Series Database)

- **功能描述**：时序数据的存储和查询管理，主要用到的是 rtdb_service 方法，controller 中的 api 方法仅用于测试
- **主要功能**：
  - RTDB 初始化
  - 时序数据查询
  - 数据点值设置
  - 实时数据监控

#### 8. 控制策略管理 (Control Strategy Management)

- **功能描述**：设备控制策略的配置和管理
- **主要功能**：
  - 控制策略列表查询
  - 策略新增、编辑、删除
  - 策略执行管理

#### 9. 历史数据管理 (History Data Management)

- **功能描述**：历史数据的存储、查询和统计
- **主要功能**：
  - 历史数据查询（支持时间范围筛选）
  - 历史数据统计
  - 数据导出功能

#### 10. 概览管理 (Overview Management)

- **功能描述**：系统首页整体概览和统计信息
- **主要功能**：
  - 设备状态概览
  - 数据统计概览
  - 告警信息概览

#### 11. 本地维护管理 (Local Maintenance Management)

- **功能描述**：本地系统维护和更新管理
- **主要功能**：
  - 系统版本管理
  - 本地更新管理
  - 维护操作记录

## 技术栈

- **框架**：FastAPI 0.115.0
- **数据库**：SQLite（支持多数据库：web.db、sqlite3.db、business.db）
- **ORM**：SQLAlchemy 2.0.31 (异步)
- **认证**：JWT (PyJWT)
- **密码加密**：bcrypt (passlib)
- **日志**：loguru
- **Web 服务器**：Gunicorn + Uvicorn
- **反向代理**：Nginx

## 项目结构

```
web-backend/
├── app.py                      # 应用启动入口
├── server.py                   # FastAPI 应用主文件
├── requirements.txt            # Python 依赖包
├── gunicorn.conf.py            # Gunicorn 配置文件
├── config/                     # 配置文件目录
│   ├── env.py                  # 环境配置
│   ├── database.py             # 数据库配置
│   ├── constant.py             # 常量定义
│   └── get_db.py               # 数据库连接
├── module_admin/               # 系统管理模块
│   ├── controller/             # 控制器层
│   ├── service/                # 服务层
│   ├── dao/                    # 数据访问层
│   └── entity/                 # 实体定义
├── module_device/              # 设备管理模块
│   ├── controller/             # 控制器层
│   ├── service/                # 服务层
│   ├── dao/                    # 数据访问层
│   └── entity/                 # 实体定义
├── utils/                      # 工具类
├── middlewares/                # 中间件
├── exceptions/                 # 异常处理
├── logs/                       # 日志目录
├── vf_admin/                   # 文件上传下载目录
│   ├── upload_path/            # 上传文件存储
│   └── download_path/          # 下载文件存储
└── sql/                        # SQL 脚本目录
```

## 环境要求

- **Python**：3.7+
- **操作系统**：Linux (Ubuntu/Debian) / Windows
- **Web 服务器**：Nginx (生产环境推荐)
- **数据库**：SQLite（已内置，无需额外安装）

## 部署指南

### 一、基础环境检查

先更新系统并检查 Python、pip、Nginx 是否安装：

```bash
# 查看 Python
python3 --version
# 如果没有，安装：
sudo apt install python3 python3-venv python3-pip -y

# 查看 pip
pip3 --version

# 查看 nginx
nginx -v
# 如果没有，安装：
sudo apt install nginx -y
```

### 二、部署后端

#### (1) 代码放置

后端代码放到 `/opt/con/web/` 或 其他自定义路径

```bash
# 创建目录
sudo mkdir -p /opt/con/web

# 将代码复制到目标目录
sudo cp -r /path/to/web-backend/* /opt/con/web/
```

#### (2) 创建虚拟环境 & 安装依赖

```bash
# 进入项目目录
cd /opt/con/web

# 创建虚拟环境
python3 -m venv venv
source venv/bin/activate

# 升级 pip
pip install --upgrade pip

# 安装项目依赖 (requirements.txt)
pip install -r requirements.txt

# 建议安装 gunicorn + uvicorn
pip install gunicorn uvicorn
```

#### (3) 配置环境变量

创建环境配置文件 `.env.prod`（生产环境）或 `.env.dev`（开发环境）：
确定配置是否正确，配置文件中的数据库路径是否与实际系统中的数据库文件一致：

```env
# 应用配置
APP_ENV=prod
APP_NAME = 'SCU-FastAPI'
APP_ROOT_PATH=/prod-api
APP_HOST=0.0.0.0
APP_PORT=8000

# 数据库配置
DB_TYPE=sqlite
DB_DATABASE=web.db
DB_ECHO=False
DB_URL_WEB=sqlite:////opt/con/db/web.db
DB_URL_SCU=sqlite:////opt/con/db/sqlite3.db
DB_URL_BUSINESS=sqlite:////opt/con/db/business.db
```

#### (4) 启动测试

```bash
# 激活虚拟环境
source venv/bin/activate

# 测试启动
python app.py --env=prod
# 或
python app.py --env=dev
```

确认接口能在 `http://<服务器IP>:8000/docs` 打开。

#### (5) 使用 Conda 环境（可选）

如果使用 Conda 环境：

```bash
# 创建迁移目录
mkdir migrate
tar -zxf migrate.tar.gz -C migrate/

# 激活环境
source migrate/bin/activate
conda unpack

# 切换到后端路径
cd /opt/con/web
# 或
cd /home/work/scu/code/

# 启动应用
python app.py --env=prod

# 或后台运行
nohup python app.py --env=prod > ems.log 2>&1 &
```

生产环境也可以选择使用 Gunicorn / systemd 管理进程

### 三、部署前端

#### (1) 构建前端

在本地或服务器构建 Vue2：

```bash
cd web-frontend   # 前端项目根路径
npm install
npm run build
```

构建完成后，会生成一个 `dist/` 文件夹。

#### (2) 放置前端静态文件

需要将打包好的前端 dist 文件夹放置到 Nginx 配置中的对应路径下。

```bash
sudo mkdir -p /usr/share/nginx/html/dist-g
sudo cp -r dist/* /usr/share/nginx/html/dist-g/

```

**注意**：上面的路径是示例，需要根据实际 Nginx 配置中的 `root` 路径来放置文件。

#### (3) 前端开发说明

**项目名称**: SCU Management System  
**版本**: 1.5.1  
**技术栈**: Vue 2.6.12 + Element UI 2.15.14 + Vuex + Vue Router + Axios

**核心依赖**:

- Vue 2.6.12
- Element UI 2.15.14
- Vuex 3.6.0（状态管理）
- Vue Router 3.4.9（路由管理）
- Axios 0.28.1（HTTP 客户端）
- ECharts 5.4.0（图表库）

**目录结构**:

```
web-frontend/
├── src/
│   ├── api/               # API 接口定义
│   ├── assets/            # 静态资源（图标、图片、样式）
│   ├── components/        # 公共组件
│   ├── layout/            # 布局组件
│   ├── router/            # 路由配置
│   ├── store/             # Vuex 状态管理
│   ├── utils/             # 工具函数（request.js、auth.js 等）
│   └── views/             # 页面组件
│       ├── login.vue      # 登录页
│       ├── dashboard/     # 仪表盘
│       ├── system/        # 系统管理（用户、角色、菜单等）
│       ├── device/        # 设备管理
│       ├── monitor/       # 监控管理（在线用户、日志等）
│       ├── control/       # 控制管理
│       └── history/       # 历史记录
├── vue.config.js          # Vue CLI 配置
└── package.json           # 项目依赖
```

**开发环境搭建**:

```bash
# 1. 环境要求：Node.js >= 8.9, npm >= 3.0.0（推荐 Node.js 14.x+）

# 2. 安装依赖
cd web-frontend
npm install

# 3. 启动开发服务器
npm run dev
# 默认访问地址：http://localhost:80

# 4. 生产环境打包
npm run build:prod
# 打包输出目录：dist/
```

**主要功能模块**:

- **系统管理** (`src/views/system/`): 用户管理、角色管理、菜单管理、字典管理、参数配置等
- **设备管理** (`src/views/device/`): 设备列表、设备信息、设备监控、设备拓扑、北向接口等
- **监控管理** (`src/views/monitor/`): 在线用户、定时任务、操作日志、登录日志等
- **控制管理** (`src/views/control/`): 运行控制、保护控制、系统控制、高级控制
- **历史记录** (`src/views/history/`): 定时历史、事件历史、故障历史
- **仪表盘** (`src/views/dashboard/`): 数据可视化展示（使用 ECharts）

**核心功能**:

- **路由管理** (`src/router/`): 静态路由 + 基于权限的动态路由
- **状态管理** (`src/store/`): 用户信息、权限、应用状态等
- **请求封装** (`src/utils/request.js`): Axios 封装，自动添加 Token、统一错误处理
- **权限控制** (`src/permission.js`): 路由级和按钮级权限控制

**API 配置**:

- API 代理地址：`http://127.0.0.1:9099`
- API 基础路径：通过 `process.env.VUE_APP_BASE_API` 配置（通常为 `/dev-api/`）
- 所有 API 接口定义在 `src/api/` 目录下

### 四、Nginx 配置

#### (1) 找到生效的 Nginx 配置文件

Nginx 配置文件通常位于以下位置之一：

```bash
# 主配置文件（通常在这里）
/etc/nginx/nginx.conf

# 或者包含的配置文件
/etc/nginx/conf.d/*.conf

# 或者站点配置（如果使用）
/etc/nginx/sites-enabled/*
```

**查找方法**：

```bash
# 查看主配置文件位置
sudo nginx -t

# 查看当前生效的配置
sudo nginx -T | grep -E "server_name|root|location"
```

#### (2) 修改 Nginx 配置

编辑找到的配置文件（通常是 `/etc/nginx/nginx.conf` 或在 `http` 块中的 `server` 配置）：

```bash
sudo nano /etc/nginx/nginx.conf
# 或
sudo vim /etc/nginx/nginx.conf
```

在 `http` 块中找到或添加 `server` 配置块，参考以下配置：

```nginx
server {
    listen 80;
    server_name your_domain.com;  # 替换为你的域名或服务器IP

    # 配置 Vue 前端
    location / {
        try_files $uri $uri/ /index.html;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header Host $http_host;
        root /usr/share/nginx/html/dist-g;  # 根据实际前端文件路径修改
        index index.html index.htm;
    }

    # 配置 FastAPI 后端 API 反向代理
    location /dev-api/ {  # 根据前端实际使用的 API 路径前缀修改
        proxy_pass http://127.0.0.1:9099/;  # 替换为实际的后端服务地址和端口
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
        proxy_ssl_server_name on;
    }
}
```

**配置说明**：

- `root`：前端静态文件的实际存放路径，需要与步骤三中放置文件的路径一致
- `location /dev-api/`：后端 API 的路径前缀，需要与前端代码中的 API 基础路径保持一致
- `proxy_pass`：后端服务的实际地址和端口，根据你的后端服务配置修改（如 `http://127.0.0.1:8000/` 或 `http://127.0.0.1:9099/`）

#### (3) 检查配置并重启 Nginx

```bash
# 检查 Nginx 配置语法是否正确
sudo nginx -t

# 如果语法检查通过，重启 Nginx 使配置生效
sudo service nginx restart

# 或者重新加载配置（不中断服务）
sudo service nginx reload

# 查看 Nginx 状态
sudo service nginx status
```

**注意事项**：

- 修改配置前建议先备份原配置文件
- 确保后端服务已启动并监听在配置的端口上
- 如果使用防火墙，确保相应端口已开放

### 五、常见启动问题

````
### 1. 端口被占用
如果 8000 端口被占用，可以修改配置文件或使用其他端口：
```bash
# 查看端口占用
sudo netstat -tlnp | grep 8000
# 修改 .env.prod 中的 APP_PORT（需要修改nginx,不建议）
````
