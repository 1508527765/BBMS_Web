<template>
  <div class="local-dashboard">
            <!-- 本机系统：仪表盘顶栏（左大卡 + 右摘要） -->
            <el-row v-if="m.isLocalSystemMonitor" :gutter="16" class="dashboard-top-row">
              <el-col :xs="24" :md="15" :lg="16">
                <div class="dashboard-hero-card">
                  <div class="dhero-kicker">{{ $t('monitor.dashboardKicker') }}</div>
                  <h2 class="dhero-heading">{{ m.currentNodeLabel }}</h2>
                  <p class="dhero-lead">{{ $t('monitor.dashboardLead') }}</p>
                  <div class="dhero-metrics">
                    <div v-for="hero in m.dashboardHeroMetrics" :key="hero.key" class="dhero-metric">
                      <div class="dhero-metric-value">{{ hero.value }}</div>
                      <div class="dhero-metric-label">{{ hero.label }}</div>
                    </div>
                  </div>
                  <div v-if="m.isLocalSystemMonitor" class="dhero-anchor-grid">
                    <div
                      v-for="a in m.localHostAnchorKpis"
                      :key="a.key"
                      class="dhero-anchor-cell"
                    >
                      <div class="dhero-anchor-value">
                        {{ a.valueText }}<span v-if="a.unit" class="dhero-anchor-unit">{{ a.unit }}</span>
                      </div>
                      <div class="dhero-anchor-label">{{ a.label }}</div>
                    </div>
                  </div>
                </div>
              </el-col>
              <el-col :xs="24" :md="9" :lg="8">
                <div
                  class="dashboard-tile-grid"
                  :class="{ 'dashboard-tile-grid--nine': m.isLocalSystemMonitor }"
                >
                  <div
                    v-for="t in m.dashboardSideTiles"
                    :key="t.key"
                    class="dashboard-tile"
                    :class="t.tone"
                  >
                    <div class="dashboard-tile-label">{{ t.label }}</div>
                    <div class="dashboard-tile-value">{{ t.value }}</div>
                  </div>
                </div>
              </el-col>
            </el-row>
            <div class="info-card monitor-toolbar-card">
              <el-row :gutter="12">
                <el-col :span="12">
                  <el-input
                    :value="m.pointKeyword" @input="v => (m.pointKeyword = v)"
                    clearable
                    :placeholder="$t('monitor.pointSearchPlaceholder')"
                    size="small"
                  />
                </el-col>
                <el-col :span="12" style="display:flex;align-items:center;justify-content:flex-end;">
                  <el-switch
                    :value="m.onlyShowValued" @input="v => (m.onlyShowValued = v)"
                    :active-text="$t('monitor.onlyValuedOn')"
                    :inactive-text="$t('monitor.onlyValuedOff')"
                  />
                </el-col>
              </el-row>
            </div>
            <div
              class="point-card-list"
              :class="{
                'point-card-list--multi-grid': m.monitorPointMultiGrid,
                'point-card-list--grid-3-fixed': m.isLocalSystemMonitor && m.monitorPointMultiGrid,
              }"
            >
              <template v-for="g in m.displayGroupedPointCards">
                <div
                  v-if="g.isPlaceholder"
                  :key="g.key"
                  class="monitor-point-card monitor-point-card--slot-empty"
                  aria-hidden="true"
                />
                <div
                  v-else
                  :key="g.key"
                  class="info-card monitor-point-card"
                  :class="[
                    g.layoutClass,
                    { 'monitor-point-card--airy': g.renderMode === 'split' || g.renderMode === 'rich' },
                  ]"
                >
                <div class="info-card-title monitor-point-title">
                  <span class="title-badge" :class="g.themeClass">{{ g.title }}</span>
                  <span class="title-count">{{ $t('monitor.cardPointCount', { n: g.points.length }) }}</span>
                </div>
                <div
                  v-if="g.renderMode === 'split'"
                  class="split-sections"
                  :class="{ 'split-sections--tabs': g.sections && g.sections.length > 1 }"
                >
                  <div
                    v-if="g.sections && g.sections.length === 1"
                    class="split-tab-body split-tab-body--flat"
                  >
                    <monitor-split-section-body :sec="g.sections[0]" :card="g" />
                  </div>
                  <el-tabs
                    v-else
                    class="monitor-split-tabs"
                    type="card"
                    :value="m.splitTabByGroupKey[String(g.key)] || (g.sections[0] && String(g.sections[0].key))"
                    @input="name => m.handleMonitorSplitTab(g.key, name)"
                  >
                    <el-tab-pane
                      v-for="sec in g.sections"
                      :key="sec.key"
                      :label="sec.title"
                      :name="String(sec.key)"
                    >
                      <div class="split-tab-body">
                        <monitor-split-section-body :sec="sec" :card="g" />
                      </div>
                    </el-tab-pane>
                  </el-tabs>
                </div>
                <div v-else-if="g.renderMode === 'rich'" class="monitor-rich-panel">
                  <div class="split-tab-body split-tab-body--flat">
                    <div class="split-rich-stack">
                      <div
                        v-if="g.displaySplit && g.displaySplit.gauges.length"
                        class="rich-gauges"
                      >
                        <div
                          v-for="(row, gidx) in g.displaySplit.gauges"
                          :key="'rg-' + g.key + '-' + gidx"
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
                      <div
                        v-if="g.displaySplit && g.displaySplit.stats.length"
                        class="rich-stats-grid"
                      >
                        <div
                          v-for="(row, sidx) in g.displaySplit.stats"
                          :key="'rs-' + g.key + '-' + sidx"
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
                      <div
                        v-if="g.displaySplit && g.displaySplit.chips.length"
                        class="rich-chip-panel rich-chip-panel--flat"
                      >
                        <div class="rich-chip-row">
                          <el-tag
                            v-for="(row, cidx) in g.displaySplit.chips"
                            :key="'rc-' + g.key + '-' + cidx"
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
                        v-if="!g.displaySplit || !g.displaySplit._hasRichBlocks"
                        :data="
                          g.displaySplit && g.displaySplit.tableRows && g.displaySplit.tableRows.length
                            ? g.displaySplit.tableRows
                            : g.points
                        "
                        border
                        size="small"
                        class="rich-detail-table"
                        :height="g.tableHeight"
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
                  </div>
                </div>
                <el-table
                  v-else
                  :data="g.points"
                  border
                  size="small"
                  class="rich-detail-table"
                  :height="g.tableHeight"
                >
                  <el-table-column prop="name" :label="$t('monitor.测点')" min-width="200">
                    <template slot-scope="scope">
                      <span>{{ m.pointDisplayName(scope.row) }}</span>
                    </template>
                  </el-table-column>
                  <el-table-column prop="value" :label="$t('monitor.值')" min-width="160">
                    <template slot-scope="scope">
                      <span :class="{ warning: Number(scope.row.groupType) === 6 }">{{ scope.row.value }}</span>
                    </template>
                  </el-table-column>
                  <el-table-column prop="unit" :label="$t('monitor.单位')" width="100">
                    <template slot-scope="scope">
                      <span>{{ scope.row.unit || '--' }}</span>
                    </template>
                  </el-table-column>
                </el-table>
              </div>
              </template>
            </div>
            <div v-if="!m.groupedPointCards.length" class="no-data">{{ $t('monitor.暂无数据') }}</div>
  </div>
</template>

<script>
import MonitorSplitSectionBody from "./MonitorSplitSectionBody.vue";

export default {
  name: "LocalDashboard",
  components: { MonitorSplitSectionBody },
  inject: {
    monitor: { default: null },
  },
  computed: {
    m() {
      return this.monitor;
    },
  },
};

</script>

<style scoped lang="scss">
@import "./styles/panel-shared.scss";
@import "./styles/local.scss";
</style>
