<template>
  <div class="bms-main-viewport">
    <!-- 左侧：运行信息 -->
    <div class="column-side">
      <div class="info-card flex-grow-box">
        <div class="card-header-refined"><span class="blue-bar"></span><h4>{{ $t("overview.运行信息") }}</h4></div>
        <div class="card-body scroll-y-auto">
          <div class="data-group">
            <div class="group-tag tag-blue">{{ $t("overview.运行状态") }}</div>
            <ul class="data-list-v3">
              <li><span class="label">{{ $t("overview.系统总电压") }}：</span><span class="value">{{ runStatus.totalVoltage }}</span></li>
              <li><span class="label">{{ $t("overview.总电流") }}：</span><span class="value">{{ runStatus.totalCurrent }}</span></li>
              <li><span class="label">{{ $t("overview.总功率") }}：</span><span class="value highlight-txt">{{ runStatus.totalPower }}</span></li>
              <li><span class="label">SOC：</span><span class="value highlight-txt">{{ runStatus.soc }}</span></li>
              <li><span class="label">SOH：</span><span class="value">{{ runStatus.soh }}</span></li>
              <li><span class="label">{{ $t("overview.最大Cell电压") }}：</span><span class="value">{{ runStatus.maxCellVoltage }}</span></li>
              <li><span class="label">{{ $t("overview.最小Cell电压") }}：</span><span class="value">{{ runStatus.minCellVoltage }}</span></li>
              <li><span class="label">{{ $t("overview.最大温度") }}：</span><span class="value">{{ runStatus.maxTemperature }}</span></li>
              <li><span class="label">{{ $t("overview.最小温度") }}：</span><span class="value">{{ runStatus.minTemperature }}</span></li>
            </ul>
          </div>
          <div class="data-group mt-20">
            <div class="group-tag tag-green">{{ $t("overview.限功率信息") }}</div>
            <ul class="data-list-v3">
              <li><span class="label">{{ $t("overview.额定容量") }}：</span><span class="value">{{ powerLimit.ratedCapacity }}</span></li>
              <li><span class="label">{{ $t("overview.最大充电功率") }}：</span><span class="value">{{ powerLimit.maxChargePower }}</span></li>
              <li><span class="label">{{ $t("overview.最大放电功率") }}：</span><span class="value">{{ powerLimit.maxDischargePower }}</span></li>
              <li><span class="label">{{ $t("overview.最大充电电流") }}：</span><span class="value">{{ powerLimit.maxChargeCurrent }}</span></li>
              <li><span class="label">{{ $t("overview.最大放电电流") }}：</span><span class="value">{{ powerLimit.maxDischargeCurrent }}</span></li>
            </ul>
          </div>
        </div>
      </div>
    </div>

    <!-- 中间：系统拓扑图 (CSS Grid 炸开版) -->
    <div class="column-main">
      <div class="info-card">
        <div class="card-header-refined"><span class="blue-bar"></span><h4>{{ $t("overview.系统拓扑图") }}</h4></div>
        <div class="card-body topology-content-wrapper">
           <BmsTopologyDiagram :topology-tree="topologyTree" />
        </div>
      </div>
    </div>

    <!-- 右侧：系统与告警 -->
    <div class="column-side">
      <div class="right-stack">
        <div class="info-card flex-half">
          <div class="card-header-refined"><span class="blue-bar"></span><h4>{{ $t("overview.系统信息") }}</h4></div>
          <div class="card-body">
            <ul class="data-list-v3">
              <li><span class="label">{{ $t("overview.总簇数") }}：</span><span class="value">{{ systemInfo.totalClusters }}</span></li>
              <li><span class="label">{{ $t("overview.在网簇数") }}：</span><span class="value">{{ systemInfo.onlineClusters }}</span></li>
              <li><span class="label">{{ $t("overview.电池总数") }}：</span><span class="value">{{ systemInfo.totalBatteries }}</span></li>
              <li><span class="label">{{ $t("overview.在网电池") }}：</span><span class="value">{{ systemInfo.onlineBatteries }}</span></li>
              <li><span class="label">{{ $t("overview.从机总数") }}：</span><span class="value">{{ systemInfo.totalSlaves }}</span></li>
              <li><span class="label">{{ $t("overview.在网从机") }}：</span><span class="value">{{ systemInfo.onlineSlaves }}</span></li>
            </ul>
          </div>
        </div>
        <div class="info-card flex-half mt-16">
          <div class="card-header-refined card-header-with-action">
            <div class="header-left"><h4>{{ $t("overview.告警信息") }}</h4></div>
            <el-button type="primary" size="small" @click="goAlarmDetail">{{ $t("overview.详细报警") }}</el-button>
          </div>
          <div class="card-body">
            <ul class="data-list-v3">
              <li><span class="label">{{ $t("overview.严重告警") }}：</span><span class="value alarm-red">{{ alarmSummary.critical }}</span></li>
              <li><span class="label">{{ $t("overview.一般告警") }}：</span><span class="value alarm-orange">{{ alarmSummary.general }}</span></li>
            </ul>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { getCombinedInfo } from "@/api/system/overview";
import BmsTopologyDiagram from "./BmsTopologyDiagram.vue";
import { listTopology } from "@/api/system/topology";

export default {
  name: "TopologyMap",
  components: { BmsTopologyDiagram },
  data() {
    return {
      runStatus: {},
      powerLimit: {},
      systemInfo: {},
      alarmSummary: {},
      topologyTree: [],
      overviewPollTimer: null,
    };
  },
  created() {
    this.fetchCombinedInfo();
    this.loadTopologyTree();
    this.overviewPollTimer = setInterval(() => this.fetchCombinedInfo(), 2000);
  },
  beforeDestroy() {
    if (this.overviewPollTimer) {
      clearInterval(this.overviewPollTimer);
      this.overviewPollTimer = null;
    }
  },
  watch: {
    "$i18n.locale"() {
      this.loadTopologyTree();
    },
  },
  methods: {
    async loadTopologyTree() {
      const res = await listTopology();
      this.topologyTree = Array.isArray(res?.data) ? res.data : [];
    },
    fetchCombinedInfo() {
      getCombinedInfo()
        .then((response) => {
          const data = response.data || {};
          // 与设备监控 BBMS 点表同步（后端按堆控模板测点匹配）
          this.runStatus = data.runStatus || {};
          this.powerLimit = data.powerLimit || {};
          this.systemInfo = data.systemInfo || {};
          this.alarmSummary = data.alarmSummary || {};
        })
        .catch((err) => console.error(err));
    },
    goAlarmDetail() { this.$router.push("/history/alarm").catch(() => {}); }
  }
};
</script>

<style scoped>
/* 保持背景和基础间距不变 */
.bms-main-viewport { 
  display: flex; 
  width: 100%; 
  height: calc(100vh - 90px); 
  min-height: 640px;
  padding: 16px; 
  gap: 16px; 
  background: linear-gradient(180deg, #f8fafc 0%, #f1f5f9 100%);
  box-sizing: border-box; 
  overflow: hidden; /* 保证整体不产生网页滚动条 */
}

/* 修改：宽度在小屏下允许自动缩小，最大保持 300px */
.column-side { 
  width: clamp(240px, 18vw, 300px); 
  flex-shrink: 0; 
  display: flex; 
  flex-direction: column; 
}

.column-main { flex: 1; min-width: 0; display: flex; flex-direction: column; }

/* 保持原有卡片质感 */
.info-card { 
  background: #fff; 
  border-radius: 14px; 
  border: 1px solid #e2e8f0; 
  display: flex; 
  flex-direction: column; 
  height: 100%; 
  min-height: 0; /* 允许内部元素自适应缩减 */
  box-shadow: 0 8px 24px rgba(15, 23, 42, 0.05);
}

/* 关键修改：如果信息变多或屏幕变矮，卡片内部自动出现滚动条，不遮挡内容 */
.card-body { 
  flex: 1; 
  padding: 12px 15px; /* 稍微收紧内边距 */
  overflow-y: auto;   /* 信息再多也不怕，内部滚动 */
  min-height: 0; 
}

/* 美化卡片内部滚动条，使其不显眼，保持美观 */
.card-body::-webkit-scrollbar { width: 5px; }
.card-body::-webkit-scrollbar-thumb { background: #e2e8f0; border-radius: 10px; }

.topology-content-wrapper {
  width: 100%;
  height: 100%;
  display: flex;
  flex-direction: column;
  overflow: hidden;
  min-height: 0;
}

/* 保持头部样式不变 */
.card-header-refined { 
  padding: 12px 16px; 
  border-bottom: 1px solid #eef2f7; 
  display: flex; 
  align-items: center; 
  flex-shrink: 0; /* 头部固定，不参与滚动 */
  background: linear-gradient(180deg, #ffffff 0%, #f8fafc 100%);
}

.blue-bar { width: 4px; height: 16px; background-color: #3498db; margin-right: 12px; border-radius: 2px; }

/* 列表样式保持不变，仅微调间距 */
.data-list-v3 { list-style: none; padding: 0; margin: 0; }
.data-list-v3 li { 
  display: flex; 
  justify-content: space-between; 
  padding: 8px 0; /* 稍微缩小一点，防止行数多时太挤 */
  border-bottom: 1px dashed #e2e8f0; 
  font-size: 13px; 
}

/* 颜色风格完全保留你的原始定义 */
.label { color: #64748b; }
.value { color: #1e293b; font-weight: 600; }
.highlight-txt { color: #3498db; font-weight: bold; }
.alarm-red { color: #e74c3c; font-weight: bold; }
.alarm-orange { color: #f39c12; font-weight: bold; }

/* 保持你的间距定义 */
.mt-20 { margin-top: 16px; } 
.group-tag { font-size: 12px; padding: 2px 8px; border-radius: 4px; margin-bottom: 8px; font-weight: 600; display: inline-block; }
.tag-blue { background: #eef7ff; color: #3498db; }
.tag-green { background: #f0fff4; color: #27ae60; }

.right-stack { display: flex; flex-direction: column; height: 100%; gap: 16px; }
.flex-half { flex: 1; min-height: 0; }
.card-header-with-action { justify-content: space-between; }
</style>

