from sqlalchemy.orm import Session
from typing import List
from collections import defaultdict
from datetime import datetime
from exceptions.exception import ServiceException
from module_device.dao.device_dao import DeviceDao
from module_device.dao.history_dao import HistoryDao
from module_device.dao.point_dao import PointDao
from module_device.entity.vo.history_vo import DataQueryModel, EventQueryModel
from utils.common_util import export_list2csv


class HistoryService:
    """
    拓扑管理模块服务层
    """

    @classmethod
    async def get_timed_data_list_services(cls, query_db_business: Session, query_object: DataQueryModel):
        """
        获取历史数据列表
        """
        # 获取历史数据和点位信息
        timed_page_model = await HistoryDao.get_timed_data_list_dao(query_db_business, query_object, True)
        device = DeviceDao.get_device_by_dev_type_and_dev_code(query_db_business, query_object.dev_type, query_object.dev_code)
        template_id = device.template_id
        if not template_id:
            raise ServiceException(message='Device does not exist')
        point_names = PointDao.get_point_names_by_dev_type_and_template_id(query_db_business, query_object.dev_type, template_id)
        
        # 格式化历史数据
        formatted_rows = []
        for item in timed_page_model.rows:
            formatted_row = {
                'id': item.get('id'),
                'devType': item.get('devType'),
                'devId': item.get('devId'),
                'devName': device.name,
                'pointName': point_names.get(item.get('pointId'), ('',''))[0],
                'pointEname': point_names.get(item.get('pointId'), ('',''))[1],
                'storageInterval': item.get('storageInterval'),
                'timedValue': item.get('timedValue'),
                'createdTime': datetime.strftime(item.get('createdTime'), '%Y-%m-%d %H:%M:%S')
            }
            formatted_rows.append(formatted_row)
        timed_page_model.rows = formatted_rows
        
        return timed_page_model


    @staticmethod
    async def export_timed_data_services(query_db_business: Session, query_object: DataQueryModel):
        """
        导出历史数据为 Excel（按时间为行，点位为列）

        :param query_db: 数据库会话
        :param query_object: 查询条件
        :return: Excel 的二进制数据
        """
        try:
            # 获取历史数据
            timed_data_data = await HistoryDao.get_timed_data_list_dao(query_db_business, query_object, False)
            # 一次性查出当前设备下所有点位ID -> 点名
            device = DeviceDao.get_device_by_dev_type_and_dev_code(query_db_business, query_object.dev_type, query_object.dev_code)
            template_id = device.template_id
            if not template_id:
                raise ServiceException(message='Export timed data failed')
            point_name_dict = PointDao.get_point_names_by_dev_type_and_template_id(query_db_business, query_object.dev_type, device.template_id)
            # 格式化为导出结构
            formatted_data = HistoryService.format_timed_data_to_export(timed_data_data, point_name_dict)

            # 转换为 Excel 二进制
            binary_data = export_list2csv(formatted_data)
            return binary_data

        except Exception as e:
            raise ServiceException(message='Export timed data failed')
        

    @staticmethod
    def format_timed_data_to_export(timed_data_data, point_name_dict):
        """
        将 timed 数据格式化为按时间为行、点位为列的二维结构
        :param timed_data_data: 历史数据列表，每条记录应包含 createdTime、point_id、timedValue
        :param query_db: EMS数据库
        :param query_object: 查询条件（包含设备类型、设备编码等）
        :return: 适合导出到 Excel 的列表[dict]，第一列为时间，其他列为点位名
        """
        # 构建 {time_str: {point_name: value}} 的结构
        table_data = defaultdict(dict)
        all_point_names = set()

        for item in timed_data_data:
            created_time = item.get('createdTime')
            point_id = item.get('pointId')
            value = item.get('timedValue')

            if not created_time or point_id not in point_name_dict:
                continue

            point_name = point_name_dict[point_id]
            time_str = datetime.strftime(created_time, '%Y-%m-%d %H:%M:%S')

            table_data[time_str][point_name] = value
            all_point_names.add(point_name)

        # 排序列名
        sorted_point_names = sorted(all_point_names)

        # 构造导出数据
        result = []
        for time_str in sorted(sorted(table_data.keys())):
            row = {'时间': time_str}
            for name in sorted_point_names:
                row[name] = table_data[time_str].get(name, None)
            result.append(row)

        return result


    @classmethod
    async def get_event_data_list_services(cls, query_db_business: Session, query_object: EventQueryModel):
        """
        获取历史事件/故障数据列表
        """
        event_data = await HistoryDao.get_event_data_list_dao(query_db_business, query_object, is_page=True)
        event_data_list = []

        if query_object.dev_type and query_object.dev_code:
            device_name, point_names = await cls.fetch_point_names(query_db_business, query_object.dev_type, query_object.dev_code)
            for item in event_data.rows:
                event_data_list.append({
                    'id': item.get('id'),
                    'devType': item.get('devType'),
                    'devId': item.get('devId'),
                    'devName': device_name,
                    'pointName': point_names.get(item.get('pointId'), ('',''))[0],
                    'pointEname': point_names.get(item.get('pointId'), ('',''))[1],
                    'faultsValue': item.get('faultsValue'),
                    'createdTime': datetime.strftime(item.get('createdTime'), '%Y-%m-%d %H:%M:%S')
                })
        else:
            for item in event_data.rows:
                device_name, point_names = await cls.fetch_point_names(query_db_business, item.get('devType'), item.get('devId'))
                event_data_list.append({
                    'id': item.get('id'),
                    'devType': item.get('devType'),
                    'devId': item.get('devId'),
                    'devName': device_name,
                    'pointName': point_names.get(item.get('pointId'), ('',''))[0],
                    'pointEname': point_names.get(item.get('pointId'), ('',''))[1],
                    'faultsValue': item.get('faultsValue'),
                    'createdTime': datetime.strftime(item.get('createdTime'), '%Y-%m-%d %H:%M:%S')
                })

        event_data.rows = event_data_list
        return event_data


    @classmethod
    async def export_event_data_list_services(cls, query_db_business: Session, query_object: EventQueryModel):
        """
        导出历史事件/故障数据列表
        """
        event_data = await HistoryDao.get_event_data_list_dao(query_db_business, query_object, is_page=False)

        mapping_dict = {
            'id': 'id',
            'devName': 'Device',
            'devId': 'Number',
            'pointEname': 'Attribute',
            'faultsValue': 'Value',
            'createdTime': 'Time'
        }

        export_data_list = []

        # 如果有固定的 dev_type + dev_code，只查一次
        if query_object.dev_type and query_object.dev_code:
            device_name, point_names = await cls.fetch_point_names(
                query_db_business, query_object.dev_type, query_object.dev_code
            )
            for item in event_data:
                base = {
                    'id': item['id'],
                    'devType': item['devType'],
                    'devId': item['devId'],
                    'devName': device_name,
                    'pointName': point_names.get(item.get('pointId'), ('',''))[0],
                    'pointEname': point_names.get(item.get('pointId'), ('',''))[1],
                    'faultsValue': item['faultsValue'],
                    'createdTime': item['createdTime'].strftime('%Y-%m-%d %H:%M:%S')
                }
                export_data_list.append({mapping_dict[k]: base[k] for k in mapping_dict})

        else:
            # 批量获取所有需要的设备和点位
            dev_pairs = {(item['devType'], item['devId']) for item in event_data}
            devices = DeviceDao.get_devices_by_dev_pairs(query_db_business, dev_pairs)

            # 构建三个字典，避免循环里查数据库
            device_name_dict = {(d.type, d.code): d.name for d in devices}
            device_template_dict = {(d.type, d.code): d.template_id for d in devices if d.template_id}
            point_names_dict = {
                (d.type, d.template_id): PointDao.get_point_names_by_dev_type_and_template_id(
                    query_db_business, d.type, d.template_id
                )
                for d in devices if d.template_id
            }

            # 构造导出数据
            for item in event_data:
                dev_type, dev_id = item['devType'], item['devId']
                device_name = device_name_dict.get((dev_type, dev_id), '')
                template_id = device_template_dict.get((dev_type, dev_id))
                point_names = point_names_dict.get((dev_type, template_id), {}) if template_id else {}
                
                base = {
                    'id': item['id'],
                    'devType': dev_type,
                    'devId': dev_id,
                    'devName': device_name,
                    'pointName': point_names.get(item.get('pointId'), ('',''))[0],
                    'pointEname': point_names.get(item.get('pointId'), ('',''))[1],
                    'faultsValue': item['faultsValue'],
                    'createdTime': item['createdTime'].strftime('%Y-%m-%d %H:%M:%S')
                }
                export_data_list.append({mapping_dict[k]: base[k] for k in mapping_dict})

        return export_list2csv(export_data_list)


    @staticmethod
    async def fetch_point_names(query_db_business, dev_type, dev_code):
        device = DeviceDao.get_device_by_dev_type_and_dev_code(query_db_business, dev_type, dev_code)
        if not device.template_id or not device.name:
            raise ServiceException(message='Device does not exist')
        device_name = device.name
        point_names = PointDao.get_point_names_by_dev_type_and_template_id(query_db_business, dev_type, device.template_id)
        return device_name, point_names
