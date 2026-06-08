-- 主站通讯协议类型 protocol_type（与 C protocol_type_master_e / kit_data.h 一致）：
-- 0本机信息 1BBMS私有 2Modbus TCP 3Modbus RTU 4RBMS TCP 5RBMS CAN 6ModbusMulti 7TMS CAN（8=kProto_Master_End 禁止入库）
-- 与前端 protocolTypes.js、后端 ProtocolTypeMap 保持一致

-- 电站表
CREATE TABLE IF NOT EXISTS 
station(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    name TEXT(100) NOT NULL, --电站名称
    address TEXT(200) NOT NULL, --电站地址
    lng REAL NOT NULL, --经度
    lat REAL NOT NULL, --纬度
    time_zone TEXT(50) NOT NULL, --时区
    remark TEXT(1200), --备注
    UNIQUE(name) -- 电站名称唯一
);

-- -- 模板表
CREATE TABLE IF NOT EXISTS  
template(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    type TINYINT NOT NULL, --模板类别（0设备配置, 1北向配置）
    protocol_type INTEGER NOT NULL, --协议类型，见文件首注释
    dev_type INTEGER, --设备类型
    name TEXT(100) NOT NULL, --名称
    remark TEXT(1200), --备注
    UNIQUE(name) --模板名称唯一
); 

-- -- 模板点位表
CREATE TABLE IF NOT EXISTS
template_point(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    template_id INTEGER, --模版Id（未分配为NULL）
    protocol_type INTEGER NOT NULL, --协议类型，见文件首注释
    dev_type INTEGER NOT NULL, --设备类型
    group_type INTEGER NOT NULL, --分组类型
    code INTEGER NOT NULL, --点位编号
    name TEXT(100) NOT NULL, --点位名称
    ename TEXT(100) NOT NULL, --点位英文名称
    data_type INTEGER NOT NULL, --数据类型
    attribute INTEGER NOT NULL, --点位属性
    func INTEGER NOT NULL, --点位功能
    reg_addr INTEGER NOT NULL, --点位地址
    bit_pos INTEGER NOT NULL, --字节开始位
    bit_num INTEGER NOT NULL, --字节数
    endian INTEGER NOT NULL, --字节顺序
    precision REAL NOT NULL, --精度
    ratio REAL NOT NULL, --系数
    offset REAL NOT NULL, --偏移量
    is_persisted TINYINT NOT NULL, --是否持久化
    storage_interval INTEGER NOT NULL, --存储周期
    mutate_bound REAL, --突变范围
    default_value REAL, --默认值
    min_value REAL, --最小值
    max_value REAL,  --最大值
    unit TEXT(20), --计量单位
    is_show TINYINT NOT NULL --是否展示
);

-- 设备表
CREATE TABLE IF NOT EXISTS
device(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    sn TEXT(50) NOT NULL, --设备sn
    type INTEGER NOT NULL, --设备类型
    code INTEGER NOT NULL, --设备编号
    name TEXT(100) NOT NULL, --设备名称
    protocol_type INTEGER NOT NULL, --协议类型，见文件首注释
    template_id INTEGER NOT NULL, --模版Id
    sort_order INTEGER, --序号
    remark TEXT(1200), --备注
    status TINYINT, --状态
    UNIQUE(name), -- 设备名称唯一
    UNIQUE(type, code) -- 设备类型和设备编号唯一
);

-- 设备/北向配置协议表
CREATE TABLE IF NOT EXISTS 
protocol(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    related_id INTEGER NOT NULL, --关联Id
    template_type TINYINT NOT NULL, --模板类别（0设备配置, 1北向配置）
    type INTEGER NOT NULL, --协议类型，见文件首注释
    name TEXT(100) NOT NULL, --协议名称
    content TEXT(1200) NOT NULL, --协议内容
    remark TEXT(1200), --备注
    UNIQUE(name) -- 协议名称唯一
);

-- 点位表
CREATE TABLE IF NOT EXISTS 
point(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    template_id INTEGER NOT NULL, --模版Id
    protocol_type INTEGER NOT NULL, --协议类型，见文件首注释
    dev_type INTEGER NOT NULL, --设备类型
    group_type INTEGER NOT NULL, --分组类型
    code INTEGER NOT NULL, --点位编号
    name TEXT(100) NOT NULL, --点位名称
    ename TEXT(100) NOT NULL, --点位英文名称
    data_type INTEGER NOT NULL, --数据类型
    attribute INTEGER NOT NULL, --点位属性
    func INTEGER NOT NULL, --点位功能
    reg_addr INTEGER NOT NULL, --点位地址
    bit_pos INTEGER NOT NULL, --字节开始位
    bit_num INTEGER NOT NULL, --字节数
    endian INTEGER NOT NULL, --字节顺序
    precision REAL NOT NULL, --精度
    ratio REAL NOT NULL, --系数
    offset REAL NOT NULL, --偏移量
    is_persisted TINYINT NOT NULL, --是否持久化
    storage_interval INTEGER NOT NULL, --存储周期
    mutate_bound REAL, --突变范围
    default_value REAL, --默认值
    min_value REAL, --最小值
    max_value REAL, --最大值
    unit TEXT(20), --计量单位
    is_show TINYINT NOT NULL --是否展示
);

-- 数据监视器
CREATE TABLE IF NOT EXISTS data_monitor (
    id INTEGER PRIMARY KEY AUTOINCREMENT,  -- 主键标识
    dev_type INTEGER NOT NULL,             -- 设备类型
    dev_id INTEGER NOT NULL,               -- 设备Id
    point_id INTEGER NOT NULL,             -- 点位Id
    interval INTEGER NOT NULL,             -- 处理周期
    level TINYINT NOT NULL,                -- 处理等级
    handle_method TINYINT NOT NULL         -- 处理方式
);

-- 拓扑结构表
CREATE TABLE IF NOT EXISTS
topology(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    parent_id INTEGER, --父Id 
    menu_tree TINYINT NOT NULL, --类型:0-虚拟节点，1-设备节点
    name TEXT(100), --层级名称
    dev_id INTEGER, --设备Id
    virtual_dev_type INTEGER, --虚拟节点设备类型(menu_tree=0)
    sort_order INTEGER NOT NULL, --序号
    remark TEXT(1200) --备注
);

-- 设备配置表
CREATE TABLE IF NOT EXISTS
ems_device_config(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    type INTEGER NOT NULL, --配置类型
    name TEXT(100) NOT NULL, --配置名称
    content TEXT(1200) NOT NULL, --配置内容
    UNIQUE(name) -- 配置名称唯一
);



-- bms配置表
-- 配置内容包括但不限于：电池组类型、电池组型号、电池组数量、簇控并机模式
-- 最小在网簇数、系统下电控制配置、电允许并网压差、脱扣脉冲时间、分闸断开延时、告警灯闪烁间隔
CREATE TABLE IF NOT EXISTS
bms_sys_config(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    type INTEGER NOT NULL, --配置类型 -- 0: 系统配置
    name TEXT(100) NOT NULL, --配置名称
    content TEXT(1200) NOT NULL, --配置内容
    UNIQUE(name) -- 配置名称唯一
);
-- 其中配置内容json格式示例：
-- {
--     "bat_type": "1",
--     "bat_model": "CATL-280AH",
--     "bat_group_num": "1",
--     "cluster_control_mode": "1",
--     "min_online_cluster_num": "1",
--     "sys_shutdown_word": "1",
--     "allow_grid_volt_diff": "1",
--     "detach_pulse_time": "1",
--     "cut_delay_time": "1"
--     "alert_led_time_interval": "1"
-- }


-- bms告警配置表
CREATE TABLE IF NOT EXISTS
bms_alert_config(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    name TEXT(100) NOT NULL, --配置名称
    alert_type INTEGER NOT NULL, --告警类型 -- 0: 簇间压差配置 1: 簇间环流配置 2: 簇间电流差配置 3: 主从通讯配置
    alert_trigger_threshold REAL NOT NULL, -- 告警阈值
    alert_trigger_time INTEGER NOT NULL, -- 告警触发时间
    alert_recover_threshold REAL NOT NULL, -- 告警恢复阈值
    alert_recover_time INTEGER NOT NULL, -- 告警恢复时间
    alert_handle_method INTEGER NOT NULL, -- 告警处理方式
    UNIQUE(name) -- 配置名称唯一
);

-- DIDO逻辑设置表
CREATE TABLE IF NOT EXISTS
di_do_set(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    seq TINYINT NOT NULL, --DIDO序列号
    type TINYINT NOT NULL, --DIDO类型: 0DI，1DO
    cabinet_code TINYINT NOT NULL, --柜子编号-- 预留用
    strategy_seq TINYINT NOT NULL, --策略序号-- 从数据表中获取
    remark TEXT(1200) --备注
);

-- DO逻辑设置表
CREATE TABLE IF NOT EXISTS
do_set_config(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    seq TINYINT NOT NULL, --DIDO序列号
    cabinet_code TINYINT NOT NULL, --柜子编号-- 预留用
    strategy_seq TINYINT NOT NULL, --策略序号-- 从数据表中获取(do_logic_dict)不展示在页面，属于后台逻辑配置项
    remark TEXT(1200) --备注
);

-- DI逻辑设置表
CREATE TABLE IF NOT EXISTS
di_set_config(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    seq TINYINT NOT NULL, --DIDO序列号
    cabinet_code TINYINT NOT NULL, --柜子编号-- 预留用
    strategy_seq TINYINT NOT NULL, --策略序号-- 从数据表中获取（di_logic_dict）不展示在页面，属于后台逻辑配置项
    remark TEXT(1200) --备注
);


-- DI逻辑字典表
CREATE TABLE IF NOT EXISTS
di_logic_dict(
  "id" INTEGER PRIMARY KEY AUTOINCREMENT,
  "strategy_seq" TINYINT NOT NULL,
  "strategy_name" TEXT(1200),
  "strategy_ename" TEXT(1200),
  UNIQUE ("strategy_seq" ASC)
);

-- DO逻辑字典表
CREATE TABLE IF NOT EXISTS
do_logic_dict(
  "id" INTEGER PRIMARY KEY AUTOINCREMENT,
  "strategy_seq" TINYINT NOT NULL,
  "strategy_name" TEXT(1200),
  "strategy_ename" TEXT(1200),
  UNIQUE ("strategy_seq" ASC)
);


-- 高级配置
CREATE TABLE IF NOT EXISTS
logic_advance_config(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    type INTEGER NOT NULL, --配置类型
    name TEXT(100) NOT NULL, --配置名称
    content TEXT(1200) NOT NULL, --配置内容
    UNIQUE(name) -- 配置名称唯一
);

-- 系统软件版本表
CREATE TABLE IF NOT EXISTS
control_soft_version(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    name TEXT(100) NOT NULL, --配置名称..
    soft_version TEXT(100) NOT NULL, --版本号
    UNIQUE(name) -- 配置名称唯一
);
-- ---------------------------------------------------------------------------
-- 本机 Local system（template_id 为 NULL 表示待分配）+ 拓扑根；清理历史 BBMS 演示设备
-- ---------------------------------------------------------------------------
DELETE FROM topology WHERE dev_id IN (
  SELECT id FROM device WHERE id != 1 AND instr(lower(coalesce(name, '')), 'bbms') > 0
);
DELETE FROM protocol WHERE related_id IN (
  SELECT id FROM device WHERE id != 1 AND instr(lower(coalesce(name, '')), 'bbms') > 0
);
DELETE FROM device WHERE id != 1 AND instr(lower(coalesce(name, '')), 'bbms') > 0;

INSERT OR IGNORE INTO device (id, sn, type, code, name, protocol_type, template_id, sort_order, remark, status)
VALUES (1, 'LOCAL001', 0, 1, 'Local system', 0, NULL, 0, '本机系统设备，禁止删除', 0);

UPDATE device SET sn = 'LOCAL001', type = 0, code = 1, name = 'Local system', protocol_type = 0,
  template_id = NULL, sort_order = 0, remark = '本机系统设备，禁止删除', status = 0 WHERE id = 1;

INSERT OR IGNORE INTO protocol (id, related_id, template_type, type, name, content, remark)
VALUES (1, 1, 0, 0, 'Local system', '{}', '本机信息协议占位');

INSERT OR IGNORE INTO topology (id, parent_id, menu_tree, name, dev_id, sort_order, remark)
VALUES (1, NULL, 1, 'Local system', 1, 0, '系统根节点，不可删除');

UPDATE topology SET
  parent_id = NULL,
  menu_tree = 1,
  name = 'Local system',
  dev_id = 1,
  sort_order = 0,
  remark = '系统根节点，不可删除'
WHERE id = 1;

UPDATE protocol SET name = 'Local system' WHERE related_id = 1;
