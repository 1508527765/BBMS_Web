import sqlite3

db=r"d:/cure_builder_kicad/BBMS/BMS/web.db"
con=sqlite3.connect(db)
cur=con.cursor()

# move leaf menu_id=2013 under device-maintenance(2003)
cur.execute("UPDATE sys_menu SET parent_id=?, menu_name=?, visible=?, order_num=? WHERE menu_id=?", (2003, "模板管理", "0", 3, 2013))

# grant to admin role_id=1
cur.execute("INSERT OR IGNORE INTO sys_role_menu (role_id, menu_id) VALUES (?,?)", (1, 2013))

con.commit()
print('done')
