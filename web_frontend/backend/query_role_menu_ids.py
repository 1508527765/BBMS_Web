import sqlite3, json

db=r"d:/cure_builder_kicad/BBMS/BMS/web.db"
con=sqlite3.connect(db)
con.row_factory=sqlite3.Row
cur=con.cursor()
ids=[2013,2012,2016,2017,2018]
for mid in ids:
    rows=cur.execute("select role_id,menu_id from sys_role_menu where menu_id=?",(mid,)).fetchall()
    print(mid,len(rows),[dict(r) for r in rows])
