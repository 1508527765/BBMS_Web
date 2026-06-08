<template>
  <div class="topo-branch" :class="{ 'topo-branch--aux': !isRbms }">
    <!-- RBMS 或辅助设备 -->
    <div class="topo-node-card" :class="nodeCardClass">
      <div class="topo-node-card__head">
        <i class="topo-node-card__icon" :class="iconClass" />
        <div class="topo-node-card__text">
          <div class="topo-node-card__title">{{ displayLabel }}</div>
          <div v-if="subLabel" class="topo-node-card__sub">{{ subLabel }}</div>
        </div>
        <span
          class="topo-node-card__dot"
          :class="onlineDotClass"
          :title="onlineTitle"
        />
      </div>
    </div>

    <!-- RBMS 下级：PBMS → 电芯 + 温度（紧凑竖排，外设无下级） -->
    <div v-if="isRbms" class="topo-rbms-sub">
      <div class="topo-v-line topo-v-line--l2" aria-hidden="true" />
      <div class="topo-rep-card topo-rep-card--pbms">
        <i class="topo-rep-card__icon el-icon-box" />
        <div class="topo-rep-card__text">
          <div class="topo-rep-card__title">PBMS</div>
          <div class="topo-rep-card__count">× {{ pbmsCount }}</div>
        </div>
      </div>
      <div class="topo-v-line topo-v-line--l3" aria-hidden="true" />
      <div class="topo-rep-row">
        <div class="topo-rep-card topo-rep-card--cell">
          <i class="topo-rep-card__icon el-icon-cpu" />
          <div class="topo-rep-card__text">
            <div class="topo-rep-card__title">{{ $t("overview.topologyCellRep") }}</div>
            <div class="topo-rep-card__count">× {{ cellCount }}</div>
          </div>
        </div>
        <div class="topo-rep-card topo-rep-card--temp">
          <i class="topo-rep-card__icon el-icon-sunny" />
          <div class="topo-rep-card__text">
            <div class="topo-rep-card__title">{{ $t("overview.topologyTempRep") }}</div>
            <div class="topo-rep-card__count">× {{ tempCount }}</div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { deviceTypes, findDeviceTypeByValue } from "@/constants/deviceTypes";
import {
  RBMS_DEVICE_TYPE,
  TOPO_PBMS_PER_RBMS,
  TOPO_CELLS_PER_PBMS,
  TOPO_TEMPS_PER_PBMS,
  isRbmsNode,
} from "@/utils/topologyDisplayTree";

export default {
  name: "TopologyTreeNode",
  props: {
    node: { type: Object, required: true },
    onlineMap: { type: Object, default: () => ({}) },
  },
  computed: {
    isRbms() {
      return isRbmsNode(this.node);
    },
    pbmsCount() {
      return TOPO_PBMS_PER_RBMS;
    },
    cellCount() {
      return TOPO_CELLS_PER_PBMS;
    },
    tempCount() {
      return TOPO_TEMPS_PER_PBMS;
    },
    isEnLocale() {
      return String((this.$i18n && this.$i18n.locale) || "")
        .toLowerCase()
        .startsWith("en");
    },
    displayLabel() {
      const raw = String((this.node && this.node.label) || "").trim();
      if (!raw) return "--";
      if (!this.isEnLocale) return raw;
      return raw;
    },
    subLabel() {
      if (this.isRbms) return this.formatDeviceType(RBMS_DEVICE_TYPE);
      if (this.node && this.node.type === "device") {
        return this.formatDeviceType(this.node.deviceType);
      }
      return "";
    },
    nodeCardClass() {
      return {
        "topo-node-card--rbms": this.isRbms,
        "topo-node-card--aux": !this.isRbms,
      };
    },
    iconClass() {
      const t = Number(this.node && this.node.deviceType);
      if (t === 2) return "el-icon-cpu";
      if (t === 5) return "el-icon-sunny";
      if (t === 6) return "el-icon-data-analysis";
      if (t === 7) return "el-icon-monitor";
      return "el-icon-coin";
    },
    onlineKey() {
      const n = this.node || {};
      const raw =
        n.devId !== undefined && n.devId !== null && n.devId !== ""
          ? n.devId
          : n.deviceId !== undefined && n.deviceId !== null && n.deviceId !== ""
          ? n.deviceId
          : n.id;
      return raw === undefined || raw === null || raw === "" ? "" : String(raw);
    },
    onlineDotClass() {
      const s = this.onlineMap[this.onlineKey] || "unknown";
      if (s === "online") return "is-online";
      if (s === "offline") return "is-offline";
      return "is-unknown";
    },
    onlineTitle() {
      const s = this.onlineMap[this.onlineKey] || "unknown";
      if (s === "online") return this.$t("overview.topologyOnline");
      if (s === "offline") return this.$t("overview.topologyOffline");
      return this.$t("overview.topologyUnknown");
    },
  },
  methods: {
    formatDeviceType(t) {
      const row = findDeviceTypeByValue(deviceTypes, t);
      if (!row) return "";
      return this.isEnLocale ? row.en_label : row.label;
    },
  },
};
</script>

<style scoped>
.topo-branch {
  display: flex;
  flex-direction: column;
  align-items: center;
  width: var(--topo-branch-w, 132px);
  min-width: 0;
}

.topo-branch--aux {
  align-self: flex-start;
}

.topo-rbms-sub {
  display: flex;
  flex-direction: column;
  align-items: center;
  width: 100%;
  margin-top: 4px;
  padding: var(--topo-rbms-sub-pad-y, 14px) var(--topo-rbms-sub-pad-x, 10px)
    calc(var(--topo-rbms-sub-pad-y, 14px) + 2px);
  border-radius: 8px;
  border: 1px solid #bbf7d0;
  background: rgba(240, 253, 244, 0.65);
  box-sizing: border-box;
  gap: var(--topo-rep-gap, 8px);
}

.topo-node-card {
  border-radius: 8px;
  border: 1px solid #dbeafe;
  background: #fff;
  box-shadow: 0 2px 8px rgba(15, 23, 42, 0.06);
  box-sizing: border-box;
  width: 100%;
  padding: var(--topo-node-pad-y, 12px) var(--topo-node-pad-x, 12px);
  min-height: 56px;
}

.topo-node-card--rbms {
  border-color: #93c5fd;
  background: linear-gradient(180deg, #eff6ff 0%, #ffffff 100%);
}

.topo-node-card--aux {
  max-width: 100%;
}

.topo-node-card__head {
  display: flex;
  align-items: flex-start;
  gap: 8px;
}

.topo-node-card__icon {
  width: var(--topo-node-icon, 28px);
  height: var(--topo-node-icon, 28px);
  border-radius: 6px;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  background: #eff6ff;
  color: #2563eb;
  font-size: calc(var(--topo-node-icon, 28px) * 0.46);
  flex-shrink: 0;
}

.topo-node-card__text {
  min-width: 0;
  flex: 1;
}

.topo-node-card__title {
  font-size: var(--topo-node-title, 13px);
  font-weight: 700;
  color: #0f172a;
  line-height: 1.3;
  word-break: break-word;
}

.topo-node-card__sub {
  margin-top: 4px;
  font-size: var(--topo-node-sub, 11px);
  color: #64748b;
  line-height: 1.25;
}

.topo-node-card__dot {
  width: 8px;
  height: 8px;
  border-radius: 50%;
  flex-shrink: 0;
  margin-top: 2px;
  border: 1px solid rgba(148, 163, 184, 0.35);
}

.topo-node-card__dot.is-online {
  background: #22c55e;
}

.topo-node-card__dot.is-offline {
  background: #ef4444;
}

.topo-node-card__dot.is-unknown {
  background: #94a3b8;
}

.topo-v-line {
  width: 2px;
  height: var(--topo-v-line-h, 16px);
  background: #3b82f6;
  border-radius: 1px;
  flex-shrink: 0;
}

.topo-v-line--l2 {
  background: #22c55e;
}

.topo-v-line--l3 {
  background: #6366f1;
}

.topo-rep-row {
  display: grid;
  grid-template-columns: 1fr 1fr;
  gap: var(--topo-rep-gap, 8px);
  width: 100%;
}

.topo-rep-card {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: var(--topo-rep-gap, 8px);
  padding: var(--topo-rep-pad-y, 12px) var(--topo-rep-pad-x, 8px);
  border-radius: 7px;
  min-height: 72px;
  border: 1px solid #e2e8f0;
  background: #fff;
  box-shadow: 0 1px 3px rgba(15, 23, 42, 0.05);
  min-width: 0;
  box-sizing: border-box;
  text-align: center;
}

.topo-rep-card--pbms {
  width: 100%;
  min-height: 64px;
  border-color: #86efac;
  background: linear-gradient(180deg, #f0fdf4 0%, #ffffff 100%);
}

.topo-rep-card--cell {
  border-color: #a5b4fc;
  background: linear-gradient(180deg, #eef2ff 0%, #ffffff 100%);
}

.topo-rep-card--temp {
  border-color: #fdba74;
  background: linear-gradient(180deg, #fff7ed 0%, #ffffff 100%);
}

.topo-rep-card__icon {
  width: var(--topo-rep-icon, 26px);
  height: var(--topo-rep-icon, 26px);
  border-radius: 5px;
  display: inline-flex;
  align-items: center;
  justify-content: center;
  font-size: calc(var(--topo-rep-icon, 26px) * 0.46);
  flex-shrink: 0;
}

.topo-rep-card--pbms .topo-rep-card__icon {
  background: #dcfce7;
  color: #15803d;
}

.topo-rep-card--cell .topo-rep-card__icon {
  background: #e0e7ff;
  color: #4338ca;
}

.topo-rep-card--temp .topo-rep-card__icon {
  background: #ffedd5;
  color: #c2410c;
}

.topo-rep-card__text {
  min-width: 0;
  width: 100%;
}

.topo-rep-card__title {
  font-size: var(--topo-rep-title, 12px);
  font-weight: 700;
  color: #0f172a;
  line-height: 1.2;
  white-space: nowrap;
  overflow: hidden;
  text-overflow: ellipsis;
}

.topo-rep-card__count {
  font-size: var(--topo-rep-count, 11px);
  font-weight: 800;
  color: #64748b;
  margin-top: 3px;
}
</style>
