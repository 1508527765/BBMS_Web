import sqlite3, json

db=r"d:/cure_builder_kicad/BBMS/BMS/web.db"
con=sqlite3.connect(db)
con.row_factory=sqlite3.Row
cur=con.cursor()
q=("select menu_id,menu_name,parent_id,path,component,menu_type,visible,order_num "
   "from sys_menu "
   "where menu_name like ? or menu_name like ? or component like ? or component like ? "
   "or path in (?,?) "
   "order by parent_id,order_num limit 200")
params=["%模板%","%历史数据%","%template%","%history/%","template","history"]
rows=cur.execute(q,params).fetchall()
print("count",len(rows))
print(json.dumps([dict(r) for r in rows], ensure_ascii=False))
