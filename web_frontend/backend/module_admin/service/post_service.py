from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.orm import Session
from typing import List
from config.constant import CommonConstant
from exceptions.exception import ServiceException
from module_admin.dao.post_dao import PostDao
from module_admin.entity.vo.common_vo import CrudResponseModel
from module_admin.entity.vo.post_vo import DeletePostModel, PostModel, PostPageQueryModel
from utils.common_util import CamelCaseUtil, export_list2excel


class PostService:
    """
    岗位管理模块服务层
    """

    @classmethod
    async def get_post_list_services(
        cls, query_db: Session, query_object: PostPageQueryModel, is_page: bool = False
    ):
        """
        获取岗位列表信息service

        :param query_db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 岗位列表信息对象
        """
        post_list_result = await PostDao.get_post_list(query_db, query_object, is_page)

        return post_list_result

    @classmethod
    async def check_post_name_unique_services(cls, query_db: Session, page_object: PostModel):
        """
        检查岗位名称是否唯一service

        :param query_db: orm对象
        :param page_object: 岗位对象
        :return: 校验结果
        """
        post_id = -1 if page_object.post_id is None else page_object.post_id
        post = PostDao.get_post_detail_by_info(query_db, PostModel(postName=page_object.post_name))
        if post and post.post_id != post_id:
            return CommonConstant.NOT_UNIQUE
        return CommonConstant.UNIQUE

    @classmethod
    async def check_post_code_unique_services(cls, query_db: Session, page_object: PostModel):
        """
        检查岗位编码是否唯一service

        :param query_db: orm对象
        :param page_object: 岗位对象
        :return: 校验结果
        """
        post_id = -1 if page_object.post_id is None else page_object.post_id
        post = PostDao.get_post_detail_by_info(query_db, PostModel(postCode=page_object.post_code))
        if post and post.post_id != post_id:
            return CommonConstant.NOT_UNIQUE
        return CommonConstant.UNIQUE

    @classmethod
    async def add_post_services(cls, query_db: Session, page_object: PostModel):
        """
        新增岗位信息service

        :param query_db: orm对象
        :param page_object: 新增岗位对象
        :return: 新增岗位校验结果
        """
        if not cls.check_post_name_unique_services(query_db, page_object):
            raise ServiceException(message=f"Failed to add post '{page_object.post_name}': Post name already exists.")
        elif not cls.check_post_code_unique_services(query_db, page_object):
            raise ServiceException(message=f"Failed to add post '{page_object.post_name}': Post code already exists.")
        else:
            try:
                PostDao.add_post_dao(query_db, page_object)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Added successfully')
            except Exception as e:
                query_db.rollback()
                raise e

    @classmethod
    async def edit_post_services(cls, query_db: Session, page_object: PostModel):
        """
        编辑岗位信息service

        :param query_db: orm对象
        :param page_object: 编辑岗位对象
        :return: 编辑岗位校验结果
        """
        edit_post = page_object.model_dump(exclude_unset=True)
        post_info = cls.post_detail_services(query_db, page_object.post_id)
        if post_info.post_id:
            if not cls.check_post_name_unique_services(query_db, page_object):
                raise ServiceException(message=f"Failed to edit post '{page_object.post_name}': Post name already exists.")
            elif not cls.check_post_code_unique_services(query_db, page_object):
                raise ServiceException(message=f"Failed to edit post '{page_object.post_name}': Post code already exists.")
            else:
                try:
                    PostDao.edit_post_dao(query_db, edit_post)
                    query_db.commit()
                    return CrudResponseModel(is_success=True, message='Updated successfully')
                except Exception as e:
                    query_db.rollback()
                    raise e
        else:
            raise ServiceException(message='Post does not exist.')

    @classmethod
    async def delete_post_services(cls, query_db: Session, page_object: DeletePostModel):
        """
        删除岗位信息service

        :param query_db: orm对象
        :param page_object: 删除岗位对象
        :return: 删除岗位校验结果
        """
        if page_object.post_ids:
            post_id_list = page_object.post_ids.split(',')
            try:
                for post_id in post_id_list:
                    post = cls.post_detail_services(query_db, int(post_id))
                    if (PostDao.count_user_post_dao(query_db, int(post_id))) > 0:
                        raise ServiceException(message=f"'{post.post_name}' has been assigned and cannot be deleted.")
                    PostDao.delete_post_dao(query_db, PostModel(postId=post_id))
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Deleted successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='Post ID is empty.')

    @classmethod
    async def post_detail_services(cls, query_db: Session, post_id: int):
        """
        获取岗位详细信息service

        :param query_db: orm对象
        :param post_id: 岗位id
        :return: 岗位id对应的信息
        """
        post = PostDao.get_post_detail_by_id(query_db, post_id=post_id)
        if post:
            result = PostModel(**CamelCaseUtil.transform_result(post))
        else:
            result = PostModel(**dict())

        return result

    @staticmethod
    async def export_post_list_services(post_list: List):
        """
        导出岗位信息service

        :param post_list: 岗位信息列表
        :return: 岗位信息对应excel的二进制数据
        """
        # 创建一个映射字典，将英文键映射到中文键
        mapping_dict = {
            'postId': '岗位编号',
            'postCode': '岗位编码',
            'postName': '岗位名称',
            'postSort': '显示顺序',
            'status': '状态',
            'createBy': '创建者',
            'createTime': '创建时间',
            'updateBy': '更新者',
            'updateTime': '更新时间',
            'remark': '备注',
        }

        data = post_list

        for item in data:
            if item.get('status') == '0':
                item['status'] = '正常'
            else:
                item['status'] = '停用'
        new_data = [
            {mapping_dict.get(key): value for key, value in item.items() if mapping_dict.get(key)} for item in data
        ]
        binary_data = export_list2excel(new_data)

        return binary_data
