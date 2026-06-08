from typing import Any, Dict, List

from fastapi import APIRouter, Depends, Request
from sqlalchemy import text
from sqlalchemy.orm import Session

from config.get_db import get_ems_db
from module_device.constant.dido_logic_dict import (
    CANONICAL_DI_STRATEGIES,
    CANONICAL_DO_STRATEGIES,
    DIDO_MAX_DI_CHANNELS,
    DIDO_MAX_DO_CHANNELS,
)
from module_device.constant.web_sign_constant import SignConstant
from module_device.service.rtdb_service import write_web_sign
from utils.response_util import ResponseUtil

batteryConfigController = APIRouter(prefix="/battery-config")


def _ensure_bms_sys_config_schema(query_db: Session) -> None:
    columns = query_db.execute(text("PRAGMA table_info(bms_sys_config)")).mappings().all()
    column_names = {col["name"] for col in columns}
    if "is_show" not in column_names:
        query_db.execute(text("ALTER TABLE bms_sys_config ADD COLUMN is_show INTEGER NOT NULL DEFAULT 1"))
        query_db.commit()


def _sync_dido_logic_dict(query_db: Session) -> None:
    """将 di/do_logic_dict 与 firmware 枚举对齐（覆盖旧版错误种子）。"""
    row = query_db.execute(
        text("SELECT strategy_name FROM di_logic_dict WHERE strategy_seq = 1 LIMIT 1")
    ).mappings().first()
    di_count = query_db.execute(text("SELECT COUNT(1) AS c FROM di_logic_dict")).mappings().first()
    do_count = query_db.execute(text("SELECT COUNT(1) AS c FROM do_logic_dict")).mappings().first()
    di_stale = (
        not di_count
        or int(di_count["c"]) != len(CANONICAL_DI_STRATEGIES)
        or not row
        or str(row["strategy_name"]) != CANONICAL_DI_STRATEGIES[1][1]
    )
    do_row = query_db.execute(
        text("SELECT strategy_name FROM do_logic_dict WHERE strategy_seq = 1 LIMIT 1")
    ).mappings().first()
    do_stale = (
        not do_count
        or int(do_count["c"]) != len(CANONICAL_DO_STRATEGIES)
        or not do_row
        or str(do_row["strategy_name"]) != CANONICAL_DO_STRATEGIES[1][1]
    )

    if di_stale:
        query_db.execute(text("DELETE FROM di_logic_dict"))
        for seq, name, ename in CANONICAL_DI_STRATEGIES:
            query_db.execute(
                text(
                    """
                    INSERT INTO di_logic_dict(strategy_seq, strategy_name, strategy_ename)
                    VALUES (:seq, :name, :ename)
                    """
                ),
                {"seq": seq, "name": name, "ename": ename},
            )

    if do_stale:
        query_db.execute(text("DELETE FROM do_logic_dict"))
        for seq, name, ename in CANONICAL_DO_STRATEGIES:
            query_db.execute(
                text(
                    """
                    INSERT INTO do_logic_dict(strategy_seq, strategy_name, strategy_ename)
                    VALUES (:seq, :name, :ename)
                    """
                ),
                {"seq": seq, "name": name, "ename": ename},
            )

    if di_stale or do_stale:
        query_db.commit()


def _seed_dido_logic_dict_if_empty(query_db: Session) -> None:
    _sync_dido_logic_dict(query_db)


def _load_logic_options(query_db: Session, table_name: str) -> List[Dict[str, Any]]:
    rows = query_db.execute(
        text(
            f"""
            SELECT strategy_seq, strategy_name, strategy_ename
            FROM {table_name}
            ORDER BY strategy_seq ASC
            """
        )
    ).mappings().all()
    options: List[Dict[str, Any]] = []
    for row in rows:
        options.append(
            {
                "value": int(row["strategy_seq"]),
                "label": str(row["strategy_name"] or row["strategy_ename"] or row["strategy_seq"]),
                "en_label": str(row["strategy_ename"] or row["strategy_name"] or row["strategy_seq"]),
            }
        )
    return options

@batteryConfigController.get("/sys-config")
async def get_bms_sys_config(
    request: Request,  # noqa: ARG001
    config_type: int = 0,
    query_db: Session = Depends(get_ems_db),
):
    _ensure_bms_sys_config_schema(query_db)
    rows = query_db.execute(
        text(
            """
            SELECT id, type, name, content, is_show
            FROM bms_sys_config
            WHERE type = :type
            ORDER BY id ASC
            """
        ),
        {"type": config_type},
    ).mappings().all()
    return ResponseUtil.success(data=[dict(row) for row in rows])


@batteryConfigController.post("/sys-config/save")
async def save_bms_sys_config(
    request: Request,  # noqa: ARG001
    payload: Dict[str, Any],
    query_db: Session = Depends(get_ems_db),
):
    _ensure_bms_sys_config_schema(query_db)
    rows: List[Dict[str, Any]] = payload.get("rows", [])
    if not isinstance(rows, list):
        return ResponseUtil.failure(msg="rows must be a list")
    config_type = int(payload.get("type", 0))
    for row in rows:
        name = str(row.get("name", "")).strip()
        if not name:
            continue
        content = str(row.get("content", ""))
        is_show = 1 if int(row.get("is_show", 1)) else 0
        exists = query_db.execute(
            text("SELECT id FROM bms_sys_config WHERE type = :type AND name = :name LIMIT 1"),
            {"type": config_type, "name": name},
        ).mappings().first()
        if exists:
            query_db.execute(
                text(
                    """
                    UPDATE bms_sys_config
                    SET content = :content, is_show = :is_show
                    WHERE id = :id
                    """
                ),
                {"content": content, "is_show": is_show, "id": exists["id"]},
            )
        else:
            query_db.execute(
                text(
                    """
                    INSERT INTO bms_sys_config(type, name, content, is_show)
                    VALUES (:type, :name, :content, :is_show)
                    """
                ),
                {"type": config_type, "name": name, "content": content, "is_show": is_show},
            )
    query_db.commit()
    return ResponseUtil.success(msg="Battery system configuration saved")


@batteryConfigController.get("/alert-config")
async def get_bms_alert_config(
    request: Request,  # noqa: ARG001
    query_db: Session = Depends(get_ems_db),
):
    rows = query_db.execute(
        text(
            """
            SELECT id, name, alert_type, alert_trigger_threshold, alert_trigger_time,
                   alert_recover_threshold, alert_recover_time, alert_handle_method
            FROM bms_alert_config
            ORDER BY alert_type ASC, id ASC
            """
        )
    ).mappings().all()
    by_type: Dict[int, List[Dict[str, Any]]] = {0: [], 1: [], 2: [], 3: []}
    for row in rows:
        alert_type = int(row["alert_type"])
        by_type.setdefault(alert_type, []).append(
            {
                "id": row["id"],
                "name": row["name"],
                "threshold": row["alert_trigger_threshold"],
                "triggerTime": row["alert_trigger_time"],
                "recoverThreshold": row["alert_recover_threshold"],
                "recoverTime": row["alert_recover_time"],
                "action": row["alert_handle_method"],
            }
        )
    return ResponseUtil.success(
        data={
            "deltaV": by_type.get(0, []),
            "circulation": by_type.get(1, []),
            "deltaI": by_type.get(2, []),
            "masterSlave": by_type.get(3, []),
        }
    )


@batteryConfigController.post("/alert-config/save")
async def save_bms_alert_config(
    request: Request,  # noqa: ARG001
    payload: Dict[str, Any],
    query_db: Session = Depends(get_ems_db),
):
    rows: List[Dict[str, Any]] = payload.get("rows", [])
    if not isinstance(rows, list):
        return ResponseUtil.failure(msg="rows must be a list")
    alert_type = int(payload.get("alert_type", 0))
    query_db.execute(text("DELETE FROM bms_alert_config WHERE alert_type = :alert_type"), {"alert_type": alert_type})
    for idx, row in enumerate(rows):
        threshold = float(row.get("threshold", 0) or 0)
        trigger_time = int(float(row.get("triggerTime", 0) or 0))
        recover_threshold = float(row.get("recoverThreshold", 0) or 0)
        recover_time = int(float(row.get("recoverTime", 0) or 0))
        action = int(float(row.get("action", 0) or 0))
        query_db.execute(
            text(
                """
                INSERT INTO bms_alert_config(
                    name, alert_type, alert_trigger_threshold, alert_trigger_time,
                    alert_recover_threshold, alert_recover_time, alert_handle_method
                ) VALUES (
                    :name, :alert_type, :alert_trigger_threshold, :alert_trigger_time,
                    :alert_recover_threshold, :alert_recover_time, :alert_handle_method
                )
                """
            ),
            {
                "name": f"alert_t{alert_type}_{idx + 1}",
                "alert_type": alert_type,
                "alert_trigger_threshold": threshold,
                "alert_trigger_time": trigger_time,
                "alert_recover_threshold": recover_threshold,
                "alert_recover_time": recover_time,
                "alert_handle_method": action,
            },
        )
    query_db.commit()
    return ResponseUtil.success(msg="Alert configuration saved")


@batteryConfigController.get("/dido-config")
async def get_bms_dido_config(
    request: Request,  # noqa: ARG001
    query_db: Session = Depends(get_ems_db),
):
    _seed_dido_logic_dict_if_empty(query_db)
    rows = query_db.execute(
        text(
            """
            SELECT id, seq, type, cabinet_code, strategy_seq, remark
            FROM di_do_set
            ORDER BY type ASC, seq ASC, id ASC
            """
        )
    ).mappings().all()
    di_rows: List[Dict[str, Any]] = []
    do_rows: List[Dict[str, Any]] = []
    for row in rows:
        item = {
            "id": row["id"],
            "name": f"{'DI' if int(row['type']) == 0 else 'DO'}{int(row['seq'])}",
            "seq": int(row["seq"]),
            "clusterNo": int(row["cabinet_code"]),
            "strategySeq": int(row["strategy_seq"]),
            "remark": str(row["remark"] or ""),
        }
        if int(row["type"]) == 0:
            di_rows.append(item)
        else:
            do_rows.append(item)
    return ResponseUtil.success(
        data={
            "diRows": di_rows,
            "doRows": do_rows,
            "diOptions": _load_logic_options(query_db, "di_logic_dict"),
            "doOptions": _load_logic_options(query_db, "do_logic_dict"),
            "maxDiChannels": DIDO_MAX_DI_CHANNELS,
            "maxDoChannels": DIDO_MAX_DO_CHANNELS,
        }
    )


@batteryConfigController.post("/dido-config/save")
async def save_bms_dido_config(
    request: Request,  # noqa: ARG001
    payload: Dict[str, Any],
    query_db: Session = Depends(get_ems_db),
):
    di_rows = payload.get("diRows", [])
    do_rows = payload.get("doRows", [])
    if not isinstance(di_rows, list) or not isinstance(do_rows, list):
        return ResponseUtil.failure(msg="diRows/doRows must be list")
    if len(di_rows) > DIDO_MAX_DI_CHANNELS:
        return ResponseUtil.failure(msg=f"DI channels exceed max {DIDO_MAX_DI_CHANNELS}")
    if len(do_rows) > DIDO_MAX_DO_CHANNELS:
        return ResponseUtil.failure(msg=f"DO channels exceed max {DIDO_MAX_DO_CHANNELS}")

    valid_di = {seq for seq, _, _ in CANONICAL_DI_STRATEGIES}
    valid_do = {seq for seq, _, _ in CANONICAL_DO_STRATEGIES}

    query_db.execute(text("DELETE FROM di_do_set WHERE type IN (0, 1)"))

    def _insert(rows: List[Dict[str, Any]], dido_type: int, valid_seq: set) -> None:
        for idx, row in enumerate(rows):
            seq = int(float(row.get("seq", idx + 1) or (idx + 1)))
            if seq < 1 or seq > (DIDO_MAX_DI_CHANNELS if dido_type == 0 else DIDO_MAX_DO_CHANNELS):
                raise ValueError(f"Invalid channel seq: {seq}")
            cabinet_code = int(float(row.get("clusterNo", 0) or 0))
            if cabinet_code < 0 or cabinet_code > 9:
                raise ValueError(f"Invalid cabinet_code: {cabinet_code}")
            strategy_seq = int(float(row.get("strategySeq", 0) or 0))
            if strategy_seq not in valid_seq:
                raise ValueError(f"Invalid strategy_seq: {strategy_seq}")
            remark = str(row.get("remark", "") or "")
            query_db.execute(
                text(
                    """
                    INSERT INTO di_do_set(seq, type, cabinet_code, strategy_seq, remark)
                    VALUES (:seq, :type, :cabinet_code, :strategy_seq, :remark)
                    """
                ),
                {
                    "seq": seq,
                    "type": dido_type,
                    "cabinet_code": cabinet_code,
                    "strategy_seq": strategy_seq,
                    "remark": remark,
                },
            )

    try:
        _insert(di_rows, 0, valid_di)
        _insert(do_rows, 1, valid_do)
    except ValueError as exc:
        query_db.rollback()
        return ResponseUtil.failure(msg=str(exc))

    query_db.commit()
    write_web_sign(SignConstant.kSign_DiDoLogicConfig, 1)
    return ResponseUtil.success(msg="DI/DO configuration saved")
