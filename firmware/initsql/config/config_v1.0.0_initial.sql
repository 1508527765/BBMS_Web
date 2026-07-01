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
    protocol_type INTEGER NOT NULL, --协议类型
    dev_type INTEGER, --设备类型
    name TEXT(100) NOT NULL, --名称
    remark TEXT(1200), --备注
    UNIQUE(name) --模板名称唯一
); 

-- -- 模板点位表
CREATE TABLE IF NOT EXISTS
template_point(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    template_id INTEGER NOT NULL, --模版Id
    protocol_type INTEGER NOT NULL, --协议类型
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
    protocol_type INTEGER NOT NULL, --协议类型, 参考： 主站通讯协议类型枚举
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
    type INTEGER NOT NULL, --协议类型
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
    protocol_type INTEGER NOT NULL, --协议类型
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
-- 最小在网簇数、系统下电控制配置、电允许并网压差、脱扣脉冲时间、分闸断开延时、告警灯闪烁间隔、绝缘使能、 绝缘监测间隔
CREATE TABLE IF NOT EXISTS
bms_sys_config(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    type INTEGER NOT NULL, --配置类型 -- 0: 系统配置1  1: 系统配置2
    name TEXT(100) NOT NULL, --配置名称
    content TEXT(1200) NOT NULL, --配置内容
    is_show TINYINT NOT NULL, --是否展示
    UNIQUE(name) -- 配置名称唯一
);

-- BMS System Config type=0（原有配置 + 字段注释）
-- bat_type                电池类型
-- bat_model               电池型号
-- bat_group_num           电池组数量
-- cluster_crtl_mode       簇控制模式 0x0: 集中式架构     0x1: 组串式架构
-- min_online_cluster_num  最小在线簇数
-- sys_shutdown_word       系统关机字
-- allow_grid_volt_diff    允许电网电压差
-- detach_pulse_time       分离脉冲时间
-- cut_delay_time          切断延时时间
-- alert_led_time_interval 告警LED时间间隔
-- insulation_enable       绝缘检测使能
-- insulation_monitor_interval 绝缘监测间隔
-- poweron_timeout_ms      并网超时时间（ms）
-- poweroff_timeout_ms     断网超时时间（ms）
-- INSERT OR IGNORE INTO bms_sys_config (type, name, content, is_show)
-- VALUES (
--     0,
--     'BBMS_System_Config',
--     '{
--         "bat_type": "1",
--         "bat_model": "CATL-280AH",
--         "bat_group_num": "1",
--         "cluster_crtl_mode": "1",
--         "min_online_cluster_num": "1",
--         "sys_shutdown_word": "1",
--         "allow_grid_volt_diff": "1",
--         "detach_pulse_time": "1",
--         "cut_delay_time": "1",
--         "alert_led_time_interval": "1",
--         "insulation_enable": "1",
--         "insulation_monitor_interval": "1",
--         "poweron_timeout_ms": "10000",
--         "poweroff_timeout_ms": "1000000"
--     }',
--     0
-- );

-- -- BBMS Control  type=1（本次属性 缩写字段 + 注释）
-- -- bat_conn        连接/断开电池串       0: NO REQ 1: NORMAL CONNECT 2: MANUAL CONNECT 3: DISCONNECT
-- -- ins_meas_en     使能/禁止绝缘检测     0x0:空闲 0x1:使能 0x2:禁止 0x3:无效
-- -- reserved        预留占位
-- -- bat_str_en      使能/禁止电池串       0x0:空闲 0x1:使能 0x2:禁止 0x3:无效
-- -- bank_hb         Bank 心跳            0x0:禁止 0x1:使能
-- -- str_en_rack     退簇后使能/禁止电池串 0:禁止 1:使能
-- -- rack_exit_flag  允许退簇系统标志位    0x0:非退簇系统 0x1:退簇系统
-- -- ctrl_mode       控制模式             0x0:空闲 0x1:调试 0x2:BMS控制 0x3:EMS控制 0x4:无效  
-- -- sys_arch_type   系统架构类型         0x0: 集中式架构     0x1: 组串式架构
-- -- bank_err_lvl    Bank故障等级         0X0: Default 0X1: Level 1 Fault; 0X2: Level 2 Fault; 0X3: Level 3 Fault; 0X7: No Fault;
-- -- disch_pwr_lim   基于故障的放电功率限制系数(%) 0-100
-- -- chg_pwr_lim     基于故障的充电功率限制系数(%) 0-100
-- INSERT OR IGNORE INTO bms_sys_config (type, name, content, is_show)
-- VALUES (
--     1,
--     'BBMS_Control',
--     '{
--         "bat_conn": "0",
--         "ins_meas_en": "0",
--         "reserved": "0",
--         "bat_str_en": "0",
--         "bank_hb": "0",
--         "str_en_rack": "0",
--         "rack_exit_flag": "0",
--         "ctrl_mode": "0",
--         "sys_arch_type": "0",
--         "bank_err_lvl": "7",
--         "disch_pwr_lim": "100",
--         "chg_pwr_lim": "100"
--     }',
--     1
-- );

-- --  BBMS_SafetySignal type=2（本次新增配置项，字段注释）
-- --  container_epo_flg 集装箱急停标志"
-- --  rolling_counter 滚动计数"
-- --  checksum 校验和"
-- INSERT OR IGNORE INTO bms_sys_config (type, name, content, is_show)
-- VALUES (
--     2,
--     'BBMS_SafetySignal',
--     '{
--         "container_epo_flg": "0",
--         "rolling_counter": "0",
--         "checksum": "0"
--     }',
--     1
-- );


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


-- DO逻辑设置表
CREATE TABLE IF NOT EXISTS
do_set_config(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    seq TINYINT NOT NULL, --DIDO序列号
    cabinet_code TINYINT NOT NULL, --柜子编号-- 预留用
    strategy_seq TINYINT NOT NULL, --策略序号-- 从数据表中获取(do_logic_dict)不展示在页面，属于后台逻辑配置项
    ctrl_mode TINYINT NOT NULL, --控制模式 0：禁用 1:正常 2:强控闭合 3:强控关闭
    remark TEXT(1200) --备注
);

-- DI逻辑设置表
CREATE TABLE IF NOT EXISTS
di_set_config(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    seq TINYINT NOT NULL, --DIDO序列号
    cabinet_code TINYINT NOT NULL, --柜子编号-- 预留用
    strategy_seq TINYINT NOT NULL, --策略序号-- 从数据表中获取（di_logic_dict）不展示在页面，属于后台逻辑配置项
    valid_mode TINYINT NOT NULL, -- 有效模式 0：禁用 1:常开 2:常闭
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