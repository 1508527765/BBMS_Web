from fastapi import Request
from module_admin.entity.vo.cache_vo import CacheInfoModel
from module_admin.entity.vo.common_vo import CrudResponseModel


class CacheService:
    """
    缓存监控模块服务层
    """

    @classmethod
    async def get_cache_monitor_statistical_info_services(cls, request: Request):
        """
        获取缓存监控信息service

        :param request: Request对象
        :return: 缓存监控信息
        """
        return CacheMonitorModel(commandStats=[], dbSize=0, info={})

    @classmethod
    async def get_cache_monitor_cache_name_services(cls):
        """
        获取缓存名称列表信息service

        :return: 缓存名称列表信息
        """
        return []

    @classmethod
    async def get_cache_monitor_cache_key_services(cls, request: Request, cache_name: str):
        """
        获取缓存键名列表信息service

        :param request: Request对象
        :param cache_name: 缓存名称
        :return: 缓存键名列表信息
        """
        return []

    @classmethod
    async def get_cache_monitor_cache_value_services(cls, request: Request, cache_name: str, cache_key: str):
        """
        获取缓存内容信息service

        :param request: Request对象
        :param cache_name: 缓存名称
        :param cache_key: 缓存键名
        :return: 缓存内容信息
        """
        return CacheInfoModel(cacheKey=cache_key, cacheName=cache_name, cacheValue=None, remark='')

    @classmethod
    async def clear_cache_monitor_cache_name_services(cls, request: Request, cache_name: str):
        """
        清除缓存名称对应所有键值service

        :param request: Request对象
        :param cache_name: 缓存名称
        :return: 操作缓存响应信息
        """
        return CrudResponseModel(is_success=True, message=f'{cache_name}对应键值清除成功')

    @classmethod
    async def clear_cache_monitor_cache_key_services(cls, request: Request, cache_key: str):
        """
        清除缓存名称对应所有键值service

        :param request: Request对象
        :param cache_key: 缓存键名
        :return: 操作缓存响应信息
        """
        return CrudResponseModel(is_success=True, message=f'{cache_key}清除成功')

    @classmethod
    async def clear_cache_monitor_all_services(cls, request: Request):
        """
        清除所有缓存service

        :param request: Request对象
        :return: 操作缓存响应信息
        """
        return CrudResponseModel(is_success=True, message='All cache cleared successfully')
