<template>
  <div style="margin-left: 20px">
    <div class="search-bar" style="margin-top: 20px">
      <el-form :inline="true" :model="searchForm">
        <el-form-item :label="$t('device.设备类型')">
          <el-select
            v-model="searchForm.devType"
            clearable
            :placeholder="$t('common.请选择')"
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
            v-model="searchForm.protocolType"
            clearable
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
            v-model="searchForm.name"
            :placeholder="$t('device.请输入模版名称')"
          />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" @click="onSearch">{{
            $t("common.搜索")
          }}</el-button>
          <el-button @click="onReset">{{ $t("common.重置") }}</el-button>
        </el-form-item>
      </el-form>
    </div>

    <div class="table-operations" style="margin-bottom: 20px">
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
          accept=".xlsx, .xls"
          :headers="upload.headers"
          :action="upload.url + '?templateType=' + templateType"
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
        <el-col :span="24">
          <el-button
            type="success"
            plain
            icon="el-icon-upload"
            size="mini"
            @click="handleImport"
          >
            {{ $t("common.导入") }}
          </el-button>
          <el-button
            type="primary"
            plain
            icon="el-icon-plus"
            size="mini"
            @click="onAdd"
            >{{ $t("common.新增") }}</el-button
          >
          <el-button
            type="danger"
            plain
            icon="el-icon-delete"
            size="mini"
            :disabled="!selectedRows.length"
            @click="onBatchDelete"
            >{{ $t("common.批量删除") }}</el-button
          >
        </el-col>
      </el-row>
    </div>

    <el-table
      :data="tableData"
      border
      style="width: 100%"
      @selection-change="onSelectionChange"
    >
      <el-table-column type="selection" width="50" />
      <el-table-column
        prop="devType"
        :label="$t('device.设备类型')"
        :formatter="formatDeviceType"
      />
      <el-table-column
        prop="protocolType"
        :label="$t('device.协议类型')"
        :formatter="formatProtocolType"
      />
      <el-table-column prop="name" :label="$t('device.模版名称')" />
      <el-table-column prop="remark" :label="$t('device.备注')" />
      <el-table-column :label="$t('common.操作')" width="250">
        <template #default="scope">
          <el-button size="mini" @click="onView(scope.row)">{{
            $t("common.查看")
          }}</el-button>
          <el-button size="mini" type="primary" @click="onEdit(scope.row)">{{
            $t("common.编辑")
          }}</el-button>
          <el-button size="mini" type="danger" @click="onDelete(scope.row)">{{
            $t("common.删除")
          }}</el-button>
        </template>
      </el-table-column>
    </el-table>

    <pagination
      v-show="total > 0"
      :total="total"
      :page.sync="searchForm.pageNum"
      :limit.sync="searchForm.pageSize"
      @pagination="getList"
    />

    <template-config-dialog
      v-model="dialogVisible"
      :edit-type="editType"
      :template-data="currentTemplate"
      @add="submitTemplateAdd"
      @edit="submitTemplateEdit"
      @close="onDialogClose"
      :active-tab="activeTab"
      :template-type="templateType"
    />
  </div>
</template>

<script>
import TemplateConfigDialog from "./TempateConfigDialog.vue";
import { getToken } from "@/utils/auth";
import {
  listTemplate,
  delTemplate,
  addTemplate,
  updateTemplate,
} from "@/api/system/template";
import { protocolTypes } from "@/constants/protocolTypes";
import { deviceTypes } from "@/constants/deviceTypes";
import i18n from "@/i18n"; // 引入i18n配置

export default {
  components: { TemplateConfigDialog },
  data() {
    return {
      protocolTypes,
      deviceTypes,
      total: 0,
      searchForm: {
        pageNum: 1,
        pageSize: 10,
        type: 0,
        devType: "",
        protocolType: "",
        name: "",
      },
      upload: {
        open: false, // 是否显示弹出层（用户导入）
        title: "", // 弹出层标题（用户导入）
        isUploading: false, // 是否禁用上传
        headers: { Authorization: "Bearer " + getToken() }, // 设置上传的请求头部
        url: process.env.VUE_APP_BASE_API + "/template/import-templates", // 上传的地址
      },
      tableData: [],
      selectedRows: [],
      dialogVisible: false,
      editType: "add",
      currentTemplate: {},
      activeTab: "templateConfig", // 新增的activeTab属性
      templateType: "device", // 新增的templateType属性
    };
  },
  computed: {
    currentLocale() {
      return i18n.locale;
    },
  },
  methods: {
    getList() {
      this.loading = true;
      listTemplate(this.searchForm).then((response) => {
        const rows = Array.isArray(response.rows) ? response.rows : (Array.isArray(response.data) ? response.data : []);
        this.tableData = rows.map((r) => ({
          ...r,
          devType: r.devType ?? r.dev_type ?? null,
          protocolType: r.protocolType ?? r.protocol_type,
          name: r.name ?? "",
        }));
        this.total = response.total;
        this.loading = false;
      });
    },
    onSearch() {
      this.getList();
    },
    onReset() {
      this.searchForm = {
        devType: "",
        protocolType: "",
        name: "",
        type: 0,
        pageNum: 1,
        pageSize: 10,
      };
      this.getList();
    },
    async onDelete(row) {
      try {
        await this.$confirm(
          this.$t("common.是否确定删除"),
          this.$t("common.提示"),
          { type: "warning" }
        );
        await delTemplate(row.id);
        this.$message.success(this.$t("common.删除成功"));
        this.getList();
      } catch (error) {
        if (error !== "cancel") {
          this.$message.error(
            this.$t("common.删除失败") + ": " + error.message
          );
        }
      }
    },
    async onBatchDelete() {
      try {
        await this.$confirm(
          this.$t("device.是否确定批量删除"),
          this.$t("common.提示"),
          { type: "warning" }
        );
        const ids = this.selectedRows.map((row) => row.id).join(",");
        await delTemplate(ids);
        this.$message.success(this.$t("device.批量删除成功"));
        this.getList();
      } catch (error) {
        if (error !== "cancel") {
          this.$message.error(
            this.$t("device.批量删除失败") + ": " + error.message
          );
        }
      }
    },
    onAdd() {
      this.editType = "add";
      this.currentTemplate = {};
      this.templateType = "device";
      this.dialogVisible = true;
    },
    async submitTemplateAdd(templateData) {
      try {
        await addTemplate(templateData);
        this.$message.success(this.$t("device.模板添加成功"));
        this.dialogVisible = false;
        this.getList();
      } catch (error) {
        this.$message.error(
          this.$t("device.模板添加失败") + ": " + error.message
        );
      }
    },
    async submitTemplateEdit(templateData) {
      try {
        await updateTemplate(templateData);
        this.$message.success(this.$t("device.模板修改成功"));
        this.dialogVisible = false;
        this.getList();
      } catch (error) {
        this.$message.error(
          this.$t("device.模板修改失败") + ": " + error.message
        );
      }
    },
    onEdit(row) {
      this.editType = "edit";
      this.currentTemplate = { ...row };
      this.templateType = "device";
      this.dialogVisible = true;
    },
    onView(row) {
      this.editType = "view";
      this.currentTemplate = { ...row };
      this.templateType = "device";
      this.dialogVisible = true;
    },
    onSelectionChange(selectedRows) {
      this.selectedRows = selectedRows;
    },
    onDialogClose() {
      this.dialogVisible = false;
      this.currentTemplate = {}; // 当对话框关闭时，清空currentTemplate
    },
    formatProtocolType(row, column, cellValue, index) {
      const protocolType = this.protocolTypes.find(
        (item) => item.value === cellValue
      );
      if (this.currentLocale === "en") {
        return protocolType ? protocolType.en_label : "";
      } else {
        return protocolType ? protocolType.label : "";
      }
    },
    formatDeviceType(row, column, cellValue) {
      const raw = cellValue ?? row.devType ?? row.dev_type;
      if (raw === null || raw === undefined || raw === "") {
        return "";
      }
      const dt = this.deviceTypes.find((item) => Number(item.value) === Number(raw));
      if (!dt) return String(cellValue);
      return this.currentLocale === "en" ? dt.en_label : dt.label;
    },

    //导入相关操作
    handleImport() {
      this.upload.title = this.$t("device.测点导入");
      this.upload.open = true;
    },

    /** 下载模板操作 */
    importTemplate() {
      this.download(
        "template/import-templates-file",
        {},
        `template_point_${new Date().getTime()}.xlsx`
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
      this.getList();
    },

    /** 提交上传文件 */
    submitFileForm() {
      this.$refs.upload.submit();
    },
  },
  created() {
    this.getList();
  },
};
</script>
