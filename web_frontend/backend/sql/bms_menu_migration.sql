-- BMS Web 菜单调整：控制策略→电池配置(无子菜单)、模板管理→历史数据(子菜单：数据查询/事件记录/告警记录)、去掉日志
-- 执行前请确认数据库中已存在 控制策略、模板管理 等菜单（若为若依默认库可能需先在 系统管理-菜单管理 中新增）

-- 1. 控制策略 改为 电池配置（一级菜单，无子菜单）
UPDATE sys_menu SET menu_name = '电池配置' WHERE menu_name = '控制策略';

-- 2. 隐藏 电池配置 下的子菜单（保护设置、运行设置、系统设置、高级设置等不再显示）
UPDATE sys_menu SET visible = '0' WHERE parent_id = (SELECT menu_id FROM sys_menu WHERE menu_name = '电池配置' LIMIT 1);

-- 3. 电池配置 改为直接指向单页（无子级时由前端路由配置；若原为目录则需改为菜单并设置 component）
-- 若 电池配置 当前为目录(M)，需改为菜单(C)并设置 component 为 battery/config/index，path 为 battery-config：
-- UPDATE sys_menu SET menu_type = 'C', component = 'battery/config/index', path = 'battery-config' WHERE menu_name = '电池配置';

-- 4. 模板管理 改为 历史数据
UPDATE sys_menu SET menu_name = '历史数据' WHERE menu_name = '模板管理';

-- 5. 历史数据 下子菜单改为：数据查询、事件记录、告警记录（按 path 或 component 匹配后更新）
-- 数据查询
UPDATE sys_menu SET menu_name = '数据查询', component = 'history/timed/index', path = 'query' WHERE parent_id = (SELECT menu_id FROM sys_menu WHERE menu_name = '历史数据' LIMIT 1) AND (path IN ('timed','query') OR component LIKE '%timed%');
-- 事件记录
UPDATE sys_menu SET menu_name = '事件记录', component = 'history/event/index', path = 'event' WHERE parent_id = (SELECT menu_id FROM sys_menu WHERE menu_name = '历史数据' LIMIT 1) AND (path = 'event' OR component LIKE '%event%');
-- 告警记录（原故障告警或 faults）
UPDATE sys_menu SET menu_name = '告警记录', component = 'history/alarm/index', path = 'alarm' WHERE parent_id = (SELECT menu_id FROM sys_menu WHERE menu_name = '历史数据' LIMIT 1) AND (path IN ('faults','alarm') OR component LIKE '%faults%' OR menu_name = '故障告警');

-- 6. 去掉日志：隐藏 日志管理 及其子菜单
UPDATE sys_menu SET visible = '0' WHERE menu_id = 108 OR parent_id = 108;
