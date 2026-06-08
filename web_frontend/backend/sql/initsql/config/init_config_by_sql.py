# -*- coding: utf-8 -*-
"""
使用标准 SQL 脚本初始化 SCU/EMS 配置数据库（DB_URL_SCU）。
执行：在 web-backend 目录下运行  python sql/initsql/config/init_config_by_sql.py
"""
import os
import sys

# 项目根目录为 web-backend
WEB_BACKEND = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
os.chdir(WEB_BACKEND)
sys.path.insert(0, WEB_BACKEND)


def load_env():
    for name in (".env.dev", ".env.prod"):
        p = os.path.join(WEB_BACKEND, name)
        if os.path.exists(p):
            with open(p, "r", encoding="utf-8") as f:
                for line in f:
                    line = line.strip()
                    if not line or line.startswith("#") or "=" not in line:
                        continue
                    k, v = line.split("=", 1)
                    os.environ[k.strip()] = v.strip().strip("'\"").strip()
            return True
    return False


def get_scu_db_path():
    load_env()
    url = os.environ.get("DB_URL_SCU", "sqlite:////opt/bbms/db/sqlite3.db").strip()
    if url.startswith("sqlite:///"):
        path = url.replace("sqlite:///", "")
    else:
        path = url.replace("sqlite://", "")
    if not os.path.isabs(path):
        path = os.path.normpath(os.path.join(WEB_BACKEND, path))
    return path


def main():
    db_path = get_scu_db_path()
    print("SCU/EMS DB path:", db_path)

    sql_dir = os.path.dirname(os.path.abspath(__file__))
    sql_file = os.path.join(sql_dir, "config_v1.0.0_initial.sql")
    if not os.path.exists(sql_file):
        print("SQL file not found:", sql_file)
        sys.exit(1)

    with open(sql_file, "r", encoding="utf-8") as f:
        sql = f.read()

    import sqlite3
    conn = sqlite3.connect(db_path)
    try:
        conn.executescript(sql)
        conn.commit()
        print("Initialized SCU/EMS DB with config_v1.0.0_initial.sql (station, template, template_point, device, protocol, point, data_monitor, topology, ems_device_config, bms_sys_config, bms_alert_config, di_do_set, ...).")
    finally:
        conn.close()


if __name__ == "__main__":
    main()

