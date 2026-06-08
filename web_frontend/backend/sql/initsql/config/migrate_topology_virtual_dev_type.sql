-- 拓扑虚拟节点：记录虚拟设备类型，供设备监控页选择布局
ALTER TABLE topology ADD COLUMN virtual_dev_type INTEGER;
