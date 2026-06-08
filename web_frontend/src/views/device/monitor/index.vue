<template>
  <div class="background device-monitor-page">
    <el-row class="layout-row">
      <el-col class="device-sidebar card left">
        <div class="tree-title">{{ $t('monitor.设备监控') }}</div>
        <el-tree
          ref="deviceTree"
          class="device-tree device-tree--topology"
          :data="deviceTree"
          :props="{ label: 'label', children: 'children' }"
          node-key="id"
          :indent="22"
          :expand-on-click-node="false"
          :default-expand-all="false"
          :default-expanded-keys="treeDefaultExpandKeys"
          highlight-current
          @current-change="handleMonitorTreeCurrentChange"
        >
          <span
            slot-scope="{ node, data }"
            class="monitor-tree-node"
            :class="[
              'monitor-tree-node--level-' + node.level,
              { 'monitor-tree-node--active': isMonitorTreeNodeActive(data) },
            ]"
          >
            <span v-if="monitorTreeTypeAbbrev(data)" class="monitor-tree-node__tag">{{
              monitorTreeTypeAbbrev(data)
            }}</span>
            <span class="monitor-tree-node__label" :title="data.label">{{ data.label }}</span>
          </span>
        </el-tree>
      </el-col>

      <el-col v-loading="devicePointsLoading" class="card right monitor-detail-pane">
        <el-alert
          v-if="monitorTemplatePending && !devicePointsLoading"
          type="info"
          show-icon
          :closable="false"
          :title="$t('monitor.templatePendingTitle')"
          :description="$t('monitor.templatePendingDesc')"
          style="margin-bottom: 16px;"
        />
        <template v-else-if="!devicePointsLoading">
          <local-dashboard v-if="monitorLayoutKind === 'local'" />
          <bbms-stack-dashboard v-else-if="monitorLayoutKind === 'bbms'" />
          <rbms-cluster-console v-else-if="monitorLayoutKind === 'rbms'" />
          <auxiliary-device-panel
            v-else-if="monitorLayoutKind === 'auxiliary'"
            :device-name="currentNodeLabel"
            :device-type-label="monitorSelectedDeviceTypeLabel"
            :protocol-label="monitorSelectedProtocolLabel"
            :device-id-text="monitorFlatDeviceIdText"
            :template-id-text="monitorFlatTemplateIdText"
            :online="monitorAuxiliaryOnline"
            :points="currentPoints"
            :rtdb-dev-type="monitorRtdbDevType"
            :rtdb-dev-id="monitorRtdbDevId"
            :write-enabled="monitorAuxWriteEnabled"
            :keyword.sync="pointKeyword"
            :only-valued.sync="onlyShowValued"
            :group-title-fn="(g) => monitorGroupTypeTitle(g)"
            :point-name-fn="(p) => pointDisplayName(p)"
            @point-dispatched="onAuxPointDispatched"
          />
          <div v-else class="no-data">{{ $t('monitor.暂无数据') }}</div>
        </template>
      </el-col>
    </el-row>
  </div>
</template>
<script>
import i18n from "@/i18n";
import * as echarts from "echarts";
import { listTopology } from "@/api/system/topology";
import { getDevicePointsWithStatus } from "@/api/system/device";
import { deviceTypes, findDeviceTypeByValue } from "@/constants/deviceTypes";
import {
  resolveBbmsSectionId,
  BBMS_SECTION_ORDER,
  BBMS_SECTION_META,
  BBMS_HERO_RULES,
  BBMS_HERO_SECONDARY_RULES,
  BBMS_STAT_SECTION_IDS,
  BBMS_STAT_GROUPS,
  BBMS_MAIN_TABS,
  bbmsPointMatchText,
  bbmsPointDedupeKey,
  bbmsPointIsStatusHeroPromoted,
  sortBbmsSeriesRows,
  buildBbmsFaultPanelSections,
} from "@/constants/bbmsPointSections";
import {
  resolveRbmsSectionId,
  RBMS_SECTION_ORDER,
  RBMS_SECTION_META,
  RBMS_HERO_SLOTS,
  RBMS_MAIN_TABS,
  rbmsPointDedupeKey,
  buildRbmsCellGrid,
  buildRbmsPackTempGrid,
  RBMS_CELL_UI,
} from "@/constants/rbmsPointSections";
import AuxiliaryDevicePanel from "./AuxiliaryDevicePanel.vue";
import LocalDashboard from "./LocalDashboard.vue";
import BbmsStackDashboard from "./BbmsStackDashboard.vue";
import RbmsClusterConsole from "./RbmsClusterConsole.vue";
import { protocolTypes } from "@/constants/protocolTypes";
import { pointGroupMeta } from "@/constants/pointGroupTypes";

/** 本机运行大卡锚点：命中者从下方测点卡剔除（与 pointIsExcludedFromLocalPointCards 中的锚点部分一致） */
const HOST_ANCHOR_SLOT_DEFS = [
  { key: "local", m: (t) => /本机数量|本机台数/i.test(t) },
  { key: "stack", m: (t) => /BMS堆控|堆控数量/i.test(t) },
  {
    key: "clusterCtrl",
    m: (t) =>
      /BMS\s*簇\s*控制器|簇\s*控制器\s*数量|BMS簇控制器数量|簇控制器数量/i.test(t) ||
      (/簇\s*控制\s*器/i.test(t) && /数量|台数/i.test(t)),
  },
  { key: "pcs", m: (t) => /PCS\s*数量|PCS台数|pcs\s*count|PCS\s*Count/i.test(t) },
];

/** 与模板 ID 同区展示的侧栏测点：从测点卡剔除，仅出现在右侧 tile */
const HOST_LOCAL_SIDE_TILE_DEFS = [
  { key: "heartbeat", m: (t) => /运行心跳|心跳计数|运行\s*心跳/i.test(t) },
  {
    key: "localOnline",
    m: (t) => /本机是否在线|本机是否离线|本机在线|本机联网|本机\s*通讯/i.test(t),
  },
  { key: "testCount", m: (t) => /测试次数|试验次数|Test\s*(Count|Times)/i.test(t) },
  { key: "upgradeProgress", m: (t) => /升级控制器进度|控制器升级进度|升级.*进度|Upgrade.*progress/i.test(t) },
  {
    key: "telem50",
    m: (t) =>
      /\bYCData50\b|YC\s*Data\s*50|（\s*YCData50\s*）|\(\s*YCData50\s*\)|遥测数据\s*第\s*50\s*槽|遥测数据第50槽|遥测.*50.*槽|第\s*50\s*槽|YC50\b|YC\s*50\b|Telemetry.*50|Slot\s*50/i.test(
        t
      ),
  },
];

export default {
  name: "DeviceMonitor",
  components: {
    AuxiliaryDevicePanel,
    LocalDashboard,
    BbmsStackDashboard,
    RbmsClusterConsole,
  },
  provide() {
    return { monitor: this };
  },
  data() {
    const isEn = i18n.locale === "en";
    const STATUS_NORMAL = isEn ? "Normal" : "正常";
    const STATUS_ALARM = isEn ? "Alarm" : "告警";
    const STATUS_FINISHED = isEn ? "Completed" : "完成";
    const STATUS_NOT_TRIGGERED = isEn ? "Not Triggered" : "未触发";

    return {
      activeView: "pointView",
      currentNode: null,
      currentNodeLabel: "Local system",
      /** 勿预置演示树：接口返回前应为空，否则点击的是旧对象，deviceType 与界面不一致会误判为本机 */
      deviceTree: [],
      rbmsDataList: (() => {
        const statusLights = ["charging", "discharge", "balance", "full", "deep"];
        return Array.from({ length: 12 }, (_, i) => ({
          clusterVoltage: `${790 + i * 2 + (i % 3)} V`,
          clusterCurrent: `${22 + (i % 5) + (i % 2)} A`,
          circulationCurrent: `${(0.8 + (i % 4) * 0.15).toFixed(1)} A`,
          tempMax: `${40 + (i % 5)} ℃`,
          tempMin: `${33 + (i % 4)} ℃`,
          insulationResistance: `${(2.2 + (i % 6) * 0.1).toFixed(1)} MΩ`,
          maxChargeCurrent: `${270 + (i % 20)} A`,
          maxDischargeCurrent: `${270 + (i % 20)} A`,
          contactorClosed: i % 4 !== 1,
          statusLight: statusLights[i % statusLights.length],
          powerSeries: Array.from({ length: 7 }, (__, h) =>
            Math.round(10 + (i + 1) * 1.5 + (h % 3) * 4 + Math.sin(h + i) * 8)
          ),
        }));
      })(),
      selectedPbmsNodeId: null,
      pbmsDataByRbms: Array.from({ length: 12 }, (_, rbmsIdx) =>
        Array.from({ length: 8 }, (_, pbmsIdx) => {
          const baseV = 3.25 + ((rbmsIdx + pbmsIdx) % 4) * 0.01;
          const cellVoltages = Array.from({ length: 32 }, (_, i) =>
            baseV + (i % 5) * 0.004 + ((rbmsIdx + pbmsIdx + i) % 3) * 0.001
          );
          const maxCellId = cellVoltages.indexOf(Math.max(...cellVoltages)) + 1;
          const minCellId = cellVoltages.indexOf(Math.min(...cellVoltages)) + 1;
          return {
            packVoltage: `${(790 + rbmsIdx * 3 + pbmsIdx).toFixed(0)} V`,
            packCurrent: `${(2.2 + ((rbmsIdx + pbmsIdx) % 6) * 0.3).toFixed(1)} A`,
            soc: `${76 + ((rbmsIdx + pbmsIdx) % 12)}%`,
            cellVoltages,
            cellTemps: Array.from({ length: 6 }, (_, i) => 34 + ((rbmsIdx + pbmsIdx + i) % 8)),
            maxCellId,
            minCellId,
            balancingCells: [maxCellId, ((pbmsIdx * 3) % 32) + 1, ((pbmsIdx * 5) % 32) + 1],
          };
        })
      ),
      pbmsAlertsByRbms: Array.from({ length: 12 }, (_, rbmsIdx) =>
        Array.from({ length: 8 }, (_, pbmsIdx) => ([
          {
            time: "14:32:01",
            msgZh: `PBMS#${pbmsIdx + 1} 单体过压告警 Cell#${10 + ((rbmsIdx + pbmsIdx) % 8)}`,
            msgEn: `PBMS#${pbmsIdx + 1} Cell overvoltage alarm Cell#${10 + ((rbmsIdx + pbmsIdx) % 8)}`,
          },
          {
            time: "14:28:15",
            msgZh: `RBMS#${rbmsIdx + 1} 压差过大`,
            msgEn: `RBMS#${rbmsIdx + 1} excessive voltage difference`,
          },
          {
            time: "14:15:00",
            msgZh: `PBMS#${pbmsIdx + 1} 均衡启动`,
            msgEn: `PBMS#${pbmsIdx + 1} balancing started`,
          },
        ]))
      ),
      pbmsAlertChart: null,
      cellDetailVisible: false,
      selectedCellId: 1,
      cellDetailChart: null,
      upsData: {
        inputVoltage: "220 V",
        outputVoltage: "220 V",
        frequency: "50 Hz",
        loadPercent: 65,
        backupMinutes: 45,
        workMode: "inverter",
      },
      meterData: {
        voltageA: "220 V",
        voltageB: "219 V",
        voltageC: "221 V",
        currentA: "120 A",
        currentB: "118 A",
        currentC: "122 A",
        activePower: "78 kW",
        reactivePower: "12 kVar",
        powerFactor: "0.98",
        totalChargeKwh: "1250",
        totalDischargeKwh: "1180",
        reverseFlow: false,
        peakKwh: 320,
        flatKwh: 480,
        valleyKwh: 450,
      },
      rbmsChart: null,
      pbmsCellChart: null,
      pbmsHeatChart: null,
      upsGaugeChart: null,
      meterChart: null,
      systemInfo: {
        soc: "80%",
        soh: "98%",
        totalVoltage: "1400 V",
        totalCurrent: "1116 A", // 按 1.56MW 计算约 1.1kA
        totalPower: "1.56 MW",
        runStatus: isEn ? "Discharging" : "放电",
        faultStatus: isEn ? "No Fault" : "无故障",
        maxChargeCurrent: "1100 A",
        maxDischargeCurrent: "1100 A",
        maxDischargePower: "1.56 MW",
        maxChargePower: "1.56 MW",
      },
      sbmsConsistency: {
        socDiff: "3%",
        voltageDiff: "12 V",
      },
      rbmsStatusMatrix: [
        { id: 1, online: true, alarm: false },
        { id: 2, online: true, alarm: false },
        { id: 3, online: true, alarm: true },
        { id: 4, online: true, alarm: false },
        { id: 5, online: true, alarm: false },
        { id: 6, online: false, alarm: false },
        { id: 7, online: true, alarm: false },
        { id: 8, online: true, alarm: true },
        { id: 9, online: true, alarm: false },
        { id: 10, online: false, alarm: false },
        { id: 11, online: true, alarm: false },
        { id: 12, online: true, alarm: false },
      ],
      baseInfo: {
        totalClusters: 4,
        onlineClusters: 4,
        totalBatteries: 1664,
        onlineBatteries: 1664,
        totalTempSensors: 1664,
        onlineTempSensors: 1664,
        totalSlaves: 4,
        onlineSlaves: 4,
        ratedCapacity: "6.25 MWh",
        ratedEnergy: "6.25 MWh",
        remainingCapacity: "5.0 MWh",
        remainingEnergy: "5.0 MWh",
        softwareVersion: "V1.0.0",
        systemTime: "2026-02-27 10:00:00",
      },
      dioInfo: {
        doStatus: Array.from({ length: 12 }, (_, i) => ({
          name: `DO${i + 1}`,
          status:
            i < 2
              ? isEn
                ? "Closed"
                : "合闸"
              : isEn
              ? "Open"
              : "分闸",
        })),
        diStatus: Array.from({ length: 12 }, (_, i) => ({
          name: `DI${i + 1}`,
          status:
            i < 2
              ? isEn
                ? "Signal"
                : "有信号"
              : isEn
              ? "No Signal"
              : "无信号",
        })),
        otherSignals: isEn
          ? ["E‑Stop Status", "Fire Linkage", "Access Control", "AC Linkage"]
          : ["急停状态", "消防联动", "门禁状态", "空调联动"],
      },
      clusterStats: [
        {
          name: isEn ? "Max Pack Voltage Diff" : "电池压差最大值",
          value: "35 mV",
          cluster: "#3",
        },
        {
          name: isEn ? "Min Pack Voltage Diff" : "电池压差最小值",
          value: "5 mV",
          cluster: "#1",
        },
        {
          name: isEn ? "Max Pack Temp Diff" : "电池温差最大值",
          value: "6 ℃",
          cluster: "#4",
        },
        {
          name: isEn ? "Min Pack Temp Diff" : "电池温差最小值",
          value: "1 ℃",
          cluster: "#2",
        },
        {
          name: isEn ? "Max Pole Temp" : "极柱温度最大值",
          value: "45 ℃",
          cluster: "#5",
        },
        {
          name: isEn ? "Max Rack Voltage" : "电池箱电压最大值",
          value: "860 V",
          cluster: "#6",
        },
        {
          name: isEn ? "Min Rack Voltage" : "电池箱电压最小值",
          value: "780 V",
          cluster: "#2",
        },
        {
          name: isEn ? "Min +Insulation" : "簇正极绝缘最小值",
          value: "2 MΩ",
          cluster: "#3",
        },
        {
          name: isEn ? "Min –Insulation" : "簇负极绝缘最小值",
          value: "2 MΩ",
          cluster: "#4",
        },
      ],
      chargeStats: [
        {
          name: isEn ? "Daily Discharge Time" : "日放电时间",
          value: "3.5 h",
        },
        {
          name: isEn ? "Daily Charge Time" : "日充电时间",
          value: "4.0 h",
        },
        {
          name: isEn ? "Daily Discharge Capacity" : "日放电容量",
          value: "120 Ah",
        },
        {
          name: isEn ? "Daily Charge Capacity" : "日充电容量",
          value: "118 Ah",
        },
        {
          name: isEn ? "Daily Discharge Energy" : "日放电电量",
          value: "120 kWh",
        },
        {
          name: isEn ? "Daily Charge Energy" : "日充电电量",
          value: "118 kWh",
        },
        {
          name: isEn ? "Single Discharge Time" : "单次放电时间",
          value: "1.2 h",
        },
        {
          name: isEn ? "Single Charge Time" : "单次充电时间",
          value: "1.5 h",
        },
        {
          name: isEn ? "Single Discharge Capacity" : "单次放电容量",
          value: "80 Ah",
        },
        {
          name: isEn ? "Single Charge Capacity" : "单次充电容量",
          value: "78 Ah",
        },
        {
          name: isEn ? "Single Discharge Energy" : "单次放电电量",
          value: "90 kWh",
        },
        {
          name: isEn ? "Single Charge Energy" : "单次充电电量",
          value: "88 kWh",
        },
        {
          name: isEn ? "Total Discharge Time" : "累计放电时间",
          value: "520 h",
        },
        {
          name: isEn ? "Total Charge Time" : "累计充电时间",
          value: "540 h",
        },
        {
          name: isEn ? "Total Discharge Capacity" : "累计放电容量",
          value: "12.5 MAh",
        },
        {
          name: isEn ? "Total Charge Capacity" : "累计充电容量",
          value: "12.8 MAh",
        },
        {
          name: isEn ? "Total Discharge Energy" : "累计放电电量",
          value: "5.3 MWh",
        },
        {
          name: isEn ? "Total Charge Energy" : "累计充电电量",
          value: "5.5 MWh",
        },
      ],
      alarmInfo: {
        battery: [
          {
            name: isEn ? "Pack Overvoltage" : "总压过压",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Pack Undervoltage" : "总压欠压",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Cell Overvoltage" : "单体过压",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Cell Undervoltage" : "单体欠压",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Cell Over‑temperature" : "单体过温",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Cell Low Temperature" : "单体低温",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Excessive Voltage Diff" : "压差过大",
            status: STATUS_ALARM,
          },
          {
            name: isEn ? "Excessive Temp Diff" : "温差过大",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "SOC High" : "SOC过高",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "SOC Low" : "SOC过低",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Fast Temp Rising" : "温升过快",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Critical Fault" : "极限故障",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Cabinet Overvoltage" : "电池箱过压",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Cabinet Undervoltage" : "电池箱欠压",
            status: STATUS_NORMAL,
          },
        ],
        system: [
          { name: isEn ? "T1 Over‑temperature" : "T1高温", status: STATUS_NORMAL },
          { name: isEn ? "T1 Low Temperature" : "T1低温", status: STATUS_NORMAL },
          { name: isEn ? "T2 Over‑temperature" : "T2高温", status: STATUS_NORMAL },
          { name: isEn ? "T2 Low Temperature" : "T2低温", status: STATUS_NORMAL },
          { name: isEn ? "Pre‑charge Status" : "预充状态", status: STATUS_FINISHED },
          { name: isEn ? "Supply Overvoltage" : "供电过高", status: STATUS_NORMAL },
          { name: isEn ? "Supply Undervoltage" : "供电过低", status: STATUS_NORMAL },
          { name: isEn ? "Master‑Slave Communication" : "主从通讯", status: STATUS_NORMAL },
          { name: isEn ? "Insulation Leakage" : "绝缘漏电", status: STATUS_ALARM },
          { name: isEn ? "HV Abnormal" : "高压异常", status: STATUS_NORMAL },
          { name: isEn ? "Pole Over‑temperature" : "极柱过温", status: STATUS_NORMAL },
          { name: isEn ? "Cluster Current Diff" : "簇间电流差", status: STATUS_NORMAL },
          { name: isEn ? "Cluster Temp Diff" : "簇间温差", status: STATUS_NORMAL },
          { name: isEn ? "Cluster Voltage Diff" : "簇间压差", status: STATUS_NORMAL },
          { name: isEn ? "Balance Over‑temperature" : "均衡过温", status: STATUS_NORMAL },
        ],
        external: [
          {
            name: isEn ? "Emergency Stop Fault" : "急停故障",
            status: STATUS_NOT_TRIGGERED,
          },
          {
            name: isEn ? "Fire Alarm" : "消防火警",
            status: STATUS_NOT_TRIGGERED,
          },
          {
            name: isEn ? "Fire Spray" : "消防喷洒",
            status: STATUS_NOT_TRIGGERED,
          },
          {
            name: isEn ? "Fire Device Fault" : "消防设备故障",
            status: STATUS_NOT_TRIGGERED,
          },
          {
            name: isEn ? "Water Immersion Fault" : "水浸故障",
            status: STATUS_NOT_TRIGGERED,
          },
          {
            name: isEn ? "Access Control Fault" : "门禁故障",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Gas Detection Fault" : "气体检测故障",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "AC Fault" : "空调故障",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "SPD Fault" : "防雷器故障",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Contact Sticking" : "粘连故障",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Open Circuit Fault" : "开路故障",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "PCS Status" : "PCS状态",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "EMS Status" : "EMS状态",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "UPS Fault" : "UPS故障",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Fuse Status" : "保险丝状态",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Feedback Abnormal" : "反馈异常",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Control Command Timeout" : "控制指令超时",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "External Communication Fault" : "外部通信故障",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Cell Voltage Harness" : "单体电压排线",
            status: STATUS_NORMAL,
          },
          {
            name: isEn ? "Cell Temp Harness" : "单体温感排线",
            status: STATUS_NORMAL,
          },
        ],
      },
      currentPoints: [],
      pointKeyword: "",
      onlyShowValued: false,
      /** 非本机统计页：按分组筛选明细表，'' 表示全部 */
      flatTableGroupFilter: "",
      splitTabByGroupKey: {},
      /** BBMS 堆控详情 Tab（统计区在 Tab 之上单独展示） */
      bbmsMainTab: "runtime",
      /** RBMS 簇控二级页 Tab */
      rbmsMainTab: "overview",
      /** 设备测点轮询：1s 刷新（与 getDevicePointsWithStatus 对齐） */
      pointsPollTimer: null,
      pointsPollInFlight: false,
      /** 拓扑默认选中序号：用户手动点树后递增，取消尚未执行的「默认本机」回调 */
      topologyDefaultSelectSeq: 0,
      topologyLoadInFlight: false,
      /** 树高亮节点 -> 右侧详情 的兜底同步锁，避免递归触发 */
      syncingTreeCurrentNode: false,
      /** 左侧树当前高亮 id（与 currentNode 可能不同：虚拟文件夹会解析到子设备展示） */
      monitorTreeHighlightId: null,
      /** 测点加载序号：防止切换设备后旧请求覆盖新数据 */
      pointsLoadSeq: 0,
      devicePointsLoading: false,
      /** setCurrentKey 时抑制 current-change 重复触发 handleNodeClick */
      suppressMonitorTreeCurrentChange: false,
      /** RBMS 电芯信息：当前抽屉电芯序号（0-based） */
      rbmsCellDetailIdx: null,
      /** RBMS 电芯信息：轮询短时曲线 { [index0]: { ts, volt } } */
      rbmsCellHist: {},
      /** RBMS 温度模块：轮询短时曲线 { [tempIndex0]: { ts, temp } } */
      rbmsTempHist: {},
      rbmsCellHistMax: 90,
    };
  },
  computed: {
    isZhLocaleComputed() {
      const locale = String((this.$i18n && this.$i18n.locale) || "").toLowerCase();
      return locale.startsWith("zh");
    },
    currentRbmsIndex() {
      if (!this.currentNode || !this.currentNode.id || this.activeView !== "rbms") return 0;
      const idText = String(this.currentNode.id || "");
      const byId = idText.match(/^rbms-(\d+)$/i);
      if (byId) {
        const n = parseInt(byId[1], 10);
        return n >= 1 && n <= 12 ? n - 1 : 0;
      }
      // 兼容拓扑配置生成的数字 id，通过节点名称提取 RBMS 序号（如 3#RBMS）
      const labelText = String(this.currentNode.label || "");
      const byLabel = labelText.match(/(\d+)\s*#\s*rbms/i);
      if (byLabel) {
        const n = parseInt(byLabel[1], 10);
        return n >= 1 && n <= 12 ? n - 1 : 0;
      }
      return 0;
    },
    rbmsData() {
      const list = this.rbmsDataList || [];
      const idx = this.currentRbmsIndex;
      return list[idx] || list[0] || {};
    },
    currentRbmsPbmsNodes() {
      if (this.activeView !== "rbms" || !this.currentNode) return [];
      const pickPbms = (n) => {
        if (!n) return [];
        const hidden = n._monitorPbmsList;
        if (Array.isArray(hidden) && hidden.length) return hidden;
        const ch = n.children || [];
        return ch.filter((c) => {
          const t = Number(c && c.deviceType);
          const lab = String((c && c.label) || "").toLowerCase();
          return t === 1 || lab.includes("pbms");
        });
      };
      if (Number(this.currentNode.deviceType) === 2) {
        return pickPbms(this.currentNode);
      }
      const path = this.findNodePathById(this.deviceTree, this.currentNode.id);
      if (!Array.isArray(path) || !path.length) return [];
      const rbmsNode = [...path].reverse().find(n => Number(n && n.deviceType) === 2 || /rbms/i.test(String(n && n.label)));
      return pickPbms(rbmsNode);
    },
    currentPbmsIndex() {
      const list = this.currentRbmsPbmsNodes || [];
      if (!list.length) return 0;
      const hit = list.findIndex(n => String(n.id) === String(this.selectedPbmsNodeId));
      return hit >= 0 ? hit : 0;
    },
    pbmsData() {
      const rbmsIdx = this.currentRbmsIndex;
      const pbmsIdx = this.currentPbmsIndex;
      const row = (this.pbmsDataByRbms[rbmsIdx] || [])[pbmsIdx];
      return row || {
        packVoltage: "--",
        packCurrent: "--",
        soc: "--",
        cellVoltages: [],
        cellTemps: [],
        maxCellId: 1,
        minCellId: 1,
        balancingCells: [],
      };
    },
    pbmsAlerts() {
      const rbmsIdx = this.currentRbmsIndex;
      const pbmsIdx = this.currentPbmsIndex;
      return ((this.pbmsAlertsByRbms[rbmsIdx] || [])[pbmsIdx]) || [];
    },
    locale() {
      return this.$i18n.locale;
    },
    /** 设备节点未分配模板（0 / 空）：监控区提示去设备维护分配；虚拟节点按类型映射真实设备，不要求自身模板 */
    monitorTemplatePending() {
      const n = this.currentNode;
      if (!n) return false;
      if (n.type === "virtual") {
        const vdt = this.effectiveMonitorDeviceType(n);
        if (vdt !== null && vdt !== undefined) {
          return !this.collectVirtualMonitorPointDevices(n).length;
        }
        const devices = [];
        this.collectDescendantMonitorDevices(n, devices);
        if (!devices.length) return true;
        return !devices.some((d) => this.deviceHasMonitorTemplate(d));
      }
      if (n.type !== "device") return false;
      const tid =
        n.templateId !== undefined && n.templateId !== null && n.templateId !== ""
          ? n.templateId
          : n.template_id;
      if (tid === null || tid === undefined || tid === "") return true;
      return Number(tid) === 0;
    },
    treeDefaultExpandKeys() {
      const keys = [];
      const walk = (nodes) => {
        (nodes || []).forEach((n) => {
          const hasKids = Array.isArray(n.children) && n.children.length;
          if (hasKids && n.id !== undefined && n.id !== null && !keys.includes(n.id)) {
            keys.push(n.id);
          }
          if (hasKids) walk(n.children);
        });
      };
      walk(this.deviceTree);
      return keys;
    },
    pbmsMaxVoltageDiff() {
      const v = this.pbmsData.cellVoltages;
      if (!v || !v.length) return "--";
      const min = Math.min(...v);
      const max = Math.max(...v);
      const diffMv = Math.round((max - min) * 1000);
      return diffMv + " mV";
    },
    pbmsMaxTempDiff() {
      const t = this.pbmsData.cellTemps;
      if (!t || !t.length) return "--";
      return Math.max(...t) - Math.min(...t) + " ℃";
    },
    pbmsAvgVoltage() {
      const v = this.pbmsData.cellVoltages;
      if (!v || !v.length) return "--";
      const avg = v.reduce((a, b) => a + b, 0) / v.length;
      return (Math.round(avg * 1000) / 1000).toFixed(3) + " V";
    },
    bbmsScopedStats() {
      const fallback = {
        rbmsCount: 0,
        pbmsCount: 0,
        rbmsIdsText: "--",
        pbmsIdsText: "--",
      };
      const onStackLayout = this.monitorLayoutKind === "bbms";
      if (!onStackLayout) return fallback;
      const root = this.currentNode;
      if (!root) return fallback;

      let scopeChildren = root.children || [];
      if (root.type === "virtual") {
        const stack = this.primaryVirtualMonitorSourceDevice(root);
        scopeChildren = (stack && stack.children) || scopeChildren;
        if (!scopeChildren.length) {
          const trail = this.findNodePathById(this.deviceTree, root.id);
          const parent = trail && trail.length >= 2 ? trail[trail.length - 2] : null;
          const peerStack = (parent && parent.children) || [];
          const hit = (peerStack || []).find(
            (n) => n && n.type === "device" && this.monitorDeviceTypeNum(n.deviceType) === 1
          );
          scopeChildren = (hit && hit.children) || [];
        }
      }

      const rbmsNodes = [];
      const pbmsNodes = [];
      const walk = (nodes) => {
        (nodes || []).forEach((n) => {
          const t = Number(n && n.deviceType);
          const label = String((n && n.label) || "").toLowerCase();
          if (t === 2 || label.includes("rbms")) rbmsNodes.push(n);
          if (t === 1 || label.includes("pbms")) pbmsNodes.push(n);
          if (Array.isArray(n && n._monitorPbmsList) && n._monitorPbmsList.length) {
            walk(n._monitorPbmsList);
          }
          if (n && Array.isArray(n.children) && n.children.length) walk(n.children);
        });
      };
      walk(scopeChildren);
      const formatId = (n) => String((n && (n.devId || n.id || n.label)) || "");
      const rbmsIds = rbmsNodes.map(formatId).filter(Boolean);
      const pbmsIds = pbmsNodes.map(formatId).filter(Boolean);
      return {
        rbmsCount: rbmsNodes.length,
        pbmsCount: pbmsNodes.length,
        rbmsIdsText: rbmsIds.length ? rbmsIds.join(", ") : "--",
        pbmsIdsText: pbmsIds.length ? pbmsIds.join(", ") : "--",
      };
    },
    /** 本机系统（deviceType=0）点表视图：仪表盘顶栏 */
    isLocalSystemMonitor() {
      const n = this.currentNode;
      const t = this.effectiveMonitorDeviceType(n);
      return !!(n && (n.type === "device" || n.type === "virtual") && t === 0);
    },
    currentMonitorDeviceType() {
      return this.effectiveMonitorDeviceType(this.currentNode);
    },
    /**
     * 四套监控布局：本机(0) / 堆控 BBMS(1) / 簇控 RBMS(2) / 其它辅机。
     */
    monitorLayoutKind() {
      const t = this.effectiveMonitorDeviceType(this.currentNode);
      if (t === 0) return "local";
      if (t === 1) return "bbms";
      if (t === 2) return "rbms";
      if (t === null || t === undefined) return "none";
      return "auxiliary";
    },
    /** @deprecated 请用 monitorLayoutKind；保留供子组件 inject 兼容 */
    monitorPointLayoutMode() {
      const k = this.monitorLayoutKind;
      if (k === "local") return "localDashboard";
      if (k === "bbms") return "bbmsStackDashboard";
      if (k === "auxiliary") return "auxiliaryDashboard";
      return "flatFullTable";
    },
    /** 非本机、非堆控（含虚拟节点配置的辅机类型）；RBMS 走 activeView=rbms */
    isAuxiliaryDeviceMonitor() {
      const n = this.currentNode;
      if (!n || (n.type !== "device" && n.type !== "virtual")) return false;
      const t = this.effectiveMonitorDeviceType(n);
      if (t === null) return false;
      return t !== 0 && t !== 1 && t !== 2;
    },
    /** 堆控 / 站级 BBMS（deviceType=1） */
    isStackBmsMonitor() {
      const n = this.currentNode;
      const t = this.effectiveMonitorDeviceType(n);
      return !!(n && (n.type === "device" || n.type === "virtual") && t === 1);
    },
    monitorSelectedDeviceTypeLabel() {
      const raw = this.currentMonitorDeviceType;
      if (raw === null || raw === undefined) return "--";
      const hit = findDeviceTypeByValue(deviceTypes, raw);
      if (!hit) return String(raw);
      const loc = String(this.locale || "").toLowerCase();
      return loc.startsWith("en") ? hit.en_label || hit.label : hit.label;
    },
    monitorSelectedProtocolLabel() {
      const n = this.currentNode || {};
      const raw =
        n.protocolType !== undefined && n.protocolType !== null && n.protocolType !== ""
          ? n.protocolType
          : n.protocol_type;
      if (raw === null || raw === undefined || raw === "") return "";
      const hit = findDeviceTypeByValue(protocolTypes, raw);
      if (!hit) return String(raw);
      const loc = String(this.locale || "").toLowerCase();
      return loc.startsWith("en") ? hit.en_label || hit.label : hit.label;
    },
    monitorAuxiliaryOnline() {
      const list = this.currentPoints || [];
      const hit = list.find((p) => String(p.ename || "").trim() === "Lqd_Online");
      if (!hit) return false;
      return this.parseAuxiliaryOnlinePointValue(hit.value);
    },
    /** 外设 RTDB 写入：设备类型（与 C dev_type_e 一致） */
    monitorRtdbDevType() {
      const t = this.effectiveMonitorDeviceType(this.currentNode);
      return t === null || t === undefined ? null : Number(t);
    },
    /** 外设 RTDB 设备序号 = 拓扑 deviceCode - 1 */
    monitorRtdbDevId() {
      const n = this.currentNode;
      if (!n || n.type !== "device") return null;
      const raw =
        n.deviceCode !== undefined && n.deviceCode !== null && n.deviceCode !== ""
          ? n.deviceCode
          : n.device_code;
      if (raw === null || raw === undefined || raw === "") return null;
      const code = Number(raw);
      if (Number.isNaN(code) || code < 1) return null;
      return code - 1;
    },
    /** 仅单台设备节点可下发（虚拟目录聚合点表不可写） */
    monitorAuxWriteEnabled() {
      const n = this.currentNode;
      return !!(n && n.type === "device" && this.monitorRtdbDevId !== null && this.monitorRtdbDevType !== null);
    },
    monitorFlatDeviceIdText() {
      const n = this.currentNode;
      if (n && n.type === "virtual") {
        const src = this.primaryVirtualMonitorSourceDevice(n);
        if (src) {
          const id = this.resolveMonitorDeviceId(src);
          if (id !== null && id !== undefined && id !== "") return String(id);
        }
        return this.$t("monitor.virtualNodeBadge");
      }
      const id = this.resolveMonitorDeviceId(n);
      return id !== null && id !== undefined && id !== "" ? String(id) : "--";
    },
    monitorFlatTemplateIdText() {
      const n = this.currentNode || {};
      if (n.type === "virtual") {
        const src = this.primaryVirtualMonitorSourceDevice(n);
        if (src) {
          const tid =
            src.templateId !== undefined && src.templateId !== null && src.templateId !== ""
              ? src.templateId
              : src.template_id;
          if (tid !== null && tid !== undefined && tid !== "") return String(tid);
        }
        return "--";
      }
      const tid =
        n.templateId !== undefined && n.templateId !== null && n.templateId !== ""
          ? n.templateId
          : n.template_id;
      return tid === null || tid === undefined || tid === "" ? "--" : String(tid);
    },
    /** 非本机整表：不过滤分组/资源类，展示模板下全部测点（仅受搜索与「仅显示有值」影响） */
    filteredPointsFlatTable() {
      void this.locale;
      const keyword = String(this.pointKeyword || "").trim().toLowerCase();
      const out = [];
      (this.currentPoints || []).forEach((p) => {
        const name = String(p.name || p.ename || "").toLowerCase();
        const hasValue = !(p.value === undefined || p.value === null || String(p.value).trim() === "");
        if (this.onlyShowValued && !hasValue) return;
        if (keyword && !name.includes(keyword)) return;
        out.push(p);
      });
      return out;
    },
    /** 非本机统计页：按当前筛选结果生成分组条（数量与搜索/有值筛选一致） */
    flatGroupSummary() {
      const list = this.filteredPointsFlatTable || [];
      const counts = {};
      list.forEach((p) => {
        const raw = p.groupType;
        const k =
          raw === null || raw === undefined || raw === "" || Number.isNaN(Number(raw))
            ? "x"
            : String(Number(raw));
        counts[k] = (counts[k] || 0) + 1;
      });
      const order = ["1", "0", "2", "3", "4", "5", "6", "7", "x"];
      return order
        .filter((k) => counts[k])
        .map((k) => ({
          key: k,
          count: counts[k],
          title: k === "x" ? this.$t("monitor.remoteGroupOther") : this.monitorGroupTypeTitle(Number(k)),
        }));
    },
    /** 非本机统计页：在搜索/有值筛选后再按分组筛选 */
    filteredPointsFlatTableDisplay() {
      const list = this.filteredPointsFlatTable;
      const f = this.flatTableGroupFilter;
      if (f === "" || f === null || f === undefined) return list;
      if (f === "x") {
        return list.filter((p) => {
          const raw = p.groupType;
          return (
            raw === null ||
            raw === undefined ||
            raw === "" ||
            Number.isNaN(Number(raw))
          );
        });
      }
      const gn = Number(f);
      return list.filter((p) => Number(p.groupType) === gn);
    },
    /** 顶栏已展示的测点（含堆控在线后的次级 KPI），下方分区不再重复 */
    bbmsHeroExcludedKeys() {
      void this.locale;
      const keys = new Set();
      (this.bbmsOverviewDeck || []).forEach((h) => {
        if (h && h.point) keys.add(bbmsPointDedupeKey(h.point));
      });
      return keys;
    },
    /** 堆控：按协议枚举语义分区后的面板列表 */
    bbmsConsoleSections() {
      void this.locale;
      if (!this.isStackBmsMonitor) return [];
      const list = this.filteredPointsFlatTable || [];
      const excluded = this.bbmsHeroExcludedKeys;
      const bucket = {};
      list.forEach((p) => {
        if (excluded.has(bbmsPointDedupeKey(p))) return;
        const id = resolveBbmsSectionId(p);
        if (!bucket[id]) bucket[id] = [];
        bucket[id].push(p);
      });
      // 合并「Rack使能与在线」+「绝缘监测」
      const mergedRackIns = [...(bucket.rack_summary || []), ...(bucket.insulation || [])];
      if (mergedRackIns.length) {
        bucket.rack_enable_insulation = mergedRackIns;
      }
      delete bucket.rack_summary;
      delete bucket.insulation;
      return BBMS_SECTION_ORDER.filter((id) => bucket[id] && bucket[id].length).map((id) => {
        const meta = BBMS_SECTION_META[id] || BBMS_SECTION_META.misc;
        let rows = bucket[id];
        if (meta.layout === "series") rows = sortBbmsSeriesRows(rows);
        return {
          id,
          titleKey: meta.titleKey,
          hintKey: meta.hintKey || "",
          layout: meta.layout,
          rows,
        };
      });
    },
    /** 详情 Tab 用分区（顶部统计区已占用的分区不再重复） */
    bbmsDetailSections() {
      const statSet = new Set(BBMS_STAT_SECTION_IDS || []);
      return (this.bbmsConsoleSections || []).filter((s) => s && !statSet.has(s.id));
    },
    bbmsStatSectionMap() {
      const map = {};
      (this.bbmsConsoleSections || []).forEach((s) => {
        if (s && s.id) map[s.id] = s;
      });
      return map;
    },
    /** 统计区分块：能量 / 功率 / 状态 */
    bbmsStatBlocks() {
      void this.locale;
      const map = this.bbmsStatSectionMap;
      const excluded = this.bbmsHeroExcludedKeys;
      return (BBMS_STAT_GROUPS || [])
        .map((g) => {
          const rows = [];
          (g.sectionIds || []).forEach((sid) => {
            const sec = map[sid];
            if (sec && sec.rows) rows.push(...sec.rows);
          });
          if (g.id === "status") {
            const chipRows = rows.filter((p) => !excluded.has(bbmsPointDedupeKey(p)));
            return {
              id: g.id,
              titleKey: g.titleKey,
              tone: g.tone,
              pointCount: chipRows.length,
              chipsOnly: true,
              chipRows,
              displaySplit: {
                gauges: [],
                stats: [],
                chips: chipRows,
                tableRows: [],
                _hasRichBlocks: chipRows.length > 0,
              },
            };
          }
          const displaySplit = this.splitPointsForRichDisplay(rows.slice(0, 28));
          return {
            id: g.id,
            titleKey: g.titleKey,
            tone: g.tone,
            pointCount: rows.length,
            chipsOnly: false,
            chipRows: [],
            displaySplit,
          };
        })
        .filter((b) => {
          if (b.id === "status") return b.chipRows && b.chipRows.length > 0;
          return (
            b.pointCount > 0 &&
            (b.displaySplit._hasRichBlocks || b.displaySplit.gauges.length > 0)
          );
        });
    },
    bbmsAlarmPoints() {
      void this.locale;
      const list = this.filteredPointsFlatTable || [];
      return list.filter((p) => {
        if (Number(p.groupType) === 6) return true;
        const t = bbmsPointMatchText(p);
        return /故障|告警|Alarm|Fault|Trip|越限/i.test(t);
      });
    },
    /** BBMS 故障 Tab：故障相关分区全部测点 */
    bbmsFaultSections() {
      void this.locale;
      if (this.activeView !== "bbms") return [];
      const tab = (BBMS_MAIN_TABS || []).find((t) => t.id === "fault");
      if (!tab || !tab.sectionIds) return [];
      const idSet = new Set(tab.sectionIds);
      return (this.bbmsDetailSections || []).filter((s) => idSet.has(s.id));
    },
    /** RBMS 故障 Tab：故障阵列等分区 */
    rbmsFaultSections() {
      void this.locale;
      if (this.activeView !== "rbms") return [];
      const tab = (RBMS_MAIN_TABS || []).find((t) => t.id === "fault");
      if (!tab || !tab.sectionIds) return [];
      const idSet = new Set(tab.sectionIds);
      return (this.rbmsConsoleSections || []).filter((s) => idSet.has(s.id));
    },
    rbmsFaultPointsFlat() {
      const seen = new Set();
      const out = [];
      (this.rbmsFaultSections || []).forEach((sec) => {
        (sec.rows || []).forEach((p) => {
          const k = rbmsPointDedupeKey(p);
          if (!seen.has(k)) {
            seen.add(k);
            out.push(p);
          }
        });
      });
      return out;
    },
    /** BBMS 故障面板：汇总后按语义/序号分桶（约 482 点拆成多组） */
    bbmsFaultPanelSections() {
      void this.locale;
      const seen = new Set();
      const all = [];
      const push = (p) => {
        const k = bbmsPointDedupeKey(p);
        if (seen.has(k)) return;
        seen.add(k);
        all.push(p);
      };
      (this.bbmsFaultSections || []).forEach((sec) => (sec.rows || []).forEach(push));
      (this.bbmsAlarmPoints || []).forEach(push);
      return buildBbmsFaultPanelSections(all);
    },
    bbmsVisibleMainTabs() {
      void this.locale;
      return (BBMS_MAIN_TABS || []).filter((tab) => {
        if (tab.flat) return true;
        if (tab.faultDedicated) {
          return (this.bbmsFaultPanelSections || []).some((s) => s.rows && s.rows.length);
        }
        const ids = new Set(tab.sectionIds || []);
        return (this.bbmsDetailSections || []).some((s) => ids.has(s.id));
      });
    },
    /** RBMS 二级页：与 filteredPointsFlatTable 同源（搜索/有值筛选） */
    rbmsFilteredPoints() {
      return this.filteredPointsFlatTable || [];
    },
    rbmsParentStackLabel() {
      const node = this.currentNode;
      if (!node || !node.id) return "";
      const path = this.findNodePathById(this.deviceTree, node.id);
      if (!Array.isArray(path) || path.length < 2) return "";
      for (let i = path.length - 2; i >= 0; i--) {
        if (this.monitorDeviceTypeNum(path[i].deviceType) === 1) {
          return String(path[i].label || "");
        }
      }
      return "";
    },
    rbmsHeroExcludedKeys() {
      const keys = new Set();
      (this.rbmsHeroStrip || []).forEach((h) => {
        if (h && h.point) keys.add(rbmsPointDedupeKey(h.point));
      });
      return keys;
    },
    rbmsConsoleSections() {
      void this.locale;
      if (this.activeView !== "rbms") return [];
      const list = this.filteredPointsFlatTable || [];
      const excluded = this.rbmsHeroExcludedKeys;
      const bucket = {};
      list.forEach((p) => {
        if (excluded.has(rbmsPointDedupeKey(p))) return;
        const id = resolveRbmsSectionId(p);
        if (!bucket[id]) bucket[id] = [];
        bucket[id].push(p);
      });
      return RBMS_SECTION_ORDER.filter((id) => bucket[id] && bucket[id].length).map((id) => {
        const meta = RBMS_SECTION_META[id] || RBMS_SECTION_META.misc;
        const rows = bucket[id].slice().sort((a, b) => Number(a.code) - Number(b.code));
        return {
          id,
          titleKey: meta.titleKey,
          hintKey: meta.hintKey || "",
          layout: meta.layout,
          rows,
        };
      });
    },
    rbmsOverviewSections() {
      const ids = new Set(["summary"]);
      return (this.rbmsConsoleSections || []).filter((s) => ids.has(s.id));
    },
    rbmsVisibleMainTabs() {
      void this.locale;
      return (RBMS_MAIN_TABS || []).filter((tab) => {
        if (tab.flat) return true;
        if (tab.cellGrid) return (this.rbmsCellGrid || []).length > 0 || this.rbmsCellInfoHasAuxSections;
        if (tab.faultDedicated) {
          return (this.rbmsFaultSections || []).some((s) => s.rows && s.rows.length);
        }
        const ids = new Set(tab.sectionIds || []);
        return (this.rbmsConsoleSections || []).some((s) => ids.has(s.id));
      });
    },
    rbmsCellInfoHasAuxSections() {
      const aux = new Set([
        "cell_volt_valid",
        "afe_volt",
        "temp_pole",
        "temp_pack",
        "temp_bal_valid",
        "temp_bal_board",
      ]);
      return (this.rbmsConsoleSections || []).some((s) => aux.has(s.id));
    },
    rbmsCellGrid() {
      void this.locale;
      if (this.activeView !== "rbms") return [];
      return buildRbmsCellGrid(this.filteredPointsFlatTable || []);
    },
    rbmsCellClusterStats() {
      void this.locale;
      if (this.activeView !== "rbms") return [];
      const byCode = {};
      (this.filteredPointsFlatTable || []).forEach((p) => {
        const c = Number(p.code);
        if (!Number.isNaN(c)) byCode[c] = p;
      });
      const slots = [
        { key: "vmax", code: 39, labelKey: "monitor.rbmsCellStatVmax" },
        { key: "vmin", code: 41, labelKey: "monitor.rbmsCellStatVmin" },
        { key: "vavg", code: 43, labelKey: "monitor.rbmsCellStatVavg" },
        { key: "tmax", code: 44, labelKey: "monitor.rbmsCellStatTmax" },
        { key: "tmin", code: 46, labelKey: "monitor.rbmsCellStatTmin" },
        { key: "tavg", code: 48, labelKey: "monitor.rbmsCellStatTavg" },
      ];
      return slots
        .map((s) => {
          const p = byCode[s.code];
          if (!p) return null;
          const u = p.unit != null && p.unit !== "" ? ` ${p.unit}` : "";
          const raw =
            p.value === undefined || p.value === null || String(p.value).trim() === ""
              ? "--"
              : `${p.value}${u}`;
          const label = this.$t(s.labelKey);
          return { key: s.key, label, valueText: raw };
        })
        .filter(Boolean);
    },
    rbmsHeroStrip() {
      void this.locale;
      const list = this.filteredPointsFlatTable || [];
      const byCode = {};
      list.forEach((p) => {
        const c = Number(p.code);
        if (!Number.isNaN(c)) byCode[c] = p;
      });
      const out = [];
      for (let i = 0; i < RBMS_HERO_SLOTS.length; i++) {
        const slot = RBMS_HERO_SLOTS[i];
        const hit = byCode[slot.code];
        if (!hit) continue;
        out.push(this.buildRbmsHeroCard(slot, hit));
      }
      return out;
    },
    /** 堆控顶栏：抓取 SOC/SOH/母线/功率等核心量 */
    bbmsHeroStrip() {
      void this.locale;
      const list = this.filteredPointsFlatTable || [];
      const out = [];
      for (let i = 0; i < BBMS_HERO_RULES.length; i++) {
        const r = BBMS_HERO_RULES[i];
        const hit = list.find((p) => r.re.test(bbmsPointMatchText(p)));
        if (!hit) continue;
        out.push(this.buildBbmsHeroCard(r, hit));
      }
      return out;
    },
    /** 堆控在线后的次级 KPI：心跳 / 系统运行状态 / 最小 Rack 数 */
    bbmsHeroSecondaryStrip() {
      void this.locale;
      const list = this.filteredPointsFlatTable || [];
      const out = [];
      for (let i = 0; i < BBMS_HERO_SECONDARY_RULES.length; i++) {
        const r = BBMS_HERO_SECONDARY_RULES[i];
        const hit = list.find((p) => r.re.test(bbmsPointMatchText(p)));
        if (!hit) continue;
        out.push(this.buildBbmsHeroCard(r, hit));
      }
      return out;
    },
    /** 系统运行概览顶栏：主 KPI + 堆控在线后插入次级 KPI */
    bbmsOverviewDeck() {
      const primary = this.bbmsHeroStrip || [];
      const secondary = this.bbmsHeroSecondaryStrip || [];
      const out = [];
      let inserted = false;
      primary.forEach((h) => {
        out.push(h);
        if (h.key === "online" && !inserted) {
          secondary.forEach((s) => out.push(s));
          inserted = true;
        }
      });
      if (!inserted && secondary.length) out.push(...secondary);
      return out;
    },
    /** 非本机统计页顶栏：与下方筛选后的点表数量对齐 */
    remoteFlatHeroMetrics() {
      void this.locale;
      const list = this.filteredPointsFlatTable || [];
      const valued = list.filter(
        (p) => !(p.value === undefined || p.value === null || String(p.value).trim() === "")
      ).length;
      const groupSet = new Set();
      list.forEach((p) => {
        const g = Number(p.groupType);
        if (!Number.isNaN(g)) groupSet.add(g);
      });
      return [
        { key: "total", label: this.$t("monitor.dashboardMetricTotal"), value: list.length },
        { key: "valued", label: this.$t("monitor.dashboardMetricValued"), value: valued },
        { key: "groups", label: this.$t("monitor.dashboardMetricGroups"), value: groupSet.size },
      ];
    },
    dashboardHeroMetrics() {
      const list = this.currentPoints || [];
      const valued = list.filter(
        (p) => !(p.value === undefined || p.value === null || String(p.value).trim() === "")
      ).length;
      const groupSet = new Set();
      list.forEach((p) => {
        const g = Number(p.groupType);
        if (!Number.isNaN(g)) groupSet.add(g);
      });
      return [
        { key: "total", label: this.$t("monitor.dashboardMetricTotal"), value: list.length },
        { key: "valued", label: this.$t("monitor.dashboardMetricValued"), value: valued },
        { key: "groups", label: this.$t("monitor.dashboardMetricGroups"), value: groupSet.size },
      ];
    },
    dashboardSideTiles() {
      const list = this.currentPoints || [];
      const alarmCt = list.filter((p) => Number(p.groupType) === 6).length;
      const eventCt = list.filter((p) => Number(p.groupType) === 7).length;
      const n = this.currentNode || {};
      const tid = n.templateId !== undefined && n.templateId !== null ? n.templateId : n.template_id;
      const tidText =
        tid === null || tid === undefined || tid === "" ? "--" : String(tid);
      const devRaw = this.resolveMonitorDeviceId(n);
      const devText = devRaw !== undefined && devRaw !== null && devRaw !== "" ? String(devRaw) : "--";
      const passValued = (p) => {
        if (!this.onlyShowValued) return true;
        return !(p.value === undefined || p.value === null || String(p.value).trim() === "");
      };
      const pickTileValue = (def) => {
        const hit = list.find(
          (p) => passValued(p) && def.m(`${p.name || ""}${p.ename || ""}`)
        );
        if (!hit || hit.value === undefined || hit.value === null || String(hit.value).trim() === "") {
          return "--";
        }
        const u = hit.unit != null && hit.unit !== "" ? ` ${hit.unit}` : "";
        return `${hit.value}${u}`;
      };
      const base = [
        { key: "alarm", label: this.$t("monitor.dashboardTileAlarm"), value: alarmCt, tone: "tone-warn" },
        { key: "event", label: this.$t("monitor.dashboardTileEvent"), value: eventCt, tone: "tone-info" },
        { key: "tpl", label: this.$t("monitor.dashboardTileTemplate"), value: tidText, tone: "tone-accent-violet" },
        { key: "dev", label: this.$t("monitor.dashboardTileDeviceId"), value: devText, tone: "tone-accent-slate" },
      ];
      if (!this.isLocalSystemMonitor) return base;
      const hbDef = HOST_LOCAL_SIDE_TILE_DEFS.find((d) => d.key === "heartbeat");
      const loDef = HOST_LOCAL_SIDE_TILE_DEFS.find((d) => d.key === "localOnline");
      const tcDef = HOST_LOCAL_SIDE_TILE_DEFS.find((d) => d.key === "testCount");
      const upDef = HOST_LOCAL_SIDE_TILE_DEFS.find((d) => d.key === "upgradeProgress");
      const y50Def = HOST_LOCAL_SIDE_TILE_DEFS.find((d) => d.key === "telem50");
      return [
        {
          key: "heartbeat",
          label: this.$t("monitor.dashboardTileHeartbeat"),
          value: pickTileValue(hbDef),
          tone: "tone-accent-teal",
        },
        {
          key: "localOnline",
          label: this.$t("monitor.anchorLocalOnline"),
          value: pickTileValue(loDef),
          tone: "tone-accent-online",
        },
        {
          key: "upgradeProgress",
          label: this.$t("monitor.dashboardTileUpgradeProgress"),
          value: pickTileValue(upDef),
          tone: "tone-accent-mint",
        },
        {
          key: "telem50",
          label: this.$t("monitor.dashboardTileTelem50"),
          value: pickTileValue(y50Def),
          tone: "tone-accent-sky",
        },
        base[0],
        base[1],
        {
          key: "testCount",
          label: this.$t("monitor.dashboardTileTestCount"),
          value: pickTileValue(tcDef),
          tone: "tone-accent-amber",
        },
        base[2],
        base[3],
      ];
    },
    groupedPointCards() {
      void this.locale;
      const keyword = String(this.pointKeyword || "").trim().toLowerCase();
      const filtered = [];
      (this.currentPoints || []).forEach((p) => {
        const name = String(this.pointDisplayName(p) || "").toLowerCase();
        const hasValue = !(p.value === undefined || p.value === null || String(p.value).trim() === "");
        if (this.onlyShowValued && !hasValue) return;
        if (keyword && !name.includes(keyword)) return;
        filtered.push(p);
      });
      if (!filtered.length) return [];

      const sys = [];
      const rt = [];
      const dido = [];
      const di = [];
      const doOnly = [];
      const res = [];
      const byG = { 2: [], 3: [], 4: [], 5: [], 6: [], 7: [] };
      const titleByG = {
        2: "monitor.groupRemote",
        3: "monitor.groupParam",
        4: "monitor.groupConfig",
        5: "monitor.groupMetering",
        6: "monitor.groupAlarm",
        7: "monitor.groupEvent",
      };
      const themeByG = {
        2: "theme-purple",
        3: "theme-orange",
        4: "theme-indigo",
        5: "theme-cyan",
        6: "theme-red",
        7: "theme-pink",
      };

      filtered.forEach((p) => {
        if (this.isLocalSystemMonitor && this.pointIsExcludedFromLocalPointCards(p)) {
          return;
        }
        if (this.pointIsResourcePerf(p)) {
          res.push(p);
          return;
        }
        const g = Number(p.groupType);
        if (this.isLocalSystemMonitor && g >= 2 && g <= 7) {
          sys.push(p);
          return;
        }
        if (this.pointIsDido(p)) {
          if (this.isLocalSystemMonitor) {
            if (this.pointIsDoChannel(p)) doOnly.push(p);
            else di.push(p);
          } else {
            dido.push(p);
          }
          return;
        }
        if (g === 0) {
          sys.push(p);
        } else if (g === 1) {
          rt.push(p);
        } else if (g >= 2 && g <= 7) {
          byG[g].push(p);
        } else {
          sys.push(p);
        }
      });

      const out = [];
      const pushSplit = (key, titleKey, pts, splitGtype, theme) => {
        const arr = pts || [];
        if (!arr.length) return;
        out.push({
          key,
          title: this.$t(titleKey),
          themeClass: theme,
          layoutClass: "monitor-point-card-key",
          renderMode: "split",
          sections: this.buildSplitSections(arr, splitGtype),
          displaySplit: null,
          points: arr,
          /* 与 rich 卡（如数字量输入）内表格高度一致 */
          tableHeight: 320,
        });
      };
      const pushRich = (key, titleKey, pts, theme) => {
        const arr = pts || [];
        if (!arr.length) return;
        out.push({
          key,
          title: this.$t(titleKey),
          themeClass: theme,
          layoutClass: "monitor-point-card-key",
          renderMode: "rich",
          sections: [],
          displaySplit: this.splitPointsForRichDisplay(arr),
          points: arr,
          tableHeight: 320,
        });
      };

      if (this.isLocalSystemMonitor) {
        const sysStatus = [];
        const sysOther = [];
        sys.forEach((p) => {
          if (this.pointIsSysStatusQuantity(p)) sysStatus.push(p);
          else sysOther.push(p);
        });
        pushRich("card_sys_other", "monitor.cardPointSysOther", sysOther, "theme-violet");
        pushRich("card_res", "monitor.cardPointResource", res, "theme-rose");
        pushRich("card_sys_status", "monitor.cardPointSysStatus", sysStatus, "theme-emerald");
        pushRich("card_rt", "monitor.cardPointRealtime", rt, "theme-teal");
        pushRich("card_di", "monitor.cardPointDi", di, "theme-indigo");
        pushRich("card_do", "monitor.cardPointDo", doOnly, "theme-amber");
      } else {
        pushSplit("card_sys", "monitor.cardPointSystem", sys, 0, "theme-blue");
        pushSplit("card_rt", "monitor.cardPointRealtime", rt, 1, "theme-teal");
        pushRich("card_res", "monitor.cardPointResource", res, "theme-cyan");
        pushRich("card_dido", "monitor.cardPointDido", dido, "theme-indigo");
        [2, 3, 4, 5, 6, 7].forEach((g) => {
          const arr = byG[g];
          if (!arr || !arr.length) return;
          pushRich(`card_g${g}`, titleByG[g], arr, themeByG[g]);
        });
      }
      return out;
    },
    /** 本机仪表盘：下方测点卡占位补满 3/6/9 格（仅本机多卡时） */
    displayGroupedPointCards() {
      const raw = [...(this.groupedPointCards || [])];
      if (!this.isLocalSystemMonitor || !raw.length) return raw;
      if (!this.monitorPointMultiGrid) return raw;
      const rem = raw.length % 3;
      const pad = rem === 0 ? 0 : 3 - rem;
      for (let i = 0; i < pad; i++) {
        raw.push({ key: `ph-${raw.length}-${i}`, isPlaceholder: true });
      }
      return raw;
    },
    /** 本机运行大卡：本机在线、本机/BMS 堆控、簇控制器、PCS（来自点表；心跳与测试次数在右侧 tile） */
    localHostAnchorKpis() {
      if (!this.isLocalSystemMonitor) return [];
      const list = this.currentPoints || [];
      const passValued = (p) => {
        if (!this.onlyShowValued) return true;
        return !(p.value === undefined || p.value === null || String(p.value).trim() === "");
      };
      const labelKey = {
        local: "monitor.anchorLocalCount",
        stack: "monitor.anchorStackBms",
        clusterCtrl: "monitor.anchorClusterCtrlBms",
        pcs: "monitor.anchorPcsCount",
      };
      return HOST_ANCHOR_SLOT_DEFS.map((slot) => {
        const hit = list.find((p) => passValued(p) && slot.m(`${p.name || ""}${p.ename || ""}`));
        const unit = hit && hit.unit != null && hit.unit !== "" ? String(hit.unit) : "";
        const valueText =
          hit && hit.value !== undefined && hit.value !== null && String(hit.value).trim() !== ""
            ? String(hit.value)
            : "--";
        return {
          key: slot.key,
          label: hit ? this.pointDisplayName(hit) : this.$t(labelKey[slot.key]),
          valueText,
          unit,
        };
      });
    },
    monitorPointMultiGrid() {
      return (this.groupedPointCards || []).length >= 2;
    },
  },
  watch: {
    currentNode(n, o) {
      if (!n || n.type !== "device") return;
      const a = o ? this.resolveMonitorDeviceId(o) : null;
      const b = this.resolveMonitorDeviceId(n);
      if (a !== b) {
        this.bbmsMainTab = "runtime";
        this.rbmsMainTab = "overview";
      }
    },
    bbmsVisibleMainTabs: {
      handler(tabs) {
        const names = (tabs || []).map((t) => t.id);
        if (!names.length) return;
        if (!names.includes(this.bbmsMainTab)) this.bbmsMainTab = names[0];
      },
      immediate: true,
    },
    rbmsVisibleMainTabs: {
      handler(tabs) {
        const names = (tabs || []).map((t) => t.id);
        if (!names.length) return;
        if (!names.includes(this.rbmsMainTab)) this.rbmsMainTab = names[0];
      },
      immediate: true,
    },
    locale() {
      this.loadTopologyFromStorage();
    },
    groupedPointCards: {
      handler(list) {
        const next = { ...this.splitTabByGroupKey };
        (list || []).forEach(g => {
          if (g.renderMode !== "split" || !g.sections || !g.sections.length) return;
          const gk = String(g.key);
          const names = g.sections.map(s => String(s.key));
          if (!next[gk] || !names.includes(String(next[gk]))) {
            next[gk] = String(g.sections[0].key);
          }
        });
        Object.keys(next).forEach(gk => {
          const alive = (list || []).some(g => String(g.key) === gk && g.renderMode === "split");
          if (!alive) delete next[gk];
        });
        this.splitTabByGroupKey = next;
      },
      immediate: true,
    },
  },
  created() {
    this.resetMonitorData();
    // 首次进入（非 keep-alive 首屏）也要加载拓扑，否则右侧会是空表。
    this.loadTopologyFromStorage();
  },
  async activated() {
    // keep-alive 回到页面时按需刷新，避免与 created 重复抢状态。
    if (!Array.isArray(this.deviceTree) || !this.deviceTree.length) {
      await this.loadTopologyFromStorage();
    }
    this.$nextTick(() => {
      if (this.shouldPollDevicePoints()) this.startMonitorPointsPoll();
    });
  },
  deactivated() {
    this.stopMonitorPointsPoll();
  },
  mounted() {
    this.$nextTick(() => this.initCharts());
  },
  beforeDestroy() {
    this.stopMonitorPointsPoll();
    this.disposeCellDetailChart();
    [this.rbmsChart, this.pbmsCellChart, this.pbmsHeatChart, this.pbmsAlertChart].forEach(c => c && c.dispose());
  },
  methods: {
    resetMonitorData() {
      this.deviceTree = [];
      this.currentNode = null;
      this.monitorTreeHighlightId = null;
      this.currentNodeLabel = "Local system";
      this.activeView = "pointView";
      this.rbmsDataList = [];
      this.pbmsDataByRbms = [];
      this.pbmsAlertsByRbms = [];
      this.upsData = {
        inputVoltage: "--",
        outputVoltage: "--",
        frequency: "--",
        loadPercent: 0,
        backupMinutes: "--",
        workMode: "--",
      };
      this.meterData = {
        voltageA: "--",
        voltageB: "--",
        voltageC: "--",
        currentA: "--",
        currentB: "--",
        currentC: "--",
        activePower: "--",
        reactivePower: "--",
        powerFactor: "--",
        totalChargeKwh: "--",
        totalDischargeKwh: "--",
        reverseFlow: false,
        peakKwh: "--",
        flatKwh: "--",
        valleyKwh: "--",
      };
      this.systemInfo = {
        soc: "--",
        soh: "--",
        totalVoltage: "--",
        totalCurrent: "--",
        totalPower: "--",
        runStatus: "--",
        faultStatus: "--",
        maxChargeCurrent: "--",
        maxDischargeCurrent: "--",
        maxDischargePower: "--",
        maxChargePower: "--",
      };
      this.sbmsConsistency = { socDiff: "--", voltageDiff: "--" };
      this.rbmsStatusMatrix = [];
      this.baseInfo = {
        totalClusters: 0,
        onlineClusters: 0,
        totalBatteries: 0,
        onlineBatteries: 0,
        totalTempSensors: 0,
        onlineTempSensors: 0,
        totalSlaves: 0,
        onlineSlaves: 0,
        ratedCapacity: "--",
        ratedEnergy: "--",
        remainingCapacity: "--",
        remainingEnergy: "--",
        softwareVersion: "--",
        systemTime: "--",
      };
      this.dioInfo = { doStatus: [], diStatus: [], otherSignals: [] };
      this.clusterStats = [];
      this.chargeStats = [];
      this.alarmInfo = { battery: [], system: [], external: [] };
      this.currentPoints = [];
      this.splitTabByGroupKey = {};
      this.flatTableGroupFilter = "";
      this.stopMonitorPointsPoll();
    },
    fmtPoint(points, ename, unit = "") {
      const p = (points || []).find((it) => it.ename === ename);
      if (!p || p.value === undefined || p.value === null || p.value === "") return "--";
      return unit ? `${p.value} ${unit}` : `${p.value}`;
    },
    /** 中文界面用点表 name；英文界面用点表 ename，未配置 ename 时回退 name */
    pointDisplayName(row) {
      const r = row || {};
      const loc = String(this.$i18n && this.$i18n.locale ? this.$i18n.locale : "").toLowerCase();
      const isEn = loc === "en" || loc.startsWith("en");
      const name = String(r.name || "").trim();
      const ename = String(r.ename || "").trim();
      if (isEn) return ename || name || "--";
      return name || "--";
    },
    /**
     * 拉测点仅用台账「设备主键 id」（拓扑里的 devId），禁止使用 node.id：
     * node.id 是拓扑表主键，与设备 id 数字可能巧合相等，会误请求本机(device.id=1)；
     * 与「设备编号 code」是否为 1 无关。
     */
    resolveMonitorDeviceId(node) {
      if (!node) return null;
      const raw =
        node.devId !== undefined && node.devId !== null && node.devId !== ""
          ? node.devId
          : node.deviceId !== undefined && node.deviceId !== null && node.deviceId !== ""
            ? node.deviceId
            : null;
      if (raw === "" || raw === null || raw === undefined) return null;
      const s = String(raw).trim();
      if (!s || /^none$/i.test(s) || s === "undefined") return null;
      const num = Number(s);
      return Number.isNaN(num) ? raw : num;
    },
    /** 拓扑 deviceType：禁止把 null 当成 0（Number(null)===0 会误判为本机） */
    monitorDeviceTypeNum(raw) {
      if (raw === null || raw === undefined || raw === "") return null;
      const n = Number(raw);
      return Number.isNaN(n) ? null : n;
    },
    /** 从虚拟节点名称推断设备类型（未保存 virtual_dev_type 时兼容旧数据） */
    inferVirtualDevTypeFromLabel(node) {
      const label = String((node && (node.label || node.name)) || "").toLowerCase();
      if (/bbms|堆控|stack\s*bms/.test(label)) return 1;
      if (/rbms|簇控|cluster/.test(label)) return 2;
      if (/pcs|变流/.test(label)) return 3;
      if (/除湿|dehumid/.test(label)) return 8;
      if (/ups/i.test(label)) return 9;
      if (/电表|meter|ammeter/.test(label)) return 10;
      if (/空调|air\s*cond/.test(label)) return 5;
      if (/本机|local\s*system/.test(label)) return 0;
      return null;
    },
    deviceHasMonitorTemplate(d) {
      if (!d) return false;
      const tid =
        d.templateId !== undefined && d.templateId !== null && d.templateId !== ""
          ? d.templateId
          : d.template_id;
      return tid !== null && tid !== undefined && tid !== "" && Number(tid) !== 0;
    },
    /** 虚拟节点测点来源：堆控优先同级真实 BBMS；子树设备须与 virtualDevType 一致 */
    collectVirtualMonitorPointDevices(virtualNode) {
      if (!virtualNode || virtualNode.type !== "virtual") return [];
      const vdt = this.effectiveMonitorDeviceType(virtualNode);

      const trail = this.findNodePathById(this.deviceTree, virtualNode.id);
      const parent = trail && trail.length >= 2 ? trail[trail.length - 2] : null;
      const scope = parent ? parent.children || [] : this.deviceTree || [];
      const peerDevicesOfType = (devType) => {
        const peers = [];
        (scope || []).forEach((n) => {
          if (!n || String(n.id) === String(virtualNode.id)) return;
          if (n.type === "device" && this.monitorDeviceTypeNum(n.deviceType) === devType) {
            peers.push(n);
          }
        });
        return peers.filter((d) => this.deviceHasMonitorTemplate(d));
      };

      // 虚拟堆控：优先用同级真实 BBMS（template 15），勿误用其下挂的 RBMS 点表
      if (vdt === 1) {
        const bbmsPeers = peerDevicesOfType(1);
        if (bbmsPeers.length) return bbmsPeers.slice(0, 1);
      }

      const under = [];
      this.collectDescendantMonitorDevices(virtualNode, under);
      const fromChildren = under.filter((d) => {
        if (!this.deviceHasMonitorTemplate(d)) return false;
        if (vdt !== null && vdt !== undefined) {
          return this.monitorDeviceTypeNum(d.deviceType) === vdt;
        }
        return true;
      });
      if (fromChildren.length) return fromChildren;

      if (vdt === null || vdt === undefined) return [];
      return peerDevicesOfType(vdt);
    },
    primaryVirtualMonitorSourceDevice(virtualNode) {
      const list = this.collectVirtualMonitorPointDevices(virtualNode);
      return list.length ? list[0] : null;
    },
    /** 监控页布局用设备类型：设备节点取台账类型，虚拟节点取 virtualDevType */
    effectiveMonitorDeviceType(node) {
      const n = node || this.currentNode;
      if (!n) return null;
      if (n.type === "virtual") {
        const v =
          n.virtualDevType !== undefined && n.virtualDevType !== null
            ? n.virtualDevType
            : n.deviceType;
        let num = this.monitorDeviceTypeNum(v);
        if (num === null) num = this.inferVirtualDevTypeFromLabel(n);
        return num;
      }
      if (n.type === "device") return this.monitorDeviceTypeNum(n.deviceType);
      return null;
    },
    monitorGroupTypeTitle(g) {
      const meta = pointGroupMeta(g);
      if (meta && meta.labelKey) {
        const t = this.$t(meta.labelKey);
        if (t !== meta.labelKey) return t;
      }
      const n = Number(g);
      const legacy = {
        0: "cardPointSystem",
        1: "cardPointRealtime",
        2: "groupRemote",
        3: "groupParam",
        4: "groupConfig",
        5: "groupMetering",
        6: "groupAlarm",
        7: "groupEvent",
      };
      const key = legacy[n];
      if (key) return this.$t(`monitor.${key}`);
      if (Number.isNaN(n)) return "--";
      return `${this.$t("monitor.monitorGroupTypeCol")} ${g}`;
    },
    remoteTableRowClass({ row }) {
      if (row && Number(row.groupType) === 6) return "remote-row-alarm";
      return "";
    },
    remoteValueClass(row) {
      if (row && Number(row.groupType) === 6) return "bbms-cell-alarm";
      return "";
    },
    buildBbmsLayoutCells(sections) {
      const list = Array.isArray(sections) ? sections : [];
      if (!list.length) return [];
      const rowCount = (sec) => ((sec && Array.isArray(sec.rows) && sec.rows.length) || 0);
      const forcedPairIds = new Set([
        "fault_list",
        "acore_fault",
        "soc_arrays",
        "soh_arrays",
        "bank_cali",
        "fault_enable",
      ]);
      const desiredSpan = (sec) => {
        if (!sec) return 12;
        if (forcedPairIds.has(sec.id)) return 6;
        if (sec.layout === "series") return 12;
        const n = rowCount(sec);
        if (sec.layout === "rail") {
          if (n <= 5) return 5;
          if (n <= 10) return 6;
          return 7;
        }
        if (n <= 6) return 5;
        if (n <= 12) return 6;
        return 7;
      };
      const adjustKvRailWidths = (row) => {
        if (row.length !== 2) return false;
        let kvCell = null;
        let railCell = null;
        for (let j = 0; j < row.length; j++) {
          const c = row[j];
          if (c.sec.layout === "rail") railCell = c;
          else if (c.sec.layout !== "series") kvCell = c;
        }
        if (!kvCell || !railCell) return false;
        const nR = rowCount(railCell.sec);
        const nK = rowCount(kvCell.sec);
        const kvFirst = row[0].sec === kvCell.sec;
        let kvSpan = 6;
        let railSpan = 6;
        if (nK >= Math.max(12, nR * 2)) {
          kvSpan = 7;
          railSpan = 5;
        } else if (nR >= Math.max(12, nK * 2)) {
          kvSpan = 5;
          railSpan = 7;
        } else if (nR <= 5 && nK <= 8) {
          kvSpan = 7;
          railSpan = 5;
        }
        if (kvFirst) {
          kvCell.span = kvSpan;
          railCell.span = railSpan;
        } else {
          railCell.span = railSpan;
          kvCell.span = kvSpan;
        }
        return true;
      };
      const adjustTwoCellByDensity = (row) => {
        if (row.length !== 2) return false;
        if (row[0].sec.layout === "series" || row[1].sec.layout === "series") return false;
        const a = row[0];
        const b = row[1];
        const nA = rowCount(a.sec);
        const nB = rowCount(b.sec);
        if (nA >= Math.max(12, nB * 2)) {
          a.span = 7;
          b.span = 5;
          return true;
        }
        if (nB >= Math.max(12, nA * 2)) {
          a.span = 5;
          b.span = 7;
          return true;
        }
        if (Math.max(nA, nB) <= 6) {
          a.span = 6;
          b.span = 6;
          return true;
        }
        return false;
      };
      const out = [];
      let i = 0;
      while (i < list.length) {
        const row = [];
        let sum = 0;
        while (i < list.length) {
          const sec = list[i];
          const d = desiredSpan(sec);
          if (d === 12) {
            if (row.length) break;
            row.push({ sec, span: 12 });
            i += 1;
            break;
          }
          if (sum + d <= 12) {
            row.push({ sec, span: d });
            sum += d;
            i += 1;
            if (sum === 12) break;
            continue;
          }
          break;
        }
        const isSingleFull = row.length === 1 && row[0].span === 12;
        if (row.length && !isSingleFull) {
          const adjusted = adjustKvRailWidths(row);
          const adjustedByDensity = adjusted || adjustTwoCellByDensity(row);
          if (!adjustedByDensity) {
            const sum2 = row.reduce((s, c) => s + c.span, 0);
            const rem = 12 - sum2;
            if (rem > 0) row[row.length - 1].span += rem;
          }
        }
        row.forEach((cell) => out.push(cell));
      }
      return out;
    },
    bbmsMainTabLabel(tab) {
      if (!tab || !tab.labelKey) return "";
      const base = this.$t(tab.labelKey);
      const n = this.bbmsMainTabBadgeCount(tab.id);
      return n > 0 ? `${base} (${n})` : base;
    },
    bbmsMainTabBadgeCount(tabId) {
      if (tabId === "flat") return this.filteredPointsFlatTable.length;
      if (tabId === "fault") {
        return (this.bbmsFaultPanelSections || []).reduce((sum, s) => sum + (s.rows ? s.rows.length : 0), 0);
      }
      const tab = (BBMS_MAIN_TABS || []).find((t) => t.id === tabId);
      if (!tab || !tab.sectionIds) return 0;
      const idSet = new Set(tab.sectionIds);
      return (this.bbmsDetailSections || [])
        .filter((s) => idSet.has(s.id))
        .reduce((sum, s) => sum + (s.rows ? s.rows.length : 0), 0);
    },
    bbmsLayoutCellsForTabId(tabId) {
      void this.locale;
      if (!tabId || tabId === "flat" || tabId === "alarms" || tabId === "fault") return [];
      const tab = (BBMS_MAIN_TABS || []).find((t) => t.id === tabId);
      if (!tab || !tab.sectionIds) return [];
      const idSet = new Set(tab.sectionIds);
      const sections = (this.bbmsDetailSections || []).filter((s) => idSet.has(s.id));
      return this.buildBbmsLayoutCells(sections);
    },
    rbmsMainTabLabel(tab) {
      if (!tab || !tab.labelKey) return "";
      const base = this.$t(tab.labelKey);
      const n = this.rbmsMainTabBadgeCount(tab.id);
      return n > 0 ? `${base} (${n})` : base;
    },
    rbmsMainTabBadgeCount(tabId) {
      if (tabId === "flat") return this.rbmsFilteredPoints.length;
      if (tabId === "cell_info") return (this.rbmsCellGrid || []).length;
      if (tabId === "fault") return this.rbmsFaultPointsFlat.length;
      const tab = (RBMS_MAIN_TABS || []).find((t) => t.id === tabId);
      if (!tab || !tab.sectionIds) return 0;
      const idSet = new Set(tab.sectionIds);
      return (this.rbmsConsoleSections || [])
        .filter((s) => idSet.has(s.id))
        .reduce((sum, s) => sum + (s.rows ? s.rows.length : 0), 0);
    },
    rbmsLayoutCellsForTabId(tabId) {
      void this.locale;
      if (!tabId || tabId === "flat" || tabId === "overview" || tabId === "cell_info" || tabId === "fault") return [];
      const tab = (RBMS_MAIN_TABS || []).find((t) => t.id === tabId);
      if (!tab || !tab.sectionIds) return [];
      const idSet = new Set(tab.sectionIds);
      const sections = (this.rbmsConsoleSections || []).filter((s) => idSet.has(s.id));
      return this.buildBbmsLayoutCells(sections);
    },
    rbmsDisplayValue(row) {
      return this.bbmsDisplayValue(row);
    },
    rbmsCardToneClass(sec) {
      const id = (sec && sec.id) || "misc";
      const energy = ["summary", "sox_io"];
      const thermal = ["temp_cell", "temp_pole", "temp_pack", "temp_bal_valid", "temp_bal_board", "tms_cluster"];
      const fault = ["fault_array", "cell_bal", "cell_sdr"];
      const cells = ["cell_volt"];
      if (energy.includes(id)) return "rbms-tone-energy";
      if (thermal.includes(id)) return "rbms-tone-thermal";
      if (fault.includes(id)) return "rbms-tone-fault";
      if (cells.includes(id) || id === "cell_volt_valid" || id === "afe_volt") return "rbms-tone-cells";
      return "rbms-tone-misc";
    },
    /** RBMS 电芯信息 Tab：轮询时追加电压短时曲线 */
    pushRbmsCellHistory() {
      if (this.activeView !== "rbms") return;
      const grid = this.rbmsCellGrid || [];
      if (!grid.length) return;
      const ts = Date.now();
      const max = this.rbmsCellHistMax || 90;
      const next = { ...this.rbmsCellHist };
      grid.forEach((cell) => {
        const i = cell.index0;
        const v =
          cell.voltValue !== null && cell.voltValue !== undefined && String(cell.voltValue).trim() !== ""
            ? Number(cell.voltValue)
            : null;
        if (v === null || Number.isNaN(v)) return;
        const slot = next[i] || { ts: [], volt: [] };
        const tsArr = slot.ts.concat([ts]);
        const vArr = slot.volt.concat([v]);
        while (tsArr.length > max) {
          tsArr.shift();
          vArr.shift();
        }
        next[i] = { ts: tsArr, volt: vArr };
      });
      this.rbmsCellHist = next;
      this.pushRbmsTempHistory();
    },
    /** RBMS 温度模块：轮询时追加温度短时曲线 */
    pushRbmsTempHistory() {
      if (this.activeView !== "rbms") return;
      const points = this.filteredPointsFlatTable || [];
      const grid = this.rbmsCellGrid || [];
      const packCount = Math.max(1, Math.ceil(grid.length / RBMS_CELL_UI.PACK_SIZE));
      const ts = Date.now();
      const max = this.rbmsCellHistMax || 90;
      const next = { ...this.rbmsTempHist };
      for (let p = 0; p < packCount; p++) {
        buildRbmsPackTempGrid(points, p).forEach((mod) => {
          const i = mod.index0;
          const t =
            mod.tempValue !== null && mod.tempValue !== undefined && String(mod.tempValue).trim() !== ""
              ? Number(mod.tempValue)
              : null;
          if (t === null || Number.isNaN(t)) return;
          const slot = next[i] || { ts: [], temp: [] };
          const tsArr = slot.ts.concat([ts]);
          const tArr = slot.temp.concat([t]);
          while (tsArr.length > max) {
            tsArr.shift();
            tArr.shift();
          }
          next[i] = { ts: tsArr, temp: tArr };
        });
      }
      this.rbmsTempHist = next;
    },
    buildRbmsHeroCard(slot, hit) {
      const s = slot || {};
      const p = hit || {};
      const u = p.unit != null && p.unit !== "" ? ` ${p.unit}` : "";
      const raw =
        p.value === undefined || p.value === null || String(p.value).trim() === ""
          ? "--"
          : `${p.value}${u}`;
      const labelKey = `monitor.rbmsHero_${s.key}`;
      const translated = this.$t(labelKey);
      const label = translated !== labelKey ? translated : this.pointDisplayName(p);
      return {
        key: s.key,
        label,
        valueText: raw,
        accent: s.accent || "sky",
        point: p,
      };
    },
    bbmsCardGridStyle(cell) {
      const n = cell && Number(cell.span);
      const s = Number.isFinite(n) && n > 0 && n <= 12 ? Math.floor(n) : 12;
      return { gridColumn: `span ${s}` };
    },
    /** rail：与其它卡片统一的栅格；≤5 点时附带紧凑列样式 */
    bbmsRailClass(sec) {
      const parts = ["bbms-rail"];
      if (!sec || sec.layout !== "rail") return parts.join(" ");
      if ((sec.rows && sec.rows.length) > 0 && sec.rows.length <= 5) parts.push("bbms-rail--tight");
      return parts.join(" ");
    },
    /** rail 少量测点：列数随个数，避免卡片内大块留白 */
    bbmsRailGridStyle(sec) {
      if (!sec || sec.layout !== "rail") return {};
      const n = (sec.rows && sec.rows.length) || 0;
      if (n < 1 || n > 5) return {};
      return {
        gridTemplateColumns: `repeat(${n}, minmax(100px, 136px))`,
        width: "fit-content",
        maxWidth: "100%",
      };
    },
    bbmsSectionRowCount(sec) {
      if (!sec || !Array.isArray(sec.rows)) return 0;
      return sec.rows.length;
    },
    /** 大量测点卡：统一控制为适中高度，内容在卡片内部滚动 */
    bbmsNeedsScroll(sec, inMainTab) {
      if (!sec) return false;
      const n = this.bbmsSectionRowCount(sec);
      if (inMainTab) {
        if (sec.layout === "series") return n >= 16;
        return n >= 18;
      }
      if (sec.layout === "series") return n >= 10;
      if (sec.id === "temp_pole_hv" || sec.id === "rack_current") return n >= 6;
      if (sec.layout === "rail") return n >= 9;
      return n >= 12;
    },
    /** BBMS 卡片配色：按语义分区 */
    bbmsCardToneClass(sec) {
      const id = (sec && sec.id) || "misc";
      const energy = [
        "soc_core",
        "dc_power",
        "cumulative",
        "power_limits",
        "rack_current",
        "cell_volt",
        "rack_summary",
        "insulation",
        "bank_status",
      ];
      const thermal = ["temp_pole_hv", "tms_runtime", "para_tms_thr", "ctrl_tms_thr", "env_aux"];
      const fault = [
        "fault_meta",
        "fault_list",
        "acore_fault",
        "flt_ov_count",
        "fault_enable",
        "bank_cali",
        "self_discharge",
      ];
      const control = ["hmi_bank", "hmi_tms", "hmi_do_lamp", "hmi_bbms_do", "ems_ctrl"];
      const arrays = ["soc_arrays", "soh_arrays"];
      if (energy.includes(id)) return "bbms-tone-energy";
      if (thermal.includes(id)) return "bbms-tone-thermal";
      if (fault.includes(id)) return "bbms-tone-fault";
      if (control.includes(id)) return "bbms-tone-control";
      if (arrays.includes(id)) return "bbms-tone-arrays";
      return "bbms-tone-misc";
    },
    bbmsDisplayValue(row) {
      const r = row || {};
      const u = r.unit != null && r.unit !== "" ? ` ${r.unit}` : "";
      if (r.value === undefined || r.value === null || String(r.value).trim() === "") {
        return u.trim() ? `--${u}` : "--";
      }
      return `${r.value}${u}`;
    },
    buildBbmsHeroCard(rule, hit) {
      const r = rule || {};
      const p = hit || {};
      const u = p.unit != null && p.unit !== "" ? ` ${p.unit}` : "";
      const raw =
        p.value === undefined || p.value === null || String(p.value).trim() === ""
          ? "--"
          : `${p.value}${u}`;
      let gaugePct = null;
      if (r.key === "soc" || r.key === "soh") {
        const cat = this.categorizePointForSplitDisplay(p);
        if (cat.kind === "gauge") gaugePct = cat.pct;
      }
      const labelKey = `monitor.bbmsHero_${r.key}`;
      const translated = this.$t(labelKey);
      const label = translated !== labelKey ? translated : this.pointDisplayName(p);
      return {
        key: r.key,
        label,
        valueText: raw,
        gaugePct,
        accent: r.accent || "sky",
        point: p,
      };
    },
    shouldPollDevicePoints() {
      const node = this.currentNode;
      if (!node) return false;
      if (this.monitorTemplatePending) return false;
      if (node.type === "device") {
        const devId = this.resolveMonitorDeviceId(node);
        return !(devId === null || devId === undefined || devId === "");
      }
      if (node.type === "virtual") {
        return this.collectVirtualMonitorPointDevices(node).some((d) => {
          const devId = this.resolveMonitorDeviceId(d);
          return !(devId === null || devId === undefined || devId === "");
        });
      }
      return false;
    },
    stopMonitorPointsPoll() {
      if (this.pointsPollTimer != null) {
        clearInterval(this.pointsPollTimer);
        this.pointsPollTimer = null;
      }
    },
    /** 选中设备后每 1s 拉取测点实时值（离开页或切走设备时停止） */
    startMonitorPointsPoll() {
      this.stopMonitorPointsPoll();
      if (!this.shouldPollDevicePoints()) return;
      const MS = 1000;
      this.pointsPollTimer = setInterval(async () => {
        if (!this.shouldPollDevicePoints()) {
          this.stopMonitorPointsPoll();
          return;
        }
        if (this.pointsPollInFlight) return;
        const node = this.currentNode;
        this.pointsPollInFlight = true;
        try {
          await this.loadCurrentNodePoints(node, { background: true });
        } finally {
          this.pointsPollInFlight = false;
        }
      }, MS);
    },
    async loadVirtualFolderPoints(node) {
      const devices = this.collectVirtualMonitorPointDevices(node);
      if (!devices.length) {
        this.currentPoints = [];
        return;
      }
      const merged = [];
      for (let i = 0; i < devices.length; i++) {
        const d = devices[i];
        const devId = this.resolveMonitorDeviceId(d);
        if (devId === null || devId === undefined || devId === "") continue;
        if (!this.deviceHasMonitorTemplate(d)) continue;
        try {
          const points = await getDevicePointsWithStatus(devId);
          const rows = Array.isArray(points?.rows)
            ? points.rows
            : Array.isArray(points?.data)
            ? points.data
            : [];
          rows.forEach((r) => {
            if (!r || typeof r !== "object") return;
            const gt =
              r.groupType !== undefined && r.groupType !== null && r.groupType !== ""
                ? r.groupType
                : r.group_type;
            merged.push(gt !== undefined && gt !== null && gt !== "" ? { ...r, groupType: gt } : r);
          });
        } catch (e) {
          /* 单个子设备失败不影响其它 */
        }
      }
      this.currentPoints = merged;
      this.pushRbmsCellHistory();
    },
    monitorPointsLoadKey(node) {
      if (!node) return "";
      if (node.type === "virtual") return `virtual:${node.id}`;
      const devId = this.resolveMonitorDeviceId(node);
      return devId !== null && devId !== undefined && devId !== "" ? `dev:${devId}` : `node:${node.id}`;
    },
    isSameMonitorPointsTarget(node, loadKey) {
      if (!node || !loadKey) return false;
      return this.monitorPointsLoadKey(node) === loadKey;
    },
    async loadCurrentNodePoints(node, opts) {
      const background = opts && opts.background === true;
      const loadSeq = opts && opts.seq !== undefined ? opts.seq : null;
      const loadKey = this.monitorPointsLoadKey(node);

      if (!node) {
        this.currentPoints = [];
        return;
      }
      if (node.type === "virtual") {
        if (!background) this.devicePointsLoading = true;
        try {
          await this.loadVirtualFolderPoints(node);
        } finally {
          if (!background) this.devicePointsLoading = false;
        }
        if (loadSeq !== null && loadSeq !== this.pointsLoadSeq) return;
        if (loadKey && !this.isSameMonitorPointsTarget(this.currentNode, loadKey)) return;
        return;
      }
      if (node.type !== "device") {
        this.currentPoints = [];
        return;
      }
      const devId = this.resolveMonitorDeviceId(node);
      if (devId === null || devId === undefined || devId === "") {
        this.currentPoints = [];
        return;
      }
      const tid =
        node.templateId !== undefined && node.templateId !== null && node.templateId !== ""
          ? node.templateId
          : node.template_id;
      if (tid === null || tid === undefined || tid === "" || Number(tid) === 0) {
        this.currentPoints = [];
        return;
      }
      if (!background) this.devicePointsLoading = true;
      try {
        const points = await getDevicePointsWithStatus(devId);
        if (loadSeq !== null && loadSeq !== this.pointsLoadSeq) return;
        if (loadKey && !this.isSameMonitorPointsTarget(this.currentNode, loadKey)) return;
        const rows = Array.isArray(points?.rows)
          ? points.rows
          : Array.isArray(points?.data)
          ? points.data
          : [];
        this.currentPoints = rows.map((r) => {
          if (!r || typeof r !== "object") return r;
          const gt =
            r.groupType !== undefined && r.groupType !== null && r.groupType !== ""
              ? r.groupType
              : r.group_type;
          return gt !== undefined && gt !== null && gt !== "" ? { ...r, groupType: gt } : r;
        });
        this.pushRbmsCellHistory();
      } catch (e) {
        if (loadSeq === null || loadSeq === this.pointsLoadSeq) {
          this.currentPoints = [];
        }
      } finally {
        if (!background) this.devicePointsLoading = false;
      }
    },
    onAuxPointDispatched({ code, value }) {
      if (code === null || code === undefined) return;
      const c = Number(code);
      this.currentPoints = (this.currentPoints || []).map((p) => {
        if (!p || Number(p.code) !== c) return p;
        return { ...p, value };
      });
      if (this.currentNode) {
        this.loadCurrentNodePoints(this.currentNode, { background: true });
      }
    },
    findNodePathById(nodes, targetId, trail = []) {
      const list = Array.isArray(nodes) ? nodes : [];
      for (const n of list) {
        const nextTrail = [...trail, n];
        if (String(n && n.id) === String(targetId)) return nextTrail;
        if (Array.isArray(n && n.children) && n.children.length) {
          const found = this.findNodePathById(n.children, targetId, nextTrail);
          if (found) return found;
        }
        if (Array.isArray(n && n._monitorPbmsList) && n._monitorPbmsList.length) {
          const foundPb = this.findNodePathById(n._monitorPbmsList, targetId, nextTrail);
          if (foundPb) return foundPb;
        }
      }
      return null;
    },
    syncSelectedPbmsNode() {
      const list = this.currentRbmsPbmsNodes || [];
      if (!list.length) {
        this.selectedPbmsNodeId = null;
        return;
      }
      if (!list.some(n => String(n.id) === String(this.selectedPbmsNodeId))) {
        this.selectedPbmsNodeId = list[0].id;
      }
    },
    handlePbmsSelectionChange(val) {
      this.selectedPbmsNodeId = val;
      this.$nextTick(() => this.initCharts());
    },
    pointIsResourcePerf(p) {
      const t = `${(p && p.name) || ""} ${(p && p.ename) || ""}`;
      if (!t.trim()) return false;
      // 系统拓扑/台账类，勿归入「资源与性能」（避免与下方系统卡风格割裂）
      if (/(网络|通讯|通信|以太网|LAN).*(状态|Status|State|连接|连通)/i.test(t)) return false;
      if (/(温度传感器|温感|Temp\s*sensor|探头).*(数量|个数|总数|Count)/i.test(t)) return false;
      return /CPU|内存|Memory|RAM|磁盘|Disk|闪存|存储空间|Utilization|utiliz|利用率|占用率|使用率|系统负载|Load\s*avg|LoadAverage|iowait|Idle|空闲率|网卡|带宽|流量|Throughput|Network|MHz|GHz(?!\s*温度)/i.test(
        t
      );
    },
    /** 识别 DI/DO 测点，单独归入「DI / DO」卡片（可从任意 groupType 抽出） */
    pointIsDido(p) {
      const t = `${(p && p.name) || ""} ${(p && p.ename) || ""}`.trim();
      if (!t) return false;
      if (/DI信号|DO输出|DI输入|DO输入|数字量输入|数字量输出|DIDO|数字输入|数字输出/i.test(t)) return true;
      if (/\bDI[\-_]?\d{1,3}\b|\bDO[\-_]?\d{1,3}\b/i.test(t)) return true;
      const compact = t.replace(/\s+/g, "");
      if (/DI\d{1,3}|DO\d{1,3}/i.test(compact)) return true;
      const u = t.toUpperCase();
      if (/\bDI_IN\b|\bDO_OUT\b|\bDIIN\b|\bDOUT\b|\bDIDO\b/i.test(u)) return true;
      return false;
    },
    /** 归入「状态量」卡：名称含「状态」/status，且排除纯在线/联网类（与已抽至大卡的点不重复） */
    pointIsSysStatusQuantity(p) {
      const s = String((p && (p.name || p.ename)) || "");
      if (!s.includes("状态") && !/\bstatus\b/i.test(s)) return false;
      if (/在线|联网|offline|online|是否在线/i.test(s) && !/(运行状态|故障状态|并网状态|充放电状态|设备状态)/i.test(s)) {
        return false;
      }
      return true;
    },
    /** 仅 DO 通道：与本机「DO」卡；其余 DIDO 归 DI 卡 */
    pointIsDoChannel(p) {
      if (!this.pointIsDido(p)) return false;
      const t = `${(p && p.name) || ""} ${(p && p.ename) || ""}`.trim();
      const compact = t.replace(/\s+/g, "");
      const hasDi =
        /DI信号|DI输入|数字量输入|\bDI[\-_]?\d{1,3}\b|\bDI_IN\b|\bDIIN\b/i.test(t) ||
        /DI\d{1,3}/i.test(compact);
      const hasDo =
        /DO输出|DO输入|数字量输出|\bDO[\-_]?\d{1,3}\b|\bDO_OUT\b|\bDOUT\b/i.test(t) ||
        /DO\d{1,3}/i.test(compact);
      if (hasDo && !hasDi) return true;
      if (/DIDO/i.test(t) && /DO|输出|OUT/i.test(t) && !/DI|输入|IN/i.test(t)) return true;
      return false;
    },
    /** 本机：大卡锚点 + 右侧 tile 独占测点，不再出现在下方测点卡 */
    pointIsExcludedFromLocalPointCards(p) {
      if (!this.isLocalSystemMonitor) return false;
      const t = `${(p && p.name) || ""}${(p && p.ename) || ""}`;
      if (HOST_ANCHOR_SLOT_DEFS.some((s) => s.m(t))) return true;
      if (HOST_LOCAL_SIDE_TILE_DEFS.some((s) => s.m(t))) return true;
      return false;
    },
    parsePointNumericValue(p) {
      const raw = p && p.value;
      if (raw === undefined || raw === null) return { hasNum: false, num: NaN, strVal: "" };
      const strVal = String(raw).trim();
      if (!strVal) return { hasNum: false, num: NaN, strVal };
      const cleaned = strVal.replace(/,/g, "").replace(/[^\d.-]/g, "");
      if (cleaned === "" || cleaned === "-" || cleaned === ".") return { hasNum: false, num: NaN, strVal };
      const num = parseFloat(cleaned);
      const hasNum = !Number.isNaN(num);
      return { hasNum, num, strVal };
    },
    estimateStatBarMax(name, num) {
      const s = String(name || "");
      const a = Math.abs(num);
      if (!a || Number.isNaN(a)) return null;
      if (/CPU|利用率|占用率|Load|Utilization|Usage/i.test(s)) return Math.max(100, a * 1.1);
      if (/电流|Current/i.test(s)) return Math.max(500, a * 1.35);
      if (/电压|Voltage|总压|簇压|母线/i.test(s)) return Math.max(1000, a * 1.25);
      if (/功率|Power|kW|MW|有功|无功|视在/i.test(s)) return Math.max(Math.abs(num) * 2, 50);
      if (/温度|Temp|温升|℃|°C/i.test(s)) return 85;
      if (/绝缘|电阻|MΩ|kΩ|Ω|阻/i.test(s)) return Math.max(a * 2, 0.001);
      if (/频率|Hz/i.test(s)) return 65;
      if (/容量|电量|能量|Ah|kWh|Wh/i.test(s)) return Math.max(a * 1.4, 1);
      return Math.max(a * 1.3, 1);
    },
    categorizePointForSplitDisplay(p) {
      const name = String((p && (p.name || p.ename)) || "");
      const blob = `${name} ${(p && p.ename) || ""} ${(p && p.name) || ""}`;
      const { hasNum, num, strVal } = this.parsePointNumericValue(p);

      // 与「其余测点 · 表格」统一：避免误入统计块 / 状态 chip 条
      if (
        /(网络|通讯|通信|以太网|LAN|Ethernet).*(状态|Status|State|连接|连通)/i.test(blob) ||
        /(网络|通讯).*(连接|连通)/i.test(blob) ||
        /(温度传感器|温感|Temp\s*sensor|探头).*(数量|个数|总数|Count)/i.test(blob) ||
        /传感器.*数量|温感.*数量/i.test(blob)
      ) {
        return { kind: "table" };
      }

      if (/SOC|SOH/i.test(name) && hasNum) {
        let n = num;
        if (n > 0 && n <= 1) n *= 100;
        const pct = Math.max(0, Math.min(100, n));
        return { kind: "gauge", pct };
      }

      if (
        /(CPU|内存|Memory|磁盘占用|存储占用|利用率|占用率|使用率|系统负载|Load\s*avg|LoadAverage)/i.test(name) &&
        hasNum
      ) {
        let n = num;
        if (n > 0 && n <= 1) n *= 100;
        const tail = String(strVal).trim();
        if (n >= 0 && n <= 100 && !/\bGB\b|\bMB\b|\bGHz\b|\bMHz\b|\bms\b/i.test(tail)) {
          return { kind: "gauge", pct: Math.min(100, Math.max(0, n)) };
        }
      }

      const statPattern =
        /电压|电流|功率|温度|Temp|Voltage|Current|Power|绝缘|电阻|环流|频率|Hz|Ah|kWh|Wh|容量|电量|能量|有功|无功|视在|内阻|压差|温差|CPU|Memory|磁盘|Disk|网络|带宽|流量/i;
      if (hasNum && statPattern.test(name)) {
        const max = this.estimateStatBarMax(name, num);
        const barPct =
          max != null && max > 0 ? Math.min(100, Math.round((Math.abs(num) / max) * 100)) : null;
        return { kind: "stat", barPct };
      }

      const chipName =
        /在线|状态|合闸|分闸|运行模式|故障|告警|开关|使能|投入|切除|离线|投运|停运|闭锁|解锁|反馈|信号|接触器|联锁|置位|DI|DO|bit|门|烟|水|消防|熔断|心跳|通信|Comm|Contactor|Breaker|Status|Fault|Trip|Enable|Online|Offline|Grid|Run/i;
      const chipVal = /^(正常|异常|告警|是|否|开|关|通|断|0|1|ON|OFF|TRUE|FALSE|投入|切除|闭合|断开|--|无|OK|NG)$/i;
      if (chipName.test(name) || chipVal.test(String(strVal).trim())) {
        return { kind: "chip" };
      }

      return { kind: "table" };
    },
    splitPointsForRichDisplay(points) {
      const gauges = [];
      const stats = [];
      const chips = [];
      const tableRows = [];
      (points || []).forEach((p) => {
        const cat = this.categorizePointForSplitDisplay(p);
        if (cat.kind === "gauge") {
          gauges.push({ ...p, _gaugePct: cat.pct });
        } else if (cat.kind === "stat") {
          const row = { ...p };
          if (cat.barPct != null) row._statBarPct = cat.barPct;
          stats.push(row);
        } else if (cat.kind === "chip") {
          chips.push(p);
        } else {
          tableRows.push(p);
        }
      });
      const onlyTable =
        gauges.length === 0 && stats.length === 0 && chips.length === 0 && tableRows.length > 0;
      const chipsMerged = onlyTable ? [] : chips.concat(tableRows);
      const _hasRichBlocks =
        gauges.length > 0 || stats.length > 0 || chipsMerged.length > 0;
      return {
        gauges,
        stats,
        chips: chipsMerged,
        tableRows: onlyTable ? tableRows : [],
        _hasRichBlocks,
      };
    },
    gaugeProgressColor(pct) {
      const v = Number(pct) || 0;
      if (v >= 80) return "#10b981";
      if (v >= 55) return "#3b82f6";
      if (v >= 30) return "#f59e0b";
      return "#ef4444";
    },
    /** 辅助设备在线：读取点表 Lqd_Online（1/非零为在线，0 为离线） */
    parseAuxiliaryOnlinePointValue(v) {
      if (v === null || v === undefined || String(v).trim() === "") return false;
      const s = String(v).trim().toLowerCase();
      if (["1", "true", "on", "online", "ok", "正常", "在线"].includes(s)) return true;
      if (["0", "false", "off", "offline", "离线"].includes(s)) return false;
      const n = Number(s);
      if (!Number.isNaN(n)) return n > 0;
      return false;
    },
    handleMonitorSplitTab(groupKey, name) {
      if (name === null || name === undefined || name === "") return;
      this.$set(this.splitTabByGroupKey, String(groupKey), String(name));
    },
    pointChipTagType(row) {
      const blob = `${String((row && row.name) || "")} ${String((row && row.ename) || "")} ${String(
        (row && row.value) != null ? row.value : ""
      )}`.toLowerCase();
      if (/告警|故障|异常|alarm|fault|trip|offline|断开|分闸|切除|false|ng/.test(blob)) return "danger";
      if (/正常|运行|是|on|true|闭合|投入|在线|正常|ok/.test(blob) && !/异常|故障|告警/.test(blob)) {
        return "success";
      }
      if (/停止|待机|离线|分闸|open|切除|关/.test(blob)) return "info";
      return "";
    },
    buildSplitSections(points, groupType) {
      const list = Array.isArray(points) ? points : [];
      const loc = String(this.$i18n && this.$i18n.locale ? this.$i18n.locale : "").toLowerCase();
      const preferEn = loc === "en" || loc.startsWith("en");
      const labelOf = (p) => String((p && (p.name || p.ename)) || "");
      const hasOnline = (s) => s.includes("在线") || /online|on-?line/i.test(s);
      const hasStatusWord = (s) =>
        s.includes("状态") || (preferEn && /\bstatus(es)?\b/i.test(s));
      const mk = (key, title, fn) => {
        const pts = list.filter(fn);
        return {
          key,
          title,
          points: pts,
          displaySplit: this.splitPointsForRichDisplay(pts),
        };
      };
      if (groupType === 0) {
        const sections = [
          mk("online", this.$t("monitor.splitTabOnline"), (p) => hasOnline(labelOf(p))),
          mk("status", this.$t("monitor.cardPointSysStatus"), (p) => hasStatusWord(labelOf(p))),
          mk("others", this.$t("monitor.splitTabOtherSysState"), (p) => {
            const s = labelOf(p);
            return !hasOnline(s) && !hasStatusWord(s);
          }),
        ];
        return sections.filter((s) => s.points.length);
      }
      if (groupType === 1) {
        const isEnergy = (p) =>
          /能量|容量|SOC|SOH|Energy|Capacity/i.test(String((p && (p.name || p.ename)) || ""));
        const powerPts = list.filter((p) => !isEnergy(p));
        const energyPts = list.filter(isEnergy);
        const merged = [...powerPts, ...energyPts];
        if (!merged.length) return [];
        return [
          {
            key: "all",
            title: this.$t("monitor.splitTabAll"),
            points: merged,
            displaySplit: this.splitPointsForRichDisplay(merged),
          },
        ];
      }
      const pts = list;
      return [
        {
          key: "all",
          title: this.$t("monitor.splitTabAll"),
          points: pts,
          displaySplit: this.splitPointsForRichDisplay(pts),
        },
      ];
    },
    /** 英文界面：拓扑树节点显示名（DB 可能仍为中文 name / device_name） */
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
      if (/中控柜\s*空调/.test(out)) out = out.replace(/中控柜\s*空调/g, "Central cabinet AC");
      if (/空调/.test(out) && /[\u4e00-\u9fff]/.test(out)) out = out.replace(/空调/g, " AC");
      return out;
    },
    localizeTopologyLabel(label, node) {
      const isEn = String((this.$i18n && this.$i18n.locale) || "")
        .toLowerCase()
        .startsWith("en");
      if (!isEn) return String(label || "").trim() || "--";
      let s = String(label || "").trim();
      if (!s) return "--";
      const codeName = s.match(/^(\d+)\s*#\s*(.+)$/);
      if (codeName) {
        return `${codeName[1]} # ${this.topologyLabelEn(codeName[2].trim())}`;
      }
      if (node && node.type === "virtual") return this.topologyLabelEn(s);
      if (/[\u4e00-\u9fff]/.test(s)) return this.topologyLabelEn(s);
      return s;
    },
    monitorNodeDisplayLabel(base, effective) {
      const isEn = String((this.$i18n && this.$i18n.locale) || "")
        .toLowerCase()
        .startsWith("en");
      const localDefault = isEn ? "Local system" : this.$t("device.localSystemName");
      const effDt = this.effectiveMonitorDeviceType(effective);
      if (effective && effective.type === "device" && effDt === 0) return localDefault;
      const raw = String(
        (base && base.type === "virtual" && (base.label || base.name)) ||
          (effective && (effective.label || effective.name)) ||
          ""
      );
      return this.localizeTopologyLabel(raw, base || effective) || "--";
    },
    normalizeTopologyLabels(nodes) {
      const acLabel = this.$i18n.locale === "en" ? "AC Meter" : this.$t("monitor.交流电表");
      const localLabel = this.$i18n.locale === "en" ? "Local system" : this.$t("device.localSystemName");
      const isEn = String((this.$i18n && this.$i18n.locale) || "")
        .toLowerCase()
        .startsWith("en");
      const walk = arr =>
        (arr || []).map(node => {
          const next = { ...node };
          const tidRaw =
            next.templateId !== undefined && next.templateId !== null && next.templateId !== ""
              ? next.templateId
              : next.template_id;
          if (tidRaw !== undefined && tidRaw !== null) next.templateId = tidRaw;
          if (next.virtualDevType !== undefined && next.virtualDevType !== null) {
            next.virtualDevType = Number(next.virtualDevType);
          }
          if (next.type === "virtual") {
            const inferred = this.inferVirtualDevTypeFromLabel(next);
            if (
              (next.virtualDevType === undefined || next.virtualDevType === null) &&
              inferred !== null
            ) {
              next.virtualDevType = inferred;
            }
            if (
              (next.deviceType === undefined || next.deviceType === null) &&
              next.virtualDevType !== undefined &&
              next.virtualDevType !== null
            ) {
              next.deviceType = next.virtualDevType;
            }
          }
          if (Array.isArray(next.children)) next.children = walk(next.children);
          const dtNum = this.monitorDeviceTypeNum(next.deviceType);
          if (next.type === "device" && dtNum === 0) {
            next.label = localLabel;
            next.name = localLabel;
          }
          if (next.deviceType === 6 || String(next.label || "").toLowerCase().includes("ac meter") || String(next.label || "").includes("交流电表")) {
            next.label = acLabel;
            next.name = acLabel;
          }
          if (isEn && next.label) {
            next.label = this.localizeTopologyLabel(next.label, next);
            next.name = next.label;
          }
          return next;
        });
      return walk(nodes);
    },
    /** 深度优先查找本机设备（type=device & deviceType=0），并返回需展开的祖先 id 列表 */
    findLocalSystemWithTrail(nodes, ancestors = []) {
      for (const n of nodes || []) {
        const t = this.monitorDeviceTypeNum(n && n.deviceType);
        if (n && n.type === "device" && t === 0) {
          return { node: n, expandIds: ancestors.map(a => a.id).filter(id => id !== undefined && id !== null) };
        }
        if (Array.isArray(n.children) && n.children.length) {
          const hit = this.findLocalSystemWithTrail(n.children, [...ancestors, n]);
          if (hit) return hit;
        }
      }
      return null;
    },
    expandTopologyNodeKeys(ids) {
      const tree = this.$refs.deviceTree;
      if (!tree || !tree.store || !tree.store.nodesMap) return;
      (ids || []).forEach((rawId) => {
        let n = tree.store.nodesMap[rawId];
        if (!n && rawId != null) n = tree.store.nodesMap[String(rawId)];
        if (n) n.expanded = true;
      });
    },
    /** 拓扑加载后展开全部有子节点的层级（default-expanded-keys 仅首渲生效） */
    expandAllTopologyTreeBranches() {
      this.$nextTick(() => {
        const keys = this.treeDefaultExpandKeys;
        if (keys && keys.length) this.expandTopologyNodeKeys(keys);
      });
    },
    async loadTopologyFromStorage() {
      if (this.topologyLoadInFlight) return;
      this.topologyLoadInFlight = true;
      const defaultSelectSeq = ++this.topologyDefaultSelectSeq;
      try {
        const res = await listTopology();
        const tree = Array.isArray(res?.data) ? res.data : [];
        const labeled = this.normalizeTopologyLabels(tree);
        const prevSelId =
          this.currentNode && this.currentNode.id !== undefined && this.currentNode.id !== null
            ? String(this.currentNode.id)
            : null;
        this.deviceTree = labeled;
        this.expandAllTopologyTreeBranches();

        /** 拓扑刷新后仍按上次选中 id 绑定最新节点对象，避免引用失效导致右侧停在「本机」视图 */
        if (prevSelId) {
          // 若加载期间用户已手动点了新节点（序号变化），放弃这次“旧选中回填”。
          if (defaultSelectSeq !== this.topologyDefaultSelectSeq) return;
          const latestCurId =
            this.currentNode && this.currentNode.id !== undefined && this.currentNode.id !== null
              ? String(this.currentNode.id)
              : null;
          // 若当前选中已变化（例如用户从 BBMS 点到 RBMS），不要再回填旧的 prevSelId。
          if (latestCurId && latestCurId !== prevSelId) return;
          const trail = this.findNodePathById(labeled, prevSelId);
          const fresh = trail && trail.length ? trail[trail.length - 1] : null;
          if (fresh) {
            this.$nextTick(() => {
              if (defaultSelectSeq !== this.topologyDefaultSelectSeq) return;
              const nowCurId =
                this.currentNode && this.currentNode.id !== undefined && this.currentNode.id !== null
                  ? String(this.currentNode.id)
                  : null;
              if (nowCurId && nowCurId !== prevSelId) return;
              if (this.$refs.deviceTree && fresh.id !== undefined && fresh.id !== null) {
                this.$refs.deviceTree.setCurrentKey(fresh.id);
              }
              this.handleNodeClick(fresh, { fromTree: false });
            });
            return;
          }
          this.currentNode = null;
        }

        const localHit = this.findLocalSystemWithTrail(labeled);
        const defaultNode =
          localHit && localHit.node ? localHit.node : (labeled && labeled[0]) || null;

        if (!this.currentNode && defaultNode) {
          this.$nextTick(() => {
            if (defaultSelectSeq !== this.topologyDefaultSelectSeq) return;
            if (this.currentNode) return;
            if (this.$refs.deviceTree && defaultNode.id !== undefined && defaultNode.id !== null) {
              this.$refs.deviceTree.setCurrentKey(defaultNode.id);
            }
            this.handleNodeClick(defaultNode, { fromTree: false });
            this.$nextTick(() => this.syncMonitorSelectionFromTree());
          });
        }
      } catch (e) {
        this.deviceTree = [];
      } finally {
        this.topologyLoadInFlight = false;
      }
    },
    toggleRbmsContactor(index) {
      const list = this.rbmsDataList || [];
      if (index >= 0 && index < list.length) {
        list[index].contactorClosed = !list[index].contactorClosed;
      }
    },
    /** 拓扑「文件夹」节点（menu_tree=0 → type=virtual）下挂的真实设备，用于点击文件夹时仍能加载测点 */
    collectDescendantMonitorDevices(node, out = []) {
      if (!node) return out;
      if (node.type === "device") out.push(node);
      (node.children || []).forEach((c) => this.collectDescendantMonitorDevices(c, out));
      if (Array.isArray(node._monitorPbmsList) && node._monitorPbmsList.length) {
        node._monitorPbmsList.forEach((c) => this.collectDescendantMonitorDevices(c, out));
      }
      return out;
    },
    /** 虚拟节点下优先选堆控（deviceType=1），否则第一个设备 */
    pickDeviceNodeUnderVirtualFolder(node) {
      const list = [];
      this.collectDescendantMonitorDevices(node, list);
      const stack = list.find((d) => this.monitorDeviceTypeNum(d && d.deviceType) === 1);
      return stack || list[0] || null;
    },
    /** 从当前 deviceTree 取同 id 节点，保证 deviceType/devId 与接口一致（勿用刷新前的陈旧引用） */
    canonicalMonitorTreeNode(node) {
      if (!node || node.id === undefined || node.id === null) return node;
      if (!Array.isArray(this.deviceTree) || !this.deviceTree.length) return node;
      const trail = this.findNodePathById(this.deviceTree, node.id);
      return trail && trail.length ? trail[trail.length - 1] : node;
    },
    /** Element UI：点击高亮节点时触发；node-click 为兜底（避免仅树变色、右侧不更新） */
    handleMonitorTreeCurrentChange(data) {
      if (!data || this.suppressMonitorTreeCurrentChange) return;
      this.handleNodeClick(data, { fromTree: true });
    },
    isMonitorTreeNodeActive(data) {
      if (!data || data.id === undefined || data.id === null) return false;
      const id = String(data.id);
      if (this.monitorTreeHighlightId !== undefined && this.monitorTreeHighlightId !== null) {
        return id === String(this.monitorTreeHighlightId);
      }
      const cn = this.currentNode;
      return !!(cn && cn.id !== undefined && cn.id !== null && id === String(cn.id));
    },
    /** 树节点类型角标：本机/堆/簇/辅/虚 */
    monitorTreeTypeAbbrev(data) {
      const n = data || {};
      if (n.type === "virtual") return this.$t("monitor.treeTagVirtual");
      const t =
        n.deviceType !== undefined && n.deviceType !== null
          ? this.monitorDeviceTypeNum(n.deviceType)
          : null;
      if (t === 0) return this.$t("monitor.treeTagLocal");
      if (t === 1) return this.$t("monitor.treeTagStack");
      if (t === 2) return this.$t("monitor.treeTagCluster");
      if (t === null) return "";
      return this.$t("monitor.treeTagAux");
    },
    /** 树高亮 id 与详情区是否已对齐（虚拟节点可能解析到子设备，不能仅比 id） */
    monitorTreeHighlightAligned(treeNode) {
      if (!treeNode || treeNode.id === undefined || treeNode.id === null) return true;
      const curId = String(treeNode.id);
      const hi =
        this.monitorTreeHighlightId !== undefined && this.monitorTreeHighlightId !== null
          ? String(this.monitorTreeHighlightId)
          : "";
      if (hi && curId === hi) return true;
      const cn = this.currentNode;
      if (cn && cn.id !== undefined && cn.id !== null && curId === String(cn.id)) return true;
      if (treeNode.type === "virtual") {
        const vdt = this.effectiveMonitorDeviceType(treeNode);
        if (vdt === null || vdt === undefined) {
          const resolved = this.pickDeviceNodeUnderVirtualFolder(treeNode);
          if (
            resolved &&
            cn &&
            cn.id !== undefined &&
            cn.id !== null &&
            String(resolved.id) === String(cn.id)
          ) {
            return true;
          }
        }
      }
      return false;
    },
    /** 兜底：树已高亮但 node-click 未触发时同步右侧（仅在 mounted / 拓扑加载后调用，勿放 updated） */
    syncMonitorSelectionFromTree() {
      if (this.syncingTreeCurrentNode) return;
      const tree = this.$refs.deviceTree;
      if (!tree || typeof tree.getCurrentNode !== "function") return;
      const cur = tree.getCurrentNode();
      if (!cur || cur.id === undefined || cur.id === null) return;
      if (this.monitorTreeHighlightAligned(cur)) return;
      this.syncingTreeCurrentNode = true;
      try {
        this.handleNodeClick(cur, { fromTree: true });
      } finally {
        this.syncingTreeCurrentNode = false;
      }
    },
    resolveMonitorActiveView(node) {
      const n = node || {};
      if (!n || (n.type !== "device" && n.type !== "virtual")) return "pointView";
      const dt = this.effectiveMonitorDeviceType(n);
      if (dt === 2) return "rbms";
      return "pointView";
    },
    handleNodeClick(node, opts) {
      if (!node) return;
      const fromTree = !opts || opts.fromTree !== false;
      if (fromTree) this.topologyDefaultSelectSeq += 1;
      const base = this.canonicalMonitorTreeNode(node);
      this.flatTableGroupFilter = "";
      let effective = base;
      if (base && base.type === "virtual") {
        const vdt = this.effectiveMonitorDeviceType(base);
        if (vdt === null || vdt === undefined) {
          const resolved = this.pickDeviceNodeUnderVirtualFolder(base);
          if (resolved) {
            effective = this.canonicalMonitorTreeNode(resolved) || resolved;
          }
        } else {
          effective = base;
        }
      }
      this.currentPoints = [];
      this.rbmsCellDetailIdx = null;
      this.rbmsCellHist = {};
      this.rbmsTempHist = {};
      this.pointsLoadSeq = (this.pointsLoadSeq || 0) + 1;
      const pointsLoadSeq = this.pointsLoadSeq;
      this.devicePointsLoading = true;
      this.currentNode = effective;
      this.monitorTreeHighlightId =
        base && base.id !== undefined && base.id !== null ? base.id : null;
      this.currentNodeLabel = this.monitorNodeDisplayLabel(base, effective);
      this.activeView = this.resolveMonitorActiveView(effective);
      this.$nextTick(() => {
        const tree = this.$refs.deviceTree;
        const treeKey =
          base && base.id !== undefined && base.id !== null ? base.id : effective && effective.id;
        if (tree && treeKey !== undefined && treeKey !== null) {
          this.suppressMonitorTreeCurrentChange = true;
          try {
            tree.setCurrentKey(treeKey);
          } finally {
            this.$nextTick(() => {
              this.suppressMonitorTreeCurrentChange = false;
            });
          }
        }
        const trail = this.findNodePathById(this.deviceTree, treeKey);
        if (trail && trail.length > 1) {
          this.expandTopologyNodeKeys(trail.slice(0, -1).map((n) => n.id));
        }
        this.loadCurrentNodePoints(this.currentNode, { seq: pointsLoadSeq }).then(() => {
          this.initCharts();
        });
        if (this.shouldPollDevicePoints()) this.startMonitorPointsPoll();
        else this.stopMonitorPointsPoll();
      });
    },
    cellVoltageByIndex(idx) {
      const v = this.pbmsData.cellVoltages;
      if (!v || v.length < idx) return "--";
      return (Math.round(v[idx - 1] * 1000) / 1000).toFixed(3);
    },
    cellTempByIndex(idx) {
      const t = this.pbmsData.cellTemps;
      if (!t || !t.length) return "--";
      // 如果只配置了部分温度点，按循环方式分配到 32 个电芯
      const arr = Array.isArray(t) ? t : [];
      const val = arr[(idx - 1) % arr.length];
      return `${val} ℃`;
    },
    formatPbmsAlertMsg(a) {
      const locale = (this.$i18n && this.$i18n.locale) || "";
      const isEn = locale.toLowerCase().startsWith("en");
      if (isEn) return a.msgEn || a.msgZh || "";
      return a.msgZh || a.msgEn || "";
    },
    openCellDetail(cellId) {
      this.selectedCellId = cellId;
      this.cellDetailVisible = true;
    },
    initCellDetailChart() {
      this.$nextTick(() => {
        if (!this.$refs.cellDetailChart) return;
        if (this.cellDetailChart) this.cellDetailChart.dispose();
        this.cellDetailChart = echarts.init(this.$refs.cellDetailChart);
        const nowLabel = new Date().toLocaleTimeString();
        const tempsArr = this.pbmsData.cellTemps || [];
        const voltsArr = this.pbmsData.cellVoltages || [];
        const idx = (this.selectedCellId || 1) - 1;
        const hasTemp = tempsArr.length > 0;
        const hasVolt = voltsArr.length > 0;
        const baseTemp = hasTemp ? Number(tempsArr[idx % tempsArr.length]) : null;
        const baseVolt = hasVolt ? Number(voltsArr[idx % voltsArr.length]) : null;
        const tempName = this.$t("overview.温度") || "温度";
        const voltName = this.$t("monitor.电压") || "电压";
        this.cellDetailChart.setOption({
          grid: { left: 56, right: 56, top: 24, bottom: 40 },
          legend: {
            data: [tempName, voltName],
            top: 0,
          },
          xAxis: {
            type: "category",
            data: [nowLabel],
            name: this.$t("monitor.时间"),
            nameGap: 22,
          },
          yAxis: [
            { type: "value", name: "℃" },
            { type: "value", name: "V", position: "right" },
          ],
          series: [
            {
              name: tempName,
              type: "line",
              smooth: true,
              yAxisIndex: 0,
              data: hasTemp ? [baseTemp] : [],
            },
            {
              name: voltName,
              type: "line",
              smooth: true,
              yAxisIndex: 1,
              data: hasVolt ? [baseVolt] : [],
            },
          ],
        });
      });
    },
    disposeCellDetailChart() {
      if (this.cellDetailChart) {
        this.cellDetailChart.dispose();
        this.cellDetailChart = null;
      }
    },
    initPbmsAlertChart() {
      if (!(this.activeView === "rbms" && this.$refs.pbmsAlertChart)) return;
      if (this.pbmsAlertChart) {
        this.pbmsAlertChart.dispose();
      }
      this.pbmsAlertChart = echarts.init(this.$refs.pbmsAlertChart);
      const alerts = this.pbmsAlerts || [];
      const times = alerts.map(a => a.time);
      const values = alerts.map((_, idx) => idx + 1);
      this.pbmsAlertChart.setOption({
        grid: { left: 40, right: 20, top: 24, bottom: 26 },
        xAxis: {
          type: "category",
          data: times,
          boundaryGap: false,
          axisLabel: { rotate: 0 },
        },
        yAxis: {
          type: "value",
          name: "",
          min: 0,
          max: Math.max(values.length, 4),
          splitNumber: 4,
        },
        series: [
          {
            type: "line",
            smooth: true,
            data: values,
            symbol: "circle",
            symbolSize: 6,
            lineStyle: { width: 2 },
            areaStyle: { opacity: 0.08 },
          },
        ],
        tooltip: {
          trigger: "axis",
          formatter: params => {
            const p = Array.isArray(params) ? params[0] : params;
            const idx = p.dataIndex;
            const a = alerts[idx];
            if (!a) return "";
            return `${a.time}<br/>${this.formatPbmsAlertMsg(a)}`;
          },
        },
      });
    },
    initCharts() {
      if (this.activeView === "rbms" && this.$refs.rbmsPowerChart) {
        if (this.rbmsChart) this.rbmsChart.dispose();
        this.rbmsChart = echarts.init(this.$refs.rbmsPowerChart);
        this.rbmsChart.setOption({
          grid: { left: 40, right: 20, top: 20, bottom: 30 },
          xAxis: { type: "category", data: ["0h", "2h", "4h", "6h", "8h", "10h", "12h"] },
          yAxis: { type: "value", name: "kW" },
          series: [{ name: this.$t("monitor.功率"), type: "line", smooth: true, data: this.rbmsData.powerSeries || [] }],
        });
      }
      if (this.activeView === "rbms" && this.$refs.pbmsCellChart) {
        if (this.pbmsCellChart) this.pbmsCellChart.dispose();
        this.pbmsCellChart = echarts.init(this.$refs.pbmsCellChart);
        const vols = this.pbmsData.cellVoltages;
        const balSet = new Set(this.pbmsData.balancingCells);
        const maxId = this.pbmsData.maxCellId - 1;
        const minId = this.pbmsData.minCellId - 1;
        this.pbmsCellChart.setOption({
          grid: { left: 40, right: 20, top: 36, bottom: 30 },
          xAxis: { type: "category", data: vols.map((_, i) => "C" + (i + 1)) },
          yAxis: { type: "value", name: "V", min: 3.2, max: 3.4 },
          series: [{
            type: "bar",
            data: vols.map((v, i) => ({
              value: Math.round(v * 1000) / 1000,
              itemStyle: {
                color: i === maxId ? "#e74c3c" : i === minId ? "#3498db" : "#2ecc71",
              },
              label: {
                show: balSet.has(i + 1),
                position: "top",
                formatter: balSet.has(i + 1) ? "⟳ " + (this.$t("monitor.均衡") || "均衡") : "",
                color: "#f39c12",
                fontSize: 11,
              },
            })),
          }],
        });
      }
      if (this.activeView === "rbms" && this.$refs.pbmsHeatChart) {
        if (this.pbmsHeatChart) this.pbmsHeatChart.dispose();
        this.pbmsHeatChart = echarts.init(this.$refs.pbmsHeatChart);
        const temps = this.pbmsData.cellTemps;
        this.pbmsHeatChart.setOption({
          grid: { left: 40, right: 20, top: 20, bottom: 30 },
          xAxis: { type: "category", data: temps.map((_, i) => "T" + (i + 1)) },
          yAxis: { type: "value", name: "℃", min: 30, max: 50 },
          visualMap: { min: 30, max: 50, inRange: { color: ["#3498db", "#2ecc71", "#f1c40f", "#e74c3c"] }, show: false },
          series: [{ type: "bar", data: temps }],
        });
      }
      if (this.activeView === "rbms") {
        this.initPbmsAlertChart();
      }
    },
  },
};
</script>

<style scoped lang="scss">
@import "./styles/monitor-shell.scss";
</style>

<!-- 左侧树选中：柔和灰底 + 深蓝左边线（非高亮整块蓝底） -->
<style lang="scss">
.device-monitor-page .device-sidebar {
  .tree-hint {
    margin: 0 8px 10px;
    font-size: 12px;
    line-height: 1.45;
    color: #64748b;
  }

  .device-tree--topology.el-tree--highlight-current {
    background: transparent;

    .el-tree-node__content {
      height: auto;
      min-height: 36px;
      line-height: 1.35;
      margin: 2px 4px;
      padding-top: 4px;
      padding-bottom: 4px;
      padding-left: 2px !important;
      border-radius: 6px;
      color: #475569;
      font-size: 14px;
      background: transparent !important;
    }

    .el-tree-node__content:hover {
      background: #f1f5f9 !important;
    }

    .el-tree-node.is-current > .el-tree-node__content {
      background: #eef2f7 !important;
    }

    /* 子层级竖向引导线，体现父子拓扑 */
    .el-tree-node__children {
      position: relative;
      margin-left: 8px;
      padding-left: 10px;
      border-left: 1px solid #c5d0de;
    }

    .el-tree-node__expand-icon {
      font-size: 15px;
      color: #475569 !important;
      padding: 4px;
      transform-origin: center;
    }

    .el-tree-node__expand-icon.is-leaf {
      color: transparent !important;
    }

    .el-tree-node__expand-icon.expanded {
      color: #2563eb !important;
    }
  }

  .monitor-tree-node {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    max-width: calc(100% - 28px);
    min-height: 28px;
    padding: 2px 8px 2px 4px;
    border-radius: 4px;
    border-left: 3px solid transparent;
    box-sizing: border-box;
  }

  .monitor-tree-node--active {
    background: #e8edf3;
    font-weight: 600;
    border-left-color: #2563eb;
  }

  .monitor-tree-node__tag {
    flex: 0 0 auto;
    min-width: 20px;
    padding: 0 5px;
    font-size: 11px;
    font-weight: 700;
    line-height: 18px;
    text-align: center;
    border-radius: 4px;
    color: #fff;
    background: #64748b;
  }

  .monitor-tree-node--active .monitor-tree-node__tag {
    background: #2563eb;
  }

  .monitor-tree-node--level-1 .monitor-tree-node__tag {
    background: #0d9488;
  }

  .monitor-tree-node--level-2 .monitor-tree-node__tag,
  .monitor-tree-node__tag:empty {
    background: #6366f1;
  }

  .monitor-tree-node__label {
    flex: 1;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    color: #1e293b;
  }

  .monitor-tree-node--active .monitor-tree-node__label {
    color: #0f172a;
  }
}
</style>
