<template>
  <el-dialog
    :title="$t('device.模版配置')"
    :visible="visible"
    :width="activeTab === 'pointConfig' ? '1500px' : '600px'"
    @close="handleClose"
  >
    <el-tabs v-model="activeTab" @tab-click="handleTabClick">
      <el-tab-pane :label="$t('device.模版配置')" name="templateConfig">
        <el-form
          :model="templateData"
          label-width="150px"
          :disabled="editType === 'view'"
        >
          <el-form-item
            v-if="templateType === 'device'"
            :label="$t('device.设备类型')"
          >
            <el-select
              v-model="templateData.devType"
              :placeholder="$t('device.请选择设备类型')"
            >
              <el-option
                v-for="item in deviceTypes"
                :key="item.value"
                :label="currentLocale === 'en' ? item.en_label : item.label"
                :value="item.value"
              />
            </el-select>
          </el-form-item>
          <el-form-item :label="$t('device.协议类型')">
            <el-select
              v-model="templateData.protocolType"
              :placeholder="$t('common.请选择')"
            >
              <el-option
                v-for="item in protocolTypes"
                :key="item.value"
                :label="currentLocale === 'en' ? item.en_label : item.label"
                :value="item.value"
              />
            </el-select>
          </el-form-item>
          <el-form-item :label="$t('device.模版名称')">
            <el-input
              v-model="templateData.name"
              :placeholder="$t('device.请输入模版名称')"
            />
          </el-form-item>
          <el-form-item :label="$t('device.备注')">
            <el-input v-model="templateData.remark" type="textarea" />
          </el-form-item>
          <el-form-item>
            <el-button type="primary" @click="submitForm">{{
              $t("common.确认")
            }}</el-button>
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
            :action="upload.url + '?templateId=' + templateData.id"
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
              $t("upload.确 定")
            }}</el-button>
            <el-button @click="upload.open = false">{{
              $t("upload.取 消")
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
              {{ $t("common.导入") }}
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
    </el-tabs>
  </el-dialog>
</template>

<script>
import { getToken } from "@/utils/auth";
import {
  getTemplatePoints,
  addPoint,
  updatePoint,
  delPoint,
} from "@/api/system/template";
import { protocolTypes } from "@/constants/protocolTypes";
import { deviceTypes } from "@/constants/deviceTypes";
import i18n from "@/i18n"; // 引入i18n配置

export default {
  props: {
    value: { type: Boolean, required: true },
    editType: { type: String, default: "add" },
    templateData: { type: Object, default: null },
    templateType: { type: String, default: "device" },
    activeTab: { type: String, default: "templateConfig" },
  },
  data() {
    return {
      protocolTypes,
      deviceTypes,
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
        url: process.env.VUE_APP_BASE_API + "/template/import-points",
      },
      dialogVisible: false,
      isEdit: false,
      currentRow: {},
      tableData: [],
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
          width: "150",
          editable: true,
          type: "input",
          required: true,
        },
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
            { value: 1, label: this.$t("common.是") },
            { value: 0, label: this.$t("common.否") },
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
      loading: false,
    };
  },
  computed: {
    currentLocale() {
      return i18n.locale;
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
    visible: {
      get() {
        return this.value;
      },
      set(val) {
        this.$emit("input", val);
      },
    },
  },
  methods: {
    async fetchTemplatePoints() {
      if (!this.templateData.id) return;
      this.loading = true;
      try {
        const res = await getTemplatePoints(this.templateData.id, {});
        if (res.code === 200) {
          const rows = Array.isArray(res.rows) ? res.rows : (Array.isArray(res.data) ? res.data : []);
          this.tableData = rows;
        } else {
          this.$message.error(res.msg || this.$t("device.获取测点数据失败"));
        }
      } catch (error) {
        this.$message.error(this.$t("device.请求失败") + ": " + error.message);
      } finally {
        this.loading = false;
      }
    },

    handleClose() {
      this.$emit("close"); // 通知父组件关闭弹窗
    },

    // 导入按钮操作
    handleImport() {
      this.upload.title = this.$t("device.测点导入");
      this.upload.open = true;
    },

    // 下载模板操作
    importTemplate() {
      this.download(
        "template/import-points-file",
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
      this.fetchTemplatePoints();
    },

    // 提交上传文件
    submitFileForm() {
      this.$refs.upload.submit();
    },

    handleExport() {
      // 获取当前语言，默认为'zh'
      const lang = this.$i18n?.locale || "zh";
      this.download(
        `/template/export/?template_id=${this.templateData.id}&lang=${lang}`,
        {},
        `template_point_${new Date().getTime()}.xlsx`
      );
    },

    handleAddPoint() {
      const newPoint = {
        editing: true,
        templateId: this.templateData.id,
        protocolType: this.templateData.protocolType,
        devType: this.templateData.devType,
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
        this.$message.error(this.$t("common.请填写所有必填字段"));
        return;
      }
      try {
        let res;
        const { editing, ...params } = row; // 去掉editing字段
        params.templateId = this.templateData.id;
        params.protocolType = this.templateData.protocolType;
        if (this.templateData.devType != null && this.templateData.devType !== "") {
          params.devType = this.templateData.devType;
        }
        if (row.id) {
          res = await updatePoint(params);
        } else {
          res = await addPoint(params);
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
        this.$message.error(this.$t("common.请求失败") + ": " + error.message);
      }
    },

    async handleDelete(row) {
      try {
        const res = await delPoint(row.id);
        if (res.code === 200) {
          this.tableData = this.tableData.filter((item) => item !== row);
          this.$message.success(this.$t("common.删除成功"));
        } else {
          this.$message.error(res.msg || this.$t("common.删除失败"));
        }
      } catch (error) {
        this.$message.error(this.$t("common.请求失败") + ": " + error.message);
      }
    },

    submitForm() {
      this.templateData.type = this.templateType === "device" ? 0 : 1;
      if (this.templateType === "device" && (this.templateData.devType === "" || this.templateData.devType == null)) {
        this.$message.warning(this.$t("device.请选择设备类型"));
        return;
      }
      if (this.editType === "add") {
        this.$emit("add", this.templateData);
      } else if (this.editType === "edit") {
        this.$emit("edit", this.templateData);
      }
      // this.$emit("close"); // 关闭弹窗
    },

    handleTabClick(tab) {
      if (tab.name === "pointConfig") {
        this.fetchTemplatePoints();
      }
    },

    getLabelByValue(options, value) {
      const option = options.find((opt) => opt.value === value);
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
.custom-table {
  min-width: 600px; /* 根据需要调整 */
  overflow-x: auto;
  margin-top: 20px;
  ::-webkit-scrollbar {
    height: 15px; /* 调整滚动条的高度 */
  }
}

.required-tip {
  color: #f56c6c;
  font-style: normal;
  margin-right: 4px;
}
</style>
