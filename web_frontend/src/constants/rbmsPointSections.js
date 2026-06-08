/**
 * RBMS 测点分区：与 firmware/kernel/kit_model.h devRBMSPoint_e 数值一致（勿人为 +6 偏移）。
 */

/** @type {Readonly<Record<string, number>>} 固件枚举边界（含 End 为下一段 Start，区间用 codeMin..codeMax 闭区间） */
export const RBMS_FW = {
  ONLINE: 0,
  SUMMARY_MAX: 150,
  CELL_BAL_MIN: 151,
  CELL_BAL_MAX: 202,
  CELL_SDR_MIN: 203,
  CELL_SDR_MAX: 618,
  FAULT_MIN: 619,
  FAULT_MAX: 643,
  CELL_VOLT_VALID_MIN: 644,
  CELL_VOLT_VALID_MAX: 695,
  CELL_VOLT_MIN: 696,
  CELL_VOLT_MAX: 1111,
  AFE_VOLT_MIN: 1112,
  AFE_VOLT_MAX: 1143,
  TEMP_CELL_MIN: 1144,
  TEMP_CELL_MAX: 1559,
  TEMP_POLE_MIN: 1560,
  TEMP_POLE_MAX: 1687,
  TEMP_PACK_MIN: 1688,
  TEMP_PACK_MAX: 1703,
  TEMP_BAL_VALID_MIN: 1704,
  TEMP_BAL_VALID_MAX: 1707,
  TEMP_BAL_BOARD_MIN: 1708,
  TEMP_BAL_BOARD_MAX: 1739,
  TMS_MIN: 1740,
  TMS_MAX: 1763,
  SOX_IO_MIN: 1764,
  SOX_IO_MAX: 1834,
  MODEL_DEBUG_MIN: 1835,
  MODEL_DEBUG_MAX: 1860,
  MAX_CELLS: 416,
};

/** RBMS 电芯信息页 UI：8 组 × 52 节/组，组内 4 行 × 13 列；每组 29 路温度 */
export const RBMS_CELL_UI = {
  PACK_SIZE: 52,
  PACK_COUNT: 8,
  TEMP_PER_PACK: 29,
  GRID_COLS: 13,
  GRID_ROWS: 4,
  TEMP_GRID_COLS: 13,
};

export function rbmsPointDedupeKey(p) {
  const r = p || {};
  if (r.code !== undefined && r.code !== null && String(r.code).trim() !== "") {
    return `c:${String(r.code)}`;
  }
  return `n:${String(r.name || "").trim()}|e:${String(r.ename || "").trim()}`;
}

/** @type {{ id: string, codeMin: number, codeMax: number, titleKey: string, hintKey?: string, layout: 'rail'|'kv'|'series' }[]} */
export const RBMS_CODE_SECTIONS = [
  {
    id: "summary",
    codeMin: RBMS_FW.ONLINE,
    codeMax: RBMS_FW.SUMMARY_MAX,
    titleKey: "monitor.rbmsSecSummary",
    hintKey: "monitor.rbmsSecSummaryHint",
    layout: "rail",
  },
  {
    id: "cell_bal",
    codeMin: RBMS_FW.CELL_BAL_MIN,
    codeMax: RBMS_FW.CELL_BAL_MAX,
    titleKey: "monitor.rbmsSecCellBal",
    hintKey: "monitor.rbmsSecCellBalHint",
    layout: "series",
  },
  {
    id: "cell_sdr",
    codeMin: RBMS_FW.CELL_SDR_MIN,
    codeMax: RBMS_FW.CELL_SDR_MAX,
    titleKey: "monitor.rbmsSecCellSdr",
    hintKey: "monitor.rbmsSecCellSdrHint",
    layout: "series",
  },
  {
    id: "fault_array",
    codeMin: RBMS_FW.FAULT_MIN,
    codeMax: RBMS_FW.FAULT_MAX,
    titleKey: "monitor.rbmsSecFaultArray",
    hintKey: "monitor.rbmsSecFaultArrayHint",
    layout: "series",
  },
  {
    id: "cell_volt_valid",
    codeMin: RBMS_FW.CELL_VOLT_VALID_MIN,
    codeMax: RBMS_FW.CELL_VOLT_VALID_MAX,
    titleKey: "monitor.rbmsSecCellVoltValid",
    hintKey: "monitor.rbmsSecCellVoltValidHint",
    layout: "series",
  },
  {
    id: "cell_volt",
    codeMin: RBMS_FW.CELL_VOLT_MIN,
    codeMax: RBMS_FW.CELL_VOLT_MAX,
    titleKey: "monitor.rbmsSecCellVolt",
    hintKey: "monitor.rbmsSecCellVoltHint",
    layout: "series",
  },
  {
    id: "afe_volt",
    codeMin: RBMS_FW.AFE_VOLT_MIN,
    codeMax: RBMS_FW.AFE_VOLT_MAX,
    titleKey: "monitor.rbmsSecAfeVolt",
    hintKey: "monitor.rbmsSecAfeVoltHint",
    layout: "series",
  },
  {
    id: "temp_cell",
    codeMin: RBMS_FW.TEMP_CELL_MIN,
    codeMax: RBMS_FW.TEMP_CELL_MAX,
    titleKey: "monitor.rbmsSecTempCell",
    hintKey: "monitor.rbmsSecTempCellHint",
    layout: "series",
  },
  {
    id: "temp_pole",
    codeMin: RBMS_FW.TEMP_POLE_MIN,
    codeMax: RBMS_FW.TEMP_POLE_MAX,
    titleKey: "monitor.rbmsSecTempPole",
    hintKey: "monitor.rbmsSecTempPoleHint",
    layout: "series",
  },
  {
    id: "temp_pack",
    codeMin: RBMS_FW.TEMP_PACK_MIN,
    codeMax: RBMS_FW.TEMP_PACK_MAX,
    titleKey: "monitor.rbmsSecTempPack",
    hintKey: "monitor.rbmsSecTempPackHint",
    layout: "series",
  },
  {
    id: "temp_bal_valid",
    codeMin: RBMS_FW.TEMP_BAL_VALID_MIN,
    codeMax: RBMS_FW.TEMP_BAL_VALID_MAX,
    titleKey: "monitor.rbmsSecTempBalValid",
    hintKey: "monitor.rbmsSecTempBalValidHint",
    layout: "series",
  },
  {
    id: "temp_bal_board",
    codeMin: RBMS_FW.TEMP_BAL_BOARD_MIN,
    codeMax: RBMS_FW.TEMP_BAL_BOARD_MAX,
    titleKey: "monitor.rbmsSecTempBalBoard",
    hintKey: "monitor.rbmsSecTempBalBoardHint",
    layout: "series",
  },
  {
    id: "tms_cluster",
    codeMin: RBMS_FW.TMS_MIN,
    codeMax: RBMS_FW.TMS_MAX,
    titleKey: "monitor.rbmsSecTmsCluster",
    hintKey: "monitor.rbmsSecTmsClusterHint",
    layout: "kv",
  },
  {
    id: "sox_io",
    codeMin: RBMS_FW.SOX_IO_MIN,
    codeMax: RBMS_FW.SOX_IO_MAX,
    titleKey: "monitor.rbmsSecSoxIo",
    hintKey: "monitor.rbmsSecSoxIoHint",
    layout: "kv",
  },
  {
    id: "model_debug",
    codeMin: RBMS_FW.MODEL_DEBUG_MIN,
    codeMax: RBMS_FW.MODEL_DEBUG_MAX,
    titleKey: "monitor.rbmsSecModelDebug",
    hintKey: "monitor.rbmsSecModelDebugHint",
    layout: "kv",
  },
];

/** 合并「电芯信息」Tab 内的分区（含簇级电芯统计量，来自 summary 子集） */
export const RBMS_CELL_INFO_SECTION_IDS = [
  "cell_volt_valid",
  "cell_volt",
  "afe_volt",
  "temp_cell",
  "temp_pole",
  "temp_pack",
  "temp_bal_valid",
  "temp_bal_board",
  "cell_bal",
  "cell_sdr",
];

export const RBMS_SECTION_META = RBMS_CODE_SECTIONS.reduce(
  (acc, s) => {
    acc[s.id] = { ...s, titleKey: s.titleKey, hintKey: s.hintKey || "", layout: s.layout };
    return acc;
  },
  { misc: { titleKey: "monitor.rbmsSecMisc", hintKey: "monitor.rbmsSecMiscHint", layout: "kv" } }
);

export const RBMS_SECTION_ORDER = [...RBMS_CODE_SECTIONS.map((s) => s.id), "misc"];

export const RBMS_HERO_SLOTS = [
  { key: "online", code: 0, accent: "teal" },
  { key: "voltage", code: 6, accent: "violet" },
  { key: "current", code: 8, accent: "amber" },
  { key: "soc", code: 27, accent: "emerald" },
  { key: "soh", code: 28, accent: "sky" },
  { key: "tmax", code: 44, accent: "orange" },
  { key: "tmin", code: 46, accent: "cyan" },
  { key: "iso", code: 9, accent: "indigo" },
];

export function resolveRbmsSectionId(p) {
  const code = Number(p && p.code);
  if (Number.isNaN(code)) return "misc";
  for (let i = 0; i < RBMS_CODE_SECTIONS.length; i++) {
    const s = RBMS_CODE_SECTIONS[i];
    if (code >= s.codeMin && code <= s.codeMax) return s.id;
  }
  return "misc";
}

export function rbmsPointMatchText(p) {
  return `${(p && p.ename) || ""} ${(p && p.name) || ""}`.trim();
}

/** 0-based 电芯序号；无法解析返回 null */
export function rbmsCellIndex0FromCode(code) {
  const c = Number(code);
  if (Number.isNaN(c)) return null;
  if (c >= RBMS_FW.CELL_VOLT_MIN && c <= RBMS_FW.CELL_VOLT_MAX) return c - RBMS_FW.CELL_VOLT_MIN;
  if (c >= RBMS_FW.TEMP_CELL_MIN && c <= RBMS_FW.TEMP_CELL_MAX) return c - RBMS_FW.TEMP_CELL_MIN;
  if (c >= RBMS_FW.CELL_SDR_MIN && c <= RBMS_FW.CELL_SDR_MAX) return c - RBMS_FW.CELL_SDR_MIN;
  return null;
}

export function rbmsBalBitmapCodeForCellIndex0(cellIndex0) {
  if (cellIndex0 < 0 || cellIndex0 >= RBMS_FW.MAX_CELLS) return null;
  return RBMS_FW.CELL_BAL_MIN + Math.floor(cellIndex0 / 8);
}

export function rbmsBalBitForCellIndex0(cellIndex0) {
  return cellIndex0 % 8;
}

function pointByCodeMap(points) {
  const map = {};
  (points || []).forEach((p) => {
    const c = Number(p && p.code);
    if (!Number.isNaN(c)) map[c] = p;
  });
  return map;
}

function displayVal(p) {
  if (!p || p.value === undefined || p.value === null || String(p.value).trim() === "") return null;
  return p.value;
}

function parseBalancingFromPoint(p, bit) {
  const v = displayVal(p);
  if (v === null) return null;
  const n = Number(v);
  if (!Number.isNaN(n)) return ((n >> bit) & 1) === 1;
  const s = String(v).trim();
  if (/^[01]+$/.test(s) && bit < s.length) return s[s.length - 1 - bit] === "1";
  return null;
}

/**
 * 由当前测点列表构建电芯网格（0-based index，展示时 +1）。
 * @param {object[]} points
 * @param {number} [cellCount]
 */
export function buildRbmsCellGrid(points, cellCount) {
  const byCode = pointByCodeMap(points);
  let maxIdx = -1;
  Object.keys(byCode).forEach((k) => {
    const idx = rbmsCellIndex0FromCode(Number(k));
    if (idx !== null && idx > maxIdx) maxIdx = idx;
  });
  if (maxIdx < 0 && !(cellCount > 0)) return [];
  const n = Math.min(RBMS_FW.MAX_CELLS, Math.max(cellCount || 0, maxIdx + 1));
  const cells = [];
  for (let i = 0; i < n; i++) {
    const voltP = byCode[RBMS_FW.CELL_VOLT_MIN + i];
    const tempP = byCode[RBMS_FW.TEMP_CELL_MIN + i];
    const sdrP = byCode[RBMS_FW.CELL_SDR_MIN + i];
    const balCode = rbmsBalBitmapCodeForCellIndex0(i);
    const balP = balCode != null ? byCode[balCode] : null;
    const balBit = rbmsBalBitForCellIndex0(i);
    cells.push({
      index0: i,
      index1: i + 1,
      volt: voltP || null,
      temp: tempP || null,
      sdr: sdrP || null,
      balancing: parseBalancingFromPoint(balP, balBit),
      voltValue: displayVal(voltP),
      tempValue: displayVal(tempP),
      sdrValue: displayVal(sdrP),
    });
  }
  return cells;
}

/**
 * 按分组构建温度模块（每组 29 路，与 52 节电芯分组对齐）。
 * @param {object[]} points
 * @param {number} packIndex0 0-based 分组序号
 */
export function buildRbmsPackTempGrid(points, packIndex0) {
  const byCode = pointByCodeMap(points);
  const pack = Math.max(0, Number(packIndex0) || 0);
  const base = pack * RBMS_CELL_UI.TEMP_PER_PACK;
  const temps = [];
  for (let t = 0; t < RBMS_CELL_UI.TEMP_PER_PACK; t++) {
    const code = RBMS_FW.TEMP_CELL_MIN + base + t;
    const tempP = byCode[code];
    temps.push({
      index0: base + t,
      index1: t + 1,
      packIndex0: pack,
      temp: tempP || null,
      tempValue: displayVal(tempP),
    });
  }
  return temps;
}

export const RBMS_MAIN_TABS = [
  {
    id: "overview",
    labelKey: "monitor.rbmsTabOverview",
    sectionIds: ["summary"],
  },
  {
    id: "cell_info",
    labelKey: "monitor.rbmsTabCellInfo",
    sectionIds: RBMS_CELL_INFO_SECTION_IDS,
    cellGrid: true,
  },
  {
    id: "fault",
    labelKey: "monitor.rbmsTabFault",
    faultDedicated: true,
    sectionIds: ["fault_array"],
  },
  {
    id: "thermal",
    labelKey: "monitor.rbmsTabThermal",
    sectionIds: ["tms_cluster"],
  },
  {
    id: "sox",
    labelKey: "monitor.rbmsTabSox",
    sectionIds: ["sox_io", "model_debug"],
  },
  { id: "flat", labelKey: "monitor.rbmsTabFlatList", flat: true },
];
