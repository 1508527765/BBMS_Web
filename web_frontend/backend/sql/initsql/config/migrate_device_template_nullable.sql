-- =============================================================================
-- 将 device.template_id 从 NOT NULL 迁移为可空（NULL 表示待分配模板）
-- 适用：SQLite
-- =============================================================================

PRAGMA foreign_keys = OFF;
BEGIN TRANSACTION;

CREATE TABLE IF NOT EXISTS device_new(
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    sn TEXT(50) NOT NULL,
    type INTEGER NOT NULL,
    code INTEGER NOT NULL,
    name TEXT(100) NOT NULL,
    protocol_type INTEGER NOT NULL,
    template_id INTEGER,
    sort_order INTEGER,
    remark TEXT(1200),
    status TINYINT,
    UNIQUE(name),
    UNIQUE(type, code)
);

INSERT INTO device_new (id, sn, type, code, name, protocol_type, template_id, sort_order, remark, status)
SELECT
    id,
    sn,
    type,
    code,
    name,
    protocol_type,
    NULLIF(template_id, 0) AS template_id,
    sort_order,
    remark,
    status
FROM device;

DROP TABLE device;
ALTER TABLE device_new RENAME TO device;

COMMIT;
PRAGMA foreign_keys = ON;
