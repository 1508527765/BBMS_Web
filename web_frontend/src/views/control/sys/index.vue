<template>
  <div class="config-page">
    <!-- 操作按钮 -->
    <div class="toolbar">
      <el-tooltip
        :content="
          selectedIds.length === 0
            ? $t('control.请先选择数据')
            : hasValidationError
            ? $t('control.有数据超出范围，请检查后重新提交')
            : ''
        "
        placement="top"
        :disabled="selectedIds.length > 0 && !hasValidationError"
      >
        <el-button
          type="success"
          plain
          icon="el-icon-save"
          size="mini"
          :disabled="selectedIds.length === 0 || hasValidationError"
          @click="saveChanges"
          >{{ $t("common.保存") }}</el-button
        >
      </el-tooltip>

      <el-tooltip
        :content="
          selectedIds.length === 0
            ? $t('control.请先选择数据')
            : hasValidationError
            ? $t('control.有数据超出范围，请检查后重新提交')
            : ''
        "
        placement="top"
        :disabled="selectedIds.length > 0 && !hasValidationError"
      >
        <el-button
          type="warning"
          plain
          icon="el-icon-upload"
          size="mini"
          :disabled="selectedIds.length === 0 || hasValidationError"
          @click="openBatchDispatch"
          >{{ $t("common.批量下发") }}</el-button
        >
      </el-tooltip>

      <!-- 自定义分页组件 -->
      <div class="dsp-pagination">
        <el-button
          type="text"
          :disabled="currentGroup === 0"
          @click="prevGroup"
          style="vertical-align: middle; font-weight: bold"
        >
          &lt;&lt;
        </el-button>
        <span
          v-for="num in 4"
          :key="num + currentGroup * 4"
          class="dsp-page-item"
          :class="{
            active: currentPage === num + currentGroup * 4,
            disabled: num + currentGroup * 4 > realDspCount,
          }"
          @click="handlePageChange(num + currentGroup * 4)"
          style="
            vertical-align: middle;
            display: inline-flex;
            align-items: center;
          "
        >
          PCS{{ num + currentGroup * 4 }}
        </span>
        <el-button
          type="text"
          :disabled="(currentGroup + 1) * 4 >= totalDSPs"
          @click="nextGroup"
          style="vertical-align: middle; font-weight: bold"
        >
          &gt;&gt;
        </el-button>
      </div>
      <div class="toolbar" style="margin-left: auto">
        <el-dialog
          :title="upload.title"
          :visible.sync="upload.open"
          width="400px"
          append-to-body
          :modal-append-to-body="false"
          :center="true"
        >
          <el-upload
            ref="upload"
            :limit="1"
            accept=".xlsx, .xls"
            :headers="upload.headers"
            :action="upload.url + '?settingType=sys' + '&lang=' + currentLocale"
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
              <span>{{ $t("upload.仅允许导入xls、xlsx格式文件。") }}</span>
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
          <el-col :span="24" style="text-align: right">
            <el-button
              type="primary"
              plain
              icon="el-icon-upload"
              size="mini"
              @click="handleImport"
            >
              {{ $t("control.导入配置") }}
            </el-button>

            <el-button
              type="info"
              plain
              icon="el-icon-download"
              size="mini"
              @click="handleExport"
              >{{ $t("control.导出配置") }}</el-button
            >
          </el-col>
        </el-row>
      </div>
    </div>

    <!-- 主表格 -->
    <div style="overflow-x: auto; margin-top: 20px">
      <el-table
        :data="tableData"
        border
        stripe
        height="750"
        class="thick-scrollbar"
        style="
          width: 100%;
          border-radius: 8px;
          overflow: hidden;
          box-shadow: 0 4px 8px rgba(0, 0, 0, 0.1);
        "
        @selection-change="onSelectChange"
      >
        <el-table-column type="selection" width="55" />
        <el-table-column prop="index" :label="$t('control.序号')" width="80" />
        <el-table-column
          v-if="isAdmin"
          prop="can_id"
          :label="$t('control.CANid')"
          width="200"
        />
        <el-table-column
          :prop="currentLocale === 'en' ? 'param_ename' : 'param_name'"
          :label="$t('control.参数名称')"
          width="300"
        />
        <el-table-column
          v-if="isAdmin"
          prop="data_type"
          :label="$t('control.类型')"
          width="180"
        />
        <el-table-column
          v-if="isAdmin"
          prop="coefficient"
          :label="$t('control.系数')"
          width="120"
        />
        <el-table-column :label="$t('control.值')" width="300">
          <template slot-scope="scope">
            <component
              :is="scope.row.ui_type === 'select' ? 'el-select' : 'el-input'"
              v-model="scope.row.edit_value"
              v-bind="getComponentProps(scope.row)"
              size="small"
              style="width: 160px"
              :placeholder="$t('control.请输入')"
              :disabled="!selectedIds.includes(scope.row.id)"
              @blur="validateValue(scope.row)"
            >
              <el-option
                v-for="opt in scope.row.value_config.options || []"
                :key="opt.value"
                :label="opt.label"
                :value="opt.value"
              />
            </component>
            <el-tooltip
              v-if="scope.row.validationError"
              :content="$t('control.输入值超出范围或小数位数不正确')"
              placement="top"
            >
              <i
                class="el-icon-warning"
                style="color: red; margin-left: 5px"
              ></i>
            </el-tooltip>
          </template>
        </el-table-column>
        <el-table-column :label="$t('control.范围说明')">
          <template slot-scope="scope">
            <div>
              <span v-if="scope.row.ui_type === 'select'">
                {{ getSelectRangeText(scope.row.value_config.options) }}
              </span>
              <span v-else>
                [{{ scope.row.value_config.min }}~{{
                  scope.row.value_config.max
                }}]
                {{ scope.row.value_config.unit || "" }}
              </span>
            </div>
          </template>
        </el-table-column>
      </el-table>
    </div>

    <!-- 批量下发弹窗 -->
    <el-dialog
      :title="$t('control.批量下发')"
      :visible.sync="batchDispatchVisible"
      width="50%"
    >
      <!-- CAN通道1 -->
      <div
        class="channel-section"
        style="
          background-color: #f0f9ff;
          padding: 10px;
          border-radius: 8px;
          margin-bottom: 20px;
        "
      >
        <div style="display: flex; align-items: center; margin-bottom: 10px">
          <h3 style="color: #409eff; margin-right: 20px">
            {{ $t("control.CAN通道1") }}
          </h3>
          <el-checkbox
            v-model="selectAllDSPs[0]"
            @change="handleSelectAllDSPs(0, $event)"
            >{{ $t("control.全选") }}</el-checkbox
          >
        </div>
        <el-checkbox-group v-model="selectedDSPs[0]">
          <div style="display: flex; flex-wrap: wrap">
            <el-checkbox
              v-for="num in dspDevices[0] || []"
              :key="'can1-' + num"
              :label="num"
              style="width: 12.5%; margin-bottom: 8px"
            >
              PCS{{ num }}
            </el-checkbox>
          </div>
        </el-checkbox-group>
      </div>

      <!-- CAN通道2 -->
      <div
        class="channel-section"
        style="background-color: #f0f9ff; padding: 10px; border-radius: 8px"
      >
        <div style="display: flex; align-items: center; margin-bottom: 10px">
          <h3 style="color: #409eff; margin-right: 20px">
            {{ $t("control.CAN通道2") }}
          </h3>
          <el-checkbox
            v-model="selectAllDSPs[1]"
            @change="handleSelectAllDSPs(1, $event)"
            >{{ $t("control.全选") }}</el-checkbox
          >
        </div>
        <el-checkbox-group v-model="selectedDSPs[1]">
          <div style="display: flex; flex-wrap: wrap">
            <el-checkbox
              v-for="num in dspDevices[1] || []"
              :key="'can2-' + num"
              :label="num"
              style="width: 12.5%; margin-bottom: 8px"
            >
              PCS{{ num }}
            </el-checkbox>
          </div>
        </el-checkbox-group>
      </div>

      <span slot="footer" class="dialog-footer">
        <el-button @click="batchDispatchVisible = false">{{
          $t("common.取消")
        }}</el-button>
        <el-button type="primary" @click="dispatchChanges">{{
          $t("common.下发")
        }}</el-button>
      </span>
    </el-dialog>
  </div>
</template>

<script>
import {
  fetchConfigList,
  updateConfigValues,
  fetchDspDevices,
} from "@/api/system/controlstrategy";
import { getToken } from "@/utils/auth";
import i18n from "@/i18n";

export default {
  data() {
    return {
      tableData: [],
      selectedIds: [],
      batchEditVisible: false,
      batchDispatchVisible: false,
      currentPage: 1,
      realDspCount: 0,
      totalDSPs: 16,
      currentGroup: 0,
      canChannelLengths: {},
      selectedDSPs: [[], []], // 修改为二维数组
      dspDevices: {}, // 存储DSP设备信息
      selectAllDSPs: [false, false], // 用于全选的状态
      hasValidationError: false, // 用于标记是否有验证错误
      upload: {
        open: false, // 是否显示弹出层
        title: "", // 弹出层标题
        isUploading: false, // 是否禁用上传
        updateSupport: false, // 是否更新已经存在的用户数据
        headers: { Authorization: "Bearer " + getToken() }, // 设置上传的请求头部
        url: process.env.VUE_APP_BASE_API + "/control-strategy/import-configs", // 上传的地址
      },
      currentLocale: i18n.locale,
    };
  },
  computed: {
    selectedRows() {
      return this.tableData.filter((row) => this.selectedIds.includes(row.id));
    },
    isAdmin() {
      const roles = this.$store.getters.roles || [];
      return roles.includes("admin");
    },
  },
  methods: {
    formatValue(value, coefficient, ui_type, value_config) {
      if (ui_type === "select") {
        return value;
      }
      if (!coefficient || coefficient <= 0) return value;
      // 计算应保留的小数位
      let decimalPlaces = 0;
      if (typeof coefficient === "number" && !isNaN(coefficient)) {
        if (coefficient === 1) {
          decimalPlaces = 0;
        } else {
          // 1000 -> 3, 100 -> 2, 10 -> 1, 1->0
          // 所以通过Math.log10获取
          decimalPlaces = Math.round(Math.log10(coefficient));
        }
      }
      // 四舍五入
      let num = Number(value);
      if (isNaN(num)) return value;
      // 只处理有限小数，避免科学计数展示
      return num.toFixed(decimalPlaces);
    },
    async fetchData() {
      try {
        const res = await fetchConfigList("sys", this.currentPage);
        this.tableData = res.data.map((item, index) => {
          const value_config = JSON.parse(item.value_config);
          const edit_value = this.formatValue(
            Number(item.current_value),
            Number(item.coefficient),
            item.ui_type,
            value_config
          );
          return {
            ...item,
            index: index + 1,
            value_config,
            edit_value,
            current_value: edit_value,
            validationError: false,
          };
        });
      } catch (err) {
        this.$message.error(this.$t("control.获取数据失败"));
      }
    },
    async fetchDspCount() {
      try {
        const dspDict = await fetchDspDevices();
        this.dspDevices = dspDict.data; // 存储DSP设备信息
        for (const [channel, devices] of Object.entries(this.dspDevices)) {
          this.canChannelLengths[channel] = devices.length;
        }
        this.realDspCount = Object.values(dspDict.data).flat().length;
      } catch (err) {
        this.$message.error(this.$t("control.获取DSP数量失败"));
      }
    },
    onSelectChange(selection) {
      this.selectedIds = selection.map((row) => row.id);
    },
    getSelectRangeText(options) {
      return options.map((opt) => `${opt.value}-${opt.label}`).join("，");
    },
    getValueLabel(options, value) {
      const option = options.find((opt) => Number(opt.value) === Number(value));
      return option ? option.label : value;
    },
    getComponentProps(row) {
      if (row.ui_type === "select") return {};
      return {
        min: row.value_config.min,
        max: row.value_config.max,
        step: row.value_config.step,
      };
    },
    openBatchEdit() {
      this.batchEditVisible = true;
    },
    openBatchDispatch() {
      if (this.hasValidationError) {
        this.$message.error(
          this.$t("control.有数据超出范围，请检查后重新提交")
        );
        return;
      }
      this.batchDispatchVisible = true;
    },
    validateValue(row) {
      if (row.ui_type === "select") {
        const validOption = row.value_config.options.some(
          (opt) => opt.value === row.edit_value
        );
        row.validationError = !validOption;
        this.hasValidationError = this.tableData.some(
          (row) => row.validationError
        );
        return;
      }

      const decimalPlaces = Math.max(
        0,
        Math.floor(Math.log10(row.coefficient))
      );
      const valueDecimalPlaces = (row.edit_value.toString().split(".")[1] || "")
        .length;
      if (
        row.edit_value < row.value_config.min ||
        row.edit_value > row.value_config.max ||
        valueDecimalPlaces > decimalPlaces
      ) {
        row.validationError = true;
        this.hasValidationError = true;
      } else {
        row.validationError = false;
        this.hasValidationError = this.tableData.some(
          (row) => row.validationError
        );
      }
    },
    async saveChanges() {
      if (this.hasValidationError) {
        this.$message.error(
          this.$t("control.有数据超出范围，请检查后重新提交")
        );
        return;
      }
      const updated = this.selectedRows.map((row) => ({
        point_id: row.point_id,
        new_value: parseFloat(row.edit_value),
      }));
      try {
        const canChannel = Object.keys(this.dspDevices).find((channel) =>
          this.dspDevices[channel].includes(this.currentPage)
        );
        await updateConfigValues({
          type: "sys",
          update_data: updated,
          update_device_list: [
            {
              can_channel: canChannel,
              device_code: [this.currentPage],
              can_count: this.canChannelLengths[canChannel],
            },
          ],
        });
        this.$message.success(this.$t("control.更新成功"));
        this.batchEditVisible = false;
        this.fetchData();
        this.selectedIds = [];
      } catch (err) {
        this.$message.error(this.$t("control.更新失败"));
      }
    },
    async dispatchChanges() {
      if (this.hasValidationError) {
        this.$message.error(
          this.$t("control.有数据超出范围，请检查后重新提交")
        );
        return;
      }
      const updated = this.selectedRows.map((row) => ({
        point_id: row.point_id,
        new_value: parseFloat(row.edit_value),
      }));
      try {
        // 分别处理两个CAN通道的下发
        const update_device_list = [];
        for (let i = 0; i < this.selectedDSPs.length; i++) {
          if (this.selectedDSPs[i].length > 0) {
            const data = {
              can_channel: i,
              can_count: this.canChannelLengths[i],
              device_code: this.selectedDSPs[i],
            };
            update_device_list.push(data);
          }
        }
        await updateConfigValues({
          update_data: updated,
          type: "sys",
          update_device_list: update_device_list,
        });
        this.$message.success(this.$t("control.批量下发成功"));
        this.batchDispatchVisible = false;
        this.selectedDSPs = [[], []];
      } catch (err) {
        this.$message.error(this.$t("control.批量下发失败"));
      }
    },
    handlePageChange(pageNum) {
      if (pageNum > this.realDspCount) return;
      this.currentPage = pageNum;
      this.fetchData(); // 重新获取该页数据
    },
    prevGroup() {
      if (this.currentGroup > 0) {
        this.currentGroup--;
      }
    },
    nextGroup() {
      if ((this.currentGroup + 1) * 4 < this.totalDSPs) {
        this.currentGroup++;
      }
    },
    handleSelectAllDSPs(channel, val) {
      if (val) {
        this.selectedDSPs[channel] = Array.from(
          { length: this.dspDevices[channel].length },
          (_, i) => this.dspDevices[channel][i]
        );
      } else {
        this.selectedDSPs[channel] = [];
      }
    },
    /** 导入按钮操作 */
    handleImport() {
      this.upload.title = this.$t("upload.配置导入");
      this.upload.open = true;
    },
    /** 下载模板操作 */
    importTemplate() {
      this.download(
        "control-strategy/import-configs-file",
        {},
        `sys_template_${new Date().getTime()}.xlsx`
      );
    },
    /** 文件上传中处理 */
    handleFileUploadProgress(event, file, fileList) {
      this.upload.isUploading = true;
    },
    /** 文件上传成功处理 */
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
      this.batchDispatchVisible = true; // 展示批量下发弹窗
    },
    /** 提交上传文件 */
    submitFileForm() {
      this.$refs.upload.submit();
    },
    /** 导出操作 */
    handleExport() {
      this.download(
        "control-strategy/export-configs/sys/" + this.currentLocale,
        {},
        `sys_${new Date().getTime()}.xlsx`
      );
    },
  },
  async mounted() {
    await this.fetchDspCount();
    this.fetchData();
  },
};
</script>

<style scoped>
.config-page {
  padding: 20px;
}
.toolbar {
  display: flex;
  align-items: center;
  gap: 12px;
}
.el-table th {
  font-weight: bold;
  background-color: #e0e0e0;
  padding-top: 10px;
  padding-left: 10px;
  height: 50px;
}
.el-dialog__footer {
  text-align: right;
}
.thick-scrollbar ::-webkit-scrollbar {
  width: 12px;
}
.thick-scrollbar ::-webkit-scrollbar-thumb {
  border-radius: 6px;
}
.dsp-pagination {
  display: inline-flex;
  margin-left: 20px;
  gap: 8px;
  align-items: center;
}

.dsp-pagination .el-button {
  font-weight: bold;
  padding: 4px 10px;
  font-size: 16px;
  color: #7bbfc0;
  transition: 0.2s;
}

.dsp-pagination .el-button:disabled {
  color: #c0c4cc;
  cursor: not-allowed;
}

.dsp-page-item {
  padding: 6px 12px;
  border-radius: 6px;
  cursor: pointer;
  user-select: none;
  transition: 0.2s;
  font-size: 14px;
  color: #606266;
  border: 1px solid transparent;
}

.dsp-page-item:hover {
  color: #7bbfc0;
  border-color: #7bbfc0;
  background-color: #f0f9ff;
}

.dsp-page-item.active {
  color: #fff;
  background-color: #7bbfc0;
  font-weight: bold;
  border-color: #7bbfc0;
}

.dsp-page-item.disabled {
  color: #c0c4cc;
  pointer-events: none;
  background-color: #f5f7fa;
  border-color: transparent;
}
</style>
