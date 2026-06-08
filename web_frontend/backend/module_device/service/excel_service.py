import io
import json
import os
from typing import List

import pandas as pd
from fastapi import UploadFile
from sqlalchemy.orm import Session

from exceptions.exception import ServiceException
from module_admin.entity.vo.common_vo import CrudResponseModel
from module_device.dao.point_dao import PointDao
from module_device.dao.template_dao import TemplateDao
from module_device.entity.vo.template_vo import TemplateCreateUpdateModel
from module_device.dao.topology_dao import TopologyDao
from module_device.dao.drynode_dao import DryNodeDao
from module_device.dao.control_strategy_dao import ControlStrategyDao
from utils.common_util import get_excel_template, export_list2excel


class ExcelService:
    """
    导入服务层
    """

    @staticmethod
    def _pick_excel_engine(filename: str):
        """
        根据上传文件名选择 pandas.read_excel 的 engine。
        """
        ext = os.path.splitext((filename or "").lower())[1]
        if ext in [".xlsx", ".xlsm", ".xltx", ".xltm"]:
            return "openpyxl"
        if ext == ".xls":
            return "xlrd"
        return None

    @classmethod
    def _read_excel_from_upload(cls, contents: bytes, filename: str, **kwargs):
        """
        读取上传的 Excel，优先按后缀选择 engine，失败后做兜底重试。
        """
        engine = cls._pick_excel_engine(filename)
        if engine:
            try:
                return pd.read_excel(io.BytesIO(contents), engine=engine, **kwargs)
            except Exception as e:
                # .xls 常见于缺少 xlrd 依赖，给出可读错误
                if engine == "xlrd":
                    raise ServiceException(
                        message=f'Failed to read .xls file "{filename}". Please upload .xlsx, or install xlrd.'
                    ) from e
                # openpyxl 失败则继续兜底
        try:
            return pd.read_excel(io.BytesIO(contents), engine="openpyxl", **kwargs)
        except Exception:
            # 兼容用户将 CSV 文件误命名为 .xlsx 的场景（点表导入常见）
            # 仅在未指定 sheet_name 时启用 CSV 兜底；需要多 sheet 的导入必须是标准 Excel。
            if "sheet_name" not in kwargs:
                try:
                    text_preview = contents[:4096].decode("utf-8-sig", errors="ignore")
                    if "," in text_preview and "\n" in text_preview:
                        return pd.read_csv(io.BytesIO(contents), encoding="utf-8-sig")
                except Exception:
                    pass
            return pd.read_excel(io.BytesIO(contents), **kwargs)

    @staticmethod
    async def get_point_import_template_services():
        """
        获取测点导入模板service

        :return: 测点导入模板excel的二进制数据
        """
        # header_list = ['模板编号', '协议类型', '设备类型', '分组类型', '代码', '测点名称', '数据类型', '属性', '功能', '寄存器地址', '位位置', '位数', '字节顺序', '精度', '比率', '偏移量', '是否持久化', '存储间隔', '变异范围', '默认值', '最小值', '最大值', '单位', '是否显示']
        header_list = [
            'Template ID',
            'Protocol Type',
            'Device Type',
            'Group Type',
            'Ename',
            'Code',
            'Name',
            'Data Type',
            'Attribute',
            'Function Code',
            'Register Address',
            'Bit Position',
            'Bit Number',
            'Endian',
            'Precision',
            'Ratio',
            'Offset',
            'Is Persisted',
            'Storage Interval',
            'Mutate Bound',
            'Default Value',
            'Min Value',
            'Max Value',
            'Unit',
            'Is Show',
        ]
        binary_data = get_excel_template(header_list=header_list)

        return binary_data

    @staticmethod
    async def batch_import_point_services(
            query_db: Session,
            file: UploadFile,
            template_id: int,
            dev_dict: dict = None,
    ):
        """
        批量导入测点service

        :param request: Request对象
        :param query_db: orm对象
        :param file: 测点导入文件对象
        :return: 批量导入测点结果
        """
        header_dict = {
            'Template ID': 'template_id',
            'Protocol Type': 'protocol_type',
            'Device Type': 'dev_type',
            'Group Type': 'group_type',
            'Ename': 'ename',
            'Code': 'code',
            'Name': 'name',
            'Data Type': 'data_type',
            'Attribute': 'attribute',
            'Function Code': 'func',
            'Register Address': 'reg_addr',
            'Bit Position': 'bit_pos',
            'Bit Number': 'bit_num',
            'Endian': 'endian',
            'Precision': 'precision',
            'Ratio': 'ratio',
            'Offset': 'offset',
            'Is Persisted': 'is_persisted',
            'Storage Interval': 'storage_interval',
            'Mutate Bound': 'mutate_bound',
            'Default Value': 'default_value',
            'Min Value': 'min_value',
            'Max Value': 'max_value',
            'Unit': 'unit',
            'Is Show': 'is_show',
        }

        contents = await file.read()
        df = ExcelService._read_excel_from_upload(contents, file.filename)
        await file.close()
        df.rename(columns=header_dict, inplace=True)

        canon_protocol = 0
        canon_dev_type = 0
        if not dev_dict:
            tpl = await TemplateDao.get_template_detail_by_id(query_db, template_id)
            if tpl is not None:
                canon_protocol = int(tpl.protocol_type)
                if tpl.dev_type is not None:
                    canon_dev_type = int(tpl.dev_type)

        try:
            if dev_dict:
                # 删除Point表中template_id和dev_type对应的point
                await PointDao.delete_points_by_template_id_and_dev_type(query_db, template_id, dev_dict['dev_type'])
            else:
                # 删除Point表中template_id对应的point
                await PointDao.delete_template_points_by_template_id(query_db, template_id)

            # 重新插入数据
            for index, row in df.iterrows():
                if dev_dict:
                    point_dev_type = dev_dict['dev_type']
                else:
                    row_dt = row.get('dev_type')
                    if row_dt is not None and pd.notna(row_dt) and int(row_dt) != 0:
                        point_dev_type = int(row_dt)
                    else:
                        point_dev_type = canon_dev_type
                point_data = {
                    'template_id': template_id,
                    'protocol_type': dev_dict['protocol_type'] if dev_dict else canon_protocol,
                    'dev_type': point_dev_type,
                    'group_type': row['group_type'],
                    'ename': row['ename'] if row.get('ename') else '',
                    'code': row['code'],
                    'name': row['name'] if row.get('name') else '',
                    'data_type': row['data_type'],
                    'attribute': row['attribute'],
                    'func': row['func'],
                    'reg_addr': row['reg_addr'],
                    'bit_pos': row['bit_pos'],
                    'bit_num': row['bit_num'],
                    'endian': row['endian'],
                    'precision': row['precision'],
                    'ratio': row['ratio'],
                    'offset': row['offset'],
                    'is_persisted': row['is_persisted'],
                    'storage_interval': row['storage_interval'],
                    'mutate_bound': row['mutate_bound'],
                    'default_value': row['default_value'],
                    'min_value': row['min_value'],
                    'max_value': row['max_value'],
                    'unit': row['unit'],
                    'is_show': row['is_show'],
                }

                if dev_dict:
                    await PointDao.add_point_dao(query_db, point_data)
                else:
                    await PointDao.add_template_point_dao(query_db, point_data)

            query_db.commit()
            return CrudResponseModel(is_success=True, message='Import successful')

        except Exception as e:
            query_db.rollback()
            raise e

    @staticmethod
    async def export_point_list_services(point_list: List, lang: str = 'zh'):
        """
        导出测点信息service

        :param point_list: 测点列表
        :param lang: 语言环境，'zh' 表示中文，'en' 表示英文
        :return: 测点对应excel的二进制数据
        """
        # 创建一个映射字典,将数据库字段映射到英文表头
        mapping_dict = {
            'code': 'Code',  # 点号
            'groupType': 'Group Type',  # 分组类型
            'attribute': 'Attribute',  # 属性
            'func': 'Function Code',  # 功能码
            'dataType': 'Data Type',  # 数据类型
            'regAddr': 'Register Address',  # 寄存器地址
            'bitPos': 'Bit Position',  # 位偏移
            'bitNum': 'Bit Number',  # 位数
            'precision': 'Precision',  # 精度
            'ratio': 'Ratio',  # 比例
            'offset': 'Offset',  # 偏移量
            'endian': 'Endian',  # 字节序
            'isPersisted': 'Is Persisted',  # 是否持久化
            'storageInterval': 'Storage Interval',  # 存储间隔
            'mutateBound': 'Mutate Bound',  # 变化死区
            'defaultValue': 'Default Value',  # 默认值
            'maxValue': 'Max Value',  # 最大值
            'minValue': 'Min Value',  # 最小值
            'unit': 'Unit',  # 单位
            'isShow': 'Is Show',  # 是否展示
        }
        
        # 根据语言环境决定使用哪个字段和表头
        # 中文环境：使用 name 字段，表头为 "Name"
        # 英文环境：使用 ename 字段，表头为 "Ename"
        if lang == 'zh':
            name_field = 'name'
            name_header = 'Name'
        else:
            name_field = 'ename'
            name_header = 'Ename'

        new_data = []
        for item in point_list:
            row_data = {}
            # 处理名称字段（根据语言环境使用 name 或 ename）
            if name_field in item:
                row_data[name_header] = item.get(name_field, '')
            # 处理其他字段（排除 name 和 ename）
            for key in mapping_dict:
                if key in item:
                    row_data[mapping_dict[key]] = item.get(key, '')
            new_data.append(row_data)

        binary_data = export_list2excel(new_data)
        return binary_data

    @staticmethod
    async def get_topology_import_template_services():
        """
        获取拓扑导入模板service

        :return: 拓扑导入模板excel的二进制数据
        """
        header_list = ['parent_id', 'menu_tree', 'name', 'dev_id', 'sort_order', 'remark']
        binary_data = get_excel_template(header_list=header_list)

        return binary_data

    @staticmethod
    async def import_topology_data_services(query_db: Session, file: UploadFile):
        """
        导入拓扑数据service

        :param query_db: 数据库会话
        :param file: 上传的文件
        :return: 导入结果
        """
        await TopologyDao.delete_all_topology_data(query_db)
        contents = await file.read()
        df = ExcelService._read_excel_from_upload(contents, file.filename)
        await file.close()

        try:
            topology_data_list = []
            for index, row in df.iterrows():
                topology_data = {
                    'parent_id': row['parent_id'],
                    'menu_tree': row['menu_tree'],
                    'name': row['name'],
                    'dev_id': row['dev_id'],
                    'sort_order': row['sort_order'],
                    'remark': row['remark'],
                }
                topology_data_list.append(topology_data)

            await TopologyDao.add_topologies_dao(query_db, topology_data_list)
            query_db.commit()
            return CrudResponseModel(is_success=True, message='Import successful')

        except Exception as e:
            query_db.rollback()
            raise e

    @staticmethod
    async def export_topology_data_services(query_db: Session):
        """
        导出拓扑数据service

        :param query_db: 数据库会话
        :return: 拓扑数据对应excel的二进制数据
        """
        try:
            topology_data = await TopologyDao.get_all_topology_data(query_db)

            mapping_dict = {
                'parent_id': 'parent_id',
                'menu_tree': 'menu_tree',
                'name': 'name',
                'dev_id': 'dev_id',
                'sort_order': 'sort_order',
                'remark': 'remark',
            }
            data = []

            for topology in topology_data:
                data.append({
                    'parent_id': topology.parent_id,
                    'menu_tree': topology.menu_tree,
                    'name': topology.name,
                    'dev_id': topology.dev_id,
                    'sort_order': topology.sort_order,
                    'remark': topology.remark,
                })

            # 导出数据到Excel
            new_data = [
                {mapping_dict.get(key): value for key, value in item.items() if mapping_dict.get(key)} for item in data
            ]
            binary_data = export_list2excel(new_data)
            return binary_data

        except Exception as e:
            raise e

    @staticmethod
    async def get_drynode_import_template_services():
        """
        获取DI/DO导入模板service

        :return: DI/DO导入模板excel的二进制数据
        """
        header_list = ['seq', 'type', 'cabinet_code', 'strategy_seq', 'remark']
        binary_data = get_excel_template(header_list=header_list)

        return binary_data

    @staticmethod
    async def batch_import_drynode_services(query_db: Session, file: UploadFile):
        """
        批量导入DI/DO数据service

        :param query_db: 数据库会话
        :param file: 上传的文件
        :return: 导入结果
        """
        await DryNodeDao.delete_all_drynode_data(query_db)
        header_dict = {
            'seq': 'seq',
            'type': 'type',
            'cabinet_code': 'cabinet_code',
            'strategy_seq': 'strategy_seq',
            'remark': 'remark',
        }

        contents = await file.read()
        df = ExcelService._read_excel_from_upload(contents, file.filename)
        await file.close()
        df.rename(columns=header_dict, inplace=True)

        try:
            type_seq_map = {}

            for index, row in df.iterrows():
                drynode_type = row['type']
                if drynode_type not in type_seq_map:
                    type_seq_map[drynode_type] = 1
                else:
                    type_seq_map[drynode_type] += 1

                dido_data = {
                    'seq': type_seq_map[drynode_type],
                    'type': drynode_type,
                    'cabinet_code': row['cabinet_code'],
                    'strategy_seq': row['strategy_seq'],
                    'remark': row['remark'],
                }
                await DryNodeDao.add_drynode_data(query_db, dido_data)

            query_db.commit()
            return CrudResponseModel(is_success=True, message='Import successful')

        except Exception as e:
            query_db.rollback()
            raise e

    @staticmethod
    async def export_drynode_data_services(query_db: Session):
        """
        导出DI/DO数据service

        :param query_db: 数据库会话
        :return: DI/DO数据对应excel的二进制数据
        """
        try:
            dido_data = DryNodeDao.get_all_drynode_data(query_db)

            mapping_dict = {
                'seq': 'seq',
                'type': 'type',
                'cabinet_code': 'cabinet_code',
                'strategy_seq': 'strategy_seq',
                'remark': 'remark',
            }
            data = []

            for dido in dido_data:
                data.append({
                    'seq': dido.seq,
                    'type': dido.type,
                    'cabinet_code': dido.cabinet_code,
                    'strategy_seq': dido.strategy_seq,
                    'remark': dido.remark,
                })

            # 导出数据到Excel
            new_data = [
                {mapping_dict.get(key): value for key, value in item.items() if mapping_dict.get(key)} for item in data
            ]
            binary_data = export_list2excel(new_data)
            return binary_data

        except Exception as e:
            raise e

    @staticmethod
    async def import_template_services(
            query_db: Session,
            file: UploadFile,
            template_type: int,
            dev_dict: dict = None
    ):
        """
        导入模板service

        :param request: Request对象
        :param query_db: orm对象
        :param file: 模板导入文件对象
        :param template_type: 模板类型
        :return: 模板导入结果
        """
        header_dict_template = {
            'Protocol Type': 'protocol_type',
            'Device Type': 'dev_type',
            'Name': 'name',
            'Remark': 'remark'
        }
        header_dict_point = {
            'Template ID': 'template_id',
            'Protocol Type': 'protocol_type',
            'Device Type': 'dev_type',
            'Group Type': 'group_type',
            'Ename': 'ename',
            'Code': 'code',
            'Name': 'name',
            'Data Type': 'data_type',
            'Attribute': 'attribute',
            'Function Code': 'func',
            'Register Address': 'reg_addr',
            'Bit Position': 'bit_pos',
            'Bit Number': 'bit_num',
            'Endian': 'endian',
            'Precision': 'precision',
            'Ratio': 'ratio',
            'Offset': 'offset',
            'Is Persisted': 'is_persisted',
            'Storage Interval': 'storage_interval',
            'Mutate Bound': 'mutate_bound',
            'Default Value': 'default_value',
            'Min Value': 'min_value',
            'Max Value': 'max_value',
            'Unit': 'unit',
            'Is Show': 'is_show',
        }

        try:
            contents = await file.read()
            # 读取template、point待插入数据
            df_template = ExcelService._read_excel_from_upload(contents, file.filename, sheet_name='Template')
            df_point = ExcelService._read_excel_from_upload(contents, file.filename, sheet_name='Point')
            await file.close()

            df_template.rename(columns=header_dict_template, inplace=True)
            df_point.rename(columns=header_dict_point, inplace=True)
            # 插入template表
            for index, row in df_template.iterrows():
                template_data = {
                    'type': template_type,
                    'protocol_type': row['protocol_type'],
                    'name': row['name'],
                    'remark': row['remark'],
                }
                if template_type == 0 and 'dev_type' in row and pd.notna(row.get('dev_type')):
                    template_data['dev_type'] = int(row['dev_type'])
                elif dev_dict and dev_dict.get('dev_type') is not None:
                    template_data['dev_type'] = int(dev_dict['dev_type'])
                db_template = await TemplateDao.add_template_dao(query_db, template_data)
                # 取回id，用于插入point表
                template_id = db_template.id
                tpl_proto = int(template_data["protocol_type"])
                tpl_dev_type = int(template_data.get("dev_type") or 0)
                # 插入point表
                for _, prow in df_point.iterrows():
                    row_dt = prow.get('dev_type')
                    if row_dt is not None and pd.notna(row_dt) and int(row_dt) != 0:
                        point_dev_type = int(row_dt)
                    elif dev_dict and dev_dict.get('dev_type') is not None:
                        point_dev_type = int(dev_dict['dev_type'])
                    else:
                        point_dev_type = tpl_dev_type
                    point_data = {
                        'template_id': template_id,
                        'protocol_type': dev_dict['protocol_type'] if dev_dict else tpl_proto,
                        'dev_type': point_dev_type,
                        'group_type': prow['group_type'],
                        'ename': prow['ename'],
                        'code': prow['code'],
                        'name': prow['name'],
                        'data_type': prow['data_type'],
                        'attribute': prow['attribute'],
                        'func': prow['func'],
                        'reg_addr': prow['reg_addr'],
                        'bit_pos': prow['bit_pos'],
                        'bit_num': prow['bit_num'],
                        'endian': prow['endian'],
                        'precision': prow['precision'],
                        'ratio': prow['ratio'],
                        'offset': prow['offset'],
                        'is_persisted': prow['is_persisted'],
                        'storage_interval': prow['storage_interval'],
                        'mutate_bound': prow['mutate_bound'],
                        'default_value': prow['default_value'],
                        'min_value': prow['min_value'],
                        'max_value': prow['max_value'],
                        'unit': prow['unit'],
                        'is_show': prow['is_show'],
                    }
                    await PointDao.add_point_dao(query_db, point_data)

            query_db.commit()
            return CrudResponseModel(is_success=True, message='Import successful')

        except Exception as e:
            query_db.rollback()
            raise e


    @staticmethod
    async def import_control_strategy_services(query_db: Session, file: UploadFile, setting_type: int, lang: str):
        """
        导入控制策略service

        :param query_db: 数据库会话
        :param file: 上传的文件
        :param setting_type: 设置类型
        :return: 导入结果
        """
        try:
            contents = await file.read()
            df = ExcelService._read_excel_from_upload(contents, file.filename)
            await file.close()

            # 重命名列以匹配数据库字段
            df.rename(columns={
                'Index': 'id',
                'Param Name': 'param_name',
                'Point ID': 'point_id',
                'Default Value': 'default_value'
            }, inplace=True)

            # 准备批量更新的数据
            data_list = []
            for index, row in df.iterrows():
                data = {
                    'id': row['id'],
                    'param_ename' if lang == 'en' else 'param_name': row['param_name'],
                    'point_id': row['point_id'],
                    'default_value': row['default_value']
                }
                data_list.append(data)

            # 使用 control_strategy_dao 的批量更新方法
            ControlStrategyDao.bulk_update_config_value(query_db, setting_type, data_list)

            query_db.commit()
            return CrudResponseModel(is_success=True, message='Import Control Strategy Success！Please select the target device to send.')

        except Exception as e:
            query_db.rollback()
            raise e

    @staticmethod
    async def export_control_strategy_services(point_list: List, lang: str):
        """
        导出控制策略信息service

        :param point_list: 控制策略列表
        :return: 控制策略对应excel的二进制数据
        """
        mapping_dict = {
            'id': 'Index',
            'param_name': 'Param Name',
            'point_id': 'Point ID',
            'default_value': 'Default Value'
        }
        new_data = []
        for item in point_list:
            data = {
                mapping_dict['id']: getattr(item, 'id', None),
                mapping_dict['param_name']: getattr(item, 'param_ename', None) if lang == 'en' else getattr(item, 'param_name', None),
                mapping_dict['point_id']: getattr(item, 'point_id', None),
                mapping_dict['default_value']: getattr(item, 'current_value', 0)
            }
            new_data.append(data)
        
        binary_data = export_list2excel(new_data)
        return binary_data

    @staticmethod
    async def get_control_strategy_import_template_services():
        """
        获取控制策略导入模板service

        :return: 控制策略导入模板excel的二进制数据
        """
        header_list = ['Index', 'Param Name', 'Point ID', 'Default Value']
        binary_data = get_excel_template(header_list=header_list)
        return binary_data