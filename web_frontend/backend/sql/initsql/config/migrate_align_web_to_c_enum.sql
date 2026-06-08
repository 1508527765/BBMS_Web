-- 将 Web 旧编号对齐为 C kit_data.h 原始 enum（一次性迁移）
-- 执行前请备份 /opt/bbms/db/sqlite3.db
--
-- 协议旧 Web(0-5) -> C(0-7):
--   0 本机信息 -> 0
--   1 Modbus TCP / BBMS私有 -> 1(BBMS私有) 或 2(Modbus TCP)
--   2 Modbus RTU -> 3
--   3 RBMS TCP -> 4
--   4 RBMS CAN -> 5
--   5 ModbusMulti -> 6
--   6(旧 BBMS私有) -> 1
--   7(旧 TMS CAN) -> 7
--
-- 设备类型旧 Web(8-16) -> C:
--   8 除湿机 -> 9, 9-15 传感器各 +1, 16 消防 -> 8

-- ===== 协议：先处理历史 6(BBMS私有) -> C 1 =====
UPDATE device SET protocol_type = 1 WHERE protocol_type = 6;
UPDATE template SET protocol_type = 1 WHERE protocol_type = 6;
UPDATE template_point SET protocol_type = 1 WHERE protocol_type = 6;
UPDATE point SET protocol_type = 1 WHERE protocol_type = 6;

-- 旧 Web 7(TMS CAN) 与 C 7 相同，无需变更

-- ===== 协议：自高向低迁移 Web 压缩编号 2-5 =====
UPDATE device SET protocol_type = 6 WHERE protocol_type = 5;
UPDATE template SET protocol_type = 6 WHERE protocol_type = 5;
UPDATE template_point SET protocol_type = 6 WHERE protocol_type = 5;
UPDATE point SET protocol_type = 6 WHERE protocol_type = 5;

UPDATE device SET protocol_type = 5 WHERE protocol_type = 4;
UPDATE template SET protocol_type = 5 WHERE protocol_type = 4;
UPDATE template_point SET protocol_type = 5 WHERE protocol_type = 4;
UPDATE point SET protocol_type = 5 WHERE protocol_type = 4;

UPDATE device SET protocol_type = 4 WHERE protocol_type = 3;
UPDATE template SET protocol_type = 4 WHERE protocol_type = 3;
UPDATE template_point SET protocol_type = 4 WHERE protocol_type = 3;
UPDATE point SET protocol_type = 4 WHERE protocol_type = 3;

UPDATE device SET protocol_type = 3 WHERE protocol_type = 2;
UPDATE template SET protocol_type = 3 WHERE protocol_type = 2;
UPDATE template_point SET protocol_type = 3 WHERE protocol_type = 2;
UPDATE point SET protocol_type = 3 WHERE protocol_type = 2;

-- Web 1=Modbus TCP -> C 2；BBMS 私有/堆控模板保持 C 1
UPDATE device SET protocol_type = 2
WHERE protocol_type = 1 AND type != 1;

UPDATE template SET protocol_type = 2
WHERE protocol_type = 1
  AND dev_type != 1
  AND name NOT LIKE '%BBMS%'
  AND name NOT LIKE '%私有%';

UPDATE template_point SET protocol_type = 2
WHERE protocol_type = 1
  AND template_id IN (
    SELECT id FROM template
    WHERE protocol_type = 2
  );

UPDATE point SET protocol_type = 2
WHERE protocol_type = 1
  AND template_id IN (
    SELECT id FROM template
    WHERE protocol_type = 2
  );

-- 设备 protocol 与模板对齐
UPDATE device
SET protocol_type = (
  SELECT protocol_type FROM template WHERE template.id = device.template_id
)
WHERE template_id IS NOT NULL AND template_id > 0;

-- ===== 设备类型 8-16：临时 +100 再映射 =====
UPDATE device SET type = type + 100 WHERE type BETWEEN 8 AND 16;
UPDATE template SET dev_type = dev_type + 100 WHERE dev_type BETWEEN 8 AND 16;
UPDATE template_point SET dev_type = dev_type + 100 WHERE dev_type BETWEEN 8 AND 16;
UPDATE point SET dev_type = dev_type + 100 WHERE dev_type BETWEEN 8 AND 16;

UPDATE device SET type = CASE type
  WHEN 116 THEN 8
  WHEN 108 THEN 9
  WHEN 109 THEN 10
  WHEN 110 THEN 11
  WHEN 111 THEN 12
  WHEN 112 THEN 13
  WHEN 113 THEN 14
  WHEN 114 THEN 15
  WHEN 115 THEN 16
  ELSE type END
WHERE type BETWEEN 108 AND 116;

UPDATE template SET dev_type = CASE dev_type
  WHEN 116 THEN 8
  WHEN 108 THEN 9
  WHEN 109 THEN 10
  WHEN 110 THEN 11
  WHEN 111 THEN 12
  WHEN 112 THEN 13
  WHEN 113 THEN 14
  WHEN 114 THEN 15
  WHEN 115 THEN 16
  ELSE dev_type END
WHERE dev_type BETWEEN 108 AND 116;

UPDATE template_point SET dev_type = CASE dev_type
  WHEN 116 THEN 8
  WHEN 108 THEN 9
  WHEN 109 THEN 10
  WHEN 110 THEN 11
  WHEN 111 THEN 12
  WHEN 112 THEN 13
  WHEN 113 THEN 14
  WHEN 114 THEN 15
  WHEN 115 THEN 16
  ELSE dev_type END
WHERE dev_type BETWEEN 108 AND 116;

UPDATE point SET dev_type = CASE dev_type
  WHEN 116 THEN 8
  WHEN 108 THEN 9
  WHEN 109 THEN 10
  WHEN 110 THEN 11
  WHEN 111 THEN 12
  WHEN 112 THEN 13
  WHEN 113 THEN 14
  WHEN 114 THEN 15
  WHEN 115 THEN 16
  ELSE dev_type END
WHERE dev_type BETWEEN 108 AND 116;
