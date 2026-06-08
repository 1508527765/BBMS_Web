/**
 * 测点分组类型（与点表 group_type / DeviceFormDialog 一致）
 * 0 遥信 · 1 遥测 · 2 遥控 · 3 遥调 · 4 设置 · 5 遥脉 · 6 故障 · 7 事件
 */

export const POINT_GROUP_TYPES = [
  { value: 0, labelKey: "monitor.groupYx", hintKey: "monitor.groupYxHint", accent: "yx" },
  { value: 1, labelKey: "monitor.groupYc", hintKey: "monitor.groupYcHint", accent: "yc" },
  { value: 2, labelKey: "monitor.groupYk", hintKey: "monitor.groupYkHint", accent: "yk" },
  { value: 3, labelKey: "monitor.groupYt", hintKey: "monitor.groupYtHint", accent: "yt" },
  { value: 4, labelKey: "monitor.groupSet", hintKey: "monitor.groupSetHint", accent: "set" },
  { value: 5, labelKey: "monitor.groupYm", hintKey: "monitor.groupYmHint", accent: "ym" },
  { value: 6, labelKey: "monitor.groupFault", hintKey: "monitor.groupFaultHint", accent: "fault" },
  { value: 7, labelKey: "monitor.groupEvent", hintKey: "monitor.groupEventHint", accent: "event" },
];

export const POINT_GROUP_ORDER = POINT_GROUP_TYPES.map((g) => g.value);

export function normalizePointGroupType(raw) {
  if (raw === null || raw === undefined || raw === "") return null;
  const n = Number(raw);
  return Number.isNaN(n) ? null : n;
}

export function resolvePointGroupType(p) {
  const r = p || {};
  const raw =
    r.groupType !== undefined && r.groupType !== null && r.groupType !== ""
      ? r.groupType
      : r.group_type;
  return normalizePointGroupType(raw);
}

export function pointGroupMeta(groupType) {
  const n = normalizePointGroupType(groupType);
  if (n === null) return null;
  return POINT_GROUP_TYPES.find((g) => g.value === n) || null;
}

/** 可通过 RTDB 下发的分组：遥控、遥调（与 C point_group_type_e / 点表 Group Type 一致） */
export const WRITABLE_POINT_GROUP_TYPES = new Set([2, 3]);

export function isPointGroupWritable(groupType) {
  const n = normalizePointGroupType(groupType);
  return n !== null && WRITABLE_POINT_GROUP_TYPES.has(n);
}
