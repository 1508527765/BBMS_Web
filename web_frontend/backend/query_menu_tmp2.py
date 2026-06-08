import sqlite3, json

db=r"d:/cure_builder_kicad/BBMS/BMS/web.db"
con=sqlite3.connect(db)
con.row_factory=sqlite3.Row
cur=con.cursor()
q="select menu_id,menu_name,parent_id,path,component,menu_type,visible,order_num from sys_menu where menu_name like ? order by order_num,menu_id limit 50"
rows=cur.execute(q,["%设备维护%"]).fetchall()
print(json.dumps([dict(r) for r in rows], ensure_ascii=False))
