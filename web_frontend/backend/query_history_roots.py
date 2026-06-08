import sqlite3, json

db=r"d:/cure_builder_kicad/BBMS/BMS/web.db"
con=sqlite3.connect(db)
con.row_factory=sqlite3.Row
cur=con.cursor()
rows=cur.execute("select menu_id,menu_name,parent_id,path,component,visible,status,order_num from sys_menu where component like 'history/%' or menu_name like '%历史数据%' or menu_name like '%数据查询%' or menu_name like '%事件记录%' or menu_name like '%告警记录%' order by parent_id,order_num").fetchall()
print('count',len(rows))
print(json.dumps([dict(r) for r in rows], ensure_ascii=False))
