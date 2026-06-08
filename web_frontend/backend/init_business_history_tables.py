# -*- coding: utf-8 -*-
"""
在 business.db 中创建历史数据相关表（faults_record、events_record、timed_data_storage）、设备与测点表（device、point），并插入 BMS 模拟数据。
执行：在 web-backend 目录下运行  python init_business_history_tables.py
"""
import os
import sys
from datetime import datetime, timedelta

os.chdir(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

def load_env():
    for name in (".env.dev", ".env.prod"):
        p = os.path.join(os.path.dirname(__file__), name)
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
        path = os.path.normpath(os.path.join(os.path.dirname(__file__), path))
    return path

def main():
    db_path = get_business_db_path()
    print("Business DB path:", db_path)

    from sqlalchemy import create_engine, text
    from config.database import Base
    from module_device.entity.do.history_do import TimedDataStorage, FaultsRecord, EventsRecord
    from module_device.entity.do.device_do import Device
    from module_device.entity.do.point_do import Point

    # Windows 路径转成 SQLite URL（用 / 或 三个斜杠）
    db_url = "sqlite:///" + db_path.replace("\\", "/")
    engine = create_engine(db_url, connect_args={"check_same_thread": False})

    # 创建历史表 + 设备表 + 测点表
    tables = [
        FaultsRecord.__table__, EventsRecord.__table__, TimedDataStorage.__table__,
        Device.__table__, Point.__table__,
    ]
    Base.metadata.create_all(bind=engine, tables=tables)
    print("Created tables: faults_record, events_record, timed_data_storage, device, point")

    # 插入 BMS 模拟数据（告警/事件各几条）
    now = datetime.now()
    with engine.connect() as conn:
        r = conn.execute(text("SELECT COUNT(*) FROM faults_record"))
        if r.scalar() == 0:
            conn.execute(
                text("""
                INSERT INTO faults_record (dev_type, dev_id, point_id, faults_value, created_time) VALUES
                (1, 1, 101, 1, :t1),
                (1, 1, 102, 0, :t2),
                (1, 1, 103, 1, :t3),
                (1, 2, 101, 0, :t4),
                (1, 2, 102, 1, :t5)
                """),
                {
                    "t1": (now - timedelta(hours=2)).strftime("%Y-%m-%d %H:%M:%S"),
                    "t2": (now - timedelta(hours=1)).strftime("%Y-%m-%d %H:%M:%S"),
                    "t3": (now - timedelta(minutes=30)).strftime("%Y-%m-%d %H:%M:%S"),
                    "t4": (now - timedelta(minutes=15)).strftime("%Y-%m-%d %H:%M:%S"),
                    "t5": now.strftime("%Y-%m-%d %H:%M:%S"),
                },
            )
            conn.commit()
            print("Inserted 5 mock rows into faults_record (BMS alarm/fault).")

        r = conn.execute(text("SELECT COUNT(*) FROM events_record"))
        if r.scalar() == 0:
            conn.execute(
                text("""
                INSERT INTO events_record (dev_type, dev_id, point_id, faults_value, created_time) VALUES
                (1, 1, 201, 1, :t1),
                (1, 1, 202, 0, :t2),
                (1, 2, 201, 1, :t3)
                """),
                {
                    "t1": (now - timedelta(hours=3)).strftime("%Y-%m-%d %H:%M:%S"),
                    "t2": (now - timedelta(hours=1, minutes=30)).strftime("%Y-%m-%d %H:%M:%S"),
                    "t3": (now - timedelta(minutes=45)).strftime("%Y-%m-%d %H:%M:%S"),
                },
            )
            conn.commit()
            print("Inserted 3 mock rows into events_record (BMS event).")

    # device 表：历史接口按 type+code 查设备，插入 type=1(DSP), code=1/2 等最小数据
    with engine.connect() as conn:
        r = conn.execute(text("SELECT COUNT(*) FROM device"))
        if r.scalar() == 0:
            conn.execute(
                text("""
                INSERT INTO device (sn, type, code, name, protocol_type, template_id, sort_order, remark, status)
                VALUES
                ('BMS-DSP-001', 1, 1, 'DSP-1', 1, 1, 1, NULL, 1),
                ('BMS-DSP-002', 1, 2, 'DSP-2', 1, 1, 2, NULL, 1)
                """)
            )
            conn.commit()
            print("Inserted 2 mock rows into device (type=1 DSP, code=1/2).")

    # point 表：历史接口用 template_id+dev_type 查点位名称，插入 template_id=1, dev_type=1 的少量点位
    with engine.connect() as conn:
        r = conn.execute(text("SELECT COUNT(*) FROM point"))
        if r.scalar() == 0:
            conn.execute(
                text("""
                INSERT INTO point (template_id, protocol_type, dev_type, group_type, code, name, ename, data_type, attribute, func, reg_addr, bit_pos, bit_num, endian, precision, ratio, offset, is_persisted, storage_interval, mutate_bound, default_value, min_value, max_value, unit, is_show)
                VALUES
                (1, 1, 1, 6, 101, '故障1', 'fault_1', 1, 0, 0, 0, NULL, NULL, 0, 0, 1.0, 0.0, 1, 60, NULL, 0.0, NULL, NULL, '', 1),
                (1, 1, 1, 6, 102, '故障2', 'fault_2', 1, 0, 0, 0, NULL, NULL, 0, 0, 1.0, 0.0, 1, 60, NULL, 0.0, NULL, NULL, '', 1),
                (1, 1, 1, 6, 103, '故障3', 'fault_3', 1, 0, 0, 0, NULL, NULL, 0, 0, 1.0, 0.0, 1, 60, NULL, 0.0, NULL, NULL, '', 1),
                (1, 1, 1, 7, 201, '事件1', 'event_1', 1, 0, 0, 0, NULL, NULL, 0, 0, 1.0, 0.0, 1, 60, NULL, 0.0, NULL, NULL, '', 1),
                (1, 1, 1, 7, 202, '事件2', 'event_2', 1, 0, 0, 0, NULL, NULL, 0, 0, 1.0, 0.0, 1, 60, NULL, 0.0, NULL, NULL, '', 1)
                """)
            )
            conn.commit()
            print("Inserted 5 mock rows into point (template_id=1, dev_type=1).")

    print("Done. 告警记录/事件记录/数据查询 等接口可正常返回数据。")

if __name__ == "__main__":
    main()
