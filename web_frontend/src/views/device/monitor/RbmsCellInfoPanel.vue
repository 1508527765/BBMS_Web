<template>
  <div class="rbms-cell-info">
    <!-- 簇级统计：左电压、右温度，强制同一行占满宽度 -->
    <div v-if="clusterStats.length" class="rbms-cell-stats-row">
      <section class="rbms-cell-stats-row__side rbms-cell-stats-row__side--v">
        <header class="rbms-cell-stats-row__head">
          <i class="el-icon-data-line" />
          <span>{{ $t("monitor.rbmsCellGroupVoltage") }}</span>
        </header>
        <div class="rbms-cell-stats-row__metrics">
          <div v-for="s in voltStats" :key="s.key" class="rbms-cell-stats-row__metric">
            <span class="rbms-cell-stats-row__metric-label">{{ s.label }}</span>
            <span class="rbms-cell-stats-row__metric-value">{{ s.valueText }}</span>
          </div>
        </div>
      </section>
      <section class="rbms-cell-stats-row__side rbms-cell-stats-row__side--t">
        <header class="rbms-cell-stats-row__head">
          <i class="el-icon-sunny" />
          <span>{{ $t("monitor.rbmsCellGroupTemperature") }}</span>
        </header>
        <div class="rbms-cell-stats-row__metrics">
          <div v-for="s in tempStats" :key="s.key" class="rbms-cell-stats-row__metric">
            <span class="rbms-cell-stats-row__metric-label">{{ s.label }}</span>
            <span class="rbms-cell-stats-row__metric-value">{{ s.valueText }}</span>
          </div>
        </div>
      </section>
    </div>

    <!-- 筛选 -->
    <div class="rbms-cell-toolbar">
      <div class="rbms-cell-toolbar__chips">
        <button
          v-for="opt in filterOptions"
          :key="opt.value"
          type="button"
          class="rbms-cell-toolbar__chip"
          :class="{ 'is-active': cellFilter === opt.value }"
          @click="cellFilter = opt.value"
        >
          {{ opt.label }}
          <b>{{ opt.count }}</b>
        </button>
      </div>
      <el-input
        v-model="jumpInput"
        size="small"
        clearable
        class="rbms-cell-toolbar__search"
        :placeholder="$t('monitor.rbmsCellJumpPlaceholder')"
        prefix-icon="el-icon-search"
        @keyup.enter.native="jumpToCell"
      />
    </div>

    <!-- 52 节 / 组：仅显示当前组，4×13 小方块 -->
    <div v-if="filteredCells.length && activePack" class="rbms-cell-pack-panel">
      <div class="rbms-cell-pack-panel__nav">
        <span class="rbms-cell-pack-panel__nav-label">{{ $t("monitor.rbmsCellPackSelect") }}</span>
        <el-select
          v-model="activePackIndex"
          size="small"
          class="rbms-cell-pack-panel__select"
          @change="onPackIndexChange"
        >
          <el-option
            v-for="(grp, idx) in displayGroups"
            :key="'opt-' + idx"
            :label="packOptionLabel(grp, idx)"
            :value="idx"
          />
        </el-select>
        <el-button-group class="rbms-cell-pack-panel__pager">
          <el-button
            size="small"
            icon="el-icon-arrow-left"
            :disabled="activePackIndex <= 0"
            @click="goPack(-1)"
          />
          <el-button
            size="small"
            icon="el-icon-arrow-right"
            :disabled="activePackIndex >= displayGroups.length - 1"
            @click="goPack(1)"
          />
        </el-button-group>
        <span class="rbms-cell-pack-panel__range">
          {{ $t("monitor.rbmsCellPackTitle", { from: activePack.from, to: activePack.to }) }}
          · {{ activePack.cells.length }} {{ $t("monitor.rbmsCellPackCells") }}
        </span>
      </div>
      <div class="rbms-cell-pack-body" :style="{ '--pack-grid-cols': gridCols }">
        <div
          class="rbms-cell-card-grid"
          :style="{ '--cell-grid-rows': gridRows }"
        >
          <button
            v-for="cell in activePack.cells"
            :id="'rbms-cell-tile-' + cell.index0"
            :key="'cell-' + cell.index0"
            type="button"
            class="rbms-cell-square"
            :class="tileClass(cell)"
            :title="tileTitle(cell)"
            @click="openDetail(cell.index0)"
          >
            <span class="rbms-cell-square__idx">{{ cell.index1 }}</span>
            <div class="rbms-cell-square__metric rbms-cell-square__metric--v">
              <span class="rbms-cell-square__lbl">{{ $t("monitor.rbmsCellTileVolt") }}</span>
              <span class="rbms-cell-square__val">{{ formatVoltShort(cell) }}</span>
            </div>
            <div class="rbms-cell-square__metric rbms-cell-square__metric--bal">
              <span class="rbms-cell-square__lbl">{{ $t("monitor.rbmsCellTileBal") }}</span>
              <span class="rbms-cell-square__val">{{ formatBalShort(cell) }}</span>
            </div>
            <div class="rbms-cell-square__metric rbms-cell-square__metric--sdr">
              <span class="rbms-cell-square__lbl">{{ $t("monitor.rbmsCellTileSdr") }}</span>
              <span class="rbms-cell-square__val">{{ formatSdrShort(cell) }}</span>
            </div>
          </button>
        </div>
        <section class="rbms-cell-temp-panel">
          <header class="rbms-cell-temp-panel__head">
            <i class="el-icon-sunny" />
            <span>{{ $t("monitor.rbmsCellPackTempTitle") }}</span>
            <b>{{ $t("monitor.rbmsCellPackTempCount", { n: tempPerPack }) }}</b>
          </header>
          <div class="rbms-cell-temp-grid">
            <button
              v-for="mod in activePackTemps"
              :key="'temp-' + mod.index0"
              type="button"
              class="rbms-cell-temp-square"
              :class="tempTileClass(mod)"
              :title="tempTileTitle(mod)"
              @click="openTempDetail(mod)"
            >
              <span class="rbms-cell-temp-square__idx">T{{ mod.index1 }}</span>
              <span class="rbms-cell-temp-square__val">{{ formatTempModuleShort(mod) }}</span>
            </button>
          </div>
        </section>
      </div>
    </div>
    <div v-else-if="cells.length" class="rbms-cell-empty">
      <i class="el-icon-warning-outline" />
      <p>{{ $t("monitor.rbmsCellFilterNoMatch") }}</p>
    </div>
    <div v-else class="rbms-cell-empty">
      <i class="el-icon-box" />
      <p>{{ $t("monitor.rbmsCellGridEmpty") }}</p>
    </div>

    <el-collapse v-if="auxSections.length" class="rbms-cell-aux-collapse" accordion>
      <el-collapse-item :title="$t('monitor.rbmsCellAuxTitle')" name="aux">
        <div v-for="sec in auxSections" :key="'aux-' + sec.id" class="rbms-cell-aux-block">
          <header class="rbms-tab-section-head">
            <h5>{{ $t(sec.titleKey) }}</h5>
            <span class="rbms-tab-section-count">{{ sec.rows.length }}</span>
          </header>
          <el-table :data="sec.rows" border stripe size="mini" class="rbms-section-table" max-height="200">
            <el-table-column :label="$t('monitor.monitorPointName')" min-width="140">
              <template v-slot="scope">{{ m.pointDisplayName(scope.row) }}</template>
            </el-table-column>
            <el-table-column :label="$t('monitor.值')" width="110">
              <template v-slot="scope">{{ m.rbmsDisplayValue(scope.row) }}</template>
            </el-table-column>
          </el-table>
        </div>
      </el-collapse-item>
    </el-collapse>

    <!-- 居中弹窗：电芯详情 / 温度模块曲线 -->
    <el-dialog
      :visible.sync="dialogVisible"
      :title="dialogTitle"
      width="760px"
      top="6vh"
      append-to-body
      custom-class="rbms-cell-dialog"
      @opened="onDialogOpened"
      @closed="onDialogClosed"
    >
      <div v-if="dialogMode === 'cell' && detailCell" class="rbms-cell-modal">
        <div class="rbms-cell-modal__hero" :class="'rbms-cell-modal__hero--' + cellStatus(detailCell)">
          <div class="rbms-cell-modal__hero-no">{{ detailCell.index1 }}</div>
          <div class="rbms-cell-modal__hero-grid rbms-cell-modal__hero-grid--3">
            <div class="rbms-cell-modal__hero-item">
              <label>{{ $t("monitor.rbmsCellDrawerVolt") }}</label>
              <strong>{{ formatVolt(detailCell) }}</strong>
            </div>
            <div class="rbms-cell-modal__hero-item">
              <label>{{ $t("monitor.rbmsCellDrawerBal") }}</label>
              <strong>{{ formatBal(detailCell) }}</strong>
            </div>
            <div class="rbms-cell-modal__hero-item">
              <label>{{ $t("monitor.rbmsCellDrawerSdr") }}</label>
              <strong>{{ formatSdr(detailCell) }}</strong>
            </div>
          </div>
        </div>
        <div class="rbms-cell-modal__charts rbms-cell-modal__charts--single">
          <div class="rbms-cell-modal__chart-box">
            <h4>{{ $t("monitor.rbmsCellChartVolt") }}</h4>
            <div ref="voltChart" class="rbms-cell-modal__chart" />
          </div>
        </div>
        <p class="rbms-cell-modal__note">{{ $t("monitor.rbmsCellChartNote") }}</p>
      </div>
      <div v-else-if="dialogMode === 'temp' && detailTemp" class="rbms-cell-modal">
        <div class="rbms-cell-modal__hero rbms-cell-modal__hero--temp" :class="'rbms-cell-modal__hero--' + tempStatus(detailTemp)">
          <div class="rbms-cell-modal__hero-no">T{{ detailTemp.index1 }}</div>
          <div class="rbms-cell-modal__hero-grid rbms-cell-modal__hero-grid--1">
            <div class="rbms-cell-modal__hero-item">
              <label>{{ $t("monitor.rbmsCellDrawerTemp") }}</label>
              <strong>{{ formatTempModule(detailTemp) }}</strong>
            </div>
          </div>
        </div>
        <div class="rbms-cell-modal__charts rbms-cell-modal__charts--single">
          <div class="rbms-cell-modal__chart-box">
            <h4>{{ $t("monitor.rbmsCellChartTemp") }}</h4>
            <div ref="tempChart" class="rbms-cell-modal__chart" />
          </div>
        </div>
        <p class="rbms-cell-modal__note">{{ $t("monitor.rbmsCellChartNote") }}</p>
      </div>
    </el-dialog>
  </div>
</template>

<script>
import * as echarts from "echarts";
import { RBMS_CELL_UI, buildRbmsPackTempGrid } from "@/constants/rbmsPointSections";

const AUX_SECTION_IDS = new Set([
  "cell_volt_valid",
  "afe_volt",
  "temp_pole",
  "temp_pack",
  "temp_bal_valid",
  "temp_bal_board",
]);

const PACK_SIZE = RBMS_CELL_UI.PACK_SIZE;
const VOLT_KEYS = new Set(["vmax", "vmin", "vavg"]);
const TEMP_KEYS = new Set(["tmax", "tmin", "tavg"]);

export default {
  name: "RbmsCellInfoPanel",
  inject: { monitor: { default: null } },
  data() {
    return {
      dialogVisible: false,
      dialogMode: "cell",
      detailTempIdx: null,
      voltChartInst: null,
      tempChartInst: null,
      jumpInput: "",
      cellFilter: "all",
      activePackIndex: 0,
    };
  },
  computed: {
    m() {
      return this.monitor;
    },
    cells() {
      return (this.m && this.m.rbmsCellGrid) || [];
    },
    clusterStats() {
      return (this.m && this.m.rbmsCellClusterStats) || [];
    },
    voltStats() {
      return this.clusterStats.filter((s) => VOLT_KEYS.has(s.key));
    },
    tempStats() {
      return this.clusterStats.filter((s) => TEMP_KEYS.has(s.key));
    },
    statusCounts() {
      const c = { all: this.cells.length, ok: 0, bal: 0, hot: 0, empty: 0 };
      this.cells.forEach((cell) => {
        const st = this.cellStatus(cell);
        if (st === "ok") c.ok += 1;
        else if (st === "bal") c.bal += 1;
        else if (st === "empty") c.empty += 1;
      });
      const points = (this.m && this.m.filteredPointsFlatTable) || [];
      const packCount = Math.max(1, Math.ceil(this.cells.length / PACK_SIZE));
      for (let p = 0; p < packCount; p++) {
        buildRbmsPackTempGrid(points, p).forEach((mod) => {
          if (this.tempStatus(mod) === "hot") c.hot += 1;
        });
      }
      return c;
    },
    filterOptions() {
      const c = this.statusCounts;
      return [
        { value: "all", label: this.$t("monitor.rbmsCellFilterAll"), count: c.all },
        { value: "bal", label: this.$t("monitor.rbmsCellFilterBal"), count: c.bal },
        { value: "hot", label: this.$t("monitor.rbmsCellFilterHot"), count: c.hot },
        { value: "empty", label: this.$t("monitor.rbmsCellFilterEmpty"), count: c.empty },
      ];
    },
    auxSections() {
      const list = (this.m && this.m.rbmsConsoleSections) || [];
      return list.filter((s) => AUX_SECTION_IDS.has(s.id));
    },
    filteredCells() {
      const list = this.cells;
      if (this.cellFilter === "all") return list;
      if (this.cellFilter === "hot") {
        const points = (this.m && this.m.filteredPointsFlatTable) || [];
        const hotPacks = new Set();
        const packCount = Math.max(1, Math.ceil(list.length / PACK_SIZE));
        for (let p = 0; p < packCount; p++) {
          const hasHot = buildRbmsPackTempGrid(points, p).some((mod) => this.tempStatus(mod) === "hot");
          if (hasHot) hotPacks.add(p);
        }
        return list.filter((c) => hotPacks.has(Math.floor(c.index0 / PACK_SIZE)));
      }
      return list.filter((c) => {
        const st = this.cellStatus(c);
        if (this.cellFilter === "bal") return st === "bal";
        if (this.cellFilter === "empty") return st === "empty";
        return true;
      });
    },
    displayGroups() {
      const list = this.filteredCells;
      const groups = [];
      for (let i = 0; i < list.length; i += PACK_SIZE) {
        const chunk = list.slice(i, i + PACK_SIZE);
        if (!chunk.length) continue;
        groups.push({
          from: chunk[0].index1,
          to: chunk[chunk.length - 1].index1,
          cells: chunk,
        });
      }
      return groups;
    },
    activePack() {
      const groups = this.displayGroups;
      if (!groups.length) return null;
      const idx = Math.min(Math.max(0, this.activePackIndex), groups.length - 1);
      return groups[idx];
    },
    activePackTemps() {
      const points = (this.m && this.m.filteredPointsFlatTable) || [];
      return buildRbmsPackTempGrid(points, this.activePackIndex);
    },
    tempPerPack() {
      return RBMS_CELL_UI.TEMP_PER_PACK;
    },
    detailIdx() {
      return this.m && this.m.rbmsCellDetailIdx != null ? this.m.rbmsCellDetailIdx : null;
    },
    detailCell() {
      if (this.detailIdx == null) return null;
      return this.cells.find((c) => c.index0 === this.detailIdx) || null;
    },
    detailTemp() {
      if (this.detailTempIdx == null) return null;
      const points = (this.m && this.m.filteredPointsFlatTable) || [];
      const packIdx = Math.floor(this.detailTempIdx / RBMS_CELL_UI.TEMP_PER_PACK);
      return (
        buildRbmsPackTempGrid(points, packIdx).find((m) => m.index0 === this.detailTempIdx) || null
      );
    },
    gridCols() {
      return RBMS_CELL_UI.GRID_COLS;
    },
    gridRows() {
      return RBMS_CELL_UI.GRID_ROWS;
    },
    dialogTitle() {
      if (this.dialogMode === "temp" && this.detailTemp) {
        return this.$t("monitor.rbmsTempDrawerTitle", { n: this.detailTemp.index1 });
      }
      if (this.dialogMode === "cell" && this.detailCell) {
        return this.$t("monitor.rbmsCellDrawerTitle", { n: this.detailCell.index1 });
      }
      return "";
    },
  },
  watch: {
    displayGroups(groups) {
      if (!groups.length) {
        this.activePackIndex = 0;
        return;
      }
      if (this.activePackIndex >= groups.length) {
        this.activePackIndex = groups.length - 1;
      }
    },
    cellFilter() {
      this.activePackIndex = 0;
    },
    cells: {
      deep: true,
      handler() {
        if (this.dialogVisible) this.$nextTick(() => this.renderCharts());
      },
    },
    activePackTemps: {
      deep: true,
      handler() {
        if (this.dialogVisible && this.dialogMode === "temp") {
          this.$nextTick(() => this.renderCharts());
        }
      },
    },
  },
  beforeDestroy() {
    this.disposeCharts();
  },
  methods: {
    cellStatus(cell) {
      if (!cell) return "empty";
      const hasV = this.hasVolt(cell);
      const hasSdr =
        cell.sdrValue != null && String(cell.sdrValue).trim() !== "" && Number(cell.sdrValue) !== 0;
      if (!hasV && cell.balancing !== true && !hasSdr) return "empty";
      if (cell.balancing === true) return "bal";
      return "ok";
    },
    tempStatus(mod) {
      if (!mod || mod.tempValue == null || String(mod.tempValue).trim() === "") return "empty";
      const n = Number(mod.tempValue);
      if (Number.isNaN(n) || n === 0) return "empty";
      if (n >= 45) return "hot";
      return "ok";
    },
    tileClass(cell) {
      const st = this.cellStatus(cell);
      return {
        ["rbms-cell-square--" + st]: true,
        "rbms-cell-square--selected": this.detailIdx === cell.index0,
      };
    },
    hasVolt(cell) {
      if (!cell || cell.voltValue == null) return false;
      const n = Number(cell.voltValue);
      return !Number.isNaN(n) && n !== 0;
    },
    voltUnit(cell) {
      return (cell.volt && cell.volt.unit) || "mV";
    },
    packOptionLabel(grp, idx) {
      return this.$t("monitor.rbmsCellPackOption", {
        n: idx + 1,
        from: grp.from,
        to: grp.to,
      });
    },
    goPack(delta) {
      const next = this.activePackIndex + delta;
      if (next < 0 || next >= this.displayGroups.length) return;
      this.activePackIndex = next;
    },
    onPackIndexChange() {
      /* v-model 已更新 */
    },
    jumpToCell() {
      const n = parseInt(String(this.jumpInput || "").replace(/\D/g, ""), 10);
      if (!n || n < 1) return;
      this.activePackIndex = Math.floor((n - 1) / PACK_SIZE);
      this.$nextTick(() => {
        const el = document.getElementById("rbms-cell-tile-" + (n - 1));
        if (el) {
          el.scrollIntoView({ behavior: "smooth", block: "center" });
          el.classList.add("rbms-cell-square--flash");
          setTimeout(() => el.classList.remove("rbms-cell-square--flash"), 1400);
        }
      });
    },
    tileTitle(cell) {
      return `#${cell.index1}  V ${this.formatVolt(cell)}  B ${this.formatBal(cell)}  SDR ${this.formatSdr(cell)}`;
    },
    tempTileClass(mod) {
      return {
        ["rbms-cell-temp-square--" + this.tempStatus(mod)]: true,
        "rbms-cell-temp-square--selected": this.dialogMode === "temp" && this.detailTempIdx === mod.index0,
      };
    },
    tempTileTitle(mod) {
      return `T${mod.index1}  ${this.formatTempModule(mod)}`;
    },
    openDetail(index0) {
      this.dialogMode = "cell";
      this.detailTempIdx = null;
      if (this.m) this.m.rbmsCellDetailIdx = index0;
      this.dialogVisible = true;
    },
    openTempDetail(mod) {
      this.dialogMode = "temp";
      this.detailTempIdx = mod.index0;
      if (this.m) this.m.rbmsCellDetailIdx = null;
      this.dialogVisible = true;
    },
    onDialogOpened() {
      this.$nextTick(() => this.renderCharts());
    },
    onDialogClosed() {
      this.dialogMode = "cell";
      this.detailTempIdx = null;
      if (this.m) this.m.rbmsCellDetailIdx = null;
      this.disposeCharts();
    },
    formatVoltShort(cell) {
      if (!this.hasVolt(cell)) return "—";
      return String(cell.voltValue);
    },
    formatBalShort(cell) {
      if (cell.balancing === true) return this.$t("monitor.rbmsCellBalBadge");
      if (cell.balancing === false) return "—";
      return "—";
    },
    formatSdrShort(cell) {
      if (!cell || cell.sdrValue == null || String(cell.sdrValue).trim() === "") return "—";
      const n = Number(cell.sdrValue);
      if (Number.isNaN(n)) return String(cell.sdrValue);
      return String(cell.sdrValue);
    },
    formatTempModuleShort(mod) {
      if (!mod || mod.tempValue == null || String(mod.tempValue).trim() === "") return "—";
      const n = Number(mod.tempValue);
      if (Number.isNaN(n) || n === 0) return "—";
      return `${n}℃`;
    },
    formatTempModule(mod) {
      if (!mod || mod.tempValue == null) return "--";
      const u = (mod.temp && mod.temp.unit) || "℃";
      return `${mod.tempValue} ${u}`.trim();
    },
    formatVolt(cell) {
      if (!cell || cell.voltValue == null) return "--";
      const u = (cell.volt && cell.volt.unit) || "mV";
      return `${cell.voltValue} ${u}`.trim();
    },
    formatTemp(cell) {
      if (!cell || cell.tempValue == null) return "--";
      const u = (cell.temp && cell.temp.unit) || "℃";
      return `${cell.tempValue} ${u}`.trim();
    },
    formatSdr(cell) {
      if (!cell || cell.sdrValue == null) return "--";
      const u = (cell.sdr && cell.sdr.unit) || "";
      return `${cell.sdrValue} ${u}`.trim();
    },
    formatBal(cell) {
      if (cell.balancing === true) return this.$t("monitor.rbmsCellBalOn");
      if (cell.balancing === false) return this.$t("monitor.rbmsCellBalOff");
      return "--";
    },
    historyFromSlot(slot, field) {
      const h = slot || null;
      if (!h) return { labels: [], values: [] };
      const labels = (h.ts || []).map((t) => {
        const d = new Date(t);
        return `${d.getHours()}:${String(d.getMinutes()).padStart(2, "0")}:${String(d.getSeconds()).padStart(2, "0")}`;
      });
      return { labels, values: (h[field] || []).slice() };
    },
    cellHistorySeries(index0, field) {
      const h = (this.m && this.m.rbmsCellHist && this.m.rbmsCellHist[index0]) || null;
      return this.historyFromSlot(h, field);
    },
    tempHistorySeries(index0) {
      const h = (this.m && this.m.rbmsTempHist && this.m.rbmsTempHist[index0]) || null;
      return this.historyFromSlot(h, "temp");
    },
    renderCharts() {
      const baseOpt = (name, labels, values, color) => ({
        grid: { left: 48, right: 16, top: 32, bottom: 36 },
        tooltip: { trigger: "axis" },
        xAxis: {
          type: "category",
          data: labels,
          boundaryGap: false,
          axisLine: { lineStyle: { color: "#e2e8f0" } },
          axisLabel: { color: "#64748b", fontSize: 11 },
        },
        yAxis: {
          type: "value",
          scale: true,
          splitLine: { lineStyle: { color: "#f1f5f9" } },
          axisLabel: { color: "#64748b", fontSize: 11 },
        },
        series: [
          {
            name,
            type: "line",
            smooth: 0.35,
            showSymbol: labels.length <= 24,
            symbolSize: 6,
            data: values,
            lineStyle: { width: 2.5, color },
            areaStyle: {
              color: {
                type: "linear",
                x: 0,
                y: 0,
                x2: 0,
                y2: 1,
                colorStops: [
                  { offset: 0, color: color + "35" },
                  { offset: 1, color: color + "06" },
                ],
              },
            },
          },
        ],
      });
      if (this.dialogMode === "cell" && this.detailCell && this.detailIdx != null) {
        const volt = this.cellHistorySeries(this.detailIdx, "volt");
        if (this.$refs.voltChart) {
          if (!this.voltChartInst) this.voltChartInst = echarts.init(this.$refs.voltChart);
          this.voltChartInst.resize();
          this.voltChartInst.setOption(
            baseOpt(this.$t("monitor.rbmsCellChartVolt"), volt.labels, volt.values, "#4f46e5"),
            true
          );
        }
        return;
      }
      if (this.dialogMode === "temp" && this.detailTemp && this.detailTempIdx != null) {
        const temp = this.tempHistorySeries(this.detailTempIdx);
        if (this.$refs.tempChart) {
          if (!this.tempChartInst) this.tempChartInst = echarts.init(this.$refs.tempChart);
          this.tempChartInst.resize();
          this.tempChartInst.setOption(
            baseOpt(this.$t("monitor.rbmsCellChartTemp"), temp.labels, temp.values, "#f97316"),
            true
          );
        }
      }
    },
    disposeCharts() {
      if (this.voltChartInst) {
        this.voltChartInst.dispose();
        this.voltChartInst = null;
      }
      if (this.tempChartInst) {
        this.tempChartInst.dispose();
        this.tempChartInst = null;
      }
    },
  },
};
</script>

<style lang="scss">
@import "./styles/rbms-cell-info.scss";
</style>
