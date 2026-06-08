<template>
  <div class="aux-monitor" v-loading="loading">
    <header class="aux-monitor__hero">
      <div class="aux-monitor__hero-main">
        <span class="aux-monitor__kicker">{{ $t("monitor.auxDashKicker") }}</span>
        <h2 class="aux-monitor__title">{{ deviceName }}</h2>
        <p class="aux-monitor__meta">
          <span class="aux-monitor__badge">{{ deviceTypeLabel }}</span>
          <span v-if="protocolLabel" class="aux-monitor__proto">{{ protocolLabel }}</span>
          <span class="aux-monitor__ids">
            ID {{ deviceIdText }} · {{ $t("monitor.dashboardTileTemplate") }} {{ templateIdText }}
          </span>
        </p>
      </div>
      <div class="aux-monitor__status" :class="online ? 'is-on' : 'is-off'">
        <i class="aux-monitor__dot" />
        {{ online ? $t("monitor.auxOnline") : $t("monitor.auxOffline") }}
      </div>
    </header>

    <div class="aux-monitor__kpi-strip">
      <div v-for="m in heroMetrics" :key="m.key" class="aux-monitor__kpi" :class="'aux-monitor__kpi--' + m.accent">
        <span class="aux-monitor__kpi-label">{{ m.label }}</span>
        <span class="aux-monitor__kpi-value">{{ m.value }}</span>
      </div>
    </div>

    <div class="aux-monitor__toolbar">
      <el-input
        :value="keyword"
        size="small"
        clearable
        prefix-icon="el-icon-search"
        class="aux-monitor__search"
        :placeholder="$t('monitor.pointSearchPlaceholder')"
        @input="$emit('update:keyword', $event)"
      />
      <el-switch
        :value="onlyValued"
        :active-text="$t('monitor.onlyValuedOn')"
        :inactive-text="$t('monitor.onlyValuedOff')"
        @change="$emit('update:onlyValued', $event)"
      />
    </div>

    <div v-if="!filteredPoints.length && !loading" class="aux-monitor__empty">
      {{ $t("monitor.auxNoPoints") }}
    </div>

    <el-tabs v-else v-model="activeTab" class="aux-monitor__tabs" type="card">
      <el-tab-pane
        v-for="tab in visibleTabs"
        :key="'aux-tab-' + tab.id"
        :label="tabLabel(tab)"
        :name="tab.id"
      >
        <div v-if="tab.id === 'overview'" class="aux-tab-overview">
          <p class="aux-tab-hint">{{ $t("monitor.auxTabOverviewHint") }}</p>
          <div class="aux-overview-grid">
            <div
              v-for="g in displayGroups"
              :key="'aux-ov-' + g.key"
              class="aux-overview-card"
              :class="'aux-overview-card--' + g.accent"
              @click="activeTab = g.key"
            >
              <div class="aux-overview-card-title">{{ g.title }}</div>
              <div class="aux-overview-card-metrics">
                <span>{{ g.points.length }} {{ $t("monitor.测点") }}</span>
                <span>{{ g.valuedCount }} {{ $t("monitor.auxMetricValued") }}</span>
              </div>
              <p v-if="g.hint" class="aux-overview-card-hint">{{ g.hint }}</p>
            </div>
          </div>
        </div>

        <div v-else-if="tab.id === 'flat'" class="aux-flat-wrap">
          <el-table :data="filteredPoints" border stripe size="small" height="520" class="aux-flat-table">
            <el-table-column prop="code" :label="$t('monitor.monitorPointCode')" width="88" sortable />
            <el-table-column :label="$t('monitor.测点')" min-width="160" show-overflow-tooltip>
              <template slot-scope="scope">
                {{ cellLabel(scope.row) }}
              </template>
            </el-table-column>
            <el-table-column :label="$t('monitor.值')" width="120">
              <template slot-scope="scope">
                <span :class="{ 'aux-val-alarm': resolvePointGroupType(scope.row) === 6 }">
                  {{ formatVal(scope.row) }}
                </span>
              </template>
            </el-table-column>
            <el-table-column prop="unit" :label="$t('monitor.单位')" width="72" />
            <el-table-column :label="$t('monitor.monitorGroupTypeCol')" width="100">
              <template slot-scope="scope">
                {{ groupTitle(scope.row) }}
              </template>
            </el-table-column>
            <el-table-column
              v-if="writeEnabled"
              :label="$t('common.下发')"
              width="200"
              fixed="right"
            >
              <template slot-scope="scope">
                <div class="aux-row-write" v-if="pointWritable(scope.row)">
                  <el-switch
                    v-if="resolvePointGroupType(scope.row) === 2"
                    :value="draftBool(scope.row)"
                    @change="onDraftBool(scope.row, $event)"
                  />
                  <el-input-number
                    v-else
                    :value="draftNumber(scope.row)"
                    size="mini"
                    controls-position="right"
                    class="aux-row-input"
                    @change="onDraftNumber(scope.row, $event)"
                  />
                  <el-button
                    type="primary"
                    size="mini"
                    :loading="isDispatching(scope.row)"
                    @click="dispatchPoint(scope.row)"
                  >
                    {{ $t("common.下发") }}
                  </el-button>
                </div>
                <span v-else class="aux-readonly-dash">—</span>
              </template>
            </el-table-column>
          </el-table>
          <p v-if="writeEnabled" class="aux-tab-hint aux-tab-hint--write">{{ $t("monitor.auxWriteHint") }}</p>
        </div>

        <div v-else class="aux-group-panel">
          <p v-if="tab.hint" class="aux-tab-hint">{{ tab.hint }}</p>
          <p v-if="tab.writable && writeEnabled" class="aux-tab-hint aux-tab-hint--write">
            {{ $t("monitor.auxWriteHint") }}
          </p>
          <p v-else-if="tab.writable && !writeEnabled" class="aux-tab-hint aux-tab-hint--muted">
            {{ $t("monitor.auxWriteDisabledHint") }}
          </p>
          <div
            v-if="tab.layout === 'chip'"
            class="aux-chip-grid"
          >
            <div
              v-for="(pt, idx) in tab.points"
              :key="pt.code != null ? 'chip-' + pt.code : 'chip-' + idx"
              class="aux-chip"
              :class="[
                'aux-chip--' + tab.accent,
                { 'is-alarm': resolvePointGroupType(pt) === 6 },
                { 'aux-chip--writable': pointWritable(pt) && writeEnabled },
              ]"
            >
              <span class="aux-chip-label" :title="cellLabel(pt)">{{ cellLabel(pt) }}</span>
              <template v-if="pointWritable(pt) && writeEnabled">
                <div class="aux-chip-write">
                  <el-switch
                    v-if="resolvePointGroupType(pt) === 2"
                    :value="draftBool(pt)"
                    @change="onDraftBool(pt, $event)"
                  />
                  <el-input-number
                    v-else
                    :value="draftNumber(pt)"
                    size="mini"
                    controls-position="right"
                    class="aux-chip-input"
                    @change="onDraftNumber(pt, $event)"
                  />
                  <el-button
                    type="primary"
                    size="mini"
                    :loading="isDispatching(pt)"
                    @click="dispatchPoint(pt)"
                  >
                    {{ $t("common.下发") }}
                  </el-button>
                </div>
                <span class="aux-chip-readback">{{ $t("monitor.值") }}: {{ formatVal(pt) }}</span>
              </template>
              <span v-else class="aux-chip-value">{{ formatVal(pt) }}<small v-if="pt.unit">{{ pt.unit }}</small></span>
            </div>
          </div>
          <el-table
            v-else
            :data="tab.points"
            border
            stripe
            size="small"
            class="aux-group-table"
            :max-height="tab.points.length > 12 ? 420 : undefined"
          >
            <el-table-column :label="$t('monitor.测点')" min-width="160" show-overflow-tooltip>
              <template slot-scope="scope">
                {{ cellLabel(scope.row) }}
              </template>
            </el-table-column>
            <el-table-column :label="$t('monitor.值')" width="120">
              <template slot-scope="scope">
                <span :class="{ 'aux-val-alarm': resolvePointGroupType(scope.row) === 6 }">
                  {{ formatVal(scope.row) }}
                </span>
              </template>
            </el-table-column>
            <el-table-column prop="unit" :label="$t('monitor.单位')" width="72" />
            <el-table-column
              v-if="tab.writable && writeEnabled"
              :label="$t('common.下发')"
              width="200"
              fixed="right"
            >
              <template slot-scope="scope">
                <div class="aux-row-write" v-if="pointWritable(scope.row)">
                  <el-switch
                    v-if="resolvePointGroupType(scope.row) === 2"
                    :value="draftBool(scope.row)"
                    @change="onDraftBool(scope.row, $event)"
                  />
                  <el-input-number
                    v-else
                    :value="draftNumber(scope.row)"
                    size="mini"
                    controls-position="right"
                    class="aux-row-input"
                    @change="onDraftNumber(scope.row, $event)"
                  />
                  <el-button
                    type="primary"
                    size="mini"
                    :loading="isDispatching(scope.row)"
                    @click="dispatchPoint(scope.row)"
                  >
                    {{ $t("common.下发") }}
                  </el-button>
                </div>
                <span v-else class="aux-readonly-dash">—</span>
              </template>
            </el-table-column>
          </el-table>
        </div>
      </el-tab-pane>
    </el-tabs>
  </div>
</template>

<script>
import { updateDeviceValue } from "@/api/system/device";
import {
  POINT_GROUP_TYPES,
  POINT_GROUP_ORDER,
  resolvePointGroupType,
  pointGroupMeta,
  isPointGroupWritable,
} from "@/constants/pointGroupTypes";

/** 遥信/遥控/故障/事件：离散量，用 chip；其余用表格 */
const CHIP_GROUP_TYPES = new Set([0, 2, 6, 7]);

export default {
  name: "AuxiliaryDevicePanel",
  props: {
    deviceName: { type: String, default: "" },
    deviceTypeLabel: { type: String, default: "" },
    protocolLabel: { type: String, default: "" },
    deviceIdText: { type: String, default: "--" },
    templateIdText: { type: String, default: "--" },
    online: { type: Boolean, default: true },
    loading: { type: Boolean, default: false },
    points: { type: Array, default: () => [] },
    keyword: { type: String, default: "" },
    onlyValued: { type: Boolean, default: false },
    /** C dev_type_e，与 device.type 一致 */
    rtdbDevType: { type: Number, default: null },
    /** RTDB 设备序号 = 设备编号 code - 1 */
    rtdbDevId: { type: Number, default: null },
    /** 单台设备节点才允许下发（虚拟聚合目录为 false） */
    writeEnabled: { type: Boolean, default: false },
    groupTitleFn: { type: Function, required: true },
    pointNameFn: { type: Function, required: true },
  },
  data() {
    return {
      activeTab: "overview",
      draftValues: {},
      dispatchingCodes: {},
    };
  },
  computed: {
    filteredPoints() {
      const kw = String(this.keyword || "")
        .trim()
        .toLowerCase();
      return (this.points || []).filter((p) => {
        const name = String(this.pointNameFn(p) || p.name || p.ename || "").toLowerCase();
        const hasValue = !(p.value === undefined || p.value === null || String(p.value).trim() === "");
        if (this.onlyValued && !hasValue) return false;
        if (kw && !name.includes(kw)) return false;
        return true;
      });
    },
    heroMetrics() {
      const list = this.filteredPoints;
      const valued = list.filter(
        (p) => !(p.value === undefined || p.value === null || String(p.value).trim() === "")
      ).length;
      const groupSet = new Set();
      list.forEach((p) => {
        const g = resolvePointGroupType(p);
        groupSet.add(g === null ? "x" : g);
      });
      return [
        { key: "total", label: this.$t("monitor.auxMetricTotal"), value: list.length, accent: "teal" },
        { key: "valued", label: this.$t("monitor.auxMetricValued"), value: valued, accent: "sky" },
        { key: "groups", label: this.$t("monitor.auxMetricGroups"), value: groupSet.size, accent: "slate" },
      ];
    },
    displayGroups() {
      const buckets = {};
      this.filteredPoints.forEach((p) => {
        const g = resolvePointGroupType(p);
        const k = g === null ? "x" : String(g);
        if (!buckets[k]) buckets[k] = [];
        buckets[k].push(p);
      });
      const misc = buckets.x || [];
      delete buckets.x;
      const out = POINT_GROUP_ORDER.filter((gv) => buckets[String(gv)] && buckets[String(gv)].length).map(
        (gv) => {
          const meta = pointGroupMeta(gv) || {};
          const pts = buckets[String(gv)];
          return {
            key: String(gv),
            groupType: gv,
            title: this.$t(meta.labelKey || "monitor.remoteGroupOther"),
            hint: meta.hintKey ? this.$t(meta.hintKey) : "",
            accent: meta.accent || "misc",
            writable: isPointGroupWritable(gv),
            layout: CHIP_GROUP_TYPES.has(gv) ? "chip" : "table",
            points: pts,
            valuedCount: pts.filter(
              (p) => !(p.value === undefined || p.value === null || String(p.value).trim() === "")
            ).length,
          };
        }
      );
      if (misc.length) {
        out.push({
          key: "x",
          groupType: null,
          title: this.$t("monitor.remoteGroupOther"),
          hint: "",
          accent: "misc",
          layout: "table",
          points: misc,
          valuedCount: misc.filter(
            (p) => !(p.value === undefined || p.value === null || String(p.value).trim() === "")
          ).length,
        });
      }
      return out;
    },
    visibleTabs() {
      const groups = this.displayGroups || [];
      const tabs = [{ id: "overview", count: this.filteredPoints.length }];
      groups.forEach((g) => {
        tabs.push({ id: g.key, count: g.points.length, ...g });
      });
      tabs.push({ id: "flat", count: this.filteredPoints.length });
      return tabs;
    },
  },
  watch: {
    visibleTabs: {
      handler(tabs) {
        const names = (tabs || []).map((t) => t.id);
        if (!names.length) return;
        if (!names.includes(this.activeTab)) {
          this.activeTab = names.includes("overview") ? "overview" : names[0];
        }
      },
      immediate: true,
    },
    deviceName() {
      this.activeTab = "overview";
      this.draftValues = {};
      this.dispatchingCodes = {};
    },
    points: {
      handler(list) {
        this.syncDraftFromPoints(list);
      },
      deep: true,
    },
  },
  methods: {
    resolvePointGroupType,
    pointWritable(pt) {
      return isPointGroupWritable(resolvePointGroupType(pt));
    },
    pointDraftKey(pt) {
      const code = pt && pt.code;
      return code === null || code === undefined || code === "" ? null : String(code);
    },
    syncDraftFromPoints(list) {
      const next = { ...this.draftValues };
      (list || []).forEach((pt) => {
        if (!this.pointWritable(pt)) return;
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
    async dispatchPoint(pt) {
      if (!this.writeEnabled) {
        this.$message.warning(this.$t("monitor.auxWriteDisabledHint"));
        return;
      }
      const devType = this.rtdbDevType;
      const devId = this.rtdbDevId;
      const pointId = pt && pt.code;
      if (devType === null || devType === undefined || devId === null || devId === undefined) {
        this.$message.warning(this.$t("monitor.auxWriteDisabledHint"));
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
        this.$emit("point-dispatched", { code: pointId, value });
      } catch (e) {
        this.$message.error(this.$t("monitor.auxDispatchFail"));
      } finally {
        if (key) this.$delete(this.dispatchingCodes, key);
      }
    },
    tabLabel(tab) {
      if (tab.id === "overview") return this.$t("monitor.auxTabOverview");
      if (tab.id === "flat") return this.$t("monitor.auxTabFlatList");
      const base = tab.title || tab.id;
      const n = tab.count || 0;
      return n > 0 ? `${base} (${n})` : base;
    },
    groupTitle(row) {
      const g = resolvePointGroupType(row);
      if (g === null) return this.$t("monitor.remoteGroupOther");
      return this.groupTitleFn(g);
    },
    cellLabel(pt) {
      return this.pointNameFn(pt) || pt.name || pt.ename || "--";
    },
    formatVal(pt) {
      const v = pt.value;
      if (v === null || v === undefined || String(v).trim() === "") return "—";
      const u = pt.unit != null && pt.unit !== "" ? ` ${pt.unit}` : "";
      return `${v}${u}`;
    },
  },
};
</script>

<style scoped>
.aux-monitor {
  padding: 4px 4px 28px;
}

.aux-monitor__hero {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  gap: 16px;
  margin-bottom: 12px;
  padding: 16px 18px;
  border-radius: 10px;
  background: #fff;
  border: 1px solid #e2e8ec;
  border-left: 4px solid #2a9d8f;
  box-shadow: 0 1px 4px rgba(15, 35, 52, 0.06);
}

.aux-monitor__kicker {
  display: block;
  font-size: 11px;
  letter-spacing: 0.12em;
  text-transform: uppercase;
  color: #2a9d8f;
  font-weight: 600;
  margin-bottom: 6px;
}

.aux-monitor__title {
  margin: 0 0 8px;
  font-size: 20px;
  font-weight: 600;
  color: #1a2b3c;
}

.aux-monitor__meta {
  margin: 0;
  font-size: 13px;
  color: #5c6b7a;
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 8px;
}

.aux-monitor__badge {
  padding: 2px 10px;
  border-radius: 4px;
  background: #e8f4f8;
  color: #1d6a7a;
  font-weight: 500;
}

.aux-monitor__proto,
.aux-monitor__ids {
  font-size: 12px;
  color: #8a96a3;
}

.aux-monitor__status {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  padding: 6px 14px;
  border-radius: 999px;
  font-size: 13px;
  font-weight: 500;
  white-space: nowrap;
}

.aux-monitor__status.is-on {
  color: #1e7e4a;
  background: #e6f6ed;
  border: 1px solid #b8e6cc;
}

.aux-monitor__status.is-off {
  color: #b45309;
  background: #fef3e2;
  border: 1px solid #fcd9a8;
}

.aux-monitor__dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: currentColor;
}

.aux-monitor__kpi-strip {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin-bottom: 12px;
}

.aux-monitor__kpi {
  flex: 1 1 100px;
  min-width: 96px;
  padding: 8px 12px;
  border-radius: 8px;
  border: 1px solid #e8ecef;
  background: #fff;
}

.aux-monitor__kpi-label {
  display: block;
  font-size: 11px;
  color: #6b7c8d;
  margin-bottom: 4px;
}

.aux-monitor__kpi-value {
  font-size: 18px;
  font-weight: 700;
  color: #1a2b3c;
}

.aux-monitor__kpi--teal { border-top: 2px solid #14b8a6; }
.aux-monitor__kpi--sky { border-top: 2px solid #0ea5e9; }
.aux-monitor__kpi--slate { border-top: 2px solid #64748b; }

.aux-monitor__toolbar {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  justify-content: space-between;
  gap: 12px;
  margin-bottom: 12px;
  padding: 10px 12px;
  background: #fff;
  border: 1px solid #e8ecef;
  border-radius: 8px;
}

.aux-monitor__search {
  max-width: 320px;
  flex: 1;
  min-width: 180px;
}

.aux-monitor__empty {
  padding: 40px;
  text-align: center;
  color: #8a96a3;
  background: #fff;
  border-radius: 8px;
  border: 1px dashed #d5dde3;
}

.aux-monitor__tabs >>> .el-tabs__header {
  margin-bottom: 12px;
}

.aux-tab-hint {
  margin: 0 0 10px;
  font-size: 12px;
  color: #6b7c8d;
}

.aux-overview-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(180px, 1fr));
  gap: 10px;
}

.aux-overview-card {
  padding: 12px;
  border-radius: 8px;
  border: 1px solid #e8ecef;
  background: #fff;
  cursor: pointer;
  transition: box-shadow 0.15s ease, transform 0.15s ease;
}

.aux-overview-card:hover {
  box-shadow: 0 4px 12px rgba(15, 35, 52, 0.08);
  transform: translateY(-1px);
}

.aux-overview-card-title {
  font-size: 14px;
  font-weight: 600;
  color: #1a2b3c;
  margin-bottom: 6px;
}

.aux-overview-card-metrics {
  display: flex;
  gap: 10px;
  font-size: 12px;
  color: #64748b;
}

.aux-overview-card-hint {
  margin: 8px 0 0;
  font-size: 11px;
  color: #94a3b8;
  line-height: 1.35;
}

.aux-overview-card--yx { border-left: 3px solid #6366f1; }
.aux-overview-card--yc { border-left: 3px solid #0ea5e9; }
.aux-overview-card--yk { border-left: 3px solid #f59e0b; }
.aux-overview-card--yt { border-left: 3px solid #8b5cf6; }
.aux-overview-card--set { border-left: 3px solid #64748b; }
.aux-overview-card--ym { border-left: 3px solid #06b6d4; }
.aux-overview-card--fault { border-left: 3px solid #ef4444; }
.aux-overview-card--event { border-left: 3px solid #ec4899; }
.aux-overview-card--misc { border-left: 3px solid #94a3b8; }

.aux-chip-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(160px, 1fr));
  gap: 8px;
}

.aux-chip {
  padding: 10px 12px;
  border-radius: 8px;
  border: 1px solid #eef2f5;
  background: #f8fafb;
}

.aux-chip.is-alarm {
  border-color: #f5c6cb;
  background: #fff5f5;
}

.aux-chip-label {
  display: block;
  font-size: 11px;
  color: #6b7c8d;
  margin-bottom: 4px;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.aux-chip-value {
  font-size: 16px;
  font-weight: 600;
  color: #1a2b3c;
}

.aux-chip-value small {
  margin-left: 4px;
  font-size: 11px;
  font-weight: 400;
  color: #8a96a3;
}

.aux-chip.is-alarm .aux-chip-value {
  color: #c0392b;
}

.aux-chip--yx { border-top: 2px solid #6366f1; }
.aux-chip--yc { border-top: 2px solid #0ea5e9; }
.aux-chip--yk { border-top: 2px solid #f59e0b; }
.aux-chip--yt { border-top: 2px solid #8b5cf6; }
.aux-chip--fault { border-top: 2px solid #ef4444; }
.aux-chip--event { border-top: 2px solid #ec4899; }

.aux-val-alarm {
  color: #c0392b;
  font-weight: 600;
}

.aux-group-table,
.aux-flat-table {
  width: 100%;
}

.aux-tab-hint--write {
  color: #0d6efd;
}

.aux-tab-hint--muted {
  color: #b45309;
}

.aux-chip--writable {
  min-height: 88px;
}

.aux-chip-write,
.aux-row-write {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 6px;
  margin-top: 6px;
}

.aux-chip-input,
.aux-row-input {
  width: 108px;
}

.aux-chip-readback {
  display: block;
  margin-top: 4px;
  font-size: 11px;
  color: #94a3b8;
}

.aux-readonly-dash {
  color: #cbd5e1;
}
</style>
