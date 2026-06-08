<template>
  <div class="topology-flex-container">
    <div v-if="architectureRows.length" class="topology-toolbar">
      <label class="topology-toolbar__label">{{ $t("overview.topologySelectArch") }}</label>
      <el-select
        v-model="selectedArchId"
        size="small"
        filterable
        class="topology-toolbar__select"
        :placeholder="$t('overview.topologySelectArch')"
      >
        <el-option
          v-for="opt in architectureOptions"
          :key="'arch-' + opt.value"
          :label="opt.label"
          :value="opt.value"
        />
      </el-select>
      <span v-if="activeSummaryText" class="topology-toolbar__meta">{{ activeSummaryText }}</span>
    </div>

    <div ref="topologyCanvas" class="topology-canvas">
      <div v-if="!architectureRows.length" class="topology-empty">
        <p class="empty-title">{{ $t("overview.拓扑图空标题") }}</p>
        <p class="empty-hint">{{ $t("overview.拓扑图空说明") }}</p>
      </div>
      <section
        v-else-if="activeRow"
        class="topo-arch"
        :class="{ 'topo-arch--virtual': activeRow.isVirtual }"
        :style="topologyLayoutVars"
      >
        <div class="topo-arch__tree">
          <div class="topo-arch__root">
            <div class="topo-root-card" :class="{ 'topo-root-card--virtual': activeRow.isVirtual }">
              <div class="topo-root-card__head">
                <i class="topo-root-card__icon" :class="deviceIconClass(activeRow.bbms.deviceType)" />
                <div class="topo-root-card__text">
                  <div class="topo-root-card__title">{{ nodeDisplayLabel(activeRow.bbms) }}</div>
                  <div class="topo-root-card__sub">{{ rowTypeLabel(activeRow.bbms, activeRow.isVirtual) }}</div>
                </div>
                <span
                  class="topo-root-card__dot"
                  :class="onlineClass(activeRow.bbms)"
                  :title="onlineTitle(activeRow.bbms)"
                />
              </div>
            </div>
          </div>

          <div class="topo-arch__fork">
            <div class="topo-arch__fork-stem" aria-hidden="true" />
            <div v-if="activeRow.children.length" class="topo-arch__level1-row">
              <div
                v-for="ch in activeRow.children"
                :key="'l1-' + ch.id"
                class="topo-arch__level1-col"
              >
                <div class="topo-arch__fork-drop" aria-hidden="true" />
                <TopologyTreeNode :node="ch" :online-map="onlineMap" />
              </div>
            </div>
            <div v-else class="topo-arch__empty">
              {{ $t("overview.topologyNoChildren") }}
            </div>
          </div>
        </div>
      </section>
    </div>
  </div>
</template>

<script>
import { deviceTypes, findDeviceTypeByValue } from "@/constants/deviceTypes";
import { getDevicePointsWithStatus } from "@/api/system/device";
import TopologyTreeNode from "./TopologyTreeNode.vue";
import {
  buildArchitectureRows,
  collectRealDeviceNodes,
  architectureSummary,
} from "@/utils/topologyDisplayTree";

export default {
  name: "BmsTopologyDiagram",
  components: { TopologyTreeNode },
  props: {
    topologyTree: {
      type: Array,
      default: () => [],
    },
  },
  data() {
    return {
      selectedArchId: "",
      onlineMap: {},
    };
  },
  computed: {
    roots() {
      return Array.isArray(this.topologyTree) ? this.topologyTree : [];
    },
    architectureRows() {
      return buildArchitectureRows(this.roots);
    },
    architectureOptions() {
      return this.architectureRows.map((row) => {
        const bbms = row.bbms || {};
        const label = this.nodeDisplayLabel(bbms);
        const suffix = row.isVirtual ? ` · ${this.$t("overview.topologyVirtualBbms")}` : "";
        return {
          value: String(bbms.id),
          label: `${label}${suffix}`,
          row,
        };
      });
    },
    activeRow() {
      const id = String(this.selectedArchId || "");
      if (!id) return this.architectureRows[0] || null;
      return (
        this.architectureRows.find((r) => String((r.bbms && r.bbms.id) || "") === id) ||
        this.architectureRows[0] ||
        null
      );
    },
    activeSummaryText() {
      const row = this.activeRow;
      if (!row) return "";
      const s = architectureSummary(row);
      const parts = [];
      if (s.rbms) parts.push(this.$t("overview.topologyRbmsCount", { n: s.rbms }));
      if (s.aux) parts.push(this.$t("overview.topologyAuxCount", { n: s.aux }));
      if (s.rbms) parts.push(this.$t("overview.topologyPbmsHint", { n: s.rbms * 8 }));
      return parts.join(" · ");
    },
    level1Count() {
      return ((this.activeRow && this.activeRow.children) || []).length;
    },
    topologyLayoutVars() {
      const n = this.level1Count || 1;
      let gap = 16;
      let branchW = 156;
      if (n <= 3) {
        gap = 18;
        branchW = 162;
      } else if (n <= 6) {
        gap = 14;
        branchW = 148;
      } else if (n <= 10) {
        gap = 10;
        branchW = 136;
      } else {
        gap = 8;
        branchW = 124;
      }
      return {
        "--topo-gap": `${gap}px`,
        "--topo-branch-w": `${branchW}px`,
      };
    },
    isEnLocale() {
      return String((this.$i18n && this.$i18n.locale) || "")
        .toLowerCase()
        .startsWith("en");
    },
  },
  watch: {
    architectureRows: {
      immediate: true,
      handler(rows) {
        if (!rows.length) {
          this.selectedArchId = "";
          return;
        }
        const ids = rows.map((r) => String((r.bbms && r.bbms.id) || ""));
        if (!ids.includes(String(this.selectedArchId))) {
          this.selectedArchId = ids[0];
        }
        this.refreshOnlineStates();
      },
    },
    selectedArchId() {
      this.refreshOnlineStates();
    },
  },
  methods: {
    rowTypeLabel(node, isVirtual) {
      if (isVirtual) return this.$t("overview.topologyVirtualBbms");
      return this.formatDeviceType(node.deviceType);
    },
    formatDeviceType(t) {
      const row = findDeviceTypeByValue(deviceTypes, t);
      if (!row) return "";
      return this.isEnLocale ? row.en_label : row.label;
    },
    topologyLabelEn(text) {
      const raw = String(text || "").trim();
      if (!raw) return raw;
      const exact = {
        虚拟BBMS: "Virtual BBMS",
        虚拟bbms: "Virtual BBMS",
        中控柜空调: "Central cabinet AC",
        交流电表: "AC Meter",
        BBMS: "BBMS",
        RBMS: "RBMS",
        PBMS: "PBMS",
      };
      if (exact[raw]) return exact[raw];
      let out = raw;
      if (/虚拟\s*bbms/i.test(out)) out = out.replace(/虚拟\s*bbms/gi, "Virtual BBMS");
      return out;
    },
    nodeDisplayLabel(node) {
      const raw = String((node && node.label) || "").trim();
      if (!raw) return "--";
      if (!this.isEnLocale) return raw;
      const codeName = raw.match(/^(\d+)\s*#\s*(.+)$/);
      if (codeName) {
        return `${codeName[1]} # ${this.topologyLabelEn(codeName[2].trim())}`;
      }
      if (node.type === "virtual") return this.topologyLabelEn(raw);
      if (/[\u4e00-\u9fff]/.test(raw)) return this.topologyLabelEn(raw);
      return raw;
    },
    deviceIconClass(t) {
      const n = Number(t);
      if (n === 1) return "el-icon-s-platform";
      if (n === 2) return "el-icon-cpu";
      if (n === 6) return "el-icon-data-analysis";
      if (n === 7) return "el-icon-monitor";
      if (n === 0) return "el-icon-house";
      return "el-icon-coin";
    },
    onlineKey(node) {
      const n = node || {};
      const raw =
        n.devId !== undefined && n.devId !== null && n.devId !== ""
          ? n.devId
          : n.deviceId !== undefined && n.deviceId !== null && n.deviceId !== ""
          ? n.deviceId
          : n.id;
      return raw === undefined || raw === null || raw === "" ? "" : String(raw);
    },
    parseOnlineValue(v) {
      const s = String(v == null ? "" : v).trim().toLowerCase();
      if (!s) return "unknown";
      if (["1", "true", "on", "online", "ok", "正常", "在线"].includes(s)) return "online";
      if (["0", "false", "off", "offline", "离线"].includes(s)) return "offline";
      const num = Number(s);
      if (!Number.isNaN(num)) return num > 0 ? "online" : "offline";
      return "unknown";
    },
    onlineTitle(node) {
      const key = this.onlineKey(node);
      const s = this.onlineMap[key] || "unknown";
      if (s === "online") return this.$t("overview.topologyOnline");
      if (s === "offline") return this.$t("overview.topologyOffline");
      return this.$t("overview.topologyUnknown");
    },
    onlineClass(node) {
      const key = this.onlineKey(node);
      const s = this.onlineMap[key] || "unknown";
      if (s === "online") return "is-online";
      if (s === "offline") return "is-offline";
      return "is-unknown";
    },
    async refreshOnlineStates() {
      const row = this.activeRow;
      const deviceNodes = row ? collectRealDeviceNodes(row) : [];
      if (!deviceNodes.length) {
        this.onlineMap = {};
        return;
      }
      const next = {};
      await Promise.all(
        deviceNodes.map(async (n) => {
          const key = this.onlineKey(n);
          if (!key) return;
          try {
            const r = await getDevicePointsWithStatus(key);
            const rows = Array.isArray(r?.rows) ? r.rows : Array.isArray(r?.data) ? r.data : [];
            const onlinePoint = rows.find((p) =>
              /在线|online|是否在线|is[_\s-]?online/i.test(`${p.name || ""} ${p.ename || ""}`)
            );
            if (onlinePoint) next[key] = this.parseOnlineValue(onlinePoint.value);
            else {
              const anyVal = rows.some(
                (p) => p.value !== null && p.value !== undefined && String(p.value).trim() !== ""
              );
              next[key] = anyVal ? "online" : "unknown";
            }
          } catch (e) {
            next[key] = "unknown";
          }
        })
      );
      this.onlineMap = next;
    },
  },
  created() {
    this.refreshOnlineStates();
  },
};
</script>

<style scoped>
.topology-flex-container {
  width: 100%;
  height: 100%;
  min-height: 0;
  display: flex;
  flex-direction: column;
  box-sizing: border-box;
}

.topology-toolbar {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 8px 12px;
  margin-bottom: 8px;
  flex-shrink: 0;
}

.topology-toolbar__label {
  font-size: 12px;
  font-weight: 600;
  color: #475569;
  white-space: nowrap;
}

.topology-toolbar__select {
  width: min(280px, 100%);
  flex: 1 1 180px;
}

.topology-toolbar__meta {
  font-size: 11px;
  color: #64748b;
  flex: 1 1 auto;
  min-width: 0;
}

/* 外框层：占满卡片剩余区域，留出边距形成层次 */
.topology-canvas {
  flex: 1;
  min-height: 0;
  width: 100%;
  display: flex;
  flex-direction: column;
  padding: 10px 12px 12px;
  border-radius: 10px;
  background: #f1f5f9;
  box-sizing: border-box;
  overflow: hidden;
}

.topology-empty {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  text-align: center;
  color: #64748b;
  padding: 24px 16px;
  flex: 1;
  min-height: 0;
  width: 100%;
  border-radius: 10px;
  border: 1px solid rgba(59, 130, 246, 0.18);
  background: linear-gradient(180deg, #ffffff 0%, #f8fafc 98%);
  box-shadow: 0 2px 8px rgba(15, 23, 42, 0.04);
  box-sizing: border-box;
}

.empty-title {
  font-size: 15px;
  font-weight: 600;
  color: #334155;
  margin: 0 0 8px;
}

.empty-hint {
  font-size: 13px;
  margin: 0;
  max-width: 420px;
  line-height: 1.5;
}

/* 内框：固定铺满画布区，尺寸不随节点多少变化，略小于外层卡片 */
.topo-arch {
  --topo-gap: 16px;
  --topo-branch-w: 156px;
  --topo-fork-stem-h: 22px;
  --topo-fork-drop-h: 16px;
  --topo-root-pad-y: 14px;
  --topo-root-pad-x: 22px;
  --topo-root-icon: 36px;
  --topo-root-title: 16px;
  --topo-root-sub: 12px;
  --topo-node-pad-y: 12px;
  --topo-node-pad-x: 12px;
  --topo-node-icon: 28px;
  --topo-node-title: 13px;
  --topo-node-sub: 11px;
  --topo-rbms-sub-pad-y: 14px;
  --topo-rbms-sub-pad-x: 10px;
  --topo-v-line-h: 16px;
  --topo-rep-pad-y: 12px;
  --topo-rep-pad-x: 8px;
  --topo-rep-icon: 26px;
  --topo-rep-title: 12px;
  --topo-rep-count: 11px;
  --topo-rep-gap: 8px;
  flex: 1;
  min-height: 0;
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  padding: 20px 24px 22px;
  border-radius: 10px;
  border: 1px solid rgba(59, 130, 246, 0.28);
  background: linear-gradient(180deg, #ffffff 0%, #f8fafc 100%);
  box-shadow:
    0 2px 10px rgba(59, 130, 246, 0.08),
    inset 0 1px 0 rgba(255, 255, 255, 0.95);
  box-sizing: border-box;
  overflow: auto;
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
}

.topo-arch--virtual {
  background: linear-gradient(180deg, #fffdf7 0%, #fffbeb 100%);
  border-color: rgba(245, 158, 11, 0.35);
}

/* 整棵树：BBMS 与下层连线保持一体，中间不留空隙 */
.topo-arch__tree {
  display: flex;
  flex-direction: column;
  align-items: center;
  flex: 0 0 auto;
  max-width: 100%;
}

.topo-arch__root {
  display: flex;
  justify-content: center;
  flex-shrink: 0;
}

.topo-root-card {
  padding: var(--topo-root-pad-y) var(--topo-root-pad-x);
  border-radius: 10px;
  border: 1px solid rgba(59, 130, 246, 0.28);
  background: linear-gradient(180deg, #eff6ff 0%, #ffffff 100%);
  box-shadow: 0 4px 12px rgba(59, 130, 246, 0.1);
  min-width: 180px;
  max-width: 300px;
}

.topo-root-card--virtual {
  border-style: dashed;
  border-color: rgba(245, 158, 11, 0.45);
  background: linear-gradient(180deg, #fffbeb 0%, #ffffff 100%);
}

.topo-root-card__head {
  display: flex;
  align-items: center;
  gap: 10px;
}

.topo-root-card__icon {
  width: var(--topo-root-icon);
  height: var(--topo-root-icon);
  border-radius: 8px;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  background: #dbeafe;
  color: #1d4ed8;
  font-size: calc(var(--topo-root-icon) * 0.46);
}

.topo-root-card--virtual .topo-root-card__icon {
  background: #fef3c7;
  color: #b45309;
}

.topo-root-card__title {
  font-weight: 700;
  font-size: var(--topo-root-title);
  color: #0f172a;
  line-height: 1.25;
}

.topo-root-card__sub {
  font-size: var(--topo-root-sub);
  color: #64748b;
  margin-top: 4px;
}

.topo-root-card__dot {
  width: 9px;
  height: 9px;
  border-radius: 50%;
  margin-left: auto;
  flex-shrink: 0;
  border: 1px solid rgba(148, 163, 184, 0.35);
}

.topo-root-card__dot.is-online {
  background: #22c55e;
}

.topo-root-card__dot.is-offline {
  background: #ef4444;
}

.topo-root-card__dot.is-unknown {
  background: #94a3b8;
}

.topo-arch__fork {
  display: flex;
  flex-direction: column;
  align-items: center;
  flex: 0 0 auto;
  width: max-content;
  max-width: calc(100% - 8px);
}

.topo-arch__fork-stem {
  width: 2px;
  height: var(--topo-fork-stem-h);
  margin-top: -1px;
  background: #3b82f6;
  border-radius: 0;
  flex-shrink: 0;
}

.topo-arch--virtual .topo-arch__fork-stem,
.topo-arch--virtual .topo-arch__fork-drop {
  background: #f59e0b;
}

.topo-arch__level1-row {
  display: flex;
  flex-direction: row;
  flex-wrap: nowrap;
  align-items: flex-start;
  justify-content: center;
  gap: var(--topo-gap);
  width: max-content;
  max-width: 100%;
  margin-top: -1px;
  padding: 0 2px;
  box-sizing: border-box;
  border-top: 2px solid #93c5fd;
}

.topo-arch--virtual .topo-arch__level1-row {
  border-top-color: #fcd34d;
}

.topo-arch__level1-col {
  display: flex;
  flex-direction: column;
  align-items: center;
  flex: 0 0 auto;
  width: var(--topo-branch-w);
  min-width: 0;
}

.topo-arch__fork-drop {
  width: 2px;
  height: var(--topo-fork-drop-h);
  background: #93c5fd;
  border-radius: 1px;
  flex-shrink: 0;
}

.topo-arch__empty {
  text-align: center;
  color: #94a3b8;
  font-size: 12px;
  padding: 8px 12px;
  width: 100%;
}
</style>
