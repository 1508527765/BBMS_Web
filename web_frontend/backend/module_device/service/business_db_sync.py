# -*- coding: utf-8 -*-
"""
将 EMS 库（sqlite3.db）设备/测点变更同步到 business.db（历史库），避免 Web 删除后 business 仍留幽灵设备。
"""
import logging
from typing import Iterable, List, Any

from sqlalchemy import delete

from config.get_db import BusinessSessionLocal
from module_device.entity.do.device_do import Device
from module_device.entity.do.point_do import Point

logger = logging.getLogger(__name__)


def _device_row(device: Any) -> dict:
    """从 ORM 或 dict 取 id / template_id / type。"""
    if isinstance(device, dict):
        return {
            "id": int(device["id"]),
            "template_id": int(device.get("template_id") or device.get("templateId")),
            "type": int(device.get("type") or device.get("dev_type")),
        }
    return {
        "id": int(device.id),
        "template_id": int(device.template_id),
        "type": int(device.type),
    }


def sync_delete_devices_to_business(devices_to_delete: Iterable[Any]) -> None:
    """
    删除 business.db 中与 EMS 已删设备对应的记录；若无其它设备共用模板+类型，则删测点。
    失败只记日志，不阻断 EMS 主流程。
    """
    rows = [_device_row(d) for d in devices_to_delete]
    if not rows:
        return
    deleting_ids = {r["id"] for r in rows}
    db = BusinessSessionLocal()
    try:
        for r in rows:
            others = (
                db.query(Device)
                .filter(
                    Device.template_id == r["template_id"],
                    Device.type == r["type"],
                    ~Device.id.in_(deleting_ids),
                )
                .count()
            )
            if others == 0:
                db.query(Point).filter(
                    Point.template_id == r["template_id"],
                    Point.dev_type == r["type"],
                ).delete(synchronize_session=False)

        db.execute(delete(Device).where(Device.id.in_(list(deleting_ids))))
        db.commit()
        logger.info("business.db synced delete devices: %s", sorted(deleting_ids))
    except Exception as exc:
        db.rollback()
        logger.warning("business.db sync delete failed: %s", exc)
    finally:
        db.close()


def purge_orphan_devices_in_business(ems_device_ids: List[int]) -> int:
    """
    删除 business.db 中不存在于 EMS 设备 id 列表里的 device 行（一次性修复用）。
    返回删除条数。
    """
    db = BusinessSessionLocal()
    try:
        if ems_device_ids:
            orphans = (
                db.query(Device)
                .filter(~Device.id.in_([int(i) for i in ems_device_ids]))
                .all()
            )
        else:
            orphans = db.query(Device).all()
        if not orphans:
            return 0
        sync_delete_devices_to_business(orphans)
        return len(orphans)
    finally:
        db.close()
