<template>
  <div class="background">
    <div
      class="card bms-card"
      :class="{
        'bms-card--wide':
          section === 'dido' || section === 'battery' || section === 'system' || section === 'alarm'
      }"
    >
      <div class="bbms-header">
        <div class="bbms-title">
          <h3>{{ pageTitle }}</h3>
          <span class="bbms-subtitle">{{ pageSubtitle }}</span>
        </div>
      </div>

      <el-row :gutter="16">
        <!-- 左侧：配置表单；DI/DO 页占满宽 -->
        <el-col :span="24">
          <el-form
            ref="configForm"
            :model="configForm"
            label-width="230px"
            class="config-form"
          >
            <!-- 后台系统 / 控制 / 安全信号（纵向分区，无 Tab） -->
            <template v-if="section === 'system'">
              <div v-if="!visibleSystemTabs.length" class="bbms-section">
                <div class="info-card">
                  <div class="config-hint">
                    {{ isZhLocale() ? '暂无可见配置分组' : 'No visible configuration groups.' }}
                  </div>
                </div>
              </div>
              <template v-else>
                <div
                  v-for="tab in visibleSystemTabs"
                  :key="tab.name"
                  class="bbms-section system-param-section"
                >
                  <div class="info-card">
                    <div class="info-card-title">{{ getSystemTabLabel(tab) }}</div>
                    <el-row :gutter="24" class="system-config-row">
                      <el-col
                        v-for="(row, idx) in getSystemConfigRows(tab.name)"
                        :key="`${tab.name}-${idx}-${row.name}`"
                        :span="8"
                      >
                        <el-form-item :label="formatSystemConfigLabel(row.name)" class="system-config-form-item">
                          <el-select
                            v-if="shouldUseControlConfigSelect(row.name, tab.name)"
                            v-model="row.content"
                            size="small"
                            class="system-config-select"
                          >
                            <el-option
                              v-for="opt in getControlConfigSelectOptions(row.name)"
                              :key="`${row.name}-${opt.value}`"
                              :label="opt.label"
                              :value="opt.value"
                            />
                          </el-select>
                          <el-input-number
                            v-else-if="shouldUseControlConfigNumberRange(row.name, tab.name)"
                            :value="getControlConfigNumberValue(row)"
                            :min="0"
                            :max="100"
                            :step="1"
                            :precision="0"
                            controls-position="right"
                            size="small"
                            class="system-config-number"
                            @input="handleControlConfigNumberInput(row, $event)"
                          />
                          <el-input v-else v-model="row.content" size="small" />
                        </el-form-item>
                      </el-col>
                    </el-row>
                  </div>
                </div>
              </template>
            </template>

          <template v-if="section === 'battery'">
            <div class="bbms-section">
              <div class="info-card info-card--hero">
                <div class="info-card-title">{{ $t('battery.电池配置') }}</div>
                <p class="config-hint config-hint--lead">
                  {{
                    isZhLocale()
                      ? '仅保留数据库与 C 侧已定义键名的一致字段。'
                      : 'Only fields aligned with DB and C-side key names are kept.'
                  }}
                </p>
                <div class="battery-core-grid">
                  <div v-for="f in coreBatteryFields" :key="f.name" class="battery-core-item">
                    <label class="battery-core-label">{{ f.label }}</label>
                    <el-select
                      v-if="f.type === 'bool'"
                      :value="getType0Content(f.name)"
                      size="small"
                      class="battery-core-input"
                      @input="setType0Content(f.name, $event)"
                    >
                      <el-option :label="isZhLocale() ? '关闭' : 'Off'" value="0" />
                      <el-option :label="isZhLocale() ? '开启' : 'On'" value="1" />
                    </el-select>
                    <el-input
                      v-else
                      :value="getType0Content(f.name)"
                      size="small"
                      class="battery-core-input"
                      @input="setType0Content(f.name, $event)"
                    />
                  </div>
                </div>
              </div>
            </div>
          </template>

          <!-- 告警配置：合并为单表 -->
          <div v-if="section === 'alarm'" class="bbms-section alarm-flat">
            <div class="info-card">
              <div class="info-card-title">{{ $t('battery.pageTitle_alarm') }}</div>
              <div class="config-table-toolbar alarm-unified-toolbar">
                <el-select
                  v-model="alarmAddCategory"
                  size="small"
                  class="alarm-category-select"
                  :placeholder="$t('battery.告警配置类型')"
                >
                  <el-option
                    v-for="opt in alarmCategoryOptions"
                    :key="opt.key"
                    :label="opt.label"
                    :value="opt.key"
                  />
                </el-select>
                <el-button type="primary" size="small" @click="addAlarmRow()">
                  {{ $t('common.新增') }}
                </el-button>
                <el-button type="success" size="small" @click="saveAllAlarmConfigFromUi">
                  {{ $t('common.保存') }}
                </el-button>
              </div>
              <el-table :data="alarmConfigRows" size="small" border row-key="_rowId">
                <el-table-column
                  :label="$t('battery.告警配置类型')"
                  min-width="168"
                  fixed="left"
                >
                  <template slot-scope="{ row }">
                    <el-select v-model="row.categoryKey" size="small" class="alarm-row-category">
                      <el-option
                        v-for="opt in alarmCategoryOptions"
                        :key="opt.key"
                        :label="opt.label"
                        :value="opt.key"
                      />
                    </el-select>
                  </template>
                </el-table-column>
                <el-table-column min-width="140">
                  <template slot="header">
                    <span>{{ $t('battery.告警阈值') }}</span>
                    <span class="alarm-col-unit-hint">({{ $t('battery.单位见配置类型') }})</span>
                  </template>
                  <template slot-scope="{ row }">
                    <el-input v-model="row.threshold" size="small" />
                  </template>
                </el-table-column>
                <el-table-column
                  :label="$t('battery.告警触发时间(s)')"
                  min-width="130"
                >
                  <template slot-scope="{ row }">
                    <el-input v-model="row.triggerTime" size="small" />
                  </template>
                </el-table-column>
                <el-table-column min-width="150">
                  <template slot="header">
                    <span>{{ $t('battery.告警恢复阈值') }}</span>
                    <span class="alarm-col-unit-hint">({{ $t('battery.单位见配置类型') }})</span>
                  </template>
                  <template slot-scope="{ row }">
                    <el-input v-model="row.recoverThreshold" size="small" />
                  </template>
                </el-table-column>
                <el-table-column
                  :label="$t('battery.告警恢复时间(s)')"
                  min-width="130"
                >
                  <template slot-scope="{ row }">
                    <el-input v-model="row.recoverTime" size="small" />
                  </template>
                </el-table-column>
                <el-table-column
                  :label="$t('battery.告警处理方式')"
                  min-width="180"
                >
                  <template slot-scope="{ row }">
                    <el-input v-model="row.action" size="small" />
                  </template>
                </el-table-column>
                <el-table-column :label="$t('common.操作')" width="72" fixed="right">
                  <template slot-scope="{ $index }">
                    <el-button
                      type="text"
                      size="mini"
                      @click="removeAlarmRow($index)"
                    >
                      {{ $t('common.删除') }}
                    </el-button>
                  </template>
                </el-table-column>
              </el-table>
            </div>
          </div>

          <!-- DI/DO 配置：宽屏左右分栏 -->
          <template v-if="section === 'dido'">
            <el-row :gutter="16" class="dido-split-row">
              <el-col :xs="24" :lg="12">
                <div class="bbms-section dido-pane">
                  <div class="info-card">
                    <div class="info-card-title">{{ $t('battery.DI配置') }}</div>
              <div class="config-table-toolbar">
                <el-button
                  type="primary"
                  size="small"
                  :disabled="diConfigs.length >= didoMaxDi"
                  @click="addDiChannel"
                >
                  {{ $t('battery.新增DI通道') }}
                </el-button>
                <el-button type="danger" size="small" @click="removeLastDiChannel">
                  {{ $t('battery.删除最后一行') }}
                </el-button>
              </div>
              <el-table :data="diConfigs" size="small" border>
                <el-table-column
                  :label="$t('battery.通道名称')"
                  prop="name"
                  min-width="120"
                />
                <el-table-column
                  :label="$t('battery.柜子编号')"
                  prop="clusterNo"
                  min-width="100"
                >
                  <template slot-scope="{ row }">
                    <el-input-number
                      v-model="row.clusterNo"
                      size="small"
                      :min="0"
                      :max="9"
                      controls-position="right"
                    />
                  </template>
                </el-table-column>
                <el-table-column
                  :label="$t('battery.策略选择')"
                  prop="strategySeq"
                  min-width="180"
                >
                  <template slot-scope="{ row }">
                    <el-select v-model="row.strategySeq" size="small" filterable>
                      <el-option
                        v-for="item in diStrategyOptions"
                        :key="'di-opt-' + item.value"
                        :label="item.label"
                        :value="item.value"
                      />
                    </el-select>
                  </template>
                </el-table-column>
                <el-table-column
                  :label="$t('battery.备注')"
                  prop="remark"
                  min-width="160"
                >
                  <template slot-scope="{ row }">
                    <el-input v-model="row.remark" size="small" />
                  </template>
                </el-table-column>
              </el-table>
                    <div class="config-hint">
                      {{ $t('battery.DI配置说明', { fw: firmwareMaxDiConfig }) }}
                    </div>
                  </div>
                </div>
              </el-col>
              <el-col :xs="24" :lg="12">
                <div class="bbms-section dido-pane">
                  <div class="info-card">
                    <div class="info-card-title">{{ $t('battery.DO配置') }}</div>
              <div class="config-table-toolbar">
                <el-button
                  type="primary"
                  size="small"
                  :disabled="doConfigs.length >= didoMaxDo"
                  @click="addDoChannel"
                >
                  {{ $t('battery.新增DO通道') }}
                </el-button>
                <el-button type="danger" size="small" @click="removeLastDoChannel">
                  {{ $t('battery.删除最后一行') }}
                </el-button>
              </div>
              <el-table :data="doConfigs" size="small" border>
                <el-table-column
                  :label="$t('battery.通道名称')"
                  prop="name"
                  min-width="120"
                />
                <el-table-column
                  :label="$t('battery.柜子编号')"
                  prop="clusterNo"
                  min-width="100"
                >
                  <template slot-scope="{ row }">
                    <el-input-number
                      v-model="row.clusterNo"
                      size="small"
                      :min="0"
                      :max="9"
                      controls-position="right"
                    />
                  </template>
                </el-table-column>
                <el-table-column
                  :label="$t('battery.策略选择')"
                  prop="strategySeq"
                  min-width="160"
                >
                  <template slot-scope="{ row }">
                    <el-select v-model="row.strategySeq" size="small" filterable>
                      <el-option
                        v-for="item in doStrategyOptions"
                        :key="item.value"
                        :label="item.label"
                        :value="item.value"
                      />
                    </el-select>
                  </template>
                </el-table-column>
                <el-table-column
                  :label="$t('battery.备注')"
                  prop="remark"
                  min-width="160"
                >
                  <template slot-scope="{ row }">
                    <el-input v-model="row.remark" size="small" />
                  </template>
                </el-table-column>
              </el-table>
                    <div class="config-hint">
                      {{ $t('battery.DO配置说明', { fw: firmwareMaxDoConfig }) }}
                    </div>
                  </div>
                </div>
              </el-col>
            </el-row>
          </template>

          </el-form>

          <div class="bbms-section toolbar-section">
            <el-button type="primary" size="small" icon="el-icon-refresh" @click="handleLoad">
              {{ $t('battery.加载当前配置') }}
            </el-button>
            <el-button type="success" size="small" icon="el-icon-check" @click="handleSave">
              {{ $t('battery.保存配置') }}
            </el-button>
          </div>
        </el-col>

      </el-row>
    </div>
  </div>
</template>

<script>
import {
  fetchBatterySysConfig,
  saveBatterySysConfig,
  fetchBatteryAlertConfig,
  saveBatteryAlertConfig,
  fetchBatteryDidoConfig,
  saveBatteryDidoConfig,
} from '@/api/system/batteryConfig';
import {
  DIDO_MAX_DI,
  DIDO_MAX_DO,
  FIRMWARE_MAX_DI_CONFIG,
  FIRMWARE_MAX_DO_CONFIG,
  didoStrategyOptions,
} from '@/constants/didoLogicDict';

export default {
  name: 'BatteryConfigPage',
  props: {
    section: {
      type: String,
      default: 'battery',
      validator: (v) => ['system', 'battery', 'alarm', 'dido'].includes(v),
    },
  },
  data() {
    return {
      configForm: {},
      allSystemTabs: [
        { name: 'type0', label: 'System Config' },
        { name: 'type1', label: 'Control Config' },
        { name: 'type2', label: 'Safety Signal Config' },
      ],
      systemConfigRows: [],
      controlConfigRows: [],
      safetySignalConfigRows: [],
      systemConfigMeta: null,
      controlConfigMeta: null,
      safetySignalConfigMeta: null,
      alarmConfigRows: [],
      alarmAddCategory: 'deltaV',
      alarmRowIdSeq: 0,
      diConfigs: [],
      doConfigs: [],
      diStrategyOptionsDb: [],
      doStrategyOptionsDb: [],
      didoMaxDi: DIDO_MAX_DI,
      didoMaxDo: DIDO_MAX_DO,
      firmwareMaxDiConfig: FIRMWARE_MAX_DI_CONFIG,
      firmwareMaxDoConfig: FIRMWARE_MAX_DO_CONFIG,
    };
  },
  created() {
    this.handleLoad();
  },
  computed: {
    pageTitle() {
      const key = `battery.pageTitle_${this.section}`;
      return this.$te(key) ? this.$t(key) : this.$t('battery.电池配置');
    },
    pageSubtitle() {
      const key = `battery.pageSubtitle_${this.section}`;
      return this.$te(key) ? this.$t(key) : this.$t('battery.Battery Configuration');
    },
    diStrategyOptions() {
      if (Array.isArray(this.diStrategyOptionsDb) && this.diStrategyOptionsDb.length) {
        return this.diStrategyOptionsDb;
      }
      return didoStrategyOptions(this.isZhLocale(), 'di');
    },
    doStrategyOptions() {
      if (Array.isArray(this.doStrategyOptionsDb) && this.doStrategyOptionsDb.length) {
        return this.doStrategyOptionsDb;
      }
      return didoStrategyOptions(this.isZhLocale(), 'do');
    },
    coreBatteryFields() {
      const zh = this.isZhLocale();
      return [
        { name: 'bat_type', label: zh ? '电池类型' : 'Battery Type' },
        { name: 'bat_model', label: zh ? '电池型号' : 'Battery Model' },
        { name: 'bat_group_num', label: zh ? '电池组数量' : 'Battery Group Count' },
        { name: 'cluster_crtl_mode', label: zh ? '簇控制模式' : 'Cluster Control Mode' },
        { name: 'min_online_cluster_num', label: zh ? '最小在线簇数' : 'Min Online Clusters' },
        { name: 'sys_shutdown_word', label: zh ? '系统下电控制' : 'System Shutdown Word' },
        { name: 'allow_grid_volt_diff', label: zh ? '允许并网压差' : 'Allowed Grid Voltage Diff' },
        { name: 'detach_pulse_time', label: zh ? '脱扣脉冲时间(ms)' : 'Detach Pulse Time(ms)' },
        { name: 'cut_delay_time', label: zh ? '分闸断开延时(ms)' : 'Cut Delay Time(ms)' },
        { name: 'alert_led_time_interval', label: zh ? '告警灯闪烁间隔(ms)' : 'Alarm LED Interval(ms)' },
        { name: 'insulation_enable', label: zh ? '绝缘使能' : 'Insulation Enable', type: 'bool' },
        { name: 'insulation_monitor_interval', label: zh ? '绝缘监测间隔(s)' : 'Insulation Check Interval(s)' },
      ];
    },
    visibleSystemTabs() {
      return this.allSystemTabs.filter((tab) => this.isConfigTypeVisible(this.tabToType(tab.name)));
    },
    alarmCategoryOptions() {
      return [
        { key: 'deltaV', label: this.$t('簇间压差配置'), unit: 'mV' },
        { key: 'circulation', label: this.$t('簇间环流配置'), unit: 'A' },
        { key: 'deltaI', label: this.$t('簇间电流差配置'), unit: 'A' },
        { key: 'masterSlave', label: this.$t('主从通讯配置'), unit: '' },
      ].map((c) => ({
        ...c,
        label: c.unit ? `${c.label} (${c.unit})` : c.label,
      }));
    },
  },
  methods: {
    getType0RowByName(name) {
      return (this.systemConfigRows || []).find((r) => String(r.name || '').trim() === name) || null;
    },
    getType0Content(name) {
      const row = this.getType0RowByName(name);
      return row ? String(row.content ?? '') : '';
    },
    setType0Content(name, value) {
      const row = this.getType0RowByName(name);
      if (row) row.content = String(value ?? '');
      else this.systemConfigRows.push({ name, content: String(value ?? ''), is_show: 1 });
    },
    async handleLoad() {
      this.configForm = {};
      this.systemConfigRows = [];
      this.controlConfigRows = [];
      this.safetySignalConfigRows = [];
      this.alarmConfigRows = [];
      this.diConfigs = [];
      this.doConfigs = [];
      if (this.section === 'alarm') {
        await this.loadAlarmConfigFromDb();
      } else if (this.section === 'dido') {
        await this.loadDidoConfigFromDb();
      } else {
        await this.loadSystemConfigFromDb();
      }
      const isZh = this.isZhLocale();
      this.$message.success(isZh ? '已加载当前电池配置' : 'Current battery configuration loaded');
    },
    async handleSave() {
      if (this.section === 'alarm') {
        await this.saveAllAlarmConfigToDb();
      } else if (this.section === 'dido') {
        await this.saveDidoConfigToDb();
      } else {
        await this.saveSystemConfigToDb();
      }
      this.$message.success(this.$t('battery.电池配置保存成功'));
    },
    tabToType(tab) {
      if (tab === 'type1') return 1;
      if (tab === 'type2') return 2;
      return 0;
    },
    getTabMetaNameByType(type) {
      if (type === 1) return 'BBMS_Control';
      if (type === 2) return 'BBMS_SafetySignal';
      return 'BBMS_System_Config';
    },
    getSystemConfigMetaByType(type) {
      if (type === 1) return this.controlConfigMeta;
      if (type === 2) return this.safetySignalConfigMeta;
      return this.systemConfigMeta;
    },
    setSystemConfigMetaByType(type, meta) {
      if (type === 1) this.controlConfigMeta = meta;
      else if (type === 2) this.safetySignalConfigMeta = meta;
      else this.systemConfigMeta = meta;
    },
    isConfigTypeVisible(type) {
      const meta = this.getSystemConfigMetaByType(type);
      if (meta && meta.name === this.getTabMetaNameByType(type)) {
        return Number(meta.is_show) === 1;
      }
      const rows = this.getSystemConfigRowsByType(type);
      if (!rows.length) return false;
      const metaName = this.getTabMetaNameByType(type);
      const metaRow = rows.find((r) => String(r.name || '').trim() === metaName);
      if (metaRow) return Number(metaRow.is_show) === 1;
      return rows.some((r) => Number(r.is_show) === 1);
    },
    getSystemConfigRowsByType(type) {
      if (type === 1) return this.controlConfigRows;
      if (type === 2) return this.safetySignalConfigRows;
      return this.systemConfigRows;
    },
    getSystemConfigRows(tab) {
      return this.getSystemConfigRowsByType(this.tabToType(tab));
    },
    isZhLocale() {
      const locale = String((this.$i18n && this.$i18n.locale) || '').toLowerCase();
      return locale.startsWith('zh');
    },
    getSystemTabLabel(tab) {
      const zhMap = {
        type0: '后台系统',
        type1: '控制配置',
        type2: '安全信号配置',
      };
      const enMap = {
        type0: 'System (BBMS)',
        type1: 'Control Config',
        type2: 'Safety Signal Config',
      };
      return this.isZhLocale() ? (zhMap[tab.name] || tab.label) : (enMap[tab.name] || tab.label);
    },
    formatSystemConfigLabel(name) {
      const key = String(name || '').trim();
      const zhNameMap = {
        bat_type: '电池组类型',
        bat_model: '电池组型号',
        bat_group_num: '电池簇数量',
        cluster_control_mode: '簇并联控制方式',
        min_online_cluster_num: '最小在线簇数量',
        sys_shutdown_word: '系统下电控制配置',
        allow_grid_volt_diff: '并网允许电压差',
        detach_pulse_time: '脱扣脉冲时间',
        cut_delay_time: '切断延时时间',
        alert_led_time_interval: '告警灯闪烁周期',
        bat_conn: '电池连接使能',
        ins_meas_en: '绝缘检测使能',
        reserved: '预留占位',
        reservepc: '预留参数',
        bat_str_en: '电池串使能',
        bank_hb: 'Bank心跳',
        str_en_rack: '退簇后电池串使能',
        bat_st_rsn: '电池状态原因',
        bank_nb: '电池簇编号',
        bank_rk: '电池架编号',
        rack_exit_flag: '电池架退出标志',
        ctrl_mode: '控制模式',
        sys_arch_type: '系统架构类型',
        bank_err_lvl: '簇故障等级',
        disch_pwr_lim: '放电功率限制',
        chg_pwr_lim: '充电功率限制',
      };
      const zh = zhNameMap[key] || key.replace(/_/g, ' ');
      return this.isZhLocale() ? `${zh} (${key})` : key;
    },
    shouldUseControlConfigSelect(name, tabName) {
      if (tabName !== 'type1') return false;
      return this.getControlConfigSelectOptions(name).length > 0;
    },
    shouldUseControlConfigNumberRange(name, tabName) {
      if (tabName !== 'type1') return false;
      const key = String(name || '').trim();
      return key === 'disch_pwr_lim' || key === 'chg_pwr_lim';
    },
    getControlConfigNumberValue(row) {
      const num = Number((row && row.content) || 0);
      if (!Number.isFinite(num)) return 0;
      return Math.min(100, Math.max(0, Math.round(num)));
    },
    handleControlConfigNumberInput(row, value) {
      const num = Number(value);
      if (!Number.isFinite(num)) {
        row.content = '0';
        return;
      }
      const clamped = Math.min(100, Math.max(0, Math.round(num)));
      row.content = String(clamped);
    },
    getControlConfigSelectOptions(name) {
      const key = String(name || '').trim();
      const isZh = this.isZhLocale();
      const map = {
        bat_conn: [
          { value: '0', label: isZh ? '0 - 无请求' : '0 - NO REQ' },
          { value: '1', label: isZh ? '1 - 正常连接' : '1 - NORMAL CONNECT' },
          { value: '2', label: isZh ? '2 - 手动连接' : '2 - MANUAL CONNECT' },
          { value: '3', label: isZh ? '3 - 断开' : '3 - DISCONNECT' },
        ],
        ins_meas_en: [
          { value: '0', label: isZh ? '0x0 - 空闲' : '0x0 - IDLE' },
          { value: '1', label: isZh ? '0x1 - 使能' : '0x1 - ENABLE' },
          { value: '2', label: isZh ? '0x2 - 禁止' : '0x2 - DISABLE' },
          { value: '3', label: isZh ? '0x3 - 无效' : '0x3 - INVALID' },
        ],
        bat_str_en: [
          { value: '0', label: isZh ? '0x0 - 空闲' : '0x0 - IDLE' },
          { value: '1', label: isZh ? '0x1 - 使能' : '0x1 - ENABLE' },
          { value: '2', label: isZh ? '0x2 - 禁止' : '0x2 - DISABLE' },
          { value: '3', label: isZh ? '0x3 - 无效' : '0x3 - INVALID' },
        ],
        bank_hb: [
          { value: '0', label: isZh ? '0x0 - 禁止' : '0x0 - DISABLE' },
          { value: '1', label: isZh ? '0x1 - 使能' : '0x1 - ENABLE' },
        ],
        str_en_rack: [
          { value: '0', label: isZh ? '0 - 禁止' : '0 - DISABLE' },
          { value: '1', label: isZh ? '1 - 使能' : '1 - ENABLE' },
        ],
        rack_exit_flag: [
          { value: '0', label: isZh ? '0x0 - 非退簇系统' : '0x0 - Non Rack-exit System' },
          { value: '1', label: isZh ? '0x1 - 退簇系统' : '0x1 - Rack-exit System' },
        ],
        ctrl_mode: [
          { value: '0', label: isZh ? '0x0 - 空闲' : '0x0 - IDLE' },
          { value: '1', label: isZh ? '0x1 - 调试' : '0x1 - DEBUG' },
          { value: '2', label: isZh ? '0x2 - BMS控制' : '0x2 - BMS Control' },
          { value: '3', label: isZh ? '0x3 - EMS控制' : '0x3 - EMS Control' },
          { value: '4', label: isZh ? '0x4 - 无效' : '0x4 - INVALID' },
        ],
        sys_arch_type: [
          { value: '0', label: isZh ? '0x0 - 集中式架构' : '0x0 - Centralized Architecture' },
          { value: '1', label: isZh ? '0x1 - 组串式架构' : '0x1 - String Architecture' },
        ],
        bank_err_lvl: [
          { value: '0', label: isZh ? '0x0 - 默认' : '0x0 - Default' },
          { value: '1', label: isZh ? '0x1 - 一级故障' : '0x1 - Level 1 Fault' },
          { value: '2', label: isZh ? '0x2 - 二级故障' : '0x2 - Level 2 Fault' },
          { value: '3', label: isZh ? '0x3 - 三级故障' : '0x3 - Level 3 Fault' },
          { value: '7', label: isZh ? '0x7 - 无故障' : '0x7 - No Fault' },
        ],
      };
      return map[key] || [];
    },
    addSystemConfigRow(tab) {
      const rows = this.getSystemConfigRows(tab);
      rows.push({ name: '', content: '', is_show: 1 });
    },
    removeLastSystemConfigRow(tab) {
      const rows = this.getSystemConfigRows(tab);
      if (rows.length) rows.pop();
    },
    normalizeRows(rows) {
      return (rows || []).map((item) => ({
        name: String(item.name || ''),
        content: String(item.content || ''),
        is_show: Number(item.is_show) ? 1 : 0,
      }));
    },
    parseMetaContentToRows(metaRow) {
      const contentText = String((metaRow && metaRow.content) || '').trim();
      if (!contentText) return [];
      try {
        const obj = JSON.parse(contentText);
        if (!obj || typeof obj !== 'object' || Array.isArray(obj)) return [];
        return Object.keys(obj).map((k) => ({
          name: String(k),
          content: String(obj[k] ?? ''),
          is_show: 1,
        }));
      } catch (e) {
        return [];
      }
    },
    async loadConfigTypeRows(configType) {
      const res = await fetchBatterySysConfig(configType);
      const rawRows = this.normalizeRows((res && res.data) || []);
      const metaName = this.getTabMetaNameByType(configType);
      const meta = rawRows.find((r) => r.name === metaName) || null;
      this.setSystemConfigMetaByType(configType, meta);
      const rows = meta ? this.parseMetaContentToRows(meta) : rawRows;
      if (configType === 1) this.controlConfigRows = rows;
      else if (configType === 2) this.safetySignalConfigRows = rows;
      else this.systemConfigRows = rows;
      return rows;
    },
    async loadSystemConfigFromDb() {
      try {
        await this.loadConfigTypeRows(0);
        await this.loadConfigTypeRows(1);
        await this.loadConfigTypeRows(2);
      } catch (e) {
        this.systemConfigRows = [];
        this.controlConfigRows = [];
        this.safetySignalConfigRows = [];
        const isZh = this.isZhLocale();
        this.$message.error(isZh ? '加载电池配置失败，请检查后端与数据库' : 'Failed to load battery config, check backend and database');
      }
    },
    async saveSystemConfigToDb() {
      try {
        const buildPayload = (type, rows) => {
          const metaName = this.getTabMetaNameByType(type);
          const meta = this.getSystemConfigMetaByType(type) || {
            name: metaName,
            is_show: this.isConfigTypeVisible(type) ? 1 : 0,
          };
          const contentObj = {};
          this.normalizeRows(rows).forEach((r) => {
            if (!String(r.name || '').trim()) return;
            contentObj[r.name] = r.content;
          });
          return [{
            name: meta.name || metaName,
            content: JSON.stringify(contentObj, null, 4),
            is_show: Number(meta.is_show) ? 1 : 0,
          }];
        };

        await saveBatterySysConfig(0, buildPayload(0, this.systemConfigRows));
        await saveBatterySysConfig(1, buildPayload(1, this.controlConfigRows));
        await saveBatterySysConfig(2, buildPayload(2, this.safetySignalConfigRows));
        await this.loadSystemConfigFromDb();
      } catch (e) {
        // 保持页面可编辑，不阻塞前端交互
      }
    },
    normalizeAlarmRows(rows) {
      return (rows || []).map((r) => ({
        threshold: r.threshold ?? '',
        triggerTime: r.triggerTime ?? '',
        recoverThreshold: r.recoverThreshold ?? '',
        recoverTime: r.recoverTime ?? '',
        action: r.action ?? 0,
      }));
    },
    nextAlarmRowId() {
      this.alarmRowIdSeq += 1;
      return `alarm-${this.alarmRowIdSeq}`;
    },
    flattenAlarmDataFromApi(data) {
      const order = [
        { key: 'deltaV', dataKey: 'deltaV' },
        { key: 'circulation', dataKey: 'circulation' },
        { key: 'deltaI', dataKey: 'deltaI' },
        { key: 'masterSlave', dataKey: 'masterSlave' },
      ];
      const rows = [];
      order.forEach(({ key, dataKey }) => {
        this.normalizeAlarmRows(data[dataKey] || []).forEach((r) => {
          rows.push({ ...r, categoryKey: key, _rowId: this.nextAlarmRowId() });
        });
      });
      return rows;
    },
    splitAlarmRowsByCategory() {
      const buckets = { deltaV: [], circulation: [], deltaI: [], masterSlave: [] };
      (this.alarmConfigRows || []).forEach((row) => {
        const key = row.categoryKey;
        if (!buckets[key]) return;
        buckets[key].push(row);
      });
      return buckets;
    },
    async loadAlarmConfigFromDb() {
      try {
        const res = await fetchBatteryAlertConfig();
        const data = (res && res.data) || {};
        this.alarmRowIdSeq = 0;
        this.alarmConfigRows = this.flattenAlarmDataFromApi(data);
      } catch (e) {
        this.alarmConfigRows = [];
        this.$message.error(this.isZhLocale() ? '告警配置读取失败' : 'Failed to load alarm configuration');
      }
    },
    async saveAllAlarmConfigToDb() {
      const buckets = this.splitAlarmRowsByCategory();
      await saveBatteryAlertConfig(0, this.normalizeAlarmRows(buckets.deltaV));
      await saveBatteryAlertConfig(1, this.normalizeAlarmRows(buckets.circulation));
      await saveBatteryAlertConfig(2, this.normalizeAlarmRows(buckets.deltaI));
      await saveBatteryAlertConfig(3, this.normalizeAlarmRows(buckets.masterSlave));
      await this.loadAlarmConfigFromDb();
    },
    saveAllAlarmConfigFromUi() {
      this.saveAllAlarmConfigToDb()
        .then(() => this.$message.success(this.$t('告警配置保存成功通用')))
        .catch(() => this.$message.error(this.$t('common.操作失败')));
    },
    normalizeDidoRows(rows, prefix) {
      return (rows || []).map((r, idx) => {
        const seq = Number(r.seq || idx + 1);
        return {
          name: `${prefix}${seq}`,
          seq,
          clusterNo: Number(r.clusterNo != null ? r.clusterNo : 0),
          strategySeq: Number(r.strategySeq != null ? r.strategySeq : 0),
          remark: r.remark || '',
        };
      });
    },
    async loadDidoConfigFromDb() {
      try {
        const res = await fetchBatteryDidoConfig();
        const data = (res && res.data) || {};
        this.diConfigs = this.normalizeDidoRows(data.diRows || [], 'DI');
        this.doConfigs = this.normalizeDidoRows(data.doRows || [], 'DO');
        if (data.maxDiChannels) this.didoMaxDi = Number(data.maxDiChannels);
        if (data.maxDoChannels) this.didoMaxDo = Number(data.maxDoChannels);
        this.diStrategyOptionsDb = (data.diOptions || []).map((i) => ({
          label: this.isZhLocale() ? i.label : (i.en_label || i.label),
          value: Number(i.value),
        }));
        this.doStrategyOptionsDb = (data.doOptions || []).map((i) => ({
          label: this.isZhLocale() ? i.label : (i.en_label || i.label),
          value: Number(i.value),
        }));
      } catch (e) {
        this.diConfigs = [];
        this.doConfigs = [];
        this.$message.error(this.isZhLocale() ? 'DI/DO 配置读取失败' : 'Failed to load DI/DO configuration');
      }
    },
    async saveDidoConfigToDb() {
      await saveBatteryDidoConfig(
        this.normalizeDidoRows(this.diConfigs, 'DI'),
        this.normalizeDidoRows(this.doConfigs, 'DO')
      );
      await this.loadDidoConfigFromDb();
    },
    addAlarmRow() {
      const key = this.alarmAddCategory || 'deltaV';
      this.alarmConfigRows.push({
        _rowId: this.nextAlarmRowId(),
        categoryKey: key,
        threshold: '',
        triggerTime: '',
        recoverThreshold: '',
        recoverTime: '',
        action: 0,
      });
    },
    removeAlarmRow(index) {
      if (!this.alarmConfigRows.length) return;
      this.alarmConfigRows.splice(index, 1);
    },
    addDiChannel() {
      if (this.diConfigs.length >= this.didoMaxDi) {
        this.$message.warning(this.$t('battery.DI通道已达上限', { n: this.didoMaxDi }));
        return;
      }
      const nextIndex = this.diConfigs.length + 1;
      this.diConfigs.push({
        name: `DI${nextIndex}`,
        seq: nextIndex,
        clusterNo: 0,
        strategySeq: Number((this.diStrategyOptions[0] && this.diStrategyOptions[0].value) || 0),
        remark: '',
      });
    },
    removeLastDiChannel() {
      if (this.diConfigs.length <= 0) return;
      this.diConfigs.pop();
    },
    addDoChannel() {
      if (this.doConfigs.length >= this.didoMaxDo) {
        this.$message.warning(this.$t('battery.DO通道已达上限', { n: this.didoMaxDo }));
        return;
      }
      const nextIndex = this.doConfigs.length + 1;
      this.doConfigs.push({
        name: `DO${nextIndex}`,
        seq: nextIndex,
        clusterNo: 0,
        strategySeq: Number((this.doStrategyOptions[0] && this.doStrategyOptions[0].value) || 0),
        remark: '',
      });
    },
    removeLastDoChannel() {
      if (this.doConfigs.length <= 0) return;
      this.doConfigs.pop();
    },
  },
};
</script>

<style scoped>
.background {
  min-height: calc(100vh - 80px);
  background: #f3f5f9;
  padding: 8px;
}

.bms-card {
  max-width: 1300px;
  margin: 0 auto;
}

.bms-card--wide {
  max-width: none;
}

.bbms-header {
  margin-bottom: 10px;
  padding-bottom: 10px;
  border-bottom: 1px solid #e8eef5;
}

.bbms-title h3 {
  margin: 0 0 4px 0;
  font-size: 18px;
  color: #303133;
}

.bbms-subtitle {
  font-size: 12px;
  color: #64748b;
}

.bbms-section {
  margin-top: 10px;
}

.system-param-section {
  margin-top: 12px;
}

.alarm-flat .alarm-intro {
  margin-top: 0;
}

.alarm-unified-toolbar {
  flex-wrap: wrap;
  align-items: center;
}

.alarm-category-select {
  width: 220px;
}

.alarm-row-category {
  width: 100%;
}

.alarm-col-unit-hint {
  margin-left: 4px;
  font-size: 11px;
  font-weight: normal;
  color: #94a3b8;
}

.info-card {
  background: linear-gradient(180deg, #fbfdff 0%, #f8fbff 100%);
  border-radius: 10px;
  padding: 12px 14px;
  border: 1px solid #e7edf6;
}

.info-card-title {
  font-size: 13px;
  font-weight: 600;
  margin-bottom: 10px;
  color: #1e293b;
}

.config-form >>> .el-form-item {
  margin-bottom: 8px;
}

.config-table-toolbar {
  margin-bottom: 10px;
  display: flex;
  gap: 8px;
}

.config-hint {
  margin-top: 8px;
  font-size: 12px;
  color: #64748b;
}

.config-hint--lead {
  margin: 0 0 10px;
}

.info-card--hero {
  border-color: #d8e5ff;
  background: linear-gradient(180deg, #f8fbff 0%, #f2f7ff 100%);
}

.battery-core-grid {
  display: grid;
  grid-template-columns: repeat(3, minmax(0, 1fr));
  gap: 10px 12px;
}

.battery-core-item {
  display: flex;
  flex-direction: column;
  gap: 6px;
}

.battery-core-label {
  font-size: 12px;
  color: #475569;
  font-weight: 600;
}

.battery-core-input {
  width: 100%;
}

.system-config-row {
  margin-top: 6px;
}

.system-config-form-item >>> .el-form-item__label {
  white-space: normal;
  word-break: break-word;
  line-height: 1.3;
  padding-top: 8px;
}

.system-config-select {
  width: 100%;
}

.system-config-number {
  width: 100%;
}

.system-config-number >>> .el-input {
  width: 100%;
}

.toolbar-section {
  display: flex;
  gap: 12px;
  margin-top: 12px;
}

.card {
  background-color: #ffffff;
  box-shadow: 0 8px 28px rgba(15, 23, 42, 0.07);
  border-radius: 12px;
  padding: 14px;
}

.alarm-flat .info-card-title {
  position: relative;
  padding-left: 10px;
}

.alarm-flat .info-card-title::before {
  content: '';
  position: absolute;
  left: 0;
  top: 2px;
  bottom: 2px;
  width: 3px;
  border-radius: 3px;
  background: linear-gradient(180deg, #3b82f6 0%, #2563eb 100%);
}

@media (max-width: 1200px) {
  .battery-core-grid {
    grid-template-columns: repeat(2, minmax(0, 1fr));
  }
}

@media (max-width: 768px) {
  .battery-core-grid {
    grid-template-columns: 1fr;
  }
}
</style>
