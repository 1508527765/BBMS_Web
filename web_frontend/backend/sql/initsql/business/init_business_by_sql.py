# -*- coding: utf-8 -*-
"""
使用标准 SQL 脚本初始化 business 数据库。
执行：在 web-backend 目录下运行  python sql/initsql/business/init_business_by_sql.py
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


def get_business_db_path():
    load_env()
    url = os.environ.get("DB_URL_BUSINESS", "sqlite:////opt/bbms/db/business.db").strip()
    if url.startswith("sqlite:///"):
        path = url.replace("sqlite:///", "")
    else:
        path = url.replace("sqlite://", "")
    if not os.path.isabs(path):
        path = os.path.normpath(os.path.join(WEB_BACKEND, path))
    return path


def main():
    db_path = get_business_db_path()
    print("Business DB path:", db_path)

    sql_dir = os.path.dirname(os.path.abspath(__file__))
    sql_file = os.path.join(sql_dir, "business_1.0.0_initial.sql")
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
        print("Initialized business.db with business_1.0.0_initial.sql (device, point, timed_data_storage, faults_record, events_record).")
    finally:
        conn.close()


if __name__ == "__main__":
    main()
