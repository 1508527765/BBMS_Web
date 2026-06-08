<template>
  <el-dialog
    title="设备配置"
    :visible="visible"
    :width="'700px'"
    @close="onClose"
  >
    <el-tabs v-model="activeTab" @tab-click="handleTabClick">
      <el-tab-pane label="连接配置" name="connectionConfig">
        <el-form
          :model="form"
          ref="deviceForm"
          :rules="rules"
          label-width="120px"
          class="device-form"
          :disabled="editType === 'view'"
        >
          <!-- 连接配置 -->
          <el-form-item label="协议类型" prop="protocolType">
            <el-select
              v-model="form.protocolType"
              placeholder="请选择协议类型"
              @change="onProtocolChange"
            >
              <el-option
                v-for="item in protocolTypes"
                :key="item.value"
                :label="item.label"
                :value="item.value"
              />
            </el-select>
          </el-form-item>

          <el-form-item label="创建模型" prop="templateId">
            <el-select
              v-model="form.templateId"
              placeholder="请选择模板"
              @change="onTemplatechange"
            >
              <el-option
                v-for="(template, key) in templateOptions"
                :key="key"
                :label="template.label"
                :value="parseInt(template.value)"
              />
            </el-select>
          </el-form-item>

          <el-form-item label="上报格式" prop="reportFormat">
            <el-input
              v-model="form.reportFormat"
              placeholder="请输入上报格式"
            />
          </el-form-item>

          <el-form-item label="备注" prop="remark">
            <el-input v-model="form.remark" placeholder="请输入备注" />
          </el-form-item>
        </el-form>
      </el-tab-pane>

      <el-tab-pane label="协议配置" name="protocolConfig">
        <el-form
          :model="form"
          ref="protocolForm"
          :rules="protocolRules"
          label-width="120px"
          class="protocol-form"
          :disabled="editType === 'view'"
        >
          <!-- 协议配置 -->
          <el-form-item label="客户端ID" prop="clientId">
            <el-input v-model="form.clientId" placeholder="请输入客户端ID" />
          </el-form-item>

          <el-form-item label="Qos等级" prop="qosLevel">
            <el-input-number v-model="form.qosLevel" :min="0" :max="2" />
          </el-form-item>

          <el-form-item label="初始化主题" prop="initTopic">
            <el-input v-model="form.initTopic" placeholder="请输入初始化主题" />
          </el-form-item>

          <el-form-item label="周期上报主题" prop="periodicReportTopic">
            <el-input
              v-model="form.periodicReportTopic"
              placeholder="请输入周期上报主题"
            />
          </el-form-item>

          <el-form-item label="变化上报主题" prop="changeReportTopic">
            <el-input
              v-model="form.changeReportTopic"
              placeholder="请输入变化上报主题"
            />
          </el-form-item>

          <el-form-item label="历史数据上报主题" prop="historyReportTopic">
            <el-input
              v-model="form.historyReportTopic"
              placeholder="请输入历史数据上报主题"
            />
          </el-form-item>

          <el-form-item label="控制下发主题" prop="controlIssueTopic">
            <el-input
              v-model="form.controlIssueTopic"
              placeholder="请输入控制下发主题"
            />
          </el-form-item>

          <el-form-item label="读取下发主题" prop="readIssueTopic">
            <el-input
              v-model="form.readIssueTopic"
              placeholder="请输入读取下发主题"
            />
          </el-form-item>

          <el-form-item label="控制下发回复主题" prop="controlIssueReplyTopic">
            <el-input
              v-model="form.controlIssueReplyTopic"
              placeholder="请输入控制下发回复主题"
            />
          </el-form-item>

          <el-form-item label="读取下发回复主题" prop="readIssueReplyTopic">
            <el-input
              v-model="form.readIssueReplyTopic"
              placeholder="请输入读取下发回复主题"
            />
          </el-form-item>

          <el-form-item label="服务器IP地址" prop="serverIp">
            <el-input
              v-model="form.serverIp"
              placeholder="请输入服务器IP地址"
            />
          </el-form-item>

          <el-form-item label="服务器端口" prop="serverPort">
            <el-input-number
              v-model="form.serverPort"
              :min="1"
              :max="65535"
              placeholder="请输入服务器端口"
            />
          </el-form-item>

          <el-form-item label="用户名" prop="username">
            <el-input v-model="form.username" placeholder="请输入用户名" />
          </el-form-item>

          <el-form-item label="密码" prop="password">
            <el-input
              v-model="form.password"
              placeholder="请输入密码"
              type="password"
            />
          </el-form-item>

          <el-form-item label="超时时间" prop="timeout">
            <el-input-number
              v-model="form.timeout"
              :step="100"
              placeholder="请输入超时时间"
            />
          </el-form-item>

          <el-form-item label="CA证书" prop="caCertificate">
            <el-input v-model="form.caCertificate" placeholder="请输入CA证书" />
          </el-form-item>

          <el-form-item label="X509证书" prop="x509Certificate">
            <el-input
              v-model="form.x509Certificate"
              placeholder="请输入X509证书"
            />
          </el-form-item>

          <el-form-item label="客户端私钥" prop="clientPrivateKey">
            <el-input
              v-model="form.clientPrivateKey"
              placeholder="请输入客户端私钥"
            />
          </el-form-item>

          <el-form-item label="SSL连接" prop="sslConnection">
            <el-switch v-model="form.sslConnection" />
          </el-form-item>

          <el-form-item label="周期发送任务时间间隔" prop="taskInterval">
            <el-input-number
              v-model="form.taskInterval"
              :step="100"
              placeholder="请输入周期发送任务时间间隔"
            />
          </el-form-item>
        </el-form>
      </el-tab-pane>

      <el-tab-pane label="上报信息配置" name="reportConfig">
        <el-row>
          <el-col :span="24">
            <el-button
              type="primary"
              plain
              icon="el-icon-plus"
              size="mini"
              @click="handleAddReport"
            >
              新增
            </el-button>
          </el-col>
        </el-row>
        <el-table
          :data="reportData"
          border
          style="margin-top: 20px"
          height="550px"
          v-loading="loading"
        >
          <el-table-column type="index" label="序号" width="50" />
          <el-table-column
            v-for="column in reportColumns"
            :key="column.prop"
            :prop="column.prop"
            :label="column.label"
            :width="column.width"
          />
          <el-table-column
            label="操作"
            width="150"
            fixed="right"
            align="center"
          >
            <template #default="scope">
              <el-button
                type="text"
                size="small"
                @click="handleEditReport(scope.row)"
                >编辑</el-button
              >
              <el-button
                type="text"
                size="small"
                @click="handleDeleteReport(scope.row)"
                >删除</el-button
              >
            </template>
          </el-table-column>
        </el-table>
      </el-tab-pane>
    </el-tabs>
  </el-dialog>
</template>

<script>
import { listPort } from "@/api/system/port";
import { listAllTemplate } from "@/api/system/template";
import { getDevicePoints } from "@/api/system/device";
import { getToken } from "@/utils/auth";
import { protocolTypes } from "@/constants/protocolTypes";
import { deviceTypes } from "@/constants/deviceTypes";

export default {
  props: {
    visible: Boolean,
    deviceData: { type: Object, default: null },
    editType: { type: String, default: "add" },
    activeTab: { type: String, default: "connectionConfig" },
  },
  data() {
    return {
      protocolTypes,
      dialogVisible: false,
      isEdit: false,
      currentRow: {},
      rules: {
        protocolType: [
          { required: true, message: "请选择协议类型", trigger: "change" },
        ],
        templateId: [
          { required: true, message: "请选择模板", trigger: "change" },
        ],
        reportFormat: [
          { required: true, message: "请输入上报格式", trigger: "blur" },
        ],
        remark: [{ required: true, message: "请输入备注", trigger: "blur" }],
      },
      protocolRules: {
        clientId: [
          { required: true, message: "请输入客户端ID", trigger: "blur" },
        ],
        qosLevel: [
          { required: true, message: "请输入Qos等级", trigger: "change" },
        ],
        initTopic: [
          { required: true, message: "请输入初始化主题", trigger: "blur" },
        ],
        periodicReportTopic: [
          { required: true, message: "请输入周期上报主题", trigger: "blur" },
        ],
        changeReportTopic: [
          { required: true, message: "请输入变化上报主题", trigger: "blur" },
        ],
        historyReportTopic: [
          {
            required: true,
            message: "请输入历史数据上报主题",
            trigger: "blur",
          },
        ],
        controlIssueTopic: [
          { required: true, message: "请输入控制下发主题", trigger: "blur" },
        ],
        readIssueTopic: [
          { required: true, message: "请输入读取下发主题", trigger: "blur" },
        ],
        controlIssueReplyTopic: [
          {
            required: true,
            message: "请输入控制下发回复主题",
            trigger: "blur",
          },
        ],
        readIssueReplyTopic: [
          {
            required: true,
            message: "请输入读取下发回复主题",
            trigger: "blur",
          },
        ],
        serverIp: [
          { required: true, message: "请输入服务器IP地址", trigger: "blur" },
        ],
        serverPort: [
          { required: true, message: "请输入服务器端口", trigger: "change" },
        ],
        username: [
          { required: true, message: "请输入用户名", trigger: "blur" },
        ],
        password: [{ required: true, message: "请输入密码", trigger: "blur" }],
        timeout: [
          { required: true, message: "请输入超时时间", trigger: "change" },
        ],
        caCertificate: [
          { required: true, message: "请输入CA证书", trigger: "blur" },
        ],
        x509Certificate: [
          { required: true, message: "请输入X509证书", trigger: "blur" },
        ],
        clientPrivateKey: [
          { required: true, message: "请输入客户端私钥", trigger: "blur" },
        ],
        sslConnection: [
          { required: true, message: "请选择是否SSL连接", trigger: "change" },
        ],
        taskInterval: [
          {
            required: true,
            message: "请输入周期发送任务时间间隔",
            trigger: "change",
          },
        ],
      },
      loading: false,
      // 表单数据
      form: {
        protocolType: "",
        templateId: 0,
        reportFormat: "",
        remark: "",
        clientId: "",
        qosLevel: 0,
        initTopic: "",
        periodicReportTopic: "",
        changeReportTopic: "",
        historyReportTopic: "",
        controlIssueTopic: "",
        readIssueTopic: "",
        controlIssueReplyTopic: "",
        readIssueReplyTopic: "",
        serverIp: "",
        serverPort: "",
        username: "",
        password: "",
        timeout: "",
        caCertificate: "",
        x509Certificate: "",
        clientPrivateKey: "",
        sslConnection: false,
        taskInterval: 0,
      },
      templateOptions: [],
      reportData: [],
      reportColumns: [
        { prop: "index", label: "序号", width: "50" },
        { prop: "name", label: "名称", width: "150" },
        { prop: "code", label: "编码", width: "100" },
        { prop: "deviceType", label: "设备类型", width: "120" },
        { prop: "deviceNumber", label: "设备编号", width: "120" },
        { prop: "pointNumber", label: "点位编号", width: "120" },
        { prop: "remark", label: "备注", width: "150" },
      ],
    };
  },
  watch: {
    deviceData: {
      immediate: true,
      handler(newData) {
        if (newData) {
          this.form = { ...newData }; // 初始化表单数据
        }
      },
    },
    visible: {
      immediate: true,
      handler(newVal) {
        if (newVal && this.form.protocolType) {
          this.loadTemplates(this.form.protocolType);
        }
      },
    },
  },
  methods: {
    async onProtocolChange(protocolType) {
      this.form.templateId = 0;
      this.templateOptions = [];
      await this.loadTemplates(protocolType);
    },

    async loadTemplates(protocolType) {
      try {
        const response = await listAllTemplate({ protocol_type: protocolType });
        this.templateOptions = response.rows.map((item) => ({
          label: item.name,
          value: item.id,
        }));
        if (!this.templateOptions || this.templateOptions.length === 0) {
          this.templateOptions = [];
        }
      } catch (error) {
        console.error("加载模板失败:", error);
        this.templateOptions = [];
      }
    },

    onTemplatechange() {
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
          this.$message.error("表单校验失败，请检查输入内容");
        }
      });
    },
    // 重置表单
    resetForm() {
      this.form = {
        protocolType: "",
        templateId: 0,
        reportFormat: "",
        remark: "",
        clientId: "",
        qosLevel: 0,
        initTopic: "",
        periodicReportTopic: "",
        changeReportTopic: "",
        historyReportTopic: "",
        controlIssueTopic: "",
        readIssueTopic: "",
        controlIssueReplyTopic: "",
        readIssueReplyTopic: "",
        serverIp: "",
        serverPort: "",
        username: "",
        password: "",
        timeout: "",
        caCertificate: "",
        x509Certificate: "",
        clientPrivateKey: "",
        sslConnection: false,
        taskInterval: 0,
      };
    },

    onClose() {
      this.resetForm();
      this.$emit("update:visible", false);
    },

    handleAddReport() {
      this.currentRow = {};
      this.isEdit = false;
      this.dialogVisible = true;
    },

    handleEditReport(row) {
      this.currentRow = { ...row };
      this.isEdit = true;
      this.dialogVisible = true;
    },

    handleDeleteReport(row) {
      this.reportData = this.reportData.filter((item) => item !== row);
    },

    handleSubmitReport(data) {
      if (this.isEdit) {
        Object.assign(this.currentRow, data);
      } else {
        this.reportData.push(data);
      }
      this.dialogVisible = false;
    },

    handleTabClick(tab) {
      if (tab.name === "reportConfig") {
        this.fetchDevicePoints();
      }
    },
  },
};
</script>

<style scoped>
.device-form {
  max-width: 600px;
}
.protocol-form {
  max-width: 600px;
}
</style>
