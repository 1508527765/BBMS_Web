<template>
  <el-dialog
    :title="$t('device.设备配置')"
    :visible="visible"
    :width="activeTab === 'deviceConfig' ? '800px' : '1500px'"
    @close="handleClose"
  >
    <el-tabs v-model="activeTab" @tab-click="handleTabClick">
      <el-tab-pane :label="$t('device.设备配置')" name="deviceConfig">
        <el-form
          :model="form"
          ref="deviceForm"
          :rules="rules"
          label-width="150px"
          class="device-form"
          :disabled="editType === 'view'"
        >
          <!-- 基本信息 -->
          <el-form-item :label="$t('device.设备名称')" prop="name">
            <el-input
              v-model="form.name"
              :placeholder="$t('device.请输入设备名称')"
              :disabled="isLocalSystemLocked"
            />
          </el-form-item>

          <el-form-item :label="$t('device.设备类型')" prop="type">
            <el-select
              v-model="form.type"
              :placeholder="$t('device.请选择设备类型')"
              :disabled="isLocalSystemLocked"
              @change="onDeviceTypeChange"
            >
              <el-option
                v-for="item in deviceTypes"
                :key="item.value"
                :label="currentLocale === 'en' ? item.en_label : item.label"
                :value="item.value"
              />
            </el-select>
          </el-form-item>

          <el-form-item :label="$t('device.协议类型')" prop="protocolType">
            <el-select
              v-model="form.protocolType"
              :placeholder="$t('device.请选择协议类型')"
              @change="onProtocolChange"
            >
              <el-option
                v-for="item in protocolTypes"
                :key="item.value"
                :label="currentLocale === 'en' ? item.en_label : item.label"
                :value="item.value"
              />
            </el-select>
          </el-form-item>

          <el-form-item :label="$t('device.模板选择')" prop="templateId">
            <div style="display: flex; align-items: center">
              <el-select
                v-model="form.templateId"
                :disabled="!canSelectTemplate"
                :placeholder="$t('device.请选择模板')"
                @change="onTemplatechange"
              >
                <el-option
                  v-for="(template, key) in templateOptions"
                  :key="key"
                  :label="template.label"
                  :value="parseInt(template.value)"
                />
              </el-select>
              <div
                v-if="selectedTemplate && !selectedTemplate.hasPoints"
                style="
                  color: #e6a23c;
                  font-size: 12px;
                  margin-left: 10px;
                  white-space: nowrap;
                "
              >
                {{ $t("device.当前选择的模板未配置测点") }}
              </div>
            </div>
          </el-form-item>

          <!-- 动态协议配置 -->
          <div
            v-if="needsProtocolConfig(form.protocolType)"
            class="protocol-config"
          >
            <el-divider>{{ $t("device.协议配置") }}</el-divider>

            <!-- RTU 协议 -->
            <div v-if="isRtuProtocol(form.protocolType)">
              <el-row>
                <el-col :span="12">
                  <el-form-item :label="$t('device.串口号')" required>
                    <el-select
                      v-model="form.uartId"
                      :placeholder="$t('device.请选择串口号')"
                      @focus="fetchSerialList"
                      @change="onSerialPortChange"
                    >
                      <el-option
                        v-for="serial in serialList"
                        :key="serial.id"
                        :label="serial.name"
                        :value="serial.uartId"
                      />
                    </el-select>
                  </el-form-item>
                </el-col>
                <el-col :span="12">
                  <el-form-item :label="$t('device.子地址')" required>
                    <el-input-number v-model="form.uId" :min="1" :max="255" />
                  </el-form-item>
                </el-col>
              </el-row>

              <el-row>
                <el-col :span="12">
                  <el-form-item :label="$t('device.超时时间')" required>
                    <el-input-number v-model="form.timeout" :step="100" />
                  </el-form-item>
                </el-col>
                <el-col :span="12">
                  <el-form-item :label="$t('device.驱动地址')" required>
                    <el-input v-model="form.address" disabled />
                  </el-form-item>
                </el-col>
              </el-row>

              <el-row>
                <el-col :span="12">
                  <el-form-item :label="$t('device.波特率')" required>
                    <span>{{ baudRates[form.baud] }}</span>
                  </el-form-item>
                </el-col>
                <el-col :span="12">
                  <el-form-item :label="$t('device.校验位')" required>
                    <span>{{ parityOptions[form.parity] }}</span>
                  </el-form-item>
                </el-col>
              </el-row>

              <el-row>
                <el-col :span="12">
                  <el-form-item :label="$t('device.数据位')" required>
                    <span>{{ dataBits[form.data] }}</span>
                  </el-form-item>
                </el-col>
                <el-col :span="12">
                  <el-form-item :label="$t('device.停止位')" required>
                    <span>{{ stopBits[form.stop] }}</span>
                  </el-form-item>
                </el-col>
              </el-row>
            </div>

            <!-- CAN 协议 -->
            <div v-else-if="isCanProtocol(form.protocolType)">
              <el-form-item :label="$t('device.CAN口号')" required>
                <el-select
                  v-model="form.canId"
                  :placeholder="$t('device.请选择CAN口号')"
                  @focus="fetchCanList"
                  @change="onCanPortChange"
                >
                  <el-option
                    v-for="can in canList"
                    :key="can.id"
                    :label="can.name"
                    :value="can.canId"
                  />
                </el-select>
              </el-form-item>

              <el-form-item :label="$t('device.波特率')" required>
                <span>{{ canBaudRates[form.baud] }}</span>
              </el-form-item>

              <el-form-item :label="$t('device.子地址')" required>
                <el-input-number v-model="form.uId" :min="1" :max="255" />
              </el-form-item>

              <el-form-item :label="$t('device.超时时间')" required>
                <el-input-number v-model="form.timeout" :step="100" />
              </el-form-item>
            </div>

            <!-- TCP 协议 -->
            <div v-else>
              <el-form-item :label="$t('device.设备IP')" required>
                <el-input
                  v-model="form.ip"
                  :placeholder="$t('device.请输入IP地址')"
                />
              </el-form-item>

              <el-form-item :label="$t('device.设备端口')" required>
                <el-input-number
                  v-model="form.port"
                  :min="1"
                  :max="65535"
                  :placeholder="$t('device.请输入端口号')"
                />
              </el-form-item>

              <el-form-item :label="$t('device.子地址')" required>
                <el-input-number v-model="form.uId" :min="1" :max="255" />
              </el-form-item>

              <el-form-item :label="$t('device.超时时间')" required>
                <el-input-number v-model="form.timeout" :step="100" />
              </el-form-item>
            </div>
          </div>

          <div
            v-if="form.type && form.protocolType && form.templateId"
            class="protocol-config"
          >
            <el-divider>{{ $t("device.批量高级配置") }}</el-divider>
            <el-form-item>
              <el-checkbox
                v-model="form.batchAddEnabled"
                @change="handleBatchAddEnabled"
              >
                {{ $t("device.使能") }}
              </el-checkbox>
            </el-form-item>
          </div>

          <!-- 按钮 -->
          <el-form-item>
            <el-button type="primary" @click="submitForm">{{
              $t("common.提交")
            }}</el-button>
            <el-button @click="resetForm">{{ $t("common.重置") }}</el-button>
          </el-form-item>
        </el-form>
      </el-tab-pane>
      <el-tab-pane
        :label="$t('device.测点配置')"
        name="pointConfig"
        :disabled="editType === 'add'"
      >
        <!-- 用户导入对话框 -->
        <el-dialog
          :title="upload.title"
          :visible.sync="upload.open"
          width="400px"
          append-to-body
        >
          <el-upload
            ref="upload"
            :limit="1"
            accept=".csv, .xlsx, .xls"
            :headers="upload.headers"
            :action="upload.url + '?deviceId=' + form.id"
            :disabled="upload.isUploading"
            :on-progress="handleFileUploadProgress"
            :on-success="handleFileSuccess"
            :auto-upload="false"
            drag
          >
            <i class="el-icon-upload"></i>
            <div class="el-upload__text">
              {{ $t("upload.将文件拖到此处，或点击上传") }}
            </div>
            <div class="el-upload__tip text-center" slot="tip">
              <span>{{ $t("upload.仅允许导入csv、xls、xlsx格式文件。") }}</span>
              <el-link
                type="primary"
                :underline="false"
                style="font-size: 12px; vertical-align: baseline"
                @click="importTemplate"
                >{{ $t("upload.下载模板") }}</el-link
              >
            </div>
          </el-upload>
          <div slot="footer" class="dialog-footer">
            <el-button type="primary" @click="submitFileForm">{{
              $t("common.确定")
            }}</el-button>
            <el-button @click="upload.open = false">{{
              $t("common.取消")
            }}</el-button>
          </div>
        </el-dialog>
        <el-row>
          <el-col :span="24">
            <el-button
              type="primary"
              plain
              icon="el-icon-upload"
              size="mini"
              @click="handleImport"
            >
              {{ $t("device.导入") }}
            </el-button>

            <el-button
              type="warning"
              plain
              icon="el-icon-download"
              size="mini"
              @click="handleExport"
              >{{ $t("common.导出") }}</el-button
            >
            <el-button
              type="primary"
              plain
              icon="el-icon-plus"
              size="mini"
              @click="handleAddPoint"
              >{{ $t("common.新增") }}</el-button
            >
          </el-col>
        </el-row>
        <div class="custom-table">
          <el-table
            ref="table"
            :data="tableData"
            border
            height="550px"
            width="100%"
            v-loading="loading"
          >
            <el-table-column
              type="index"
              :label="$t('device.序号')"
              width="50"
            />
            <el-table-column
              v-for="column in displayedColumns"
              :key="column.prop"
              :prop="column.prop"
              :width="column.width"
            >
              <template #header>
                <span>
                  {{ column.label }}
                  <span v-if="column.required">
                    <el-tooltip
                      :content="$t('device.带*号的为必填项')"
                      placement="top"
                    >
                      <span class="required-tip">*</span>
                    </el-tooltip>
                  </span>
                </span>
              </template>
              <template v-slot="scope">
                <template v-if="scope.row.editing && column.editable">
                  <el-input
                    v-if="column.type === 'input'"
                    v-model="scope.row[column.prop]"
                    :placeholder="column.required ? $t('device.必填项') : ''"
                    @blur="validateField(scope.row, column)"
                  />
                  <el-select
                    v-else-if="column.type === 'select'"
                    v-model="scope.row[column.prop]"
                    :placeholder="column.required ? $t('device.必填项') : ''"
                    @change="validateField(scope.row, column)"
                  >
                    <el-option
                      v-for="option in column.options"
                      :key="option.value"
                      :label="
                        currentLocale === 'en' && option.en_label
                          ? option.en_label
                          : option.label
                      "
                      :value="option.value"
                    />
                  </el-select>
                </template>
                <template v-else>
                  <template v-if="column.type === 'select'">
                    {{
                      getLabelByValue(column.options, scope.row[column.prop])
                    }}
                  </template>
                  <template v-else>
                    {{ scope.row[column.prop] }}
                  </template>
                </template>
              </template>
            </el-table-column>

            <el-table-column
              :label="$t('common.操作')"
              width="150"
              fixed="right"
              align="center"
            >
              <template #default="scope">
                <el-button
                  v-if="!scope.row.editing"
                  type="text"
                  size="small"
                  @click="handleEdit(scope.row)"
                  >{{ $t("common.编辑") }}</el-button
                >
                <el-button
                  v-else
                  type="text"
                  size="small"
                  @click="handleSave(scope.row)"
                  :disabled="!isValidRow(scope.row)"
                  >{{ $t("common.保存") }}</el-button
                >
                <el-button
                  type="text"
                  size="small"
                  @click="handleDelete(scope.row)"
                  :disabled="scope.$index !== tableData.length - 1"
                  >{{ $t("common.删除") }}</el-button
                >
              </template>
            </el-table-column>
          </el-table>
        </div>
      </el-tab-pane>
      <el-tab-pane
        :label="$t('device.高级配置')"
        name="advancedConfig"
        :disabled="!form.batchAddEnabled"
      >
        <el-form>
          <el-form-item :label="$t('device.批量新增设备数量')">
            <el-input-number
              v-model="batchDeviceCount"
              :min="1"
              @change="generateBatchDevices"
            />
          </el-form-item>
        </el-form>
        <div class="custom-table">
          <el-table
            :data="batchTableData"
            border
            height="550px"
            width="100%"
            v-loading="loading"
          >
            <el-table-column
              type="index"
              :label="$t('device.序号')"
              width="50"
            />
            <el-table-column
              prop="name"
              :label="$t('device.设备名称')"
              width="150"
              editable
            >
              <template v-slot="scope">
                <el-input
                  v-model="scope.row.name"
                  :placeholder="$t('device.请输入设备名称')"
                />
              </template>
            </el-table-column>
            <el-table-column
              prop="type"
              :label="$t('device.设备类型')"
              width="150"
            >
              <template v-slot="scope">
                {{ getLabelByValue(deviceTypes, scope.row.type) }}
              </template>
            </el-table-column>
            <el-table-column
              prop="protocolType"
              :label="$t('device.协议类型')"
              width="150"
            >
              <template v-slot="scope">
                {{ getLabelByValue(protocolTypes, scope.row.protocolType) }}
              </template>
            </el-table-column>
            <el-table-column
              prop="templateId"
              :label="$t('device.模板选择')"
              width="150"
            >
              <template v-slot="scope">
                {{ getLabelByValue(templateOptions, scope.row.templateId) }}
              </template>
            </el-table-column>
            <!-- 动态协议配置列 -->
            <el-table-column
              v-if="isRtuProtocol(form.protocolType)"
              prop="uartId"
              :label="$t('device.串口号')"
              width="150"
              editable
            >
              <template v-slot="scope">
                <el-select
                  v-model="scope.row.uartId"
                  :placeholder="$t('device.请选择串口号')"
                  @focus="fetchSerialList"
                  @change="(value) => onSerialPortChange(value, scope.row)"
                >
                  <el-option
                    v-for="serial in serialList"
                    :key="serial.id"
                    :label="serial.name"
                    :value="serial.uartId"
                  />
                </el-select>
              </template>
            </el-table-column>
            <el-table-column
              v-if="isCanProtocol(form.protocolType)"
              prop="canId"
              :label="$t('device.CAN口号')"
              width="150"
              editable
            >
              <template v-slot="scope">
                <el-select
                  v-model="scope.row.canId"
                  :placeholder="$t('device.请选择CAN口号')"
                  @focus="fetchCanList"
                  @change="(value) => onCanPortChange(value, scope.row)"
                >
                  <el-option
                    v-for="can in canList"
                    :key="can.id"
                    :label="can.name"
                    :value="can.canId"
                  />
                </el-select>
              </template>
            </el-table-column>
            <el-table-column
              v-if="isTcpProtocol(form.protocolType)"
              prop="ip"
              :label="$t('device.设备IP')"
              width="150"
              editable
            >
              <template v-slot="scope">
                <el-input
                  v-model="scope.row.ip"
                  :placeholder="$t('device.请输入IP地址')"
                />
              </template>
            </el-table-column>
            <el-table-column
              v-if="isTcpProtocol(form.protocolType)"
              prop="port"
              :label="$t('device.设备端口')"
              editable
            >
              <template v-slot="scope">
                <el-input-number
                  v-model="scope.row.port"
                  :min="1"
                  :max="65535"
                  :placeholder="$t('device.请输入设备端口')"
                />
              </template>
            </el-table-column>
            <el-table-column
              v-if="needsProtocolConfig(form.protocolType)"
              prop="uId"
              :label="$t('device.子地址')"
              editable
              width="200"
            >
              <template v-slot="scope">
                <el-input-number v-model="scope.row.uId" :min="1" :max="255" />
              </template>
            </el-table-column>
            <el-table-column
              v-if="needsProtocolConfig(form.protocolType)"
              prop="timeout"
              :label="$t('device.超时时间')"
              editable
              width="200"
            >
              <template v-slot="scope">
                <el-input-number v-model="scope.row.timeout" :step="100" />
              </template>
            </el-table-column>
            <el-table-column
              v-if="isRtuProtocol(form.protocolType)"
              prop="address"
              :label="$t('device.驱动地址')"
              width="150"
            >
              <template v-slot="scope">
                <span>{{ scope.row.address }}</span>
              </template>
            </el-table-column>
            <el-table-column
              v-if="isRtuProtocol(form.protocolType) || isCanProtocol(form.protocolType)"
              prop="baud"
              :label="$t('device.波特率')"
              width="150"
            >
              <template v-slot="scope">
                <span>
                  {{
                    isRtuProtocol(form.protocolType)
                      ? baudRates[scope.row.baud]
                      : canBaudRates[scope.row.baud]
                  }}
                </span>
              </template>
            </el-table-column>
            <el-table-column
              v-if="isRtuProtocol(form.protocolType)"
              prop="parity"
              :label="$t('device.校验位')"
              width="150"
            >
              <template v-slot="scope">
                <span>{{ scope.row.parity }}</span>
              </template>
            </el-table-column>
            <el-table-column
              v-if="isRtuProtocol(form.protocolType)"
              prop="data"
              :label="$t('device.数据位')"
              width="150"
            >
              <template v-slot="scope">
                <span>{{ scope.row.data }}</span>
              </template>
            </el-table-column>
            <el-table-column
              v-if="isRtuProtocol(form.protocolType)"
              prop="stop"
              :label="$t('device.停止位')"
              width="150"
            >
              <template v-slot="scope">
                <span>{{ scope.row.stop }}</span>
              </template>
            </el-table-column>
          </el-table>
        </div>
        <el-button
          type="primary"
          @click="submitBatchAdd"
          style="margin-top: 10px"
          >{{ $t("device.提交批量新增") }}</el-button
        >
      </el-tab-pane>
    </el-tabs>
  </el-dialog>
</template>

<script>
import { listPort } from "@/api/system/port";
import { listAllTemplate } from "@/api/system/template";
import {
  getDevicePoints,
  addDevicePoint,
  updateDevicePoint,
  delDevicePoint,
  batchAddSystemDevices,
} from "@/api/system/device";
import { getToken } from "@/utils/auth";
import {
  protocolTypes,
  ProtocolType,
  isRtuProtocol,
  isCanProtocol,
  isTcpProtocol,
  needsProtocolConfig,
} from "@/constants/protocolTypes";
import { deviceTypes } from "@/constants/deviceTypes";
import i18n from "@/i18n"; // 引入i18n配置

export default {
  props: {
    visible: Boolean,
    deviceData: { type: Object, default: null },
    editType: { type: String, default: "add" },
    activeTab: { type: String, default: "deviceConfig" },
  },
  data() {
    return {
      tableData: [],
      batchTableData: [],
      batchDeviceCount: 1,
      editableRowId: null,
      isAddingNewRow: false,
      protocolTypes,
      // 测点导入参数
      upload: {
        // 是否显示弹出层（用户导入）
        open: false,
        // 弹出层标题（用户导入）
        title: "",
        // 是否禁用上传
        isUploading: false,
        // 设置上传的请求头部
        headers: { Authorization: "Bearer " + getToken() },
        // 上传的地址
        url: process.env.VUE_APP_BASE_API + "/device/import-points",
      },
      dialogVisible: false,
      isEdit: false,
      currentRow: {},
      rules: {
        name: [
          {
            required: true,
            message: this.$t("device.请输入设备名称"),
            trigger: "blur",
          },
        ],
        type: [
          {
            required: true,
            message: this.$t("device.请选择设备类型"),
            trigger: "change",
          },
        ],
        protocolType: [
          {
            required: true,
            message: this.$t("device.请选择协议类型"),
            trigger: "change",
          },
        ],
        templateId: [
          {
            validator: (rule, value, callback) => {
              const isLocal = Number(this.form && this.form.id) === 1;
              if (isLocal && (value === "" || value === null || value === undefined)) {
                callback();
                return;
              }
              if (value === "" || value === null || value === undefined) {
                callback(new Error(this.$t("device.请选择模板")));
                return;
              }
              if (Number(value) <= 0) {
                callback(new Error(this.$t("device.请选择模板")));
                return;
              }
              callback();
            },
            trigger: "change",
          },
        ],
      },
      serialList: [],
      canList: [],
      serialDetail: {},
      loading: false,
      selectedTemplate: null,
      // 表单数据
      form: {
        name: "",
        type: "",
        protocolType: "",
        templateId: null,
        remark: "",
        uartId: "",
        canId: "",
        baud: "",
        parity: "",
        data: "",
        stop: "",
        address: "",
        ip: "",
        port: "",
        timeout: "",
        uId: "",
        batchAddEnabled: false,
      },
      baudRates: {
        0: "115200",
        1: "57600",
        2: "38400",
        3: "19200",
        4: "9600",
        5: "4800",
        6: "2400",
        7: "1800",
        8: "1200",
        9: "600",
        10: "300",
        11: "200",
        12: "150",
      },
      canBaudRates: {
        0: "125000", // kCan_125k
        1: "250000", // kCan_250k
        2: "500000", // kCan_500k
      },
      parityOptions: {
        0: "None",
        1: "Odd",
        2: "Even",
        3: "Mark",
        4: "Space",
      },
      stopBits: {
        0: "1",
        1: "2",
      },
      dataBits: {
        0: "5",
        1: "6",
        2: "7",
        3: "8",
      },
      deviceTypes: deviceTypes,
      ProtocolType,
      isRtuProtocol,
      isCanProtocol,
      isTcpProtocol,
      needsProtocolConfig,
      templateOptions: [],
      columns: [
        {
          prop: "name",
          label: this.$t("device.名称"),
          width: "150",
          editable: true,
          type: "input",
        },
        {
          prop: "ename",
          label: this.$t("device.英文名称"),
          width: "150",
          editable: true,
          type: "input",
        },
        {
          prop: "code",
          label: this.$t("device.编码"),
          width: "100",
          editable: true,
          type: "input",
          required: true,
        },
        // {
        //   prop: "devType",
        //   label: this.$t('device.设备类型'),
        //   width: "150",
        //   editable: true,
        //   type: "select",
        //   required: true,
        //   options: deviceTypes,
        // },
        {
          prop: "groupType",
          label: this.$t("device.分组类型"),
          width: "100",
          editable: true,
          type: "select",
          required: true,
          options: [
            { value: 0, label: "遥信", en_label: "Status Indication" },
            { value: 1, label: "遥测", en_label: "Measurement Data" },
            { value: 2, label: "遥控", en_label: "Remote Control" },
            { value: 3, label: "遥调", en_label: "Setpoint Control" },
            { value: 4, label: "设置", en_label: "Configuration" },
            { value: 5, label: "遥脉", en_label: "Pulse Signals" },
            { value: 6, label: "故障", en_label: "Fault Alarm" },
            { value: 7, label: "事件", en_label: "Event Record" },
          ],
        },
        {
          prop: "dataType",
          label: this.$t("device.数据类型"),
          width: "100",
          editable: true,
          type: "select",
          required: true,
          options: [
            { value: 0, label: "UInt8" },
            { value: 1, label: "Int8" },
            { value: 2, label: "UInt16" },
            { value: 3, label: "Int16" },
            { value: 4, label: "UInt32" },
            { value: 5, label: "Int32" },
            { value: 6, label: "UInt64" },
            { value: 7, label: "Int64" },
            { value: 8, label: "Float32" },
            { value: 9, label: "Float64" },
            { value: 10, label: "Boolean" },
          ],
        },
        {
          prop: "attribute",
          label: this.$t("device.属性"),
          width: "100",
          editable: true,
          type: "select",
          required: true,
          options: [
            { value: 0, label: "Read Only" },
            { value: 1, label: "Write Only" },
            { value: 2, label: "Read/Write" },
            { value: 3, label: "Static" },
          ],
        },
        {
          prop: "func",
          label: this.$t("device.功能码"),
          width: "200",
          editable: true,
          type: "select",
          required: true,
          options: [
            { value: 0, label: "None" },
            { value: 0x01, label: "Read Coils (RC)" },
            { value: 0x02, label: "Read Discrete Inputs (RDI)" },
            { value: 0x03, label: "Read Holding Registers (RHR)" },
            { value: 0x04, label: "Read Input Registers (RIR)" },
            { value: 0x05, label: "Write Single Coil (WSC)" },
            { value: 0x06, label: "Write Single Register (WSR)" },
            { value: 0x0f, label: "Write Multiple Coils (WMC)" },
            { value: 0x10, label: "Write Multiple Registers (WMR)" },
          ],
        },
        {
          prop: "regAddr",
          label: this.$t("device.寄存器地址"),
          width: "120",
          editable: true,
          type: "input",
          required: true,
        },
        {
          prop: "bitPos",
          label: this.$t("device.位偏移"),
          width: "80",
          editable: true,
          type: "input",
          required: true,
          default: 0,
        },
        {
          prop: "bitNum",
          label: this.$t("device.位数"),
          width: "80",
          editable: true,
          type: "input",
          required: true,
          default: 0,
        },
        {
          prop: "endian",
          label: this.$t("device.字节序"),
          width: "100",
          editable: true,
          type: "select",
          required: true,
          default: 0,
          options: [
            { value: 0, label: "Big Endian" },
            { value: 1, label: "Little Endian" },
            { value: 2, label: "Big Endian" },
            { value: 3, label: "Little-Endian Word, Big-Endian Byte" },
            { value: 4, label: "Big-Endian Word, Little-Endian Byte" },
            { value: 5, label: "Little Endian" },
            { value: 6, label: "Big Endian" },
            { value: 7, label: "Little-Endian Word, Big-Endian Byte" },
            { value: 8, label: "Big-Endian Word, Little-Endian Byte" },
            { value: 9, label: "Little Endian" },
          ],
        },
        {
          prop: "precision",
          label: this.$t("device.精度"),
          width: "100",
          editable: true,
          type: "input",
          required: true,
          default: 1,
        },
        {
          prop: "ratio",
          label: this.$t("device.比例"),
          width: "100",
          editable: true,
          type: "input",
          required: true,
          default: 1,
        },
        {
          prop: "offset",
          label: this.$t("device.偏移量"),
          width: "100",
          editable: true,
          type: "input",
          required: true,
          default: 0,
        },
        {
          prop: "isPersisted",
          label: this.$t("device.是否持久化"),
          width: "150",
          editable: true,
          type: "select",
          required: true,
          default: 0,
          options: [
            { value: 1, label: "Yes" },
            { value: 0, label: "No" },
          ],
        },
        {
          prop: "storageInterval",
          label: this.$t("device.存储间隔"),
          width: "150",
          editable: true,
          type: "select",
          required: true,
          default: 30000,
          options: [
            { value: 1000, label: "1s" },
            { value: 2000, label: "2s" },
            { value: 3000, label: "3s" },
            { value: 4000, label: "4s" },
            { value: 5000, label: "5s" },
            { value: 6000, label: "6s" },
            { value: 7000, label: "7s" },
            { value: 8000, label: "8s" },
            { value: 9000, label: "9s" },
            { value: 10000, label: "10s" },
            { value: 15000, label: "15s" },
            { value: 20000, label: "20s" },
            { value: 30000, label: "30s" },
          ],
        },
        {
          prop: "mutateBound",
          label: this.$t("device.变化范围"),
          width: "150",
          editable: true,
          type: "input",
          default: 1,
        },
        {
          prop: "defaultValue",
          label: this.$t("device.默认值"),
          width: "120",
          editable: true,
          type: "input",
        },
        {
          prop: "minValue",
          label: this.$t("device.最小值"),
          width: "120",
          editable: true,
          type: "input",
          default: null,
        },
        {
          prop: "maxValue",
          label: this.$t("device.最大值"),
          width: "120",
          editable: true,
          type: "input",
          default: null,
        },
        {
          prop: "unit",
          label: this.$t("device.单位"),
          width: "100",
          editable: true,
          type: "input",
        },
        {
          prop: "isShow",
          label: this.$t("device.是否显示"),
          width: "120",
          editable: true,
          type: "select",
          required: true,
          default: 1,
          options: [
            { value: 1, label: this.$t("common.是") },
            { value: 0, label: this.$t("common.否") },
          ],
        },
      ],
    };
  },
  computed: {
    currentLocale() {
      return i18n.locale;
    },
    /** 仅锁定本机设备行（id=1）；勿用 type===0 判断，否则新增时空 type 会变成 Number('')===0 误锁整表 */
    isLocalSystemLocked() {
      const raw = this.form && this.form.id;
      if (raw === null || raw === undefined || raw === "") return false;
      return Number(raw) === 1;
    },
    displayedColumns() {
      if (this.currentLocale === "zh") {
        return this.columns.filter((col) => col.prop !== "ename");
      }
      if (this.currentLocale === "en") {
        return this.columns.filter((col) => col.prop !== "name");
      }
      return this.columns;
    },
    canSelectTemplate() {
      const pt = this.form.protocolType;
      const dt = this.form.type;
      return pt !== '' && pt !== null && pt !== undefined && dt !== '' && dt !== null && dt !== undefined;
    },
  },
  watch: {
    deviceData: {
      immediate: true,
      handler(newData) {
        if (newData) {
          const d = { ...newData };
          ["type", "protocolType", "templateId"].forEach((k) => {
            if (d[k] !== "" && d[k] != null && d[k] !== undefined) {
              const n = Number(d[k]);
              if (!Number.isNaN(n)) d[k] = n;
            }
          });
          if (Number(d.id) === 1 && Number(d.templateId) === 0) {
            d.templateId = null;
          }
          this.form = d;
        }
      },
    },
    visible: {
      immediate: true,
      handler(newVal) {
        if (newVal && isRtuProtocol(this.form.protocolType)) {
          this.fetchSerialList();
        }
        if (newVal && isCanProtocol(this.form.protocolType)) {
          this.fetchCanList();
        }
        if (newVal && this.form.protocolType != null && this.form.protocolType !== '' && this.form.type != null && this.form.type !== '') {
          this.loadTemplates(this.form.protocolType, this.form.type);
        }
      },
    },
  },
  methods: {
    async fetchSerialList() {
      this.loading = true;
      const response = await listPort({
        type: 0,
        pageNum: 1,
        pageSize: 100, //todo:写得太粗暴了需要改
      });
      // this.serialList = response.rows;
      this.$set(this, "serialList", response.rows);
      this.loading = false;
    },

    async fetchCanList() {
      const response = await listPort({
        type: 7,
        pageNum: 1,
        pageSize: 100, //todo:写得太粗暴了需要改
      });
      this.$set(this, "canList", response.rows);
    },

    onSerialPortChange(uartId, row = this.form) {
      const selectedPort = this.serialList.find(
        (serial) => serial.uartId === uartId
      );
      if (!selectedPort) return;

      // 根据 uartId 设置不同的配置
      row.address = selectedPort.address;
      row.baud = selectedPort.baud;
      row.parity = selectedPort.parity;
      row.data = selectedPort.data;
      row.stop = selectedPort.stop;
      row.uartId = selectedPort.uartId;
    },

    onCanPortChange(canId, row = this.form) {
      const selectedPort = this.canList.find((can) => can.canId === canId);
      if (!selectedPort) return;
      row.address = selectedPort.address;
      row.baud = selectedPort.baud;
      row.canId = selectedPort.canId;
    },

    async onDeviceTypeChange() {
      this.form.templateId = null;
      this.selectedTemplate = null;
      this.templateOptions = [];
      if (this.canSelectTemplate) {
        await this.loadTemplates(this.form.protocolType, this.form.type);
      }
    },

    // 切换协议类型
    async onProtocolChange(protocolType) {
      this.form.templateId = null;
      this.form.uartId = "";
      this.form.canId = "";
      this.form.uId = "";
      this.form.timeout = "";
      this.form.baud = "";
      this.form.parity = "";
      this.form.data = "";
      this.form.stop = "";
      this.form.address = "";
      this.form.ip = "";
      this.form.port = "";

      this.templateOptions = [];
      if (this.form.type !== '' && this.form.type != null && this.form.type !== undefined) {
        await this.loadTemplates(protocolType, this.form.type);
      }
    },

    async loadTemplates(protocolType, devType) {
      try {
        // 与旧项目一致：先按协议拉全量模板，再按设备类型收窄（devType 为空的历史模板仍可见）
        const response = await listAllTemplate({ protocol_type: protocolType });
        const typeNum = devType === "" || devType == null ? null : Number(devType);
        const rows = (response.rows || []).filter((item) => {
          if (typeNum === null || Number.isNaN(typeNum)) return true;
          const td = item.devType ?? item.dev_type;
          if (td === null || td === undefined || td === "") return true;
          return Number(td) === typeNum;
        });
        this.templateOptions = rows.map((item) => ({
          label: item.name,
          value: item.id,
          hasPoints: item.hasPoints,
        }));
        if (!this.templateOptions || this.templateOptions.length === 0) {
          this.templateOptions = [];
        }
      } catch (error) {
        console.error(this.$t("device.加载模板失败") + ":", error);
        this.templateOptions = [];
      }
    },

    onTemplatechange(templateId) {
      this.selectedTemplate = this.templateOptions.find(
        (t) => parseInt(t.value) === templateId
      );
      this.$forceUpdate();
    },

    // 提交表单
    submitForm() {
      this.$refs.deviceForm.validate((valid) => {
        if (valid) {
          if (this.editType === "add") {
            this.$emit("add", {
              ...this.form,
            });
          } else if (this.editType === "edit") {
            this.$emit("edit", {
              ...this.form,
            });
          }
        } else {
          this.$message.error(this.$t("device.表单校验失败，请检查输入内容"));
        }
      });
    },
    // 重置表单
    resetForm() {
      this.form = {
        id: "",
        name: "",
        type: "",
        protocolType: "",
        templateId: null,
        remark: "",
        uartId: "",
        canId: "",
        baud: "",
        parity: "",
        data: "",
        stop: "",
        address: "",
        ip: "",
        port: "",
        timeout: "",
        uId: "",
        batchAddEnabled: 0,
      };
    },

    handleClose() {
      this.resetForm();
      this.batchTableData = [];
      this.batchDeviceCount = 1;
      this.$emit("update:visible", false);
    },

    async fetchDevicePoints() {
      if (!this.form.id) return;
      this.loading = true;
      try {
        const res = await getDevicePoints(this.form.id, {});
        if (res.code === 200) {
          // /device/points 返回 data；历史接口有些返回 rows，这里兼容两种结构
          const rows = Array.isArray(res.rows)
            ? res.rows
            : Array.isArray(res.data)
            ? res.data
            : [];
          this.tableData = rows;
        } else {
          this.$message.error(res.msg || this.$t("device.获取测点数据失败"));
        }
      } catch (error) {
        this.$message.error(this.$t("common.请求失败") + "： " + error.message);
      } finally {
        this.loading = false;
      }
    },

    // 导入按钮操作
    handleImport() {
      this.upload.title = this.$t("device.测点导入");
      this.upload.open = true;
    },

    // 下载模板操作
    importTemplate() {
      this.download(
        "template/importTemplate",
        {},
        `template_point_${new Date().getTime()}.xlsx`
      );
    },

    // 文件上传中处理
    handleFileUploadProgress(event, file, fileList) {
      this.upload.isUploading = true;
    },

    // 文件上传成功处理
    handleFileSuccess(response, file, fileList) {
      this.upload.open = false;
      this.upload.isUploading = false;
      this.$refs.upload.clearFiles();
      this.$alert(
        "<div style='overflow: auto;overflow-x: hidden;max-height: 70vh;padding: 10px 20px 0;'>" +
          response.msg +
          "</div>",
        this.$t("upload.导入结果"),
        { dangerouslyUseHTMLString: true }
      );
      this.fetchDevicePoints();
    },

    // 提交上传文件
    submitFileForm() {
      this.$refs.upload.submit();
    },

    handleExport() {
      // 获取当前语言，默认为'zh'
      const lang = this.$i18n?.locale || "zh";
      this.download(
        `/device/export/?device_id=${this.form.id}&lang=${lang}`,
        {},
        `device_point_${new Date().getTime()}.xlsx`
      );
    },

    handleAddPoint() {
      const newPoint = {
        editing: true,
        devType: this.form.type,
        templateId: this.form.templateId,
        protocolType: this.form.protocolType,
        ratio: 1,
        offset: 0,
        precision: 1,
        isPersisted: 0,
        storageInterval: 30000,
        mutateBound: 1,
        minValue: null,
        maxValue: null,
        bitPos: 0,
        bitNum: 0,
        endian: 0,
        isShow: 1,
      };
      this.tableData.push(newPoint);
      this.$nextTick(() => {
        const table = this.$refs.table;
        if (table?.$el?.querySelector(".el-table__body-wrapper")) {
          table.$el.querySelector(".el-table__body-wrapper").scrollTop =
            table.$el.querySelector(".el-table__body-wrapper").scrollHeight;
        }
      });
    },

    async handleEdit(row) {
      this.tableData.forEach((item) => {
        if (item !== row) {
          this.$set(item, "editing", false);
        }
      });
      this.$set(row, "editing", true);
    },

    async handleSave(row) {
      if (!this.isValidRow(row)) {
        this.$message.error(this.$t("device.请填写所有必填字段"));
        return;
      }

      try {
        let res;
        const { editing, ...params } = row; // 去掉editing字段
        params.templateId = this.form.templateId;
        params.protocolType = this.form.protocolType;
        if (row.id) {
          res = await updateDevicePoint(params);
        } else {
          res = await addDevicePoint(params);
          if (res.code === 200 && res.data) {
            row.id = res.data; // 更新表格的属性，避免误判为新增
          }
        }
        if (res.code === 200) {
          this.$set(row, "editing", false);
          this.$message.success(this.$t("common.保存成功"));
        } else {
          this.$message.error(res.msg || this.$t("common.保存失败"));
        }
      } catch (error) {
        this.$message.error(this.$t("common.请求失败") + "： " + error.message);
      }
    },

    async handleDelete(row) {
      try {
        const res = await delDevicePoint(row.id);
        if (res.code === 200) {
          this.tableData = this.tableData.filter((item) => item !== row);
          this.$message.success(this.$t("common.删除成功"));
        } else {
          this.$message.error(res.msg || this.$t("common.删除失败"));
        }
      } catch (error) {
        this.$message.error(this.$t("common.请求失败") + "： " + error.message);
      }
    },

    handleTabClick(tab) {
      if (tab.name === "pointConfig") {
        this.fetchDevicePoints();
      }
    },

    handleBatchAddEnabled() {
      if (this.form.batchAddEnabled) {
        this.activeTab = "advancedConfig";
        this.generateBatchDevices(this.batchDeviceCount);
      } else {
        this.batchTableData = [];
        this.batchDeviceCount = 1;
      }
    },

    generateBatchDevices(count) {
      const currentCount = this.batchTableData.length;

      // 增加设备数量时
      if (count > currentCount) {
        const firstDevice = this.batchTableData[0] || this.getDefaultDevice(); // 如果还没有数据就创建一个默认设备

        const additionalDevices = Array.from(
          { length: count - currentCount },
          () => ({
            ...JSON.parse(JSON.stringify(firstDevice)), // 深拷贝，防止引用问题
            name: "", // 清空设备名，避免重复
          })
        );

        this.batchTableData = [...this.batchTableData, ...additionalDevices];
      } else {
        // 设备数量减少时，只保留前 count 个
        this.batchTableData = this.batchTableData.slice(0, count);
      }
    },

    getDefaultDevice() {
      const baseData = {
        name: "",
        type: this.form.type,
        protocolType: this.form.protocolType,
        templateId: this.form.templateId,
      };

      switch (this.form.protocolType) {
        case 1:
        case 3:
        case 5:
        case 10:
          return {
            ...baseData,
            ip: "",
            port: "",
            uId: "",
            timeout: "",
          };
        case 2:
          return {
            ...baseData,
            uartId: "",
            address: "",
            baud: "",
            parity: "",
            data: "",
            stop: "",
            uId: "",
            timeout: "",
          };
        case 9:
          return {
            ...baseData,
            canId: "",
            baud: "",
            uId: "",
            timeout: "",
          };
        default:
          return { ...baseData };
      }
    },

    async submitBatchAdd() {
      try {
        const response = await batchAddSystemDevices(this.batchTableData);
        if (response.code === 200) {
          this.$message.success(this.$t("device.批量新增设备成功"));
          this.batchTableData = [];
          this.batchDeviceCount = 1;
        } else {
          this.$message.error(
            response.msg || this.$t("device.批量新增设备失败")
          );
        }
      } catch (error) {
        this.$message.error(this.$t("common.请求失败") + "： " + error.message);
      }
    },

    getLabelByValue(options, value) {
      let option = options.find((opt) => opt.value === value);
      if (!option && value !== null && value !== undefined && value !== "") {
        const nv = Number(value);
        if (!Number.isNaN(nv)) {
          option = options.find((opt) => Number(opt.value) === nv);
        }
      }
      if (option) {
        return this.currentLocale === "en" && option.en_label
          ? option.en_label
          : option.label;
      }
      return value;
    },

    validateField(row, column) {
      if (column.required && !row[column.prop]) {
        this.$set(row, `${column.prop}Error`, true);
      } else {
        this.$set(row, `${column.prop}Error`, false);
      }
    },

    isValidRow(row) {
      return this.columns
        .filter((col) => col.required)
        .every((col) => row[col.prop] !== undefined && row[col.prop] !== "");
    },
  },
};
</script>

<style lang="scss" scoped>
.device-form {
  max-width: 780px;
  /* margin: 0 auto; */
}
.protocol-config {
  margin-top: 20px;
}
.custom-table {
  min-width: 780px; /* 根据需要调整 */
  overflow-x: auto;
  margin-top: 20px;
  ::-webkit-scrollbar {
    height: 15px; /* 调整滚动条的高度 */
  }
}
</style>
