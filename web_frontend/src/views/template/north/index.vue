<template>
  <div style="margin-left: 20px">
    <div class="search-bar" style="margin-top: 20px">
      <el-form :inline="true" :model="searchForm">
        <el-form-item label="协议类型">
          <el-select v-model="searchForm.protocolType" placeholder="请选择">
            <el-option
              v-for="item in protocolTypes"
              :key="item.value"
              :label="item.label"
              :value="item.value"
            />
          </el-select>
        </el-form-item>
        <el-form-item label="模版名称">
          <el-input v-model="searchForm.name" placeholder="请输入模版名称" />
        </el-form-item>
        <el-form-item>
          <el-button type="primary" @click="onSearch">搜索</el-button>
          <el-button @click="onReset">重置</el-button>
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
          <div class="el-upload__text">将文件拖到此处，或<em>点击上传</em></div>
          <div class="el-upload__tip text-center" slot="tip">
            <span>仅允许导入xls、xlsx格式文件。</span>
            <el-link
              type="primary"
              :underline="false"
              style="font-size: 12px; vertical-align: baseline"
              @click="importTemplate"
              >下载模板</el-link
            >
          </div>
        </el-upload>
        <div slot="footer" class="dialog-footer">
          <el-button type="primary" @click="submitFileForm">确 定</el-button>
          <el-button @click="upload.open = false">取 消</el-button>
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
            导入
          </el-button>
          <el-button
            type="primary"
            plain
            icon="el-icon-plus"
            size="mini"
            @click="onAdd"
            >新增</el-button
          >
          <el-button
            type="danger"
            plain
            icon="el-icon-delete"
            size="mini"
            :disabled="!selectedRows.length"
            @click="onBatchDelete"
            >批量删除</el-button
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
        prop="protocolType"
        label="协议类型"
        :formatter="formatProtocolType"
      />
      <el-table-column prop="name" label="模版名称" />
      <el-table-column prop="remark" label="备注" />
      <el-table-column label="操作" width="250">
        <template #default="scope">
          <el-button size="mini" @click="onView(scope.row)">查看</el-button>
          <el-button size="mini" type="primary" @click="onEdit(scope.row)"
            >编辑</el-button
          >
          <el-button size="mini" type="danger" @click="onDelete(scope.row)"
            >删除</el-button
          >
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
import TemplateConfigDialog from "../manage/TempateConfigDialog.vue";
import { getToken } from "@/utils/auth";
import {
  listTemplate,
  delTemplate,
  addTemplate,
  updateTemplate,
} from "@/api/system/template";
import { protocolTypes } from "@/constants/protocolTypes";

export default {
  components: { TemplateConfigDialog },
  data() {
    return {
      protocolTypes,
      total: 0,
      searchForm: {
        pageNum: 1,
        pageSize: 10,
        type: 1,
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
      templateType: "north", // 新增的templateType属性
    };
  },
  methods: {
    getList() {
      this.loading = true;
      listTemplate(this.searchForm).then((response) => {
        const rows = Array.isArray(response.rows) ? response.rows : (Array.isArray(response.data) ? response.data : []);
        this.tableData = rows;
        this.total = response.total;
        this.loading = false;
      });
    },
    onSearch() {
      this.getList();
    },
    onReset() {
      this.searchForm = {
        protocolType: "",
        name: "",
        type: 1,
        pageNum: 1,
        pageSize: 10,
      };
      this.getList();
    },
    async onDelete(row) {
      try {
        await this.$confirm("是否确定删除？", "提示", { type: "warning" });
        await delTemplate(row.id);
        this.$message.success("删除成功");
        this.getList();
      } catch (error) {
        if (error !== "cancel") {
          this.$message.error("删除失败: " + error.message);
        }
      }
    },
    async onBatchDelete() {
      try {
        await this.$confirm("是否确定批量删除？", "提示", { type: "warning" });
        const ids = this.selectedRows.map((row) => row.id).join(",");
        await delTemplate(ids);
        this.$message.success("批量删除成功");
        this.getList();
      } catch (error) {
        if (error !== "cancel") {
          this.$message.error("批量删除失败: " + error.message);
        }
      }
    },
    onAdd() {
      this.editType = "add";
      this.currentTemplate = {};
      this.templateType = "north";
      this.dialogVisible = true;
    },
    async submitTemplateAdd(templateData) {
      try {
        await addTemplate(templateData);
        this.$message.success("模板添加成功");
        this.dialogVisible = false;
        this.getList();
      } catch (error) {
        this.$message.error("模板添加失败: " + error.message);
      }
    },
    async submitTemplateEdit(templateData) {
      try {
        await updateTemplate(templateData);
        this.$message.success("模板修改成功");
        this.dialogVisible = false;
        this.getList();
      } catch (error) {
        this.$message.error("模板修改失败: " + error.message);
      }
    },
    onEdit(row) {
      this.editType = "edit";
      this.currentTemplate = { ...row };
      this.templateType = "north";
      this.dialogVisible = true;
    },
    onView(row) {
      this.editType = "view";
      this.currentTemplate = { ...row };
      this.templateType = "north";
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
      return protocolType ? protocolType.label : "";
    },

    //导入相关操作
    handleImport() {
      this.upload.title = "测点导入";
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
        "导入结果",
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
