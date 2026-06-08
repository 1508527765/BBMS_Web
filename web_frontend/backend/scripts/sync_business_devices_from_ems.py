#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""按 EMS(sqlite3.db) 设备列表清理 business.db 中的幽灵设备。用法：cd web-backend && .venv-lnx/bin/python scripts/sync_business_devices_from_ems.py"""
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
os.chdir(ROOT)
os.environ.setdefault("APP_ENV", "dev")

from sqlalchemy import create_engine, text
from config.env import DataBaseConfig
from module_device.service.business_db_sync import purge_orphan_devices_in_business


def ems_device_ids():
    url = DataBaseConfig.db_url_scu.replace("sqlite:///", "")
    eng = create_engine(f"sqlite:///{url}", connect_args={"check_same_thread": False})
    with eng.connect() as conn:
        rows = conn.execute(text("SELECT id FROM device ORDER BY id")).fetchall()
    return [int(r[0]) for r in rows]


def main():
    ids = ems_device_ids()
    print("EMS device ids:", ids)
    n = purge_orphan_devices_in_business(ids)
    print(f"Removed {n} orphan device(s) from business.db")


if __name__ == "__main__":
    main()
