import sqlite3, json

db=r"d:/cure_builder_kicad/BBMS/BMS/web.db"
con=sqlite3.connect(db)
con.row_factory=sqlite3.Row
cur=con.cursor()
rows=cur.execute("select role_id,menu_id from sys_role_menu where menu_id=?",(2013,)).fetchall()
print([dict(r) for r in rows])
