import sqlite3, json

db=r"d:/cure_builder_kicad/BBMS/BMS/web.db"
con=sqlite3.connect(db)
con.row_factory=sqlite3.Row
cur=con.cursor()
row=cur.execute("select menu_id,menu_name,parent_id,path,component,menu_type,visible,status,order_num from sys_menu where menu_id=?",(2013,)).fetchone()
print(json.dumps(dict(row), ensure_ascii=False))
role_rows=cur.execute("select role_id,menu_id from sys_role_menu where menu_id=?",(2013,)).fetchall()
print('role_rows', [dict(r) for r in role_rows])
