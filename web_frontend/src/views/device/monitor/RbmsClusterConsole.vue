<template>
          <div class="rbms-cluster-console">
            <header class="rbms-cluster-masthead">
              <div class="rbms-cluster-tier">{{ $t("monitor.rbmsTierLabel") }}</div>
              <h3 class="rbms-cluster-title">{{ m.currentNodeLabel }}</h3>
              <p class="rbms-cluster-sub">{{ $t("monitor.RBMS_subtitle") }}</p>
              <div class="rbms-cluster-idline">
                <span>{{ m.monitorSelectedDeviceTypeLabel }}</span>
                <span class="rbms-cluster-dot">·</span>
                <span>{{ m.monitorSelectedProtocolLabel || "--" }}</span>
                <span class="rbms-cluster-dot">·</span>
                <span>ID {{ m.monitorFlatDeviceIdText }}</span>
                <span class="rbms-cluster-dot">·</span>
                <span>{{ $t("monitor.dashboardTileTemplate") }} {{ m.monitorFlatTemplateIdText }}</span>
              </div>
              <div v-if="m.rbmsParentStackLabel" class="rbms-cluster-parent">
                {{ $t("monitor.rbmsUnderStack", { name: m.rbmsParentStackLabel }) }}
              </div>
              <div class="rbms-cluster-count">
                {{ $t("monitor.rbmsConsolePoints", { n: m.rbmsFilteredPoints.length }) }}
              </div>
            </header>

            <el-alert
              v-if="m.monitorTemplatePending"
              type="info"
              show-icon
              :closable="false"
              :title="$t('monitor.templatePendingTitle')"
              :description="$t('monitor.templatePendingDesc')"
              style="margin-bottom: 12px;"
            />

            <template v-else>
              <div v-if="m.rbmsHeroStrip.length" class="rbms-cluster-kpi-strip">
                <div
                  v-for="h in m.rbmsHeroStrip"
                  :key="'rbms-h-' + h.key"
                  class="rbms-cluster-kpi"
                  :class="'rbms-cluster-kpi--' + h.accent"
                >
                  <span class="rbms-cluster-kpi-label">{{ h.label }}</span>
                  <span class="rbms-cluster-kpi-value">{{ h.valueText }}</span>
                </div>
              </div>
              <div v-else class="rbms-cluster-kpi-empty">{{ $t("monitor.rbmsHeroEmpty") }}</div>

              <div class="rbms-cluster-toolbar">
                <el-input
                  :value="m.pointKeyword" @input="v => (m.pointKeyword = v)"
                  clearable
                  class="rbms-cluster-search"
                  :placeholder="$t('monitor.pointSearchPlaceholder')"
                  size="small"
                  prefix-icon="el-icon-search"
                />
                <el-switch
                  :value="m.onlyShowValued" @input="v => (m.onlyShowValued = v)"
                  :active-text="$t('monitor.onlyValuedOn')"
                  :inactive-text="$t('monitor.onlyValuedOff')"
                />
              </div>

              <el-tabs
                :value="m.rbmsMainTab"
                class="rbms-cluster-tabs"
                type="card"
                @input="onRbmsMainTabInput"
                @tab-click="onRbmsTabClick"
              >
                <el-tab-pane
                  v-for="tab in m.rbmsVisibleMainTabs"
                  :key="'rbms-tab-' + tab.id"
                  :label="m.rbmsMainTabLabel(tab)"
                  :name="tab.id"
                >
                  <div v-if="tab.id === 'overview'" class="rbms-tab-overview">
                    <p v-if="m.rbmsOverviewSections.length" class="rbms-tab-hint">
                      {{ $t("monitor.rbmsTabOverviewHint") }}
                    </p>
                    <div
                      v-for="sec in m.rbmsOverviewSections"
                      :key="'rbms-ov-' + sec.id"
                      class="rbms-tab-section"
                    >
                      <header class="rbms-tab-section-head">
                        <h4>{{ $t(sec.titleKey) }}</h4>
                        <span class="rbms-tab-section-count">{{ sec.rows.length }}</span>
                      </header>
                      <div class="rbms-overview-chip-grid">
                        <div
                          v-for="(row, idx) in sec.rows"
                          :key="'rbms-ov-chip-' + sec.id + '-' + idx"
                          class="rbms-overview-chip"
                          :class="'rbms-overview-chip--' + (m.pointChipTagType(row) || 'default')"
                        >
                          <span class="rbms-overview-chip-label">{{ m.pointDisplayName(row) }}</span>
                          <span class="rbms-overview-chip-value">{{ m.rbmsDisplayValue(row) }}</span>
                        </div>
                      </div>
                    </div>
                    <div v-if="!m.rbmsOverviewSections.length" class="rbms-tab-empty">{{ $t("monitor.暂无数据") }}</div>
                  </div>

                  <rbms-cell-info-panel v-else-if="tab.id === 'cell_info'" />

                  <bms-fault-panel
                    v-else-if="tab.id === 'fault'"
                    :points="m.rbmsFaultPointsFlat"
                  />

                  <div v-else-if="tab.id === 'flat'" class="rbms-flat-wrap">
                    <el-table
                      v-show="m.rbmsMainTab === 'flat'"
                      ref="rbmsFlatTable"
                      :key="'rbms-flat-' + (m.monitorFlatDeviceIdText || '') + '-' + m.rbmsFilteredPoints.length"
                      :data="m.rbmsFilteredPoints"
                      border
                      stripe
                      size="small"
                      class="rbms-flat-table"
                      max-height="520"
                      :row-class-name="m.remoteTableRowClass"
                    >
                      <el-table-column prop="code" :label="$t('monitor.monitorPointCode')" width="90" sortable />
                      <el-table-column prop="name" :label="$t('monitor.monitorPointName')" min-width="160">
                        <template slot-scope="scope">
                          <span class="remote-point-name">{{ m.pointDisplayName(scope.row) }}</span>
                        </template>
                      </el-table-column>
                      <el-table-column prop="value" :label="$t('monitor.值')" width="120">
                        <template slot-scope="scope">
                          <span :class="m.remoteValueClass(scope.row)">{{ m.rbmsDisplayValue(scope.row) }}</span>
                        </template>
                      </el-table-column>
                      <el-table-column prop="unit" :label="$t('monitor.单位')" width="72" />
                      <el-table-column prop="groupType" :label="$t('monitor.monitorGroupTypeCol')" width="88">
                        <template slot-scope="scope">
                          {{ m.monitorGroupTypeTitle(scope.row.groupType) }}
                        </template>
                      </el-table-column>
                    </el-table>
                  </div>

                  <div v-else class="rbms-tab-sections">
                    <div
                      v-for="cell in m.rbmsLayoutCellsForTabId(tab.id)"
                      :key="'rbms-cell-' + tab.id + '-' + (cell.sec && cell.sec.id || 'x')"
                      class="rbms-tab-section-card"
                      :class="m.rbmsCardToneClass(cell.sec)"
                      :style="m.bbmsCardGridStyle(cell)"
                    >
                      <header class="rbms-tab-section-head">
                        <h4>{{ $t(cell.sec.titleKey) }}</h4>
                        <span class="rbms-tab-section-count">{{ cell.sec.rows.length }}</span>
                      </header>
                      <p v-if="cell.sec.hintKey" class="rbms-tab-section-hint">{{ $t(cell.sec.hintKey) }}</p>
                      <div
                        v-if="cell.sec.layout === 'rail'"
                        class="bbms-rail rbms-section-rail"
                        :class="m.bbmsRailClass(cell.sec)"
                        :style="m.bbmsRailGridStyle(cell.sec)"
                      >
                        <div v-for="(row, ridx) in cell.sec.rows" :key="'rbms-r-' + ridx" class="bbms-rail-item">
                          <div class="bbms-rail-label">{{ m.pointDisplayName(row) }}</div>
                          <div class="bbms-rail-value">{{ m.rbmsDisplayValue(row) }}</div>
                        </div>
                      </div>
                      <el-table
                        v-else
                        :data="cell.sec.rows"
                        border
                        stripe
                        size="mini"
                        class="rbms-section-table"
                        :max-height="m.bbmsNeedsScroll(cell.sec, true) ? 360 : undefined"
                      >
                        <el-table-column :label="$t('monitor.测点')" min-width="140">
                          <template v-slot="scope">
                            {{ m.pointDisplayName(scope.row) }}
                          </template>
                        </el-table-column>
                        <el-table-column :label="$t('monitor.值')" width="110">
                          <template v-slot="scope">
                            {{ m.rbmsDisplayValue(scope.row) }}
                          </template>
                        </el-table-column>
                      </el-table>
                    </div>
                    <div v-if="!m.rbmsLayoutCellsForTabId(tab.id).length" class="rbms-tab-empty">{{ $t("monitor.暂无数据") }}</div>
                  </div>
                </el-tab-pane>
              </el-tabs>
            </template>
          </div>
</template>

<script>
import RbmsCellInfoPanel from "./RbmsCellInfoPanel.vue";
import BmsFaultPanel from "./BmsFaultPanel.vue";

export default {
  components: { RbmsCellInfoPanel, BmsFaultPanel },
  name: "RbmsClusterConsole",
  inject: {
    monitor: { default: null },
  },
  computed: {
    m() {
      return this.monitor;
    },
  },
  watch: {
    "m.rbmsMainTab"(tab) {
      if (tab === "flat") this.scheduleRbmsFlatTableLayout();
    },
    "m.rbmsFilteredPoints.length"() {
      if (this.m && this.m.rbmsMainTab === "flat") this.scheduleRbmsFlatTableLayout();
    },
    "m.devicePointsLoading"(loading) {
      if (!loading && this.m && this.m.rbmsMainTab === "flat") this.scheduleRbmsFlatTableLayout();
    },
  },
  mounted() {
    if (this.m && this.m.rbmsMainTab === "flat") this.scheduleRbmsFlatTableLayout();
  },
  methods: {
    onRbmsMainTabInput(v) {
      if (this.m) this.m.rbmsMainTab = v;
      if (v === "flat") this.scheduleRbmsFlatTableLayout();
    },
    onRbmsTabClick(tab) {
      const name = tab && (tab.name || tab.paneName);
      if (name === "flat") this.scheduleRbmsFlatTableLayout();
    },
    scheduleRbmsFlatTableLayout() {
      this.$nextTick(() => {
        this.layoutRbmsFlatTable();
        this.$nextTick(() => this.layoutRbmsFlatTable());
      });
      if (this._rbmsFlatLayoutTimer) clearTimeout(this._rbmsFlatLayoutTimer);
      this._rbmsFlatLayoutTimer = setTimeout(() => {
        this._rbmsFlatLayoutTimer = null;
        this.layoutRbmsFlatTable();
      }, 120);
    },
    layoutRbmsFlatTable() {
      const ref = this.$refs.rbmsFlatTable;
      const table = Array.isArray(ref) ? ref[0] : ref;
      if (table && typeof table.doLayout === "function") table.doLayout();
    },
  },
  beforeDestroy() {
    if (this._rbmsFlatLayoutTimer) {
      clearTimeout(this._rbmsFlatLayoutTimer);
      this._rbmsFlatLayoutTimer = null;
    }
  },
};

</script>

<style scoped lang="scss">
@import "./styles/panel-shared.scss";
@import "./styles/rbms.scss";
</style>
