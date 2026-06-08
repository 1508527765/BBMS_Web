from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.orm import Session
from config.constant import CommonConstant
from exceptions.exception import ServiceException
from module_admin.dao.notice_dao import NoticeDao
from module_admin.entity.vo.common_vo import CrudResponseModel
from module_admin.entity.vo.notice_vo import DeleteNoticeModel, NoticeModel, NoticePageQueryModel
from utils.common_util import CamelCaseUtil


class NoticeService:
    """
    通知公告管理模块服务层
    """

    @classmethod
    async def get_notice_list_services(
        cls, query_db: Session, query_object: NoticePageQueryModel, is_page: bool = True
    ):
        """
        获取通知公告列表信息service

        :param query_db: orm对象
        :param query_object: 查询参数对象
        :param is_page: 是否开启分页
        :return: 通知公告列表信息对象
        """
        notice_list_result = await NoticeDao.get_notice_list(query_db, query_object, is_page)

        return notice_list_result

    @classmethod
    async def check_notice_unique_services(cls, query_db: Session, page_object: NoticeModel):
        """
        校验通知公告是否存在service

        :param query_db: orm对象
        :param page_object: 通知公告对象
        :return: 校验结果
        """
        notice_id = -1 if page_object.notice_id is None else page_object.notice_id
        notice = NoticeDao.get_notice_detail_by_info(query_db, page_object)
        if notice and notice.notice_id != notice_id:
            return CommonConstant.NOT_UNIQUE
        return CommonConstant.UNIQUE

    @classmethod
    async def add_notice_services(cls, query_db: Session, page_object: NoticeModel):
        """
        新增通知公告信息service

        :param query_db: orm对象
        :param page_object: 新增通知公告对象
        :return: 新增通知公告校验结果
        """
        if not cls.check_notice_unique_services(query_db, page_object):
            raise ServiceException(message=f'Failed to add notice {page_object.notice_title}, the notice already exists')
        else:
            try:
                NoticeDao.add_notice_dao(query_db, page_object)
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Added successfully')
            except Exception as e:
                query_db.rollback()
                raise e

    @classmethod
    async def edit_notice_services(cls, query_db: Session, page_object: NoticeModel):
        """
        编辑通知公告信息service

        :param query_db: orm对象
        :param page_object: 编辑通知公告对象
        :return: 编辑通知公告校验结果
        """
        edit_notice = page_object.model_dump(exclude_unset=True)
        notice_info = cls.notice_detail_services(query_db, page_object.notice_id)
        if notice_info.notice_id:
            if not cls.check_notice_unique_services(query_db, page_object):
                raise ServiceException(message=f'Failed to update notice {page_object.notice_title}, the notice already exists')
            else:
                try:
                    NoticeDao.edit_notice_dao(query_db, edit_notice)
                    query_db.commit()
                    return CrudResponseModel(is_success=True, message='Updated successfully')
                except Exception as e:
                    query_db.rollback()
                    raise e
        else:
            raise ServiceException(message='Notice does not exist')

    @classmethod
    async def delete_notice_services(cls, query_db: Session, page_object: DeleteNoticeModel):
        """
        删除通知公告信息service

        :param query_db: orm对象
        :param page_object: 删除通知公告对象
        :return: 删除通知公告校验结果
        """
        if page_object.notice_ids:
            notice_id_list = page_object.notice_ids.split(',')
            try:
                for notice_id in notice_id_list:
                    NoticeDao.delete_notice_dao(query_db, NoticeModel(noticeId=notice_id))
                query_db.commit()
                return CrudResponseModel(is_success=True, message='Deleted successfully')
            except Exception as e:
                query_db.rollback()
                raise e
        else:
            raise ServiceException(message='The notice id provided is empty')

    @classmethod
    async def notice_detail_services(cls, query_db: Session, notice_id: int):
        """
        获取通知公告详细信息service

        :param query_db: orm对象
        :param notice_id: 通知公告id
        :return: 通知公告id对应的信息
        """
        notice = NoticeDao.get_notice_detail_by_id(query_db, notice_id=notice_id)
        if notice:
            result = NoticeModel(**CamelCaseUtil.transform_result(notice))
        else:
            result = NoticeModel(**dict())

        return result
