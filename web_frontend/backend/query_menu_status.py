import sqlite3, json

db=r"d:/cure_builder_kicad/BBMS/BMS/web.db"
con=sqlite3.connect(db)
con.row_factory=sqlite3.Row
cur=con.cursor()
ids=[2003,2012,2013,2016,2017,2018]
q="select menu_id,menu_name,parent_id,path,component,menu_type,visible,status,order_num from sys_menu where menu_id in (%s)"%(','.join(['?']*len(ids)))
rows=cur.execute(q,ids).fetchall()
print(json.dumps([dict(r) for r in rows], ensure_ascii=False))
