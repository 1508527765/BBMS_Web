#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
将 template_point 覆盖同步到 EMS point 与 business.db point（同 template_id + dev_type）。

用法（在 web-backend 目录）:
  .venv-lnx/bin/python scripts/sync_device_points_from_template.py
  .venv-lnx/bin/python scripts/sync_device_points_from_template.py --template-id 1 --dev-type 0
  .venv-lnx/bin/python scripts/sync_device_points_from_template.py --all-devices
"""
from __future__ import annotations

import argparse
import os
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
sys.path.insert(0, ROOT)
os.chdir(ROOT)
os.environ.setdefault("APP_ENV", "dev")

from sqlalchemy import create_engine, text
from sqlalchemy.orm import sessionmaker

from config.env import DataBaseConfig
from module_device.entity.do.point_do import Point
from module_device.entity.do.template_point_do import TemplatePoint


def _engine(url: str):
    path = url.replace("sqlite:///", "")
    return create_engine(f"sqlite:///{path}", connect_args={"check_same_thread": False})


def _row_to_point_dict(tp: TemplatePoint, protocol_type: int, dev_type: int) -> dict:
    return {
        "template_id": int(tp.template_id),
        "protocol_type": int(protocol_type),
        "dev_type": int(dev_type),
        "group_type": int(tp.group_type),
        "code": int(tp.code),
        "name": tp.name,
        "ename": tp.ename,
        "data_type": int(tp.data_type),
        "attribute": int(tp.attribute),
        "func": int(tp.func),
        "reg_addr": int(tp.reg_addr),
        "bit_pos": int(tp.bit_pos or 0),
        "bit_num": int(tp.bit_num or 0),
        "endian": int(tp.endian),
        "precision": tp.precision,
        "ratio": float(tp.ratio),
        "offset": float(tp.offset),
        "is_persisted": int(tp.is_persisted),
        "storage_interval": int(tp.storage_interval),
        "mutate_bound": tp.mutate_bound,
        "default_value": tp.default_value,
        "min_value": tp.min_value,
        "max_value": tp.max_value,
        "unit": tp.unit,
        "is_show": int(tp.is_show),
    }


def sync_one_pair(ems_sess, biz_sess, template_id: int, dev_type: int, protocol_type: int) -> int:
    tpl_points = (
        ems_sess.query(TemplatePoint)
        .filter(TemplatePoint.template_id == template_id)
        .order_by(TemplatePoint.code)
        .all()
    )
    if not tpl_points:
        print(f"  [跳过] template_id={template_id} 无 template_point")
        return 0

    for sess in (ems_sess, biz_sess):
        sess.query(Point).filter(
            Point.template_id == template_id,
            Point.dev_type == dev_type,
        ).delete(synchronize_session=False)

    rows = [_row_to_point_dict(tp, protocol_type, dev_type) for tp in tpl_points]
    ems_sess.bulk_insert_mappings(Point, rows)
    biz_sess.bulk_insert_mappings(Point, rows)
    return len(rows)


def main():
    parser = argparse.ArgumentParser(description="Sync template_point -> point (EMS + business)")
    parser.add_argument("--template-id", type=int, default=1)
    parser.add_argument("--dev-type", type=int, default=0)
    parser.add_argument(
        "--all-devices",
        action="store_true",
        help="按 EMS device 表逐台同步（template_id + type）",
    )
    parser.add_argument("--dry-run", action="store_true", help="只统计，不写库")
    args = parser.parse_args()

    ems_eng = _engine(DataBaseConfig.db_url_scu)
    biz_eng = _engine(DataBaseConfig.db_url_business)
    EmsSession = sessionmaker(bind=ems_eng)
    BizSession = sessionmaker(bind=biz_eng)

    ems = EmsSession()
    biz = BizSession()
    try:
        if args.all_devices:
            dev_rows = ems.execute(
                text(
                    "SELECT id, name, template_id, type, protocol_type "
                    "FROM device WHERE template_id IS NOT NULL AND template_id > 0"
                )
            ).fetchall()
            pairs = [
                (int(r.template_id), int(r.type), int(r.protocol_type), r.name)
                for r in dev_rows
            ]
        else:
            dev = ems.execute(
                text(
                    "SELECT id, name, protocol_type FROM device "
                    "WHERE template_id = :tid AND type = :dt LIMIT 1"
                ),
                {"tid": args.template_id, "dt": args.dev_type},
            ).fetchone()
            if not dev:
                print(
                    f"未找到 device: template_id={args.template_id}, type={args.dev_type}"
                )
                sys.exit(1)
            pairs = [(args.template_id, args.dev_type, int(dev.protocol_type), dev.name)]

        print("待同步:")
        for tid, dt, pt, name in pairs:
            cnt = ems.query(TemplatePoint).filter(TemplatePoint.template_id == tid).count()
            print(f"  设备 {name!r}: template_id={tid}, dev_type={dt}, 模板点={cnt}")

        if args.dry_run:
            print("dry-run，未写入")
            return

        total = 0
        for tid, dt, protocol_type, name in pairs:
            n = sync_one_pair(ems, biz, tid, dt, protocol_type)
            total += n
            print(f"  已同步 {name!r}: {n} 条 -> point (EMS + business)")

        ems.commit()
        biz.commit()
        print(f"完成，共写入 {total} 条（两库各 {total} 条）")
    except Exception:
        ems.rollback()
        biz.rollback()
        raise
    finally:
        ems.close()
        biz.close()


if __name__ == "__main__":
    main()
