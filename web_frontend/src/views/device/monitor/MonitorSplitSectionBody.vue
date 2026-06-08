<template>
  <div class="split-rich-stack">
    <div v-if="displaySplit.gauges.length" class="rich-gauges">
      <div
        v-for="(row, gidx) in displaySplit.gauges"
        :key="'g-' + sectionKey + '-' + gidx"
        class="rich-gauge-item"
      >
        <el-progress
          type="dashboard"
          :percentage="Math.round(row._gaugePct)"
          :color="m.gaugeProgressColor(row._gaugePct)"
          :width="132"
          :stroke-width="8"
        />
        <div class="rich-gauge-caption">{{ m.pointDisplayName(row) }}</div>
        <div class="rich-gauge-value">
          {{ row.value }}
          <span v-if="row.unit" class="rich-gauge-unit">{{ row.unit }}</span>
        </div>
      </div>
    </div>
    <div v-if="displaySplit.stats.length" class="rich-stats-grid">
      <div
        v-for="(row, sidx) in displaySplit.stats"
        :key="'s-' + sectionKey + '-' + sidx"
        class="rich-stat-tile"
      >
        <div class="rst-label">{{ m.pointDisplayName(row) }}</div>
        <div class="rst-value-row">
          <span class="rst-value">{{ row.value }}</span>
          <span v-if="row.unit" class="rst-unit">{{ row.unit }}</span>
        </div>
        <el-progress
          v-if="row._statBarPct != null"
          :show-text="false"
          :stroke-width="5"
          :percentage="row._statBarPct"
          class="rst-bar"
        />
      </div>
    </div>
    <div v-if="displaySplit.chips.length" class="rich-chip-panel rich-chip-panel--flat">
      <div class="rich-chip-row">
        <el-tag
          v-for="(row, cidx) in displaySplit.chips"
          :key="'c-' + sectionKey + '-' + cidx"
          :type="m.pointChipTagType(row)"
          effect="plain"
          size="small"
          class="rich-chip-tag"
        >
          <span class="chip-name">{{ m.pointDisplayName(row) }}</span>
          <span class="chip-sep">:</span>
          <span class="chip-val">{{ row.value }}</span>
          <span v-if="row.unit" class="chip-unit">{{ row.unit }}</span>
        </el-tag>
      </div>
    </div>
    <el-table
      v-if="!displaySplit._hasRichBlocks"
      :data="tableData"
      border
      size="small"
      class="rich-detail-table"
      :height="card.tableHeight"
    >
      <el-table-column prop="name" :label="$t('monitor.测点')" min-width="160">
        <template slot-scope="scope">
          <span>{{ m.pointDisplayName(scope.row) }}</span>
        </template>
      </el-table-column>
      <el-table-column prop="value" :label="$t('monitor.值')" min-width="110">
        <template slot-scope="scope">
          <span :class="{ warning: Number(scope.row.groupType) === 6 }">{{ scope.row.value }}</span>
        </template>
      </el-table-column>
      <el-table-column prop="unit" :label="$t('monitor.单位')" width="80">
        <template slot-scope="scope">
          <span>{{ scope.row.unit || '--' }}</span>
        </template>
      </el-table-column>
    </el-table>
  </div>
</template>

<script>
export default {
  name: "MonitorSplitSectionBody",
  inject: {
    monitor: { default: null },
  },
  props: {
    sec: { type: Object, required: true },
    card: { type: Object, required: true },
  },
  computed: {
    m() {
      return this.monitor;
    },
    sectionKey() {
      return String((this.sec && this.sec.key) || "sec");
    },
    displaySplit() {
      const ds = (this.sec && this.sec.displaySplit) || {};
      return {
        gauges: ds.gauges || [],
        stats: ds.stats || [],
        chips: ds.chips || [],
        tableRows: ds.tableRows || [],
        _hasRichBlocks: !!ds._hasRichBlocks,
      };
    },
    tableData() {
      const rows = this.displaySplit.tableRows;
      if (rows && rows.length) return rows;
      return (this.sec && this.sec.points) || [];
    },
  },
};
</script>
