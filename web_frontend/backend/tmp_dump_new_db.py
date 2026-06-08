import sqlite3

db = r"d:\cure_builder_kicad\BBMS\BMS\sqlite3.db"
c = sqlite3.connect(db)
cur = c.cursor()
print("table_info:", cur.execute("pragma table_info(bms_sys_config)").fetchall())
rows = cur.execute("select * from bms_sys_config order by id").fetchall()
print("row_count:", len(rows))
for r in rows:
    print(r)
c.close()
