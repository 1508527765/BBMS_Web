from sqlalchemy.orm import Session
from module_device.entity.do.pcs_config_do import PcsProtectConfig, PcsSysConfig, PcsRunConfig, LogicAdvanceConfig
from sqlalchemy import update
import json


class ControlStrategyDao:

    @staticmethod
    def get_all_configs(type: str, db: Session):
        if type == "sys":
            return db.query(PcsSysConfig).order_by(PcsSysConfig.sort_order).all()
        elif type == "protect":
            return db.query(PcsProtectConfig).order_by(PcsProtectConfig.sort_order).all()
        elif type == "run":
            return db.query(PcsRunConfig).order_by(PcsRunConfig.sort_order).all()

    @staticmethod
    def update_config_value(db: Session, config_id: int, new_value: str, type: str) -> bool:
        if type == "sys":
            model = PcsSysConfig
        elif type == "protect":
            model = PcsProtectConfig
        elif type == "run":
            model = PcsRunConfig
        else:
            return False
            
        affected_rows = db.query(model).filter(model.id == config_id).update({
            model.current_value: new_value
        })
        db.commit()
        return affected_rows > 0

    @staticmethod
    def bulk_update_config_value(db: Session, type: str, data_list: list):
        if type == "sys":
            model = PcsSysConfig
        elif type == "protect":
            model = PcsProtectConfig
        elif type == "run":
            model = PcsRunConfig
        db.bulk_update_mappings(model, data_list)


    @staticmethod
    def get_advanced_configs(db: Session):
        """
        获取所有高级配置，并将content字段解析为字典
        
        Returns:
            List[Dict[str, Any]]: 配置列表，每个配置是一个字典
        """
        results = db.query(LogicAdvanceConfig).all()
        advanced_configs = []
        
        for result in results:
            config_dict = {
                'id': result.id,
                'type': result.type,
                'name': result.name,
                'content': result.content  # 先保留原始content
            }
            
            content = result.content
            # 处理不同类型的content
            try:
                if isinstance(content, str):
                    content_data = json.loads(content)
                elif isinstance(content, dict):
                    content_data = content
                elif content is None:
                    content_data = {}
                else:
                    content_data = json.loads(str(content))
            except (json.JSONDecodeError, TypeError) as e:
                content_data = {"raw_content": content}
            
            # 将解析后的content合并到配置字典中
            if isinstance(content_data, dict):
                config_dict.update(content_data) # 如果是字典，合并到顶层
            elif isinstance(content_data, list):
                config_dict['items'] = content_data # 如果是列表，单独作为一个字段
            else:
                config_dict['value'] = content_data # 其他类型，作为单独字段
            config_dict.pop('content', None) # 移除原始的content字段
            advanced_configs.append(config_dict)   
        return advanced_configs

    @staticmethod
    def save_advanced_config(db: Session, config_data: dict) -> bool:
        try:
            # 获取配置类型
            config_type = config_data.get("type")
            config_name = config_data.get("name")
            
            if not config_type:
                return False
            
            # 根据content字段是否为字符串判断是否需要解析
            content = config_data.get("content")
            
            if isinstance(content, str):
                # 如果content已经是字符串，直接使用
                try:
                    # 验证是否是有效的JSON
                    json.loads(content)
                    content_str = content
                except json.JSONDecodeError:
                    # 如果不是有效JSON，当作普通字符串处理
                    content_str = content
            else:
                # 如果content是字典或列表，转换为JSON字符串
                content_str = json.dumps(content)
            
            # 根据type更新对应记录
            affected_rows = db.query(LogicAdvanceConfig).filter(
                LogicAdvanceConfig.type == config_type
            ).update({
                LogicAdvanceConfig.name: config_name,
                LogicAdvanceConfig.content: content_str
            })
            
            # 如果对应type的记录不存在，可以选择创建新记录
            if affected_rows == 0:
                new_config = LogicAdvanceConfig(
                    type=config_type,
                    name=config_name,
                    content=content_str
                )
                db.add(new_config)
            
            db.commit()
            return True
        except Exception as e:
            db.rollback()
            return False
