--record-- 设备表
CREATE TABLE IF NOT EXISTS
device(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    sn TEXT(50) NOT NULL, --设备sn
    type INTEGER NOT NULL, --设备类型
    code INTEGER NOT NULL, --设备编号
    name TEXT(100) NOT NULL, --设备名称
    protocol_type INTEGER NOT NULL, --协议类型
    template_id INTEGER NOT NULL, --模版Id
    sort_order INTEGER, --序号
    remark TEXT(1200), --备注
    status TINYINT, --状态
    UNIQUE(name), -- 设备名称唯一
    UNIQUE(type, code) -- 设备类型和设备编号唯一
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

-- 设备时序数据存储表
CREATE TABLE IF NOT EXISTS
timed_data_storage(
    id INTEGER PRIMARY KEY AUTOINCREMENT, --主键标识
    dev_type INTEGER NOT NULL, --设备类型
    dev_id INTEGER NOT NULL, --设备Id
    point_id INTEGER NOT NULL, --点位Id
    -- dev_name TEXT(100) NOT NULL, --设备名称
    -- point_name TEXT(100) NOT NULL, --点位名称
    storage_interval INTEGER NOT NULL, --存储周期
    timed_value REAL NOT NULL, --值
    created_time TIMESTAMP DEFAULT CURRENT_TIMESTAMP -- 备份时间
);

CREATE INDEX IF NOT EXISTS idx_dev_time ON timed_data_storage (dev_type, dev_id, created_time);