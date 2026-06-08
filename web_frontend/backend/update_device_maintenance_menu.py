#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
从数据库层面调整“设备维护”菜单结构：

- 在“设备维护”一级菜单下，仅保留二级菜单：
  - 设备管理
  - 拓扑配置（原名称可能为“设备拓扑”）
- 删除“干节点管理”菜单记录

只修改 SQLite 中的 sys_menu 表，不依赖前端硬编码。
"""

import os
import sqlite3
import sys

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
os.chdir(BASE_DIR)
sys.path.insert(0, BASE_DIR)


def load_env():
  """
  兼容 .env.dev / .env.prod，宽松解析，忽略编码问题。
  只关心 DB_URL_WEB。
  """
  for name in (".env.dev", ".env.prod"):
    env_path = os.path.join(BASE_DIR, name)
    if not os.path.exists(env_path):
      continue
    try:
      with open(env_path, "r", encoding="utf-8", errors="ignore") as f:
        for line in f:
          line = line.strip()
          if not line or line.startswith("#") or "=" not in line:
            continue
          k, v = line.split("=", 1)
          os.environ[k.strip()] = v.strip().strip("'\"").strip()
      return True
    except Exception:
      continue
  return False


def get_db_path():
  load_env()
  url = os.environ.get("DB_URL_WEB", "sqlite:////opt/bbms/db/web.db").strip().strip("'\"")
  if url.startswith("sqlite:///"):
    path = url.replace("sqlite:///", "")
  else:
    path = url.replace("sqlite://", "")
  if not os.path.isabs(path):
    path = os.path.normpath(os.path.join(BASE_DIR, path))
  return path


def main():
  db_path = get_db_path()
  print("使用数据库文件:", db_path)
  if not os.path.exists(db_path):
    print("数据库文件不存在，请确认后端至少启动过一次，或检查 DB_URL_WEB 配置。")
    sys.exit(1)

  conn = sqlite3.connect(db_path)
  conn.row_factory = sqlite3.Row
  cur = conn.cursor()

  try:
    # 1) 查找“设备维护”一级菜单
    cur.execute(
      "SELECT menu_id, menu_name FROM sys_menu WHERE menu_name = '设备维护' LIMIT 1"
    )
    row = cur.fetchone()
    if not row:
      print("未找到“设备维护”菜单，不做修改。")
      return

    parent_id = row["menu_id"]
    print("设备维护 menu_id =", parent_id)

    # 2) 将“设备拓扑”重命名为“拓扑配置”（仅针对该父节点下）
    cur.execute(
      """
      UPDATE sys_menu
         SET menu_name = '拓扑配置'
       WHERE parent_id = ?
         AND menu_name = '设备拓扑'
      """,
      (parent_id,),
    )
    print(f"重命名设备拓扑 -> 拓扑配置，受影响行数: {cur.rowcount}")

    # 3) 删除“干节点管理”菜单（仅该父节点下）
    cur.execute(
      """
      DELETE FROM sys_menu
       WHERE parent_id = ?
         AND menu_name = '干节点管理'
      """,
      (parent_id,),
    )
    print(f"删除干节点管理菜单，受影响行数: {cur.rowcount}")

    conn.commit()

    # 4) 打印“设备维护”下当前子菜单结构，便于核对
    cur.execute(
      """
      SELECT menu_id, menu_name, path, component, visible, order_num
        FROM sys_menu
       WHERE parent_id = ?
       ORDER BY order_num, menu_id
      """,
      (parent_id,),
    )
    print("\n设备维护子菜单现状:")
    for r in cur.fetchall():
      print(dict(r))

    print("\n处理完成，请重新登录前端查看左侧菜单。")
  finally:
    cur.close()
    conn.close()


if __name__ == "__main__":
  main()

