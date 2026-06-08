# -*- coding: utf-8 -*-
"""
系统概览：从堆控(BBMS)设备点表提取运行/限功率/告警等信息，与设备监控 BBMS 页语义一致。
"""
import re
from typing import Any, Dict, List, Optional

from sqlalchemy.orm import Session

from module_device.constant.device_constant import DeviceGroupMap, DeviceTypeMap
from module_device.dao.device_dao import DeviceDao
from module_device.service.device_service import DeviceService


def _point_text(p: dict) -> str:
    return f"{p.get('name') or ''}{p.get('ename') or ''}"


def _pick_first(points: List[dict], pattern: str) -> Optional[dict]:
    if not points:
        return None
    rx = re.compile(pattern, re.I)
    for p in points:
        if rx.search(_point_text(p)):
            return p
    return None


def _fmt_value(p: Optional[dict], default: str = "--") -> str:
    if not p:
        return default
    v = p.get("value")
    if v is None or v == "":
        return default
    unit = p.get("unit")
    if unit is not None and str(unit).strip() != "":
        return f"{v} {unit}".strip()
    return str(v)


def _group_type(p: dict) -> Optional[int]:
    raw = p.get("group_type")
    if raw is None:
        raw = p.get("groupType")
    try:
        return int(raw)
    except (TypeError, ValueError):
        return None


def _is_active_alarm_value(v: Any) -> bool:
    if v is None or v == "":
        return False
    try:
        return float(v) != 0.0
    except (TypeError, ValueError):
        s = str(v).strip().lower()
        return s not in ("0", "false", "normal", "ok", "正常", "无", "--")


async def _get_bbms_points(db: Session) -> tuple:
    """返回 (bbms_device_id, points)；无堆控设备时 ([], None)。"""
    stacks = DeviceDao.get_devices_by_type(db, DeviceTypeMap.STACK_BMS)
    if not stacks:
        return None, []
    device = stacks[0]
    points = await DeviceService.get_device_points_with_status_services(db, int(device.id))
    return int(device.id), points or []


def build_run_status(points: List[dict]) -> Dict[str, str]:
    return {
        "totalVoltage": _fmt_value(_pick_first(points, r"DCBusV|母线电压|BankVoltage")),
        "totalCurrent": _fmt_value(_pick_first(points, r"TotalDCCur|母线电流|总电流")),
        "totalPower": _fmt_value(_pick_first(points, r"TotalPower|总功率|kBbms_TotalPower")),
        "soc": _fmt_value(_pick_first(points, r"kBbms_SOC\b|系统SOC|\bSOC\s*\(%")),
        "soh": _fmt_value(_pick_first(points, r"kBbms_SOH\b|系统SOH|\bSOH\s*\(%")),
        "maxCellVoltage": _fmt_value(
            _pick_first(points, r"MaxCellV|MaxVCell|最大.*电芯.*电压|Cell.*Max.*V")
        ),
        "minCellVoltage": _fmt_value(
            _pick_first(points, r"MinCellV|MinVCell|最小.*电芯.*电压|Cell.*Min.*V")
        ),
        "maxTemperature": _fmt_value(
            _pick_first(points, r"MaxTCell|MaxCellT|MaxPoleT|最高.*温度|Max.*Temp")
        ),
        "minTemperature": _fmt_value(
            _pick_first(points, r"MinTCell|MinCellT|MinPoleT|最低.*温度|Min.*Temp")
        ),
    }


def build_power_limit(points: List[dict]) -> Dict[str, str]:
    return {
        "ratedCapacity": _fmt_value(
            _pick_first(points, r"额定容量|RatedCap|BankCellCap|RackRealCapAh")
        ),
        "maxChargePower": _fmt_value(
            _pick_first(points, r"MaxContChgP|MaxChrgPower|最大.*充电功率|MaxCont.*Chrg.*P")
        ),
        "maxDischargePower": _fmt_value(
            _pick_first(points, r"MaxContDischgP|MaxDisChrgPower|最大.*放电功率|MaxCont.*Dis.*P")
        ),
        "maxChargeCurrent": _fmt_value(
            _pick_first(points, r"MaxContChgCur|MaxChrgCur|最大.*充电电流")
        ),
        "maxDischargeCurrent": _fmt_value(
            _pick_first(points, r"MaxContDischgCur|MaxDisChrgCur|最大.*放电电流")
        ),
    }


def build_system_info(db: Session, points: List[dict]) -> Dict[str, Any]:
    """拓扑/库中设备计数 + BBMS 点表簇统计（RackOnlineNbr 等）。"""
    from module_device.entity.do.device_do import Device

    all_dev = db.query(Device).all()
    cluster_devices = [d for d in all_dev if int(d.type) == DeviceTypeMap.CLUSTER_BMS]
    total_clusters_db = len(cluster_devices)
    online_clusters_db = sum(1 for d in cluster_devices if int(d.status or 0) == 1)

    online_p = _pick_first(points, r"RackOnlineNbr|在网簇数|在线簇")
    total_p = _pick_first(points, r"RackEnaNbr|总簇数|Rack使能|Rack.*Nbr")

    def _as_int(p: Optional[dict], fallback: int) -> int:
        if not p or p.get("value") in (None, ""):
            return fallback
        try:
            return int(float(p.get("value")))
        except (TypeError, ValueError):
            return fallback

    total_clusters = _as_int(total_p, total_clusters_db)
    online_clusters = _as_int(online_p, online_clusters_db)
    if total_clusters and online_clusters > total_clusters:
        online_clusters = total_clusters

    # 从机：除本机外的配置设备
    slave_devs = [d for d in all_dev if int(d.type) != DeviceTypeMap.LOCAL_SYSTEM]
    total_slaves = len(slave_devs)
    online_slaves = sum(1 for d in slave_devs if int(d.status or 0) == 1)

    return {
        "totalClusters": total_clusters,
        "onlineClusters": online_clusters,
        "totalBatteries": 0,
        "onlineBatteries": 0,
        "totalSlaves": total_slaves,
        "onlineSlaves": online_slaves,
    }


def build_alarm_summary(points: List[dict]) -> Dict[str, int]:
    critical = 0
    general = 0
    flt_rx = re.compile(r"FaultList|kBbms_Flt_|\bFlt_\d|故障\d", re.I)
    for p in points:
        if not _is_active_alarm_value(p.get("value")):
            continue
        gt = _group_type(p)
        if gt == DeviceGroupMap.FAULT or flt_rx.search(_point_text(p)):
            critical += 1
        elif gt == DeviceGroupMap.EVENT:
            general += 1
    return {"critical": critical, "general": general}


async def build_bbms_overview_payload(db: Session) -> Dict[str, Any]:
    """系统概览页所需的四块数据。"""
    _dev_id, points = await _get_bbms_points(db)
    return {
        "runStatus": build_run_status(points),
        "powerLimit": build_power_limit(points),
        "systemInfo": build_system_info(db, points),
        "alarmSummary": build_alarm_summary(points),
        "bbmsDeviceId": _dev_id,
        "pointCount": len(points),
    }
