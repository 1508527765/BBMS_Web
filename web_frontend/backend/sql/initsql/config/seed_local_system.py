#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
补种/校正本机 Local system（template_id=NULL 待分配），并清理名称含 BBMS 的旧设备。
用法：在 web-backend 目录执行  python sql/initsql/config/seed_local_system.py
"""
import os
import sqlite3
import sys

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


SCRIPT = """
DELETE FROM topology WHERE dev_id IN (
  SELECT id FROM device WHERE id != 1 AND instr(lower(coalesce(name, '')), 'bbms') > 0
);
DELETE FROM protocol WHERE related_id IN (
  SELECT id FROM device WHERE id != 1 AND instr(lower(coalesce(name, '')), 'bbms') > 0
);
DELETE FROM device WHERE id != 1 AND instr(lower(coalesce(name, '')), 'bbms') > 0;

INSERT OR IGNORE INTO device (id, sn, type, code, name, protocol_type, template_id, sort_order, remark, status)
VALUES (1, 'LOCAL001', 0, 1, 'Local system', 0, NULL, 0, '本机系统设备，禁止删除', 0);

UPDATE device SET sn = 'LOCAL001', type = 0, code = 1, name = 'Local system', protocol_type = 0,
  template_id = NULL, sort_order = 0, remark = '本机系统设备，禁止删除', status = 0 WHERE id = 1;

INSERT OR IGNORE INTO protocol (id, related_id, template_type, type, name, content, remark)
VALUES (1, 1, 0, 0, 'Local system', '{}', '本机信息协议占位');

INSERT OR IGNORE INTO topology (id, parent_id, menu_tree, name, dev_id, sort_order, remark)
VALUES (1, NULL, 1, 'Local system', 1, 0, '系统根节点，不可删除');
"""


def main():
    db_path = get_scu_db_path()
    print("DB:", db_path)
    if not os.path.exists(db_path):
        print("DB not found")
        sys.exit(1)
    conn = sqlite3.connect(db_path)
    try:
        conn.executescript(SCRIPT)
        conn.commit()
    finally:
        conn.close()
    print("Done (cleanup BBMS + Local system template_id=NULL).")


if __name__ == "__main__":
    main()
