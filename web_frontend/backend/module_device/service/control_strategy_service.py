from typing import List

from module_device.dao.control_strategy_dao import ControlStrategyDao
from module_device.entity.vo.control_strategy_vo import UpdateConfigModel
from module_device.service.rtdb_service import get_rtdb_point_value, set_rtdb_point_value
from module_admin.entity.vo.common_vo import CrudResponseModel


class ControlStrategyService:

    @staticmethod
    async def get_protect_config_list(type:str, device_code:int, db):
        configs = ControlStrategyDao.get_all_configs(type, db)
        for config in configs:
            config.current_value = get_rtdb_point_value(1, device_code - 1, config.point_id)
        return configs

    @staticmethod
    async def update_protect_config_value(db, device_code:List[int], update_data: List[UpdateConfigModel]):
        for code in device_code:
            for data in update_data:
                set_rtdb_point_value(1, code - 1, data.point_id, data.new_value)

        return CrudResponseModel(is_success=True, message=f'控制策略更新成功')

    @staticmethod
    async def get_advanced_config_list(db):
        """
        获取高级配置列表
        """
        return ControlStrategyDao.get_advanced_configs(db)

    @staticmethod
    async def save_advanced_config(db, config_data: dict) -> bool:
        """
        保存高级配置
        """
        return ControlStrategyDao.save_advanced_config(db, config_data)

    @staticmethod
    async def get_default_config_list(db, type:str):
        configs = ControlStrategyDao.get_all_configs(type, db)
        return configs
