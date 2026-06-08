import sqlite3, json

db=r"d:/cure_builder_kicad/BBMS/BMS/web.db"
con=sqlite3.connect(db)
con.row_factory=sqlite3.Row
cur=con.cursor()
row=cur.execute("select menu_id,menu_name,parent_id,path,component,menu_type,visible,status,order_num from sys_menu where menu_id=?",(2013,)).fetchone()
print(dict(row) if row else None)
