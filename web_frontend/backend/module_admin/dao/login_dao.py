from sqlalchemy import and_, select
from sqlalchemy.orm import Session
from module_admin.entity.do.dept_do import SysDept
from module_admin.entity.do.user_do import SysUser


def login_by_account(db: Session, user_name: str):
    """
    根据用户名查询用户信息
    :param db: orm对象
    :param user_name: 用户名
    :return: 用户对象
    """
    # 创建查询对象
    query = db.query(SysUser, SysDept).filter(
        SysUser.user_name == user_name, SysUser.del_flag == '0'
    ).join(
        SysDept,
        and_(SysUser.dept_id == SysDept.dept_id, SysDept.status == '0', SysDept.del_flag == '0'),
        isouter=True
    ).distinct()

    # 打印 SQL 语句
    print(str(query.statement.compile(compile_kwargs={"literal_binds": True})))

    # 执行查询并返回结果
    user = query.first()

    return user