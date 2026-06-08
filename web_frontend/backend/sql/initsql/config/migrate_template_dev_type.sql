-- 模板绑定设备类型：同一设备类型可配置多个模板（名称仍全局唯一）
-- 执行前请备份 /opt/bbms/db/sqlite3.db
-- 若列已存在可忽略报错

ALTER TABLE template ADD COLUMN dev_type INTEGER;

-- 可选：按业务回填历史模板，例如将某模板绑定到 BMS簇(2)
-- UPDATE template SET dev_type = 2 WHERE name = '你的模板名' AND type = 0;
