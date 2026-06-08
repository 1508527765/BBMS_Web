import { resolvePointGroupType, isPointGroupWritable } from "./pointGroupTypes";

/**
 * BBMS 测点按协议枚举语义分组（匹配 ename / name，兼容库中略写）。
 * 规则顺序：先匹配更具体的段，避免被宽泛规则抢走。
 */

export function bbmsPointMatchText(p) {
  return `${(p && p.ename) || ""} ${(p && p.name) || ""}`.trim();
}

/** 与顶栏去重：同一条测点只展示一处 */
export function bbmsPointDedupeKey(p) {
  const r = p || {};
  if (r.code !== undefined && r.code !== null && String(r.code).trim() !== "") {
    return `c:${String(r.code)}`;
  }
  return `n:${String(r.name || "").trim()}|e:${String(r.ename || "").trim()}`;
}

/** 提取末尾数字用于 Rack/Cell 序列排序 */
export function bbmsSeriesSortKey(p) {
  const raw = `${(p && p.ename) || ""} ${(p && p.code) || ""}`;
  const m = raw.match(/_(\d+)\s*$/);
  if (m) return Number(m[1]);
  const m2 = raw.match(/(\d+)\s*$/);
  return m2 ? Number(m2[1]) : 0;
}

export function sortBbmsSeriesRows(rows) {
  return [...(rows || [])].sort((a, b) => bbmsSeriesSortKey(a) - bbmsSeriesSortKey(b));
}

/**
 * @typedef {{ id: string, re: RegExp }} BbmsRule
 * @type {BbmsRule[]}
 */
const BBMS_SECTION_RULES = [
  { id: "ems_ctrl", re: /EMSCtrlPower|A_EMSCtrl|CtlWord|控制字|上下高压/i },
  { id: "fault_enable", re: /FaultEnaFlg|故障使能/i },
  {
    id: "bank_cali",
    re: /BankErrorCnt|RackAllFltHistAccu|RackFltHistInfo|故障累积|越限次数标定/i,
  },
  { id: "hmi_bbms_do", re: /HMI_BBMSDO|Hmi_BBMSDO|BBMSDO\d|DO\d+控制/i },
  { id: "hmi_do_lamp", re: /Hmi_DO_Control|DOManCtrl|强控DO|三级板DO/i },
  { id: "hmi_tms", re: /Hmi_TMSCtrl|TMSManCtrl|强控TMS|TMSCtrlWord|TMS控制指令/i },
  {
    id: "hmi_bank",
    re: /Hmi_(Alm|Set|EP|Ena|Loc|PCS|Pmt|ID|Auto|Con|Ins)|Hmi_Control|故障复位|绝缘检测|电池串使能|EPO|控制模式/i,
  },
  { id: "ctrl_tms_thr", re: /Crtl_TMS_|设置.*阈值|制热启动|制冷A启动/i },
  { id: "para_tms_thr", re: /CcTHMC_|ParaThr_TMS|参数阈值.*TMS/i },
  {
    id: "tms_runtime",
    re: /TMS_|TmsSum|BBMS_cellTmp|BBMS_WtrTmp|BMS_TMS|TMSCool|进出水|水压|环境温度|设定温度|TMS故障|TMS工作|TMS编号/i,
  },
  {
    id: "soh_arrays",
    re: /SaSOHB_|A_SOHInfo|DFCLCap|MFCLCap|BankCellCap|RackRealCapAh|RackMaxCapAh|RackMinCapAh|电芯容量|DFCL|MFCL/i,
  },
  {
    id: "soc_arrays",
    re: /SaSOCB_|A_SOCInfo|BankCellSOC|RackRealSOC|RackMaxSOC|RackMinSOC|等间隔.*SOC|Rack.*SOC\[/i,
  },
  { id: "flt_ov_count", re: /FltOvTiNbr|BBMS_FltOvTi|越限次数统计|故障越限/i },
  { id: "acore_fault", re: /ACore_Fault|A核障/i },
  { id: "fault_list", re: /FaultList|kBbms_Flt_\d|\bFlt_\d|故障\d|Fault\s*\d/i },
  {
    id: "self_discharge",
    re: /Selfdr|SbEMCR|ScEMCR|自放电|CellUsedMonth|CellDischargeRate/i,
  },
  {
    id: "cumulative",
    re: /AccuChrg|AccuDis|LstCh|LstDch|最近一次|累计充|累计放|可充容量|可放容量|Remaining.*Cap|MaxChrgCap|MaxDisChrgCap/i,
  },
  { id: "insulation", re: /IsoR|绝缘|Insulation/i },
  { id: "rack_summary", re: /RackEnaNbr|RackOnlineNbr|Rack使能|Rack在线数量|在网簇数/i },
  {
    id: "temp_pole_hv",
    re: /MaxPoleT|MinPoleT|MaxPoleT|MinPoleT|MaxHvBoxT|MinHvBoxT|MaxTCell|MinTCell|MaxHvBox|MinHvBox|极柱温度|高压箱温度|电芯温度.*Rack/i,
  },
  {
    id: "cell_volt",
    re: /MaxCellV|MinCellV|AvgCellV|MaxVCell|MinVCell|电芯电压|Cell\s*Voltage|mV\)/i,
  },
  {
    id: "bank_status",
    re: /RackFltLvl1Nbr|Bbms_RackFltLvl1|存在严重故障的Rack/i,
  },
  { id: "rack_current", re: /MaxRackCur|MinRackCur|CurRackNr|Rack电流|MaxCurRackNr|MinCurRackNr/i },
  { id: "dc_power", re: /DCBus|TotalDCCur|TotalPower|母线|总功率|总电流/i },
  {
    id: "power_limits",
    re: /MaxCont|Max60s|ChrgPower|DisChrgPower|ChrgCur|DisChrgCur|功率限制|电流限制|Pulse|Const/i,
  },
  {
    id: "soc_core",
    re: /kBbms_SOC\b|系统SOC|Bank.*SOC(?!B)|\bSOC\s*\(%|kBbms_SOH\b|系统SOH|可充能量|可放能量|MaxChrgE|MaxDisChrgE|SOE/i,
  },
  { id: "env_aux", re: /HumSensor|TmpSensor|湿度|Bank温度|水浸|Heartbeat|心跳/i },
  {
    id: "fault_meta",
    re: /ProhUpgrade|BMSMaxFlt|FaultLevel|故障等级|IDLearn|编码状态|编码失败|ConFail|高压连接失败/i,
  },
  {
    id: "bank_status",
    re: /kBbms_Online\b|BatSt|ChSt|SysOperSt|CntrSt|PmtRk|SetInverter|接触器|Bank工作状态|堆.*工作状态|充放电状态|系统运行状态|总合闸|分闸/i,
  },
];

export const BBMS_SECTION_ORDER = [
  "bank_status",
  "soc_core",
  "power_limits",
  "dc_power",
  "rack_current",
  "cell_volt",
  "temp_pole_hv",
  "rack_enable_insulation",
  "cumulative",
  "env_aux",
  "fault_meta",
  "self_discharge",
  "fault_list",
  "acore_fault",
  "flt_ov_count",
  "soc_arrays",
  "soh_arrays",
  "tms_runtime",
  "para_tms_thr",
  "ctrl_tms_thr",
  "hmi_bank",
  "hmi_tms",
  "hmi_do_lamp",
  "hmi_bbms_do",
  "bank_cali",
  "fault_enable",
  "ems_ctrl",
  "misc",
];

/** @type {Record<string, { titleKey: string; hintKey?: string; layout: 'rail'|'kv'|'series' }>} */
export const BBMS_SECTION_META = {
  bank_status: {
    titleKey: "monitor.bbmsSecBankStatus",
    hintKey: "monitor.bbmsSecBankStatusHint",
    layout: "rail",
  },
  soc_core: {
    titleKey: "monitor.bbmsSecSocSoh",
    hintKey: "monitor.bbmsSecSocSohHint",
    layout: "rail",
  },
  power_limits: {
    titleKey: "monitor.bbmsSecPowerLimits",
    hintKey: "monitor.bbmsSecPowerLimitsHint",
    layout: "kv",
  },
  dc_power: {
    titleKey: "monitor.bbmsSecDcPower",
    hintKey: "monitor.bbmsSecDcPowerHint",
    layout: "rail",
  },
  rack_current: {
    titleKey: "monitor.bbmsSecRackCurrent",
    hintKey: "monitor.bbmsSecRackCurrentHint",
    layout: "kv",
  },
  cell_volt: {
    titleKey: "monitor.bbmsSecCellVoltage",
    hintKey: "monitor.bbmsSecCellVoltageHint",
    layout: "kv",
  },
  temp_pole_hv: {
    titleKey: "monitor.bbmsSecTemperature",
    hintKey: "monitor.bbmsSecTemperatureHint",
    layout: "kv",
  },
  rack_summary: {
    titleKey: "monitor.bbmsSecRackSummary",
    hintKey: "monitor.bbmsSecRackSummaryHint",
    layout: "rail",
  },
  insulation: {
    titleKey: "monitor.bbmsSecInsulation",
    hintKey: "monitor.bbmsSecInsulationHint",
    layout: "kv",
  },
  rack_enable_insulation: {
    titleKey: "monitor.bbmsSecRackEnableInsulation",
    hintKey: "monitor.bbmsSecRackEnableInsulationHint",
    layout: "rail",
  },
  cumulative: {
    titleKey: "monitor.bbmsSecCumulative",
    hintKey: "monitor.bbmsSecCumulativeHint",
    layout: "kv",
  },
  env_aux: {
    titleKey: "monitor.bbmsSecEnvAux",
    hintKey: "monitor.bbmsSecEnvAuxHint",
    layout: "rail",
  },
  fault_meta: {
    titleKey: "monitor.bbmsSecFaultMeta",
    hintKey: "monitor.bbmsSecFaultMetaHint",
    layout: "rail",
  },
  self_discharge: {
    titleKey: "monitor.bbmsSecSelfDischarge",
    hintKey: "monitor.bbmsSecSelfDischargeHint",
    layout: "kv",
  },
  fault_list: {
    titleKey: "monitor.bbmsSecFaultList",
    hintKey: "monitor.bbmsSecFaultListHint",
    layout: "series",
  },
  acore_fault: {
    titleKey: "monitor.bbmsSecAcoreFault",
    hintKey: "monitor.bbmsSecAcoreFaultHint",
    layout: "series",
  },
  flt_ov_count: {
    titleKey: "monitor.bbmsSecFltOvCount",
    hintKey: "monitor.bbmsSecFltOvCountHint",
    layout: "series",
  },
  soc_arrays: {
    titleKey: "monitor.bbmsSecSocArrays",
    hintKey: "monitor.bbmsSecSocArraysHint",
    layout: "series",
  },
  soh_arrays: {
    titleKey: "monitor.bbmsSecSohArrays",
    hintKey: "monitor.bbmsSecSohArraysHint",
    layout: "series",
  },
  tms_runtime: {
    titleKey: "monitor.bbmsSecTmsRuntime",
    hintKey: "monitor.bbmsSecTmsRuntimeHint",
    layout: "kv",
  },
  para_tms_thr: {
    titleKey: "monitor.bbmsSecTmsThrRead",
    hintKey: "monitor.bbmsSecTmsThrReadHint",
    layout: "kv",
  },
  ctrl_tms_thr: {
    titleKey: "monitor.bbmsSecTmsThrWrite",
    hintKey: "monitor.bbmsSecTmsThrWriteHint",
    layout: "kv",
  },
  hmi_bank: {
    titleKey: "monitor.bbmsSecHmiBank",
    hintKey: "monitor.bbmsSecHmiBankHint",
    layout: "kv",
  },
  hmi_tms: {
    titleKey: "monitor.bbmsSecHmiTms",
    hintKey: "monitor.bbmsSecHmiTmsHint",
    layout: "kv",
  },
  hmi_do_lamp: {
    titleKey: "monitor.bbmsSecHmiDo",
    hintKey: "monitor.bbmsSecHmiDoHint",
    layout: "kv",
  },
  hmi_bbms_do: {
    titleKey: "monitor.bbmsSecHmiBbmsDo",
    hintKey: "monitor.bbmsSecHmiBbmsDoHint",
    layout: "series",
  },
  bank_cali: {
    titleKey: "monitor.bbmsSecBankCali",
    hintKey: "monitor.bbmsSecBankCaliHint",
    layout: "series",
  },
  fault_enable: {
    titleKey: "monitor.bbmsSecFaultEnable",
    hintKey: "monitor.bbmsSecFaultEnableHint",
    layout: "series",
  },
  ems_ctrl: {
    titleKey: "monitor.bbmsSecEmsCtrl",
    hintKey: "monitor.bbmsSecEmsCtrlHint",
    layout: "kv",
  },
  misc: {
    titleKey: "monitor.bbmsSecMisc",
    hintKey: "monitor.bbmsSecMiscHint",
    layout: "kv",
  },
};

export function resolveBbmsSectionId(p) {
  const t = bbmsPointMatchText(p);
  if (!t) return "misc";
  for (let i = 0; i < BBMS_SECTION_RULES.length; i++) {
    if (BBMS_SECTION_RULES[i].re.test(t)) return BBMS_SECTION_RULES[i].id;
  }
  if (/bbms|kbbms|堆控|bank|机架|rack/i.test(t)) return "misc";
  return "misc";
}

/** 顶栏大卡：按枚举核心量抓取（有则显示） */
export const BBMS_HERO_RULES = [
  { key: "soc", re: /kBbms_SOC\b|\bSOC\s*\(%|系统SOC/i, accent: "emerald" },
  { key: "soh", re: /kBbms_SOH\b|\bSOH\s*\(%|系统SOH/i, accent: "sky" },
  { key: "dcv", re: /DCBusV|母线电压/i, accent: "violet" },
  { key: "dct", re: /TotalDCCur|母线电流/i, accent: "amber" },
  { key: "tpw", re: /TotalPower|总功率/i, accent: "orange" },
  { key: "online", re: /kBbms_Online|堆控在线/i, accent: "teal" },
];

/** 紧接「堆控在线」之后展示的核心状态量（样式与堆控在线一致） */
export const BBMS_HERO_SECONDARY_RULES = [
  { key: "str_hb", re: /StrCtrlHb|BMS心跳信号|Bank BMS Heartbeat/i, accent: "teal" },
  { key: "sys_oper", re: /SysOperSt|Bank系统运行状态|System Operating Status/i, accent: "indigo" },
  { key: "pmt_rk", re: /PmtRkFltNum|允许运行的最小Rack|Min number of Racks Allowed/i, accent: "slate" },
];

export function bbmsPointIsStatusHeroPromoted(p) {
  const t = bbmsPointMatchText(p);
  if (!t) return false;
  return (BBMS_HERO_SECONDARY_RULES || []).some((r) => r.re.test(t));
}

/** 顶部统计区独占的分区（下方 Tab 不再重复展示） */
export const BBMS_STAT_SECTION_IDS = [
  "soc_core",
  "bank_status",
  "rack_enable_insulation",
  "dc_power",
  "power_limits",
  "rack_current",
  "cumulative",
  "env_aux",
];

/**
 * 统计区分组：从对应语义分区取点，前端再按 gauge/stat/chip 多样化渲染。
 * @type {{ id: string, titleKey: string, tone: string, sectionIds: string[] }[]}
 */
export const BBMS_STAT_GROUPS = [
  {
    id: "energy",
    titleKey: "monitor.bbmsStatEnergy",
    tone: "energy",
    sectionIds: ["soc_core", "cumulative"],
  },
  {
    id: "power",
    titleKey: "monitor.bbmsStatPower",
    tone: "power",
    sectionIds: ["dc_power", "power_limits", "rack_current"],
  },
  {
    id: "status",
    titleKey: "monitor.bbmsStatStatus",
    tone: "status",
    sectionIds: ["bank_status", "rack_enable_insulation", "env_aux"],
  },
];

/**
 * 详情 Tab：相关语义分区归入同一页。
 * @type {{ id: string, labelKey: string, sectionIds?: string[], flat?: boolean, alarms?: boolean }[]}
 */
export const BBMS_MAIN_TABS = [
  {
    id: "runtime",
    labelKey: "monitor.bbmsTabRuntime",
    sectionIds: ["cell_volt", "temp_pole_hv", "self_discharge"],
  },
  {
    id: "fault",
    labelKey: "monitor.bbmsTabFault",
    faultDedicated: true,
    sectionIds: [
      "fault_meta",
      "fault_list",
      "acore_fault",
      "flt_ov_count",
      "fault_enable",
      "bank_cali",
    ],
  },
  {
    id: "arrays",
    labelKey: "monitor.bbmsTabArrays",
    sectionIds: ["soc_arrays", "soh_arrays"],
  },
  {
    id: "thermal",
    labelKey: "monitor.bbmsTabThermal",
    sectionIds: ["tms_runtime", "para_tms_thr", "ctrl_tms_thr", "hmi_tms"],
  },
  {
    id: "control",
    labelKey: "monitor.bbmsTabControl",
    sectionIds: ["hmi_bank", "hmi_do_lamp", "hmi_bbms_do", "ems_ctrl"],
  },
  { id: "other", labelKey: "monitor.bbmsTabOther", sectionIds: ["misc"] },
  { id: "flat", labelKey: "monitor.bbmsTabFlatList", flat: true },
];

export function bbmsTabDefById(tabId) {
  return (BBMS_MAIN_TABS || []).find((t) => t.id === tabId) || null;
}

/** 热管理 Tab 内允许 Web 下发的语义分区 */
export const BBMS_THERMAL_WRITABLE_SECTION_IDS = new Set(["ctrl_tms_thr", "hmi_tms"]);

export function bbmsSectionSupportsThermalWrite(sectionId) {
  return BBMS_THERMAL_WRITABLE_SECTION_IDS.has(sectionId);
}

function bbmsThermalPointReserved(p) {
  const t = bbmsPointMatchText(p);
  if (!t || /预留|Reserved/i.test(t)) return true;
  const en = String((p && p.ename) || "");
  if (/Crtl_TMS_Start_\d|Crtl_TMS_Reserved|TMSCtrlReserved/i.test(en)) return true;
  return false;
}

/** 热管理 Tab 内单点是否展示下发控件（按分区 + 排除预留） */
export function bbmsThermalPointWritable(p, sectionId) {
  if (!bbmsSectionSupportsThermalWrite(sectionId)) return false;
  if (bbmsThermalPointReserved(p)) return false;
  return true;
}

/** 热管理下发控件：遥控 switch / 遥调 number */
export function bbmsThermalPointControlKind(p) {
  const g = resolvePointGroupType(p);
  if (g === 2) return "yk";
  if (g === 3) return "yt";
  const en = String((p && p.ename) || "");
  if (/TMSCtrlWord_Start_1|TMSCtrlWord_Start_3|TMSManCtrlEna/i.test(en)) return "yk";
  return "yt";
}

/** 从点表推断是否属于热管理可下发测点（草稿同步用） */
export function bbmsThermalDispatchPoint(p) {
  if (bbmsThermalPointReserved(p)) return false;
  if (isPointGroupWritable(resolvePointGroupType(p))) return true;
  const en = String((p && p.ename) || "");
  return /^Bbms_Crtl_TMS_/.test(en) || /^Bbms_Hmi_TMSCtrlWord_Start_[1-4]$/.test(en);
}

/** 故障 Tab 大数组每段条数（FltOvTiNbr、RackAllFltHistAccuCaliVal 等） */
export const BBMS_FAULT_PANEL_BUCKET_SIZE = 50;

/** 故障面板固定分组（非序号分桶） */
export const BBMS_FAULT_PANEL_STATIC_META = {
  fault_meta: {
    titleKey: "monitor.bbmsSecFaultMeta",
    hintKey: "monitor.bbmsSecFaultMetaHint",
  },
  fault_list: {
    titleKey: "monitor.bbmsSecFaultList",
    hintKey: "monitor.bbmsSecFaultListHint",
  },
  acore_fault: {
    titleKey: "monitor.bbmsSecAcoreFault",
    hintKey: "monitor.bbmsSecAcoreFaultHint",
  },
  rack_fault: {
    titleKey: "monitor.bbmsFaultSecRackFault",
    hintKey: "monitor.bbmsFaultSecRackFaultHint",
  },
  tms_fault: {
    titleKey: "monitor.bbmsFaultSecTmsFault",
    hintKey: "monitor.bbmsFaultSecTmsFaultHint",
  },
  rack_hist_indicator: {
    titleKey: "monitor.bbmsFaultSecRackHistIndicator",
    hintKey: "monitor.bbmsFaultSecRackHistIndicatorHint",
  },
  fault_enable: {
    titleKey: "monitor.bbmsSecFaultEnable",
    hintKey: "monitor.bbmsSecFaultEnableHint",
  },
  bank_cali: {
    titleKey: "monitor.bbmsSecBankCali",
    hintKey: "monitor.bbmsSecBankCaliHint",
  },
  fault_other: {
    titleKey: "monitor.bbmsFaultSecOther",
    hintKey: "monitor.bbmsFaultSecOtherHint",
  },
};

const BBMS_FAULT_PANEL_STATIC_ORDER = [
  "fault_meta",
  "fault_list",
  "acore_fault",
  "rack_fault",
  "tms_fault",
  "rack_hist_indicator",
  "fault_enable",
  "bank_cali",
  "fault_other",
];

function bbmsFaultIndexedBucket(p, indexRe, prefix) {
  const t = bbmsPointMatchText(p);
  const m = t.match(indexRe);
  if (!m) return null;
  const idx = Number(m[1]);
  if (Number.isNaN(idx)) return null;
  const start = Math.floor(idx / BBMS_FAULT_PANEL_BUCKET_SIZE) * BBMS_FAULT_PANEL_BUCKET_SIZE;
  const end = start + BBMS_FAULT_PANEL_BUCKET_SIZE - 1;
  return {
    id: `${prefix}_${start}_${end}`,
    rangeStart: start,
    rangeEnd: end,
    titleKey:
      prefix === "flt_ov"
        ? "monitor.bbmsFaultSecFltOvRange"
        : "monitor.bbmsFaultSecRackHistRange",
  };
}

/**
 * 故障 Tab 专用分组（细于详情页分区，大数组按 50 条分桶）。
 * @returns {{ id: string, rangeStart?: number, rangeEnd?: number, titleKey?: string }}
 */
export function resolveBbmsFaultPanelGroupId(p) {
  const t = bbmsPointMatchText(p);
  if (!t) return { id: "fault_other" };
  if (/ProhUpgrade|BMSMaxFlt|FaultLevel|故障等级|RackFltLvl|编码状态|编码失败|ConFail|高压连接失败/i.test(t)) {
    return { id: "fault_meta" };
  }
  if (/ACore_Fault|A核障/i.test(t)) return { id: "acore_fault" };
  if (/FaultEnaFlg|故障使能/i.test(t)) return { id: "fault_enable" };
  if (/RackFltHistInfoIndicator|故障历史指示/i.test(t)) return { id: "rack_hist_indicator" };

  const histBucket = bbmsFaultIndexedBucket(p, /RackAllFltHistAccuCaliVal[_\s]*(\d+)/i, "rack_hist_cali");
  if (histBucket) return histBucket;

  const ovBucket = bbmsFaultIndexedBucket(p, /FltOvTiNbr[_\s]*(\d+)/i, "flt_ov");
  if (ovBucket) return ovBucket;

  if (/FaultList|kBbms_Flt_\d|\bBbms_Flt_\d/i.test(t) && !/FltOv|FaultEna|Hist/i.test(t)) {
    return { id: "fault_list" };
  }
  if (/BankErrorCnt|RackFltHist|故障累积/i.test(t) && !/RackAllFltHistAccuCaliVal/i.test(t)) {
    return { id: "bank_cali" };
  }
  if (/TMS_.*Flt|TMS.*Fault|TMS故障/i.test(t)) return { id: "tms_fault" };
  if (/RackFlt|簇.*故障/i.test(t) && !/Hist|FltOv/i.test(t)) return { id: "rack_fault" };
  return { id: "fault_other" };
}

function faultPanelDynamicSortKey(id) {
  const m = String(id).match(/^(flt_ov|rack_hist_cali)_(\d+)_(\d+)$/);
  if (!m) return 0;
  const base = m[1] === "flt_ov" ? 1000 : 2000;
  return base + Number(m[2] || 0);
}

function sortFaultPanelGroupIds(ids) {
  return [...ids].sort((a, b) => faultPanelDynamicSortKey(a) - faultPanelDynamicSortKey(b));
}

/**
 * 将故障相关测点重组为故障面板分区（合并原多分区 + 告警兜底测点）。
 * @param {object[]} points
 */
export function buildBbmsFaultPanelSections(points) {
  const bucket = {};
  const dynamicMeta = {};
  (points || []).forEach((p) => {
    const g = resolveBbmsFaultPanelGroupId(p);
    const id = g.id;
    if (!bucket[id]) bucket[id] = [];
    bucket[id].push(p);
    if (g.rangeStart != null && g.titleKey) {
      dynamicMeta[id] = {
        titleKey: g.titleKey,
        rangeStart: g.rangeStart,
        rangeEnd: g.rangeEnd,
      };
    }
  });

  const dynamicIds = sortFaultPanelGroupIds(
    Object.keys(bucket).filter(
      (id) => bucket[id].length && (id.startsWith("flt_ov_") || id.startsWith("rack_hist_cali_"))
    )
  );
  const finalOrder = [
    ...BBMS_FAULT_PANEL_STATIC_ORDER.filter((id) => bucket[id] && bucket[id].length),
    ...dynamicIds,
  ];

  return finalOrder.map((id) => {
    const rows = sortBbmsSeriesRows(bucket[id]);
    const staticM = BBMS_FAULT_PANEL_STATIC_META[id] || {};
    const dynM = dynamicMeta[id] || {};
    return {
      id,
      titleKey: dynM.titleKey || staticM.titleKey || "monitor.bbmsFaultSecOther",
      hintKey: staticM.hintKey || "",
      rangeStart: dynM.rangeStart,
      rangeEnd: dynM.rangeEnd,
      layout: "series",
      rows,
      collapseDefault: rows.length > 40,
    };
  });
}
