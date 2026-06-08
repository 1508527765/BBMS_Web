<template>
  <div class="thermal-console">
    <header class="thermal-hero">
      <div class="thermal-hero-head">
        <div>
          <h3 class="thermal-hero-title">{{ $t("monitor.bbmsThermalPanelTitle") }}</h3>
          <p class="thermal-hero-sub">{{ $t("monitor.bbmsThermalPanelSub") }}</p>
        </div>
        <div v-if="faultBadge" class="thermal-fault-pill" :class="faultBadge.tone">
          <span class="thermal-fault-label">{{ faultBadge.label }}</span>
          <span class="thermal-fault-value">{{ faultBadge.value }}</span>
        </div>
      </div>
      <div v-if="heroMetrics.length" class="thermal-hero-grid">
        <div
          v-for="item in heroMetrics"
          :key="'hero-' + item.key"
          class="thermal-hero-tile"
          :class="'thermal-hero-tile--' + item.accent"
        >
          <span class="thermal-hero-tile-icon" aria-hidden="true">{{ item.icon }}</span>
          <span class="thermal-hero-tile-label" :title="item.title">{{ item.label }}</span>
          <span class="thermal-hero-tile-value">{{ item.value }}</span>
        </div>
      </div>
      <p v-else class="thermal-empty">{{ $t("monitor.暂无数据") }}</p>
    </header>

    <div class="thermal-body">
      <section v-if="hasRuntimeDetail" class="thermal-zone thermal-zone--runtime">
        <div class="thermal-rt-head">
          <div>
            <h4 class="thermal-zone-title">{{ $t("monitor.bbmsSecTmsRuntime") }}</h4>
            <p class="thermal-zone-hint">{{ $t("monitor.bbmsThermalRtHint") }}</p>
          </div>
          <span class="thermal-rt-count">{{ runtimeDetailCount }}</span>
        </div>

        <div class="thermal-rt-board">
          <div v-if="runtimeLayout.cellAvg" class="thermal-rt-feature thermal-rt-feature--cell">
            <div class="thermal-rt-feature-icon" aria-hidden="true">▦</div>
            <div class="thermal-rt-feature-body">
              <span class="thermal-rt-feature-label" :title="pointTooltip(runtimeLayout.cellAvg)">
                {{ thermalPointLabel(runtimeLayout.cellAvg) }}
              </span>
              <span class="thermal-rt-feature-value">{{ displayValue(runtimeLayout.cellAvg) }}</span>
            </div>
          </div>

          <div v-if="runtimeLayout.water.length" class="thermal-rt-panel thermal-rt-panel--water">
            <div class="thermal-rt-panel-title">
              <span class="thermal-rt-panel-icon" aria-hidden="true">≋</span>
              {{ $t("monitor.bbmsThermalRtWaterSys") }}
            </div>
            <div class="thermal-rt-water-flow">
              <template v-for="(node, idx) in runtimeLayout.water">
                <div
                  :key="'wf-' + node.key"
                  class="thermal-rt-flow-node"
                  :class="'thermal-rt-flow-node--' + node.kind"
                >
                  <span class="thermal-rt-flow-glyph" aria-hidden="true">{{ node.icon }}</span>
                  <span class="thermal-rt-flow-label" :title="pointTooltip(node.row)">
                    {{ thermalPointLabel(node.row) }}
                  </span>
                  <span class="thermal-rt-flow-value">{{ displayValue(node.row) }}</span>
                </div>
                <div
                  v-if="idx < runtimeLayout.water.length - 1"
                  :key="'wa-' + node.key"
                  class="thermal-rt-flow-connector"
                  aria-hidden="true"
                />
              </template>
            </div>
          </div>

          <div v-if="runtimeLayout.status.length" class="thermal-rt-panel thermal-rt-panel--status">
            <div class="thermal-rt-panel-title">
              <span class="thermal-rt-panel-icon" aria-hidden="true">◈</span>
              {{ $t("monitor.bbmsThermalRtOperState") }}
            </div>
            <div class="thermal-rt-status-grid">
              <div
                v-for="row in runtimeLayout.status"
                :key="'st-' + row.ename"
                class="thermal-rt-status-chip"
                :class="statusChipClass(row)"
              >
                <span class="thermal-rt-status-k" :title="pointTooltip(row)">{{ thermalPointLabel(row) }}</span>
                <span class="thermal-rt-status-v">{{ displayValue(row) }}</span>
              </div>
            </div>
          </div>
        </div>
      </section>

      <section v-if="thresholdPairs.length" class="thermal-zone thermal-zone--threshold">
        <div class="thermal-zone-head">
          <h4 class="thermal-zone-title">{{ $t("monitor.bbmsThermalThrZoneTitle") }}</h4>
          <p class="thermal-zone-hint">{{ $t("monitor.bbmsThermalThrZoneHint") }}</p>
        </div>
        <div class="thermal-thr-table-wrap">
          <table class="thermal-thr-table">
            <thead>
              <tr>
                <th>{{ $t("monitor.bbmsThermalThrParam") }}</th>
                <th class="thermal-thr-col-read">{{ $t("monitor.bbmsThermalThrColRead") }}</th>
                <th class="thermal-thr-col-write">{{ $t("monitor.bbmsThermalThrColWrite") }}</th>
              </tr>
            </thead>
            <tbody>
              <tr
                v-for="pair in thresholdPairs"
                :key="'thr-' + pair.key"
                :class="'thermal-thr-row--' + pair.tone"
              >
                <td class="thermal-thr-name">
                  <span class="thermal-thr-badge" :class="'thermal-thr-badge--' + pair.tone">{{
                    pair.badge
                  }}</span>
                  {{ pair.label }}
                </td>
                <td class="thermal-thr-read">{{ pair.readText }}</td>
                <td class="thermal-thr-write">
                  <template v-if="pair.write">
                    <div v-if="writeReady" class="thermal-write-inline">
                      <el-input-number
                        :value="draftNumber(pair.write)"
                        size="mini"
                        controls-position="right"
                        class="thermal-write-input"
                        @change="onDraftNumber(pair.write, $event)"
                      />
                      <el-button
                        type="primary"
                        size="mini"
                        :loading="isDispatching(pair.write)"
                        @click="dispatchThermalPoint(pair.write)"
                      >
                        {{ $t("common.下发") }}
                      </el-button>
                    </div>
                    <span v-else class="thermal-write-readonly">{{ displayValue(pair.write) }}</span>
                    <span v-if="writeReady" class="thermal-write-back">
                      {{ $t("monitor.值") }}: {{ displayValue(pair.write) }}
                    </span>
                  </template>
                  <span v-else class="thermal-thr-dash">—</span>
                </td>
              </tr>
            </tbody>
          </table>
        </div>
        <p v-if="!writeReady" class="thermal-zone-foot">{{ $t("monitor.bbmsThermalWriteDisabledHint") }}</p>
        <p v-else class="thermal-zone-foot thermal-zone-foot--write">{{ $t("monitor.bbmsThermalWriteHint") }}</p>
      </section>

      <section v-if="hmiRows.length" class="thermal-zone thermal-zone--manual">
        <div class="thermal-zone-head">
          <h4 class="thermal-zone-title">{{ $t("monitor.bbmsSecHmiTms") }}</h4>
          <p class="thermal-zone-hint">{{ $t("monitor.bbmsSecHmiTmsHint") }}</p>
        </div>
        <div class="thermal-manual-grid">
          <div
            v-for="row in hmiRows"
            :key="'hmi-' + (row.code != null ? row.code : row.ename)"
            class="thermal-manual-card"
          >
            <div class="thermal-manual-label" :title="pointTooltip(row)">{{ thermalPointLabel(row) }}</div>
            <template v-if="writeReady">
              <el-switch
                v-if="controlKind(row) === 'yk'"
                :value="draftBool(row)"
                @change="onDraftBool(row, $event)"
              />
              <el-input-number
                v-else
                :value="draftNumber(row)"
                size="mini"
                controls-position="right"
                class="thermal-write-input thermal-write-input--wide"
                @change="onDraftNumber(row, $event)"
              />
              <el-button
                type="primary"
                size="mini"
                plain
                :loading="isDispatching(row)"
                @click="dispatchThermalPoint(row)"
              >
                {{ $t("common.下发") }}
              </el-button>
              <div class="thermal-manual-readback">{{ $t("monitor.值") }}: {{ displayValue(row) }}</div>
            </template>
            <div v-else class="thermal-manual-value">{{ displayValue(row) }}</div>
          </div>
        </div>
      </section>
    </div>
  </div>
</template>

<script>
import { updateDeviceValue } from "@/api/system/device";
import {
  bbmsPointMatchText,
  bbmsThermalPointControlKind,
  bbmsThermalDispatchPoint,
} from "@/constants/bbmsPointSections";

const HERO_SLOT_ENAME = {
  setTemp: "Bbms_TmsSumInfo_Start",
  inlet: "Bbms_TmsSumInfo_Start_5",
  outlet: "Bbms_TmsSumInfo_Start_4",
  cellMax: "Bbms_TmsSumInfo_Start_1",
  cellMin: "Bbms_TmsSumInfo_Start_2",
  workMode: "Bbms_TmsSumInfo_Start_12",
};

const HERO_SLOTS = [
  { key: "setTemp", labelKey: "monitor.bbmsThermalPtSetTemp", icon: "◎", accent: "cyan" },
  { key: "inlet", labelKey: "monitor.bbmsThermalPtInletTemp", icon: "↓", accent: "blue" },
  { key: "outlet", labelKey: "monitor.bbmsThermalPtOutletTemp", icon: "↑", accent: "indigo" },
  { key: "cellMax", labelKey: "monitor.bbmsThermalPtCellMax", icon: "▲", accent: "amber" },
  { key: "cellMin", labelKey: "monitor.bbmsThermalPtCellMin", icon: "▼", accent: "teal" },
  { key: "workMode", labelKey: "monitor.bbmsThermalPtWorkMode", icon: "◈", accent: "violet" },
];

/** 英文界面短标签（按 ename 精确匹配，避免 Start_11 误命中 Start_1） */
const THERMAL_POINT_LABEL_BY_ENAME = {
  Bbms_TmsSumInfo_Start: "monitor.bbmsThermalPtSetTemp",
  Bbms_TmsSumInfo_Start_1: "monitor.bbmsThermalPtCellMax",
  Bbms_TmsSumInfo_Start_2: "monitor.bbmsThermalPtCellMin",
  Bbms_TmsSumInfo_Start_3: "monitor.bbmsThermalPtCellAvg",
  Bbms_TmsSumInfo_Start_4: "monitor.bbmsThermalPtOutletTemp",
  Bbms_TmsSumInfo_Start_5: "monitor.bbmsThermalPtInletTemp",
  Bbms_TmsSumInfo_Start_6: "monitor.bbmsThermalPtEnvTemp",
  Bbms_TmsSumInfo_Start_7: "monitor.bbmsThermalPtInletPress",
  Bbms_TmsSumInfo_Start_8: "monitor.bbmsThermalPtOutletPress",
  Bbms_TmsSumInfo_Start_9: "monitor.bbmsThermalPtFaultLevel",
  Bbms_TmsSumInfo_Start_10: "monitor.bbmsThermalPtFaultCode",
  Bbms_TmsSumInfo_Start_11: "monitor.bbmsThermalPtBmsReqMode",
  Bbms_TmsSumInfo_Start_12: "monitor.bbmsThermalPtWorkMode",
  Bbms_TmsSumInfo_Start_13: "monitor.bbmsThermalPtTmsId",
  Bbms_TmsSumInfo_Start_14: "monitor.bbmsThermalPtCoolState",
  Bbms_TmsInfo_Reserver: "monitor.bbmsThermalPtReserved",
  Bbms_Hmi_TMSCtrlWord_Start_1: "monitor.bbmsThermalPtManMode",
  Bbms_Hmi_TMSCtrlWord_Start_2: "monitor.bbmsThermalPtManTemp",
  Bbms_Hmi_TMSCtrlWord_Start_3: "monitor.bbmsThermalPtManEnable",
  Bbms_Hmi_TMSCtrlWord_Start_4: "monitor.bbmsThermalPtManTmsId",
};

const THR_META = [
  {
    key: "entHeatMin",
    suffix: "EntHeatMinTemLpLmt",
    badgeKey: "monitor.bbmsThermalBadgeHeat",
    tone: "heat",
    labelKey: "monitor.bbmsThermalLblEntHeatMin",
  },
  {
    key: "entHeatAvg",
    suffix: "EntHeatAvgTemLpLmt",
    badgeKey: "monitor.bbmsThermalBadgeHeat",
    tone: "heat",
    labelKey: "monitor.bbmsThermalLblEntHeatAvg",
  },
  {
    key: "extHeatMin",
    suffix: "ExtHeatMinTemLowLmt",
    badgeKey: "monitor.bbmsThermalBadgeHeat",
    tone: "heat",
    labelKey: "monitor.bbmsThermalLblExtHeatMin",
  },
  {
    key: "extHeatAvg",
    suffix: "ExtHeatAvgTemLowLmt",
    badgeKey: "monitor.bbmsThermalBadgeHeat",
    tone: "heat",
    labelKey: "monitor.bbmsThermalLblExtHeatAvg",
  },
  {
    key: "entCoolMax",
    suffix: "EntCoolMaxTemLowLimtA",
    badgeKey: "monitor.bbmsThermalBadgeCool",
    tone: "cool",
    labelKey: "monitor.bbmsThermalLblEntCoolMax",
  },
  {
    key: "entCoolAvg",
    suffix: "EntCoolAvgTemLowLimtA",
    badgeKey: "monitor.bbmsThermalBadgeCool",
    tone: "cool",
    labelKey: "monitor.bbmsThermalLblEntCoolAvg",
  },
  {
    key: "extCoolMax",
    suffix: "ExtCoolMaxTemUpLimtA",
    badgeKey: "monitor.bbmsThermalBadgeCool",
    tone: "cool",
    labelKey: "monitor.bbmsThermalLblExtCoolMax",
  },
  {
    key: "extCoolAvg",
    suffix: "ExtCoolAvgTemUpLimtA",
    badgeKey: "monitor.bbmsThermalBadgeCool",
    tone: "cool",
    labelKey: "monitor.bbmsThermalLblExtCoolAvg",
  },
];

export default {
  name: "BbmsThermalPanel",
  props: {
    cells: { type: Array, default: () => [] },
  },
  inject: {
    monitor: { default: null },
  },
  data() {
    return {
      draftValues: {},
      dispatchingCodes: {},
    };
  },
  computed: {
    m() {
      return this.monitor;
    },
    writeReady() {
      const mon = this.m;
      return !!(mon && mon.monitorAuxWriteEnabled);
    },
    sectionMap() {
      const map = {};
      (this.cells || []).forEach((cell) => {
        if (cell && cell.sec && cell.sec.id) map[cell.sec.id] = cell.sec;
      });
      return map;
    },
    runtimeRows() {
      const sec = this.sectionMap.tms_runtime;
      return (sec && sec.rows) || [];
    },
    readThrRows() {
      const sec = this.sectionMap.para_tms_thr;
      return (sec && sec.rows) || [];
    },
    writeThrRows() {
      const sec = this.sectionMap.ctrl_tms_thr;
      return (sec && sec.rows) || [];
    },
    hmiRows() {
      const sec = this.sectionMap.hmi_tms;
      return ((sec && sec.rows) || []).filter((p) => !/预留|Reserved/i.test(bbmsPointMatchText(p)));
    },
    heroMetrics() {
      const used = new Set();
      return HERO_SLOTS.map((slot) => {
        const want = HERO_SLOT_ENAME[slot.key];
        const hit = this.runtimeRows.find((p) => String(p.ename || "") === want);
        if (!hit) return null;
        used.add(hit);
        return {
          key: slot.key,
          icon: slot.icon,
          accent: slot.accent,
          label: this.resolveLabel(slot.labelKey, hit),
          title: this.pointTooltip(hit),
          value: this.displayValue(hit),
        };
      }).filter(Boolean);
    },
    faultBadge() {
      const flt = this.runtimeRows.find((p) => String(p.ename || "") === "Bbms_TmsSumInfo_Start_9");
      if (!flt) return null;
      const raw = flt.value;
      const n = raw !== null && raw !== undefined && String(raw).trim() !== "" ? Number(raw) : NaN;
      let tone = "ok";
      if (!Number.isNaN(n) && n > 0) tone = n >= 2 ? "crit" : "warn";
      return {
        label: this.thermalPointLabel(flt),
        value: this.displayValue(flt),
        tone,
      };
    },
    heroEnameSet() {
      return new Set(Object.values(HERO_SLOT_ENAME));
    },
    runtimeByEname() {
      const map = {};
      (this.runtimeRows || []).forEach((r) => {
        const en = String(r.ename || "");
        if (en) map[en] = r;
      });
      return map;
    },
    runtimeLayout() {
      const hero = this.heroEnameSet;
      const map = this.runtimeByEname;
      const pick = (ename) => (hero.has(ename) ? null : map[ename] || null);
      const water = [
        { key: "inPress", kind: "pressure", icon: "◐", row: pick("Bbms_TmsSumInfo_Start_7") },
        { key: "outPress", kind: "pressure", icon: "◑", row: pick("Bbms_TmsSumInfo_Start_8") },
        { key: "ambient", kind: "ambient", icon: "☁", row: pick("Bbms_TmsSumInfo_Start_6") },
      ].filter((n) => n.row);
      const status = [
        pick("Bbms_TmsSumInfo_Start_11"),
        pick("Bbms_TmsSumInfo_Start_13"),
        pick("Bbms_TmsSumInfo_Start_14"),
        pick("Bbms_TmsSumInfo_Start_10"),
        pick("Bbms_TmsInfo_Reserver"),
      ].filter(Boolean);
      return {
        cellAvg: pick("Bbms_TmsSumInfo_Start_3"),
        water,
        status,
      };
    },
    hasRuntimeDetail() {
      const l = this.runtimeLayout;
      return !!(l.cellAvg || l.water.length || l.status.length);
    },
    runtimeDetailCount() {
      const l = this.runtimeLayout;
      return (l.cellAvg ? 1 : 0) + l.water.length + l.status.length;
    },
    thresholdPairs() {
      const readMap = {};
      this.readThrRows.forEach((p) => {
        const en = String(p.ename || "");
        const m = en.match(/CcTHMC_(.+)$/);
        if (m) readMap[m[1]] = p;
      });
      const writeMap = {};
      this.writeThrRows.forEach((p) => {
        const en = String(p.ename || "");
        const m = en.match(/Crtl_TMS_(.+)$/);
        if (m && !/Reserved|Start_8/.test(m[1])) writeMap[m[1]] = p;
      });
      return THR_META.map((meta) => {
        const read = readMap[meta.suffix];
        const write = writeMap[meta.suffix];
        if (!read && !write) return null;
        return {
          key: meta.key,
          tone: meta.tone,
          badge: this.$t(meta.badgeKey),
          label: this.$t(meta.labelKey),
          readText: read ? this.displayValue(read) : "—",
          write,
        };
      }).filter(Boolean);
    },
  },
  watch: {
    "m.currentPoints": {
      handler(list) {
        this.syncThermalDrafts(list);
      },
      deep: true,
    },
    "m.currentNode"() {
      this.draftValues = {};
      this.dispatchingCodes = {};
    },
  },
  methods: {
    pointName(row) {
      const mon = this.m;
      return mon && mon.pointDisplayName ? mon.pointDisplayName(row) : row.name || row.ename || "";
    },
    isEnglishLocale() {
      const loc = String(this.$i18n && this.$i18n.locale ? this.$i18n.locale : "").toLowerCase();
      return loc === "en" || loc.startsWith("en");
    },
    resolveLabel(labelKey, row) {
      if (labelKey) {
        const t = this.$t(labelKey);
        if (t && t !== labelKey) return t;
      }
      return this.thermalPointLabel(row);
    },
    statusChipClass(row) {
      const en = String((row && row.ename) || "");
      if (en === "Bbms_TmsSumInfo_Start_10") {
        const raw = row.value;
        const n = raw !== null && raw !== undefined && String(raw).trim() !== "" ? Number(raw) : NaN;
        if (!Number.isNaN(n) && n > 0) return "thermal-rt-status-chip--alert";
      }
      if (en === "Bbms_TmsInfo_Reserver") return "thermal-rt-status-chip--muted";
      return "";
    },
    thermalPointLabelKey(row) {
      const en = String((row && row.ename) || "").trim();
      return en ? THERMAL_POINT_LABEL_BY_ENAME[en] || null : null;
    },
    thermalPointLabel(row) {
      const name = String((row && row.name) || "").trim();
      if (!this.isEnglishLocale() && name) return name;
      const labelKey = this.thermalPointLabelKey(row);
      if (labelKey) {
        const t = this.$t(labelKey);
        if (t && t !== labelKey) return t;
      }
      const en = String((row && row.ename) || "").trim();
      if (this.isEnglishLocale()) return en || "--";
      return name || en || "--";
    },
    pointTooltip(row) {
      const parts = [];
      const name = String((row && row.name) || "").trim();
      const ename = String((row && row.ename) || "").trim();
      if (name) parts.push(name);
      if (ename && ename !== name) parts.push(ename);
      return parts.join(" · ") || "";
    },
    displayValue(row) {
      const mon = this.m;
      return mon && mon.bbmsDisplayValue ? mon.bbmsDisplayValue(row) : row.value;
    },
    controlKind(row) {
      return bbmsThermalPointControlKind(row);
    },
    pointDraftKey(pt) {
      const code = pt && pt.code;
      return code === null || code === undefined || code === "" ? null : String(code);
    },
    syncThermalDrafts(list) {
      const next = { ...this.draftValues };
      (list || []).forEach((pt) => {
        if (!bbmsThermalDispatchPoint(pt)) return;
        const key = this.pointDraftKey(pt);
        if (!key || next[key] !== undefined) return;
        const v = pt.value;
        if (v === null || v === undefined || String(v).trim() === "") return;
        next[key] = String(v);
      });
      this.draftValues = next;
    },
    draftRaw(pt) {
      const key = this.pointDraftKey(pt);
      if (!key) return "";
      if (this.draftValues[key] !== undefined) return this.draftValues[key];
      const v = pt.value;
      if (v === null || v === undefined || String(v).trim() === "") return "";
      return String(v);
    },
    draftBool(pt) {
      const v = this.parseDispatchValue(pt);
      return v !== null && v !== 0;
    },
    draftNumber(pt) {
      const raw = this.draftRaw(pt);
      if (raw === "") return undefined;
      const n = Number(raw);
      return Number.isNaN(n) ? undefined : n;
    },
    onDraftBool(pt, val) {
      const key = this.pointDraftKey(pt);
      if (!key) return;
      this.$set(this.draftValues, key, val ? "1" : "0");
    },
    onDraftNumber(pt, val) {
      const key = this.pointDraftKey(pt);
      if (!key) return;
      if (val === null || val === undefined || val === "") {
        this.$delete(this.draftValues, key);
        return;
      }
      this.$set(this.draftValues, key, String(val));
    },
    isDispatching(pt) {
      const key = this.pointDraftKey(pt);
      return !!(key && this.dispatchingCodes[key]);
    },
    parseDispatchValue(pt) {
      const raw = this.draftRaw(pt);
      if (raw === "") return null;
      const n = Number(raw);
      return Number.isNaN(n) ? null : n;
    },
    async dispatchThermalPoint(pt) {
      const mon = this.m;
      if (!this.writeReady) {
        this.$message.warning(this.$t("monitor.bbmsThermalWriteDisabledHint"));
        return;
      }
      const devType = mon.monitorRtdbDevType;
      const devId = mon.monitorRtdbDevId;
      const pointId = pt && pt.code;
      if (devType === null || devType === undefined || devId === null || devId === undefined) {
        this.$message.warning(this.$t("monitor.bbmsThermalWriteDisabledHint"));
        return;
      }
      if (pointId === null || pointId === undefined || pointId === "") {
        this.$message.error(this.$t("monitor.auxInvalidValue"));
        return;
      }
      const value = this.parseDispatchValue(pt);
      if (value === null) {
        this.$message.error(this.$t("monitor.auxInvalidValue"));
        return;
      }
      const key = this.pointDraftKey(pt);
      if (key) this.$set(this.dispatchingCodes, key, true);
      try {
        await updateDeviceValue({
          dev_type: Number(devType),
          dev_id: Number(devId),
          point_id: Number(pointId),
          value,
        });
        this.$message.success(this.$t("monitor.auxDispatchSuccess"));
        if (mon.onAuxPointDispatched) {
          mon.onAuxPointDispatched({ code: pointId, value });
        }
      } catch (e) {
        this.$message.error(this.$t("monitor.auxDispatchFail"));
      } finally {
        if (key) this.$delete(this.dispatchingCodes, key);
      }
    },
  },
};
</script>

<style scoped lang="scss">
@import "./styles/thermal.scss";
</style>
