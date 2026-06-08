<template>
              <div class="bbms-console">
                <div class="bbms-console-masthead">
                  <div class="bbms-console-masthead-left">
                    <div class="bbms-console-eyebrow">
                      <span class="bbms-live-dot" aria-hidden="true" />
                      {{ $t("monitor.bbmsConsoleEyebrow") }}
                    </div>
                    <h2 class="bbms-console-title">{{ m.currentNodeLabel }}</h2>
                    <p class="bbms-console-scope">
                      {{
                        $t("monitor.bbmsScopeLine", {
                          rbms: m.bbmsScopedStats.rbmsCount,
                          pbms: m.bbmsScopedStats.pbmsCount,
                        })
                      }}
                    </p>
                    <div class="bbms-console-idline">
                      <span>{{ m.monitorSelectedDeviceTypeLabel }}</span>
                      <span class="bbms-console-idsep">·</span>
                      <span>ID {{ m.monitorFlatDeviceIdText }}</span>
                      <span class="bbms-console-idsep">·</span>
                      <span>{{ $t("monitor.dashboardTileTemplate") }} {{ m.monitorFlatTemplateIdText }}</span>
                    </div>
                    <div class="bbms-scope-pills">
                      <span class="bbms-scope-pill bbms-scope-pill--rbms">
                        {{ $t("monitor.bbmsTileRbms") }} {{ m.bbmsScopedStats.rbmsCount }}
                      </span>
                      <span class="bbms-scope-pill bbms-scope-pill--pbms">
                        {{ $t("monitor.bbmsTilePbms") }} {{ m.bbmsScopedStats.pbmsCount }}
                      </span>
                    </div>
                  </div>
                </div>

                <section class="bbms-stats-overview">
                  <div class="bbms-stats-overview-head">
                    <h3 class="bbms-stats-overview-title">{{ $t("monitor.bbmsStatOverview") }}</h3>
                    <span class="bbms-stats-overview-sub">{{ $t("monitor.bbmsConsolePoints", { n: m.filteredPointsFlatTable.length }) }}</span>
                  </div>

                  <div v-if="m.bbmsOverviewDeck.length" class="bbms-hero-deck">
                    <div
                      v-for="h in m.bbmsOverviewDeck"
                      :key="'hero-' + h.key"
                      class="bbms-hero-card"
                      :class="[
                        'bbms-hero-card--' + h.key,
                        'bbms-hero-card--accent-' + h.accent,
                        { 'bbms-hero-card--ring': h.gaugePct != null },
                      ]"
                    >
                      <div v-if="h.gaugePct != null" class="bbms-hero-ring-wrap">
                        <el-progress
                          type="dashboard"
                          :percentage="Math.round(h.gaugePct)"
                          :color="m.gaugeProgressColor(h.gaugePct)"
                          :width="108"
                          :stroke-width="7"
                        />
                      </div>
                      <div class="bbms-hero-card-body">
                        <div class="bbms-hero-card-label">{{ h.label }}</div>
                        <div class="bbms-hero-card-value">{{ h.valueText }}</div>
                      </div>
                    </div>
                  </div>
                  <div v-else class="bbms-hero-empty">{{ $t("monitor.bbmsHeroEmpty") }}</div>

                  <div v-if="m.bbmsStatBlocks.length" class="bbms-stat-groups">
                    <div
                      v-for="block in m.bbmsStatBlocks"
                      :key="'stat-' + block.id"
                      class="bbms-stat-group"
                      :class="'bbms-stat-group--' + block.tone"
                    >
                      <header class="bbms-stat-group-head">
                        <h4>{{ $t(block.titleKey) }}</h4>
                        <span class="bbms-stat-group-count">{{ block.pointCount }}</span>
                      </header>
                      <div class="bbms-stat-group-body">
                        <div
                          v-if="block.id === 'status' && block.chipRows.length"
                          class="bbms-status-chip-grid"
                        >
                          <div
                            v-for="(row, cidx) in block.chipRows"
                            :key="'sq-' + block.id + '-' + cidx"
                            class="bbms-status-chip-cell"
                            :class="'bbms-status-chip-cell--' + (m.pointChipTagType(row) || 'default')"
                          >
                            <span class="bbms-status-chip-label">{{ m.pointDisplayName(row) }}</span>
                            <span class="bbms-status-chip-value">{{ m.bbmsDisplayValue(row) }}</span>
                          </div>
                        </div>
                        <template v-else>
                        <div
                          v-if="block.displaySplit.gauges.length"
                          class="bbms-stat-gauges"
                        >
                          <div
                            v-for="(row, gidx) in block.displaySplit.gauges"
                            :key="'sg-' + block.id + '-' + gidx"
                            class="bbms-stat-gauge-item"
                          >
                            <el-progress
                              type="dashboard"
                              :percentage="Math.round(row._gaugePct)"
                              :color="m.gaugeProgressColor(row._gaugePct)"
                              :width="88"
                              :stroke-width="6"
                            />
                            <div class="bbms-stat-gauge-label">{{ m.pointDisplayName(row) }}</div>
                            <div class="bbms-stat-gauge-value">{{ m.bbmsDisplayValue(row) }}</div>
                          </div>
                        </div>
                        <div
                          v-if="block.displaySplit.stats.length"
                          class="bbms-stat-tiles"
                        >
                          <div
                            v-for="(row, sidx) in block.displaySplit.stats"
                            :key="'st-' + block.id + '-' + sidx"
                            class="bbms-stat-tile"
                          >
                            <div class="bbms-stat-tile-label">{{ m.pointDisplayName(row) }}</div>
                            <div class="bbms-stat-tile-value">{{ m.bbmsDisplayValue(row) }}</div>
                            <el-progress
                              v-if="row._statBarPct != null"
                              :show-text="false"
                              :stroke-width="4"
                              :percentage="row._statBarPct"
                              class="bbms-stat-tile-bar"
                            />
                          </div>
                        </div>
                        <div
                          v-if="block.displaySplit.chips.length"
                          class="bbms-stat-chips"
                        >
                          <el-tag
                            v-for="(row, cidx) in block.displaySplit.chips"
                            :key="'sc-' + block.id + '-' + cidx"
                            :type="m.pointChipTagType(row)"
                            effect="plain"
                            size="small"
                            class="bbms-stat-chip"
                          >
                            <span class="chip-name">{{ m.pointDisplayName(row) }}</span>
                            <span class="chip-sep">:</span>
                            <span class="chip-val">{{ row.value }}</span>
                          </el-tag>
                        </div>
                        </template>
                      </div>
                    </div>
                  </div>
                </section>

                <div class="bbms-console-toolbar">
                  <el-input
                    :value="m.pointKeyword" @input="v => (m.pointKeyword = v)"
                    clearable
                    class="bbms-console-search"
                    :placeholder="$t('monitor.pointSearchPlaceholder')"
                    size="small"
                    prefix-icon="el-icon-search"
                  />
                  <div class="bbms-console-toolbar-meta">
                    <el-switch
                      :value="m.onlyShowValued" @input="v => (m.onlyShowValued = v)"
                      :active-text="$t('monitor.onlyValuedOn')"
                      :inactive-text="$t('monitor.onlyValuedOff')"
                    />
                  </div>
                </div>

                <el-tabs :value="m.bbmsMainTab" @input="v => (m.bbmsMainTab = v)" class="bbms-main-tabs" type="border-card">
                  <el-tab-pane
                    v-for="tab in m.bbmsVisibleMainTabs"
                    :key="'bbms-tab-' + tab.id"
                    :label="m.bbmsMainTabLabel(tab)"
                    :name="tab.id"
                  >
                    <div v-if="tab.id === 'flat'" class="bbms-flat-wrap">
                      <el-table
                        :data="m.filteredPointsFlatTable"
                        border
                        stripe
                        size="small"
                        class="bbms-flat-table"
                        height="520"
                        :row-class-name="m.remoteTableRowClass"
                      >
                        <el-table-column prop="code" :label="$t('monitor.monitorPointCode')" width="88" />
                        <el-table-column prop="name" :label="$t('monitor.测点')" min-width="180" show-overflow-tooltip>
                          <template slot-scope="scope">
                            <span>{{ m.pointDisplayName(scope.row) }}</span>
                          </template>
                        </el-table-column>
                        <el-table-column prop="value" :label="$t('monitor.值')" min-width="100">
                          <template slot-scope="scope">
                            <span :class="{ 'bbms-cell-alarm': Number(scope.row.groupType) === 6 }">{{
                              scope.row.value
                            }}</span>
                          </template>
                        </el-table-column>
                        <el-table-column prop="unit" :label="$t('monitor.单位')" width="72" />
                      </el-table>
                    </div>

                    <bms-fault-panel
                      v-else-if="tab.id === 'fault'"
                      :sections="m.bbmsFaultPanelSections"
                    />

                    <bbms-thermal-panel
                      v-else-if="tab.id === 'thermal'"
                      :cells="m.bbmsLayoutCellsForTabId('thermal')"
                    />

                    <div v-else class="bbms-board-grid bbms-board-grid--tab">
                      <article
                        v-for="cell in m.bbmsLayoutCellsForTabId(tab.id)"
                        :key="cell.sec.id + '-' + tab.id"
                        class="bbms-board-card bbms-board-card-dyn"
                        :class="[
                          'bbms-panel-' + cell.sec.layout,
                          m.bbmsCardToneClass(cell.sec),
                          {
                            'bbms-card-heavy': m.bbmsSectionRowCount(cell.sec) >= 14,
                            'bbms-card-scroll': m.bbmsNeedsScroll(cell.sec, true),
                          },
                        ]"
                        :style="m.bbmsCardGridStyle(cell)"
                      >
                        <header class="bbms-board-card-head">
                          <h3>{{ $t(cell.sec.titleKey) }}</h3>
                          <span class="bbms-board-count">{{ cell.sec.rows.length }}</span>
                        </header>
                        <div class="bbms-board-card-inner">
                          <p v-if="cell.sec.hintKey" class="bbms-board-hint">{{ $t(cell.sec.hintKey) }}</p>
                          <div class="bbms-board-card-body">
                            <div
                              v-if="cell.sec.layout === 'rail'"
                              :class="m.bbmsRailClass(cell.sec)"
                              :style="m.bbmsRailGridStyle(cell.sec)"
                            >
                              <div
                                v-for="row in cell.sec.rows"
                                :key="'rail-' + (row.code != null ? row.code : row.ename) + '-' + cell.sec.id"
                                class="bbms-rail-cell"
                              >
                                <div class="bbms-rail-label">{{ m.pointDisplayName(row) }}</div>
                                <div
                                  class="bbms-rail-value"
                                  :class="{ 'bbms-rail-alarm': Number(row.groupType) === 6 }"
                                >
                                  {{ m.bbmsDisplayValue(row) }}
                                </div>
                              </div>
                            </div>

                            <div v-else-if="cell.sec.layout === 'series'" class="bbms-series-wrap">
                              <el-table
                                :data="cell.sec.rows"
                                border
                                stripe
                                size="mini"
                                class="bbms-series-table"
                                :max-height="440"
                                :row-class-name="m.remoteTableRowClass"
                              >
                                <el-table-column prop="name" :label="$t('monitor.测点')" min-width="160" show-overflow-tooltip>
                                  <template slot-scope="scope">
                                    <span>{{ m.pointDisplayName(scope.row) }}</span>
                                  </template>
                                </el-table-column>
                                <el-table-column prop="value" :label="$t('monitor.值')" min-width="100">
                                  <template slot-scope="scope">
                                    <span :class="{ 'bbms-cell-alarm': Number(scope.row.groupType) === 6 }">{{
                                      scope.row.value
                                    }}</span>
                                  </template>
                                </el-table-column>
                                <el-table-column prop="unit" :label="$t('monitor.单位')" width="72" />
                              </el-table>
                            </div>

                            <div v-else class="bbms-kv">
                              <div
                                v-for="row in cell.sec.rows"
                                :key="'kv-' + (row.code != null ? row.code : row.ename) + '-' + cell.sec.id"
                                class="bbms-kv-row"
                              >
                                <span class="bbms-kv-k">{{ m.pointDisplayName(row) }}</span>
                                <span
                                  class="bbms-kv-v"
                                  :class="{ 'bbms-cell-alarm': Number(row.groupType) === 6 }"
                                >{{ m.bbmsDisplayValue(row) }}</span>
                              </div>
                            </div>
                          </div>
                        </div>
                      </article>
                      <div
                        v-if="!m.bbmsLayoutCellsForTabId(tab.id).length"
                        class="bbms-console-empty"
                      >
                        {{ $t("monitor.暂无数据") }}
                      </div>
                    </div>
                  </el-tab-pane>
                </el-tabs>
              </div>
</template>

<script>
import BmsFaultPanel from "./BmsFaultPanel.vue";
import BbmsThermalPanel from "./BbmsThermalPanel.vue";

export default {
  name: "BbmsStackDashboard",
  components: { BmsFaultPanel, BbmsThermalPanel },
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
@import "./styles/bbms.scss";
</style>
