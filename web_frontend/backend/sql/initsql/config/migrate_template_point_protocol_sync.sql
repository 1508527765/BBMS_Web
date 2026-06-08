-- 将 template_point.protocol_type 与所属 template.protocol_type 对齐（一次性修复历史数据）
-- SQLite 3.33+ 支持 UPDATE FROM；若版本较旧可改用应用内 PointDao.sync_template_points_protocol_from_template 逐模板执行

UPDATE template_point
SET protocol_type = (
    SELECT t.protocol_type FROM template t WHERE t.id = template_point.template_id
)
WHERE EXISTS (SELECT 1 FROM template t WHERE t.id = template_point.template_id);
