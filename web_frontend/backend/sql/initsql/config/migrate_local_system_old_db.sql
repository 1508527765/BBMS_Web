-- =============================================================================
-- 旧库一键迁移：本机 Local system + 清理名称含 bbms 的演示设备（SQLite）
-- 使用方式：sqlite3 /path/to/your_config.db < migrate_local_system_old_db.sql
-- 或在 sqlite3 交互中：.read migrate_local_system_old_db.sql
-- =============================================================================

-- 将 device.template_id 调整为可空（NULL 表示待分配），并把历史 0 转为 NULL
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
SELECT id, sn, type, code, name, protocol_type, NULLIF(template_id, 0), sort_order, remark, status FROM device;
DROP TABLE device;
ALTER TABLE device_new RENAME TO device;
COMMIT;
PRAGMA foreign_keys = ON;

-- 最小补丁（可单独执行）：本机行仍为历史名「BBMS」时，先改设备名与协议名
UPDATE device SET name = 'Local system' WHERE id = 1;
UPDATE protocol SET name = 'Local system' WHERE related_id = 1;

-- 先解除对将被删除设备的拓扑、协议依赖
DELETE FROM topology WHERE dev_id IN (
  SELECT id FROM device WHERE id != 1 AND instr(lower(coalesce(name, '')), 'bbms') > 0
);
DELETE FROM protocol WHERE related_id IN (
  SELECT id FROM device WHERE id != 1 AND instr(lower(coalesce(name, '')), 'bbms') > 0
);
DELETE FROM device WHERE id != 1 AND instr(lower(coalesce(name, '')), 'bbms') > 0;

-- 本机设备：编号 1、类型 0（本机系统）、协议 0（本机信息协议）、模板 NULL（待分配）
INSERT OR IGNORE INTO device (id, sn, type, code, name, protocol_type, template_id, sort_order, remark, status)
VALUES (1, 'LOCAL001', 0, 1, 'Local system', 0, NULL, 0, '本机系统设备，禁止删除', 0);

UPDATE device SET
  sn = 'LOCAL001',
  type = 0,
  code = 1,
  name = 'Local system',
  protocol_type = 0,
  template_id = NULL,
  sort_order = 0,
  remark = '本机系统设备，禁止删除',
  status = 0
WHERE id = 1;

-- 本机协议占位（若已存在 id=1 则忽略）
INSERT OR IGNORE INTO protocol (id, related_id, template_type, type, name, content, remark)
VALUES (1, 1, 0, 0, 'Local system', '{}', '本机信息协议占位');

-- 拓扑根节点：设备监控树挂载本机（若已存在 id=1 则忽略）
INSERT OR IGNORE INTO topology (id, parent_id, menu_tree, name, dev_id, sort_order, remark)
VALUES (1, NULL, 1, 'Local system', 1, 0, '系统根节点，不可删除');

-- 同步已存在根行的名称与 dev_id（避免旧库根节点仍指向已删设备）
UPDATE topology SET
  parent_id = NULL,
  menu_tree = 1,
  name = 'Local system',
  dev_id = 1,
  sort_order = 0,
  remark = '系统根节点，不可删除'
WHERE id = 1;
