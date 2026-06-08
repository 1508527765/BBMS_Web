/**
 * DI/DO 策略 — 与 firmware logic_di.h (di_signal_type_t)、logic_do.h (do_ctrl_type_t) 下标一致。
 * strategy_seq === C 端 logic_idx
 */

export const DIDO_MAX_DI = 40;
export const DIDO_MAX_DO = 40;
export const FIRMWARE_MAX_DI_CONFIG = 12;
export const FIRMWARE_MAX_DO_CONFIG = 12;

/** @type {{ value: number, label: string, enLabel: string }[]} */
export const DIDO_DI_STRATEGIES = [
  { value: 0, label: "无", enLabel: "None" },
  { value: 1, label: "水浸一级告警", enLabel: "Water alarm L1" },
  { value: 2, label: "消防故障", enLabel: "Fire fault" },
  { value: 3, label: "消防一级告警", enLabel: "Fire alarm L1" },
  { value: 4, label: "消防二级告警", enLabel: "Fire alarm L2" },
  { value: 5, label: "可燃气体告警", enLabel: "Gas alarm L1" },
  { value: 6, label: "直流SPD故障", enLabel: "DC SPD fault" },
  { value: 7, label: "集装箱急停", enLabel: "Container E-stop" },
  { value: 8, label: "QF1状态反馈", enLabel: "QF1 feedback" },
  { value: 9, label: "门禁三级告警", enLabel: "Door alarm L3" },
  { value: 10, label: "交流SPD1告警", enLabel: "AC SPD1 alarm" },
  { value: 11, label: "防爆风机故障", enLabel: "Fan fault" },
  { value: 12, label: "汇流熔断器故障", enLabel: "Fuse fault" },
  { value: 13, label: "中控柜行程开关", enLabel: "Ctrl switch alarm" },
  { value: 14, label: "FM200低气压告警", enLabel: "Low pressure alarm" },
  { value: 15, label: "PCS急停", enLabel: "PCS E-stop" },
  { value: 16, label: "配电柜急停", enLabel: "Power E-stop" },
  { value: 17, label: "备电SPD1故障", enLabel: "Backup SPD1 fault" },
  { value: 18, label: "客户外部急停", enLabel: "Customer E-stop" },
];

/** @type {{ value: number, label: string, enLabel: string }[]} */
export const DIDO_DO_STRATEGIES = [
  { value: 0, label: "无", enLabel: "None" },
  { value: 1, label: "强控闭合", enLabel: "Force close" },
  { value: 2, label: "强控断开", enLabel: "Force open" },
  { value: 3, label: "绿灯", enLabel: "Green light" },
  { value: 4, label: "红灯", enLabel: "Red light" },
  { value: 5, label: "黄灯", enLabel: "Yellow light" },
  { value: 6, label: "QF1断路器控制", enLabel: "QF1 control" },
  { value: 7, label: "PCS急停控制", enLabel: "PCS E-stop control" },
];

export function didoStrategyOptions(isZh, kind) {
  const list = kind === "do" ? DIDO_DO_STRATEGIES : DIDO_DI_STRATEGIES;
  return list.map((item) => ({
    value: item.value,
    label: isZh ? item.label : item.enLabel,
  }));
}
