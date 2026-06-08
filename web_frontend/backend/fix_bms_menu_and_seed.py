# -*- coding: utf-8 -*-
"""BMS 菜单修正：一级「系统配置」目录（紧随设备维护）+ 子菜单：电池配置、系统参数、告警、DI/DO；历史数据三子菜单；隐藏日志"""
import os
import sqlite3
import sys

os.chdir(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

def load_env():
    env_path = os.path.join(os.path.dirname(__file__), ".env.dev")
    if not os.path.exists(env_path):
        env_path = os.path.join(os.path.dirname(__file__), ".env.prod")
    if not os.path.exists(env_path):
        return None
    with open(env_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#") or "=" not in line:
                continue
            k, v = line.split("=", 1)
            os.environ[k.strip()] = v.strip().strip("'\"").strip()
    return True

def get_db_path():
    load_env()
    url = os.environ.get("DB_URL_WEB", "sqlite:////opt/bbms/db/web.db").strip()
    if url.startswith("sqlite:///"):
        path = url.replace("sqlite:///", "")
    else:
        path = url.replace("sqlite://", "")
    if not os.path.isabs(path):
        path = os.path.normpath(os.path.join(os.path.dirname(__file__), path))
    return path

def main():
    db_path = get_db_path()
    print("DB path:", db_path)
    if not os.path.exists(db_path):
        print("DB file not found. Exit.")
        sys.exit(1)

    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    cur = conn.cursor()

    # 1) 查当前菜单（系统配置/电池配置、模板管理/历史数据）
    cur.execute("SELECT menu_id, menu_name, parent_id, path, component, visible FROM sys_menu WHERE menu_name IN ('控制策略','系统配置','电池配置','模板管理','历史数据','数据查询','事件记录','告警记录','故障告警','日志管理','告警配置','DI/DO配置') OR menu_id = 108 OR parent_id = 108 OR path IN ('battery-config','system-config') ORDER BY parent_id, order_num")
    rows = cur.fetchall()
    print("\n--- Related menus ---")
    for r in rows:
        print(dict(r))

    # 2) 一级「电池配置」改为「系统配置」目录(M) + 四个子菜单(C)；子项顺序：电池配置、系统参数、告警、DI/DO；一级排序在设备维护之后
    cur.execute(
        """SELECT menu_id FROM sys_menu WHERE path IN ('battery-config', 'system-config')
           OR (menu_name IN ('电池配置', '控制策略', '系统配置') AND IFNULL(parent_id, 0) = 0)
           ORDER BY CASE WHEN path = 'system-config' THEN 0 WHEN path = 'battery-config' THEN 1 ELSE 2 END,
                    menu_id LIMIT 1"""
    )
    root = cur.fetchone()
    if not root:
        cur.execute(
            "SELECT menu_id FROM sys_menu WHERE component = 'battery/config/index' AND IFNULL(parent_id, 0) = 0 LIMIT 1"
        )
        root = cur.fetchone()
    if root:
        pid = root["menu_id"]
        # 一级目录必须用 Layout，否则 /system-config/* 会整页只有业务组件（无顶栏侧栏）
        cur.execute(
            """UPDATE sys_menu SET menu_name = '系统配置', menu_type = 'M', path = 'system-config',
               component = 'Layout', visible = '0' WHERE menu_id = ?""",
            (pid,),
        )
        # 隐藏原有一级下的旧子节点，再写入标准四子菜单
        cur.execute("UPDATE sys_menu SET visible = '1' WHERE parent_id = ?", (pid,))
        subs = [
            (1, "电池配置", "battery", "battery/config/battery/index"),
            (2, "系统参数", "system", "battery/config/system/index"),
            (3, "告警配置", "alarm", "battery/config/alarm/index"),
            (4, "DI/DO配置", "dido", "battery/config/dido/index"),
        ]
        for order_num, mname, mpath, comp in subs:
            cur.execute("SELECT menu_id FROM sys_menu WHERE parent_id = ? AND path = ?", (pid, mpath))
            row = cur.fetchone()
            if row:
                cur.execute(
                    """UPDATE sys_menu SET menu_name = ?, component = ?, menu_type = 'C', visible = '0',
                       order_num = ?, is_frame = 1, status = '0' WHERE menu_id = ?""",
                    (mname, comp, order_num, row["menu_id"]),
                )
            else:
                cur.execute(
                    """INSERT INTO sys_menu (menu_name, parent_id, order_num, path, component, is_frame, menu_type,
                       visible, status, perms, icon, create_by, create_time, remark)
                       VALUES (?, ?, ?, ?, ?, 1, 'C', '0', '0', '', 'setting', 'admin', datetime('now'), ?)""",
                    (mname, pid, order_num, mpath, comp, "系统配置子菜单"),
                )
        conn.commit()
        # 一级排序：紧挨在「设备维护」之后（避免与历史数据等 order 冲突：先整体后移再定本目录）
        cur.execute(
            "SELECT order_num FROM sys_menu WHERE IFNULL(parent_id,0)=0 AND path='device-maintenance' LIMIT 1"
        )
        devo = cur.fetchone()
        base = int(devo["order_num"]) + 1 if devo and devo["order_num"] is not None else 4
        cur.execute(
            """UPDATE sys_menu SET order_num = order_num + 1
               WHERE IFNULL(parent_id,0)=0 AND menu_id != ? AND order_num >= ?""",
            (pid, base),
        )
        cur.execute("UPDATE sys_menu SET order_num = ? WHERE menu_id = ?", (base, pid))
        conn.commit()
        print("[OK] 系统配置 directory (menu_id=%s) order=%s; sub: battery/system/alarm/dido" % (pid, base))
    else:
        print("[WARN] No top-level 电池配置/system-config row found; skip system-config menu migration")

    # 3) 历史数据：模板管理(2012)->历史数据；三子菜单直接挂在历史数据下
    cur.execute("SELECT menu_id FROM sys_menu WHERE menu_name = '模板管理' LIMIT 1")
    tpl = cur.fetchone()
    if tpl:
        cur.execute("UPDATE sys_menu SET menu_name = '历史数据' WHERE menu_id = ?", (tpl["menu_id"],))
        conn.commit()
    cur.execute("SELECT menu_id FROM sys_menu WHERE menu_name = '历史数据' LIMIT 1")
    hist = cur.fetchone()
    hid = hist["menu_id"] if hist else None
    if hid is not None:
        # 三子菜单常见 menu_id：2016 数据查询, 2017 故障告警, 2018 事件记录；移到历史数据(hid)下并改名
        for mid, name, path, comp in [(2016, "数据查询", "query", "history/timed/index"), (2018, "事件记录", "event", "history/event/index"), (2017, "告警记录", "alarm", "history/alarm/index")]:
            cur.execute("UPDATE sys_menu SET parent_id = ?, menu_name = ?, path = ?, component = ?, visible = '0' WHERE menu_id = ?", (hid, name, path, comp, mid))
        # 隐藏历史数据下其它节点（如原 2015 目录）
        cur.execute("UPDATE sys_menu SET visible = '1' WHERE parent_id = ? AND menu_id NOT IN (2016,2017,2018)", (hid,))
        cur.execute("UPDATE sys_menu SET visible = '1' WHERE menu_id = 2015")
        print("[OK] 历史数据 sub-menus: 数据查询、事件记录、告警记录")

    # 4) 日志：隐藏 日志管理(108) 及其子菜单
    cur.execute("UPDATE sys_menu SET visible = '1' WHERE menu_id = 108 OR parent_id = 108")
    print("[OK] Hidden log menu (108) and its children")

    conn.commit()

    # 5) 角色菜单：确保 admin 角色能看到新菜单（若用 role_menu 表）
    try:
        cur.execute("SELECT role_id FROM sys_role WHERE role_key = 'admin' OR role_id = 1 LIMIT 1")
        role = cur.fetchone()
        if role:
            rid = role["role_id"]
            cur.execute("SELECT menu_id FROM sys_menu WHERE path = 'system-config' LIMIT 1")
            sc = cur.fetchone()
            mids = set()
            if sc:
                for r in cur.execute(
                    "SELECT menu_id FROM sys_menu WHERE menu_id = ? OR parent_id = ?", (sc["menu_id"], sc["menu_id"])
                ):
                    mids.add(r["menu_id"])
            for r in cur.execute(
                "SELECT menu_id FROM sys_menu WHERE menu_name IN ('历史数据','数据查询','事件记录','告警记录') AND visible = '0'"
            ):
                mids.add(r["menu_id"])
            for mid in mids:
                cur.execute("INSERT OR IGNORE INTO sys_role_menu (role_id, menu_id) VALUES (?,?)", (rid, mid))
            conn.commit()
            print("[OK] Role menu synced for admin")
    except sqlite3.OperationalError as e:
        if "sys_role_menu" in str(e) or "no such table" in str(e).lower():
            pass
        else:
            print("[WARN]", e)

    cur.close()
    conn.close()
    print("\nDone. Re-login or refresh to see menu changes.")

if __name__ == "__main__":
    main()
