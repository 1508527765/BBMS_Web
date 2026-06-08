from config.get_db import get_ems_db
from sqlalchemy import update
from module_device.entity.do.pcs_soft_version_do import PcsSoftVersion
from utils.log_util import logger

def init_web_version():
    """初始化Web版本，更新或插入第18行用于存储web版本号"""
    WEB_VERSION = 'scu_web_v1.6_1221'  # Web版本号定义
    web_version = {
        'id': 18,
        'name': 'SCU-Web',
        'cpld_version': '/',
        'dsp_cpu1_version': '/',
        'dsp_cpu2_version': '/',
        'pcs_version': WEB_VERSION
    }
    return
    # 正确获取数据库会话
    db_gen = get_ems_db()
    db = next(db_gen)
    
    try:
        existing = db.query(PcsSoftVersion).filter_by(id=18).first()
        if existing:
            db.execute(update(PcsSoftVersion).where(PcsSoftVersion.id == 18).values(web_version))
        else:
            db.add(PcsSoftVersion(**web_version))
        
        db.commit()
        logger.info("SCU version initialization succeeded")
        print("SCU version initialization succeeded")
    except Exception as e:
        db.rollback()
        logger.error(f"SCU version initialization failed: {str(e)}")
        print(f"SCU version initialization failed: {str(e)}")
        # Keep backend alive when imported DB omits pcs_soft_version.
        # Battery config and other modules should still be accessible.
        return
    finally:
        db.close()
        try:
            next(db_gen)  # 确保生成器完成
        except StopIteration:
            pass