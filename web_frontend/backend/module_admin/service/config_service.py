from fastapi import Request
from sqlalchemy.orm import Session
from typing import List
from config.constant import CommonConstant
from exceptions.exception import ServiceException
from module_admin.dao.config_dao import ConfigDao
from module_admin.entity.vo.common_vo import CrudResponseModel
from module_admin.entity.vo.config_vo import ConfigModel, ConfigPageQueryModel, DeleteConfigModel
from utils.common_util import CamelCaseUtil, export_list2excel


class ConfigService:
    """
    参数配置管理模块服务层
    """

    @classmethod
    async def get_config_list_services(
        cls, query_db: Session, query_object: ConfigPageQueryModel, is_page: bool = False
    ):
        """
        获取参数配置列表信息service

        :param query_db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 参数配置列表信息对象
        """
        config_list_result = await ConfigDao.get_config_list(query_db, query_object, is_page)

        return config_list_result

    @classmethod
    async def check_config_key_unique_services(cls, query_db: Session, page_object: ConfigModel):
        """
        校验参数键名是否唯一service

        :param query_db: orm对象
        :param page_object: 参数配置对象
        :return: 校验结果
        """
        config_id = -1 if page_object.config_id is None else page_object.config_id
        config = await ConfigDao.get_config_detail_by_info(query_db, ConfigModel(configKey=page_object.config_key))
        if config and config.config_id != config_id:
            return CommonConstant.NOT_UNIQUE
        return CommonConstant.UNIQUE

    @classmethod
    async def add_config_services(cls, request: Request, query_db: Session, page_object: ConfigModel):
        """
        新增参数配置信息service

        :param request: Request对象
        :param query_db: orm对象
        :param page_object: 新增参数配置对象
        :return: 新增参数配置校验结果
        """
        if not cls.check_config_key_unique_services(query_db, page_object):
            raise ServiceException(message=f"Failed to add parameter '{page_object.config_name}', the config key already exists.")
        else:
            try:
                ConfigDao.add_config_dao(query_db, page_object)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Added successfully')
            except Exception as e:
                query_db.rollback()
                raise e

    @classmethod
    async def edit_config_services(cls, request: Request, query_db: Session, page_object: ConfigModel):
        """
        编辑参数配置信息service

        :param request: Request对象
        :param query_db: orm对象
        :param page_object: 编辑参数配置对象
        :return: 编辑参数配置校验结果
        """
        edit_config = page_object.model_dump(exclude_unset=True)
        config_info = await cls.config_detail_services(query_db, page_object.config_id)
        if config_info.config_id:
            if not await cls.check_config_key_unique_services(query_db, page_object):
                raise ServiceException(message=f"Failed to update parameter '{page_object.config_name}', the config key already exists.")
            else:
                try:
                    await ConfigDao.edit_config_dao(query_db, edit_config)
                    query_db.commit()
                    return CrudResponseModel(is_success=True, message='Updated successfully')
                except Exception as e:
                    query_db.rollback()
                    raise e
        else:
            raise ServiceException(message='The parameter configuration does not exist.')

    @classmethod
    async def delete_config_services(cls, request: Request, query_db: Session, page_object: DeleteConfigModel):
        """
        删除参数配置信息service

        :param request: Request对象
        :param query_db: orm对象
        :param page_object: 删除参数配置对象
        :return: 删除参数配置校验结果
        """
        if page_object.config_ids:
            config_id_list = page_object.config_ids.split(',')
            try:
                for config_id in config_id_list:
                    config_info = cls.config_detail_services(query_db, int(config_id))
                    if config_info.config_type == CommonConstant.YES:
                        raise ServiceException(message=f"Built-in parameter '{config_info.config_key}' cannot be deleted.")
                    else:
                        ConfigDao.delete_config_dao(query_db, ConfigModel(configId=int(config_id)))
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Deleted successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='The passed config_id is empty.')

    @classmethod
    async def config_detail_services(cls, query_db: Session, config_id: int):
        """
        获取参数配置详细信息service

        :param query_db: orm对象
        :param config_id: 参数配置id
        :return: 参数配置id对应的信息
        """
        config = await ConfigDao.get_config_detail_by_id(query_db, config_id=config_id)
        if config:
            result = ConfigModel(**CamelCaseUtil.transform_result(config))
        else:
            result = ConfigModel(**dict())

        return result

    @staticmethod
    async def export_config_list_services(config_list: List):
        """
        导出参数配置信息service

        :param config_list: 参数配置信息列表
        :return: 参数配置信息对应excel的二进制数据
        """
        # 创建一个映射字典，将英文键映射到中文键
        mapping_dict = {
            'configId': '参数主键',
            'configName': '参数名称',
            'configKey': '参数键名',
            'configValue': '参数键值',
            'configType': '系统内置',
            'createBy': '创建者',
            'createTime': '创建时间',
            'updateBy': '更新者',
            'updateTime': '更新时间',
            'remark': '备注',
        }

        data = config_list

        for item in data:
            if item.get('configType') == 'Y':
                item['configType'] = '是'
            else:
                item['configType'] = '否'
        new_data = [
            {mapping_dict.get(key): value for key, value in item.items() if mapping_dict.get(key)} for item in data
        ]
        binary_data = export_list2excel(new_data)

        return binary_data
