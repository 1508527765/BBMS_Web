#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
BMS 菜单迁移脚本：在项目使用的 SQLite 数据库上执行 sql/bms_menu_migration.sql
使用方式：在 web-backend 目录下执行  python run_bms_menu_migration.py
"""
import os
import sqlite3
import sys

# 确保能加载项目配置
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
os.chdir(os.path.dirname(os.path.abspath(__file__)))

def load_dotenv(env_file=".env.dev"):
    """简单解析 .env 文件中的变量"""
    env_path = os.path.join(os.path.dirname(__file__), env_file)
    if not os.path.exists(env_path):
        return None
    with open(env_path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            if "=" in line:
                k, v = line.split("=", 1)
                k, v = k.strip(), v.strip().strip("'\"").strip()
                os.environ[k] = v
    return True

def get_db_path():
    """从环境变量或 .env 得到 SQLite 的 web 库路径"""
    load_dotenv(".env.dev") or load_dotenv(".env.prod")
    url = os.environ.get("DB_URL_WEB", "sqlite:////opt/bbms/db/web.db").strip().strip("'\"").strip()
    # sqlite:////opt/bbms/db/web.db -> 绝对路径
    if url.startswith("sqlite:///"):
        path = url.replace("sqlite:///", "")
    else:
        path = url.replace("sqlite://", "")
    if not os.path.isabs(path):
        path = os.path.normpath(os.path.join(os.path.dirname(__file__), path))
    return path

def main():
    db_path = get_db_path()
    if not os.path.exists(db_path):
        print(f"数据库文件不存在: {db_path}")
        print("请确认后端已至少启动过一次，或检查 .env.dev 中 DB_URL_WEB 的路径。")
        sys.exit(1)

    sql_file = os.path.join(os.path.dirname(__file__), "sql", "bms_menu_migration.sql")
    if not os.path.exists(sql_file):
        print(f"SQL 文件不存在: {sql_file}")
        sys.exit(1)

    with open(sql_file, "r", encoding="utf-8") as f:
        content = f.read()

    # 按行处理：去掉注释和空行，把以 ; 结尾的行作为一条 SQL
    statements = []
    current = []
    for line in content.split("\n"):
        line_stripped = line.strip()
        if not line_stripped or line_stripped.startswith("--"):
            continue
        current.append(line)
        if line_stripped.endswith(";"):
            stmt = " ".join(current).strip()
            if stmt:
                statements.append(stmt)
            current = []
    if current:
        stmt = " ".join(current).strip()
        if stmt:
            statements.append(stmt)

    conn = sqlite3.connect(db_path)
    conn.row_factory = sqlite3.Row
    cur = conn.cursor()
    try:
        for i, sql in enumerate(statements):
            if not sql:
                continue
            try:
                cur.execute(sql)
                print(f"[OK] 第 {i+1} 条执行成功")
            except Exception as e:
                print(f"[WARN] 第 {i+1} 条: {e}")
                # 若库里没有「控制策略」「模板管理」等菜单，会更新 0 行或报错，属正常
        conn.commit()
        print("迁移执行完成。请重新登录或刷新页面查看菜单。")
    finally:
        cur.close()
        conn.close()

if __name__ == "__main__":
    main()
