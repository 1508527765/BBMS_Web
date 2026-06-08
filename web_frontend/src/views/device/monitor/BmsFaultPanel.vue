<template>
  <div class="bms-fault-panel">
    <div v-if="!allPoints.length" class="bms-fault-empty">
      <i class="el-icon-circle-check" />
      <p>{{ $t("monitor.faultPanelEmpty") }}</p>
    </div>
    <template v-else>
      <div class="bms-fault-summary">
        <div class="bms-fault-summary-card bms-fault-summary-card--active">
          <span class="bms-fault-summary-num">{{ stats.active }}</span>
          <span class="bms-fault-summary-label">{{ $t("monitor.faultPanelActive") }}</span>
        </div>
        <div class="bms-fault-summary-card bms-fault-summary-card--clear">
          <span class="bms-fault-summary-num">{{ stats.cleared }}</span>
          <span class="bms-fault-summary-label">{{ $t("monitor.faultPanelCleared") }}</span>
        </div>
        <div class="bms-fault-summary-card bms-fault-summary-card--unknown">
          <span class="bms-fault-summary-num">{{ stats.unknown }}</span>
          <span class="bms-fault-summary-label">{{ $t("monitor.faultPanelUnknown") }}</span>
        </div>
        <div class="bms-fault-summary-card bms-fault-summary-card--total">
          <span class="bms-fault-summary-num">{{ stats.total }}</span>
          <span class="bms-fault-summary-label">{{ $t("monitor.faultPanelTotal") }}</span>
        </div>
      </div>

      <div class="bms-fault-toolbar">
        <button
          v-for="opt in filterOptions"
          :key="opt.value"
          type="button"
          class="bms-fault-filter"
          :class="{ 'is-active': faultFilter === opt.value }"
          @click="faultFilter = opt.value"
        >
          {{ opt.label }}
          <b>{{ opt.count }}</b>
        </button>
        <el-input
          v-model="localKeyword"
          size="small"
          clearable
          class="bms-fault-search"
          :placeholder="$t('monitor.pointSearchPlaceholder')"
          prefix-icon="el-icon-search"
        />
      </div>

      <div class="bms-fault-body">
        <section
          v-for="grp in filteredGroups"
          :key="'fg-' + (grp.id || 'all')"
          class="bms-fault-group"
        >
          <header
            v-if="grp.titleKey || grp.rangeStart != null"
            class="bms-fault-group-head"
            :class="{ 'is-clickable': grp.collapseDefault }"
            @click="grp.collapseDefault && toggleGroupCollapse(grp.id)"
          >
            <h4>{{ groupTitle(grp) }}</h4>
            <span class="bms-fault-group-count">{{ grp.points.length }}</span>
            <button
              v-if="grp.collapseDefault"
              type="button"
              class="bms-fault-group-toggle"
              @click.stop="toggleGroupCollapse(grp.id)"
            >
              {{
                isGroupCollapsed(grp.id)
                  ? $t("monitor.faultPanelExpand")
                  : $t("monitor.faultPanelCollapse")
              }}
            </button>
          </header>
          <div v-show="!isGroupCollapsed(grp.id)" class="bms-fault-grid">
            <article
              v-for="(row, idx) in grp.points"
              :key="faultRowKey(row, idx)"
              class="bms-fault-card"
              :class="[
                'bms-fault-card--' + faultState(row),
                { 'bms-fault-card--metric': isFaultMetric(row) },
              ]"
            >
              <div class="bms-fault-card__status">
                <span class="bms-fault-card__dot" />
                <span class="bms-fault-card__badge">{{ faultStateLabel(row) }}</span>
              </div>
              <h5 class="bms-fault-card__name" :title="pointName(row)">{{ pointName(row) }}</h5>
              <div class="bms-fault-card__value-row">
                <span class="bms-fault-card__value">{{ displayValue(row) }}</span>
                <span v-if="row.unit" class="bms-fault-card__unit">{{ row.unit }}</span>
              </div>
            </article>
          </div>
        </section>
      </div>
    </template>
  </div>
</template>

<script>
export default {
  name: "BmsFaultPanel",
  inject: { monitor: { default: null } },
  props: {
    /** 扁平故障点列表（RBMS 或简单模式） */
    points: {
      type: Array,
      default: () => [],
    },
    /** 分组展示：[{ id, titleKey, rows }]（BBMS 多分区） */
    sections: {
      type: Array,
      default: () => [],
    },
  },
  data() {
    return {
      faultFilter: "all",
      localKeyword: "",
      collapsedGroups: {},
    };
  },
  computed: {
    m() {
      return this.monitor;
    },
    groups() {
      const secs = Array.isArray(this.sections) ? this.sections : [];
      if (secs.length) {
        return secs
          .filter((s) => s && Array.isArray(s.rows) && s.rows.length)
          .map((s) => ({
            id: s.id,
            titleKey: s.titleKey || "",
            rangeStart: s.rangeStart,
            rangeEnd: s.rangeEnd,
            collapseDefault: !!s.collapseDefault,
            points: s.rows,
          }));
      }
      const pts = Array.isArray(this.points) ? this.points : [];
      return pts.length ? [{ id: "all", titleKey: "", points: pts }] : [];
    },
    allPoints() {
      const out = [];
      this.groups.forEach((g) => out.push(...(g.points || [])));
      return out;
    },
    enrichedPoints() {
      return this.allPoints.map((row) => ({
        row,
        state: this.faultState(row),
        name: this.pointName(row),
      }));
    },
    stats() {
      const s = { active: 0, cleared: 0, unknown: 0, metric: 0, total: 0 };
      this.enrichedPoints.forEach(({ state }) => {
        s.total += 1;
        if (state === "active") s.active += 1;
        else if (state === "cleared") s.cleared += 1;
        else if (state === "metric") s.metric += 1;
        else s.unknown += 1;
      });
      return s;
    },
    filterOptions() {
      const s = this.stats;
      return [
        { value: "all", label: this.$t("monitor.faultFilterAll"), count: s.total },
        { value: "active", label: this.$t("monitor.faultFilterActive"), count: s.active },
        { value: "cleared", label: this.$t("monitor.faultFilterCleared"), count: s.cleared },
        { value: "unknown", label: this.$t("monitor.faultFilterUnknown"), count: s.unknown },
        { value: "metric", label: this.$t("monitor.faultFilterMetric"), count: s.metric },
      ];
    },
    filteredGroups() {
      const kw = String(this.localKeyword || "")
        .trim()
        .toLowerCase();
      return this.groups
        .map((g) => {
          let pts = g.points || [];
          if (this.faultFilter !== "all") {
            pts = pts.filter((row) => this.faultState(row) === this.faultFilter);
          }
          if (kw) {
            pts = pts.filter((row) => this.pointName(row).toLowerCase().includes(kw));
          }
          return { ...g, points: pts };
        })
        .filter((g) => g.points.length);
    },
  },
  watch: {
    groups: {
      immediate: true,
      handler(groups) {
        (groups || []).forEach((g) => {
          if (!g.collapseDefault || this.collapsedGroups[g.id] !== undefined) return;
          const hasActive = (g.points || []).some((row) => this.faultState(row) === "active");
          this.$set(this.collapsedGroups, g.id, !hasActive);
        });
      },
    },
    faultFilter() {
      if (this.faultFilter === "all") return;
      this.$nextTick(() => {
        (this.filteredGroups || []).forEach((g) => {
          if (g.points && g.points.length) this.$set(this.collapsedGroups, g.id, false);
        });
      });
    },
  },
  methods: {
    groupTitle(grp) {
      if (grp.rangeStart != null && grp.rangeEnd != null && grp.titleKey) {
        return this.$t(grp.titleKey, { start: grp.rangeStart, end: grp.rangeEnd });
      }
      return grp.titleKey ? this.$t(grp.titleKey) : "";
    },
    isGroupCollapsed(id) {
      return !!this.collapsedGroups[id];
    },
    toggleGroupCollapse(id) {
      this.$set(this.collapsedGroups, id, !this.collapsedGroups[id]);
    },
    faultRowKey(row, idx) {
      const code = row && row.code != null ? row.code : "";
      const name = row && (row.name || row.ename) ? row.name || row.ename : "";
      return `f-${code}-${name}-${idx}`;
    },
    pointName(row) {
      if (this.m && typeof this.m.pointDisplayName === "function") {
        return this.m.pointDisplayName(row);
      }
      return String((row && row.name) || (row && row.ename) || "--");
    },
    displayValue(row) {
      if (this.m && typeof this.m.bbmsDisplayValue === "function") {
        return this.m.bbmsDisplayValue(row);
      }
      const v = row && row.value;
      if (v === undefined || v === null || String(v).trim() === "") return "--";
      return String(v);
    },
    isFaultMetric(row) {
      const t = `${this.pointName(row)} ${row && row.ename ? row.ename : ""}`.toLowerCase();
      return /次数|count|累计|统计|number of|times/i.test(t);
    },
    faultState(row) {
      if (this.isFaultMetric(row)) return "metric";
      const v = row && row.value;
      if (v === undefined || v === null || String(v).trim() === "") return "unknown";
      const s = String(v).trim().toLowerCase();
      const n = Number(s);
      if (!Number.isNaN(n)) {
        if (n > 0) return "active";
        return "cleared";
      }
      if (/^(1|true|on|yes|是|有|动作|发生|告警|故障|异常|alarm|fault|trip|ng)$/i.test(s)) {
        return "active";
      }
      if (
        /^(0|false|off|no|否|无|正常|恢复|复归|ok|clear|normal|inactive|reset|recovered|idle)$/i.test(
          s
        )
      ) {
        return "cleared";
      }
      if (
        /正常|复归|运行|闭合|投入|在线|normal|ok|clear|reset|recovered|no\s*fault|not\s*active/i.test(
          s
        ) &&
        !/异常|故障|告警|fault|alarm|trip/i.test(s)
      ) {
        return "cleared";
      }
      if (/告警|故障|异常|alarm|fault|trip|offline|断开|分闸|切除/.test(s)) {
        return "active";
      }
      return "unknown";
    },
    faultStateLabel(row) {
      const st = this.faultState(row);
      if (st === "active") return this.$t("monitor.faultPanelActive");
      if (st === "cleared") return this.$t("monitor.faultPanelCleared");
      if (st === "metric") return this.$t("monitor.faultPanelMetric");
      return this.$t("monitor.faultPanelUnknown");
    },
  },
};
</script>

<style lang="scss" scoped>
.bms-fault-panel {
  min-height: 320px;
}

.bms-fault-empty {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 48px 20px;
  color: #64748b;
  text-align: center;

  i {
    font-size: 40px;
    color: #22c55e;
    margin-bottom: 12px;
  }

  p {
    margin: 0;
    font-size: 14px;
  }
}

.bms-fault-summary {
  display: grid;
  grid-template-columns: repeat(4, minmax(0, 1fr));
  gap: 10px;
  margin-bottom: 14px;

  @media (max-width: 720px) {
    grid-template-columns: repeat(2, 1fr);
  }
}

.bms-fault-summary-card {
  padding: 12px 10px;
  border-radius: 10px;
  text-align: center;
  border: 1px solid #e2e8f0;
  background: #fff;
}

.bms-fault-summary-num {
  display: block;
  font-size: 22px;
  font-weight: 800;
  line-height: 1.1;
}

.bms-fault-summary-label {
  display: block;
  margin-top: 4px;
  font-size: 11px;
  color: #64748b;
}

.bms-fault-summary-card--active {
  border-color: #fecaca;
  background: #fef2f2;

  .bms-fault-summary-num {
    color: #dc2626;
  }
}

.bms-fault-summary-card--clear {
  border-color: #bbf7d0;
  background: #f0fdf4;

  .bms-fault-summary-num {
    color: #16a34a;
  }
}

.bms-fault-summary-card--unknown {
  .bms-fault-summary-num {
    color: #64748b;
  }
}

.bms-fault-summary-card--total {
  border-color: #bfdbfe;
  background: #eff6ff;

  .bms-fault-summary-num {
    color: #2563eb;
  }
}

.bms-fault-toolbar {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 8px 10px;
  margin-bottom: 14px;
}

.bms-fault-filter {
  display: inline-flex;
  align-items: center;
  gap: 6px;
  padding: 6px 12px;
  border: 1px solid #e2e8f0;
  border-radius: 8px;
  background: #fff;
  font-size: 12px;
  color: #475569;
  cursor: pointer;
  font-family: inherit;

  b {
    font-size: 11px;
    padding: 1px 6px;
    border-radius: 6px;
    background: #f1f5f9;
    color: #64748b;
  }

  &.is-active {
    border-color: #ef4444;
    background: #ef4444;
    color: #fff;

    b {
      background: rgba(255, 255, 255, 0.25);
      color: #fff;
    }
  }
}

.bms-fault-search {
  width: 200px;
  max-width: 100%;
  margin-left: auto;
}

.bms-fault-body {
  max-height: 520px;
  overflow-y: auto;
  padding-right: 4px;
}

.bms-fault-group {
  margin-bottom: 18px;
}

.bms-fault-group-head {
  display: flex;
  align-items: center;
  gap: 10px;
  margin: 0 0 10px;
  padding-bottom: 8px;
  border-bottom: 1px dashed #e2e8f0;

  h4 {
    margin: 0;
    font-size: 14px;
    font-weight: 700;
    color: #1e293b;
  }
}

.bms-fault-group-count {
  font-size: 11px;
  font-weight: 700;
  padding: 2px 8px;
  border-radius: 8px;
  background: #f1f5f9;
  color: #64748b;
}

.bms-fault-group-head.is-clickable {
  cursor: pointer;
  user-select: none;

  &:hover h4 {
    color: #dc2626;
  }
}

.bms-fault-group-toggle {
  margin-left: auto;
  padding: 4px 10px;
  border: 1px solid #e2e8f0;
  border-radius: 6px;
  background: #fff;
  font-size: 11px;
  color: #475569;
  cursor: pointer;
  font-family: inherit;

  &:hover {
    border-color: #cbd5e1;
    background: #f8fafc;
  }
}

.bms-fault-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(200px, 1fr));
  gap: 10px;
}

.bms-fault-card {
  padding: 12px 12px 11px;
  border-radius: 10px;
  border: 1px solid #e2e8f0;
  background: #fff;
  box-shadow: 0 1px 3px rgba(15, 23, 42, 0.05);
  border-left-width: 4px;
  border-left-color: #94a3b8;
}

.bms-fault-card--active {
  border-left-color: #ef4444;
  background: linear-gradient(135deg, #fff 0%, #fef2f2 100%);
  box-shadow: 0 2px 10px rgba(239, 68, 68, 0.12);
}

.bms-fault-card--cleared {
  border-left-color: #22c55e;
  background: linear-gradient(135deg, #fff 0%, #f0fdf4 100%);
}

.bms-fault-card--unknown {
  border-left-color: #94a3b8;
}

.bms-fault-card--metric {
  border-left-color: #f59e0b;
  background: linear-gradient(135deg, #fff 0%, #fffbeb 100%);
}

.bms-fault-card__status {
  display: flex;
  align-items: center;
  gap: 6px;
  margin-bottom: 8px;
}

.bms-fault-card__dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  background: #94a3b8;
}

.bms-fault-card--active .bms-fault-card__dot {
  background: #ef4444;
  box-shadow: 0 0 0 3px rgba(239, 68, 68, 0.25);
}

.bms-fault-card--cleared .bms-fault-card__dot {
  background: #22c55e;
}

.bms-fault-card--metric .bms-fault-card__dot {
  background: #f59e0b;
}

.bms-fault-card__badge {
  font-size: 10px;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.04em;
  color: #64748b;
}

.bms-fault-card--active .bms-fault-card__badge {
  color: #dc2626;
}

.bms-fault-card__name {
  margin: 0 0 8px;
  font-size: 12px;
  font-weight: 600;
  color: #334155;
  line-height: 1.35;
  display: -webkit-box;
  -webkit-line-clamp: 3;
  -webkit-box-orient: vertical;
  overflow: hidden;
}

.bms-fault-card__value-row {
  display: flex;
  align-items: baseline;
  gap: 6px;
  flex-wrap: wrap;
}

.bms-fault-card__value {
  font-size: 18px;
  font-weight: 800;
  font-variant-numeric: tabular-nums;
  color: #0f172a;
}

.bms-fault-card--active .bms-fault-card__value {
  color: #dc2626;
}

.bms-fault-card__unit {
  font-size: 11px;
  color: #64748b;
}
</style>
