"""
DI/DO 策略字典 — 与 firmware logic_di.h (di_signal_type_t)、logic_do.h (do_ctrl_type_t) 下标一致。
strategy_seq 即 C 端 logic_idx。
"""

from typing import List, Tuple

# (strategy_seq, strategy_name, strategy_ename)
CANONICAL_DI_STRATEGIES: List[Tuple[int, str, str]] = [
    (0, "无", "none"),
    (1, "水浸一级告警", "water_alarm_level1"),
    (2, "消防故障", "fire_fault"),
    (3, "消防一级告警", "fire_alarm_level1"),
    (4, "消防二级告警", "fire_alarm_level2"),
    (5, "可燃气体告警", "gas_alarm_level1"),
    (6, "直流SPD故障", "dc_spd_fault"),
    (7, "集装箱急停", "container_estop"),
    (8, "QF1状态反馈", "qf1_feedback"),
    (9, "门禁三级告警", "door_alarm_level3"),
    (10, "交流SPD1告警", "ac_spd1_alarm"),
    (11, "防爆风机故障", "fan_fault"),
    (12, "汇流熔断器故障", "fuse_fault"),
    (13, "中控柜行程开关", "ctrl_switch_alarm"),
    (14, "FM200低气压告警", "low_pressure_alarm"),
    (15, "PCS急停", "pcs_estop"),
    (16, "配电柜急停", "power_estop"),
    (17, "备电SPD1故障", "backup_spd1_fault"),
    (18, "客户外部急停", "cust_ext_estop"),
]

CANONICAL_DO_STRATEGIES: List[Tuple[int, str, str]] = [
    (0, "无", "none"),
    (1, "强控闭合", "force_on"),
    (2, "强控断开", "force_off"),
    (3, "绿灯", "green_light"),
    (4, "红灯", "red_light"),
    (5, "黄灯", "yellow_light"),
    (6, "QF1断路器控制", "qf1_control"),
    (7, "PCS急停控制", "pcs_estop_control"),
]

# kit_model.h: DI1–DI40 / DO1–DO40
DIDO_MAX_DI_CHANNELS = 40
DIDO_MAX_DO_CHANNELS = 40

# logic_di.h / logic_do.h 当前配置数组上限（C 端尚未全部打通 DI 加载）
FIRMWARE_MAX_DI_CONFIG_NUM = 12
FIRMWARE_MAX_DO_CONFIG_NUM = 12
