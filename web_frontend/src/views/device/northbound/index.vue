<template>
  <div class="northbound-management">
    <!-- 搜索栏 -->
    <el-form :inline="true" :model="searchForm" class="search-form">
      <el-form-item label="协议类型">
        <el-select
          v-model="searchForm.protocolType"
          placeholder="请选择协议类型"
        >
          <el-option
            v-for="item in protocolTypes"
            :key="item.value"
            :label="item.label"
            :value="item.value"
          />
        </el-select>
      </el-form-item>
      <el-form-item label="配置名称">
        <el-input
          v-model="searchForm.name"
          placeholder="输入配置名称"
        ></el-input>
      </el-form-item>
      <el-form-item label="创建模板">
        <el-input
          v-model="searchForm.template"
          placeholder="输入创建模板"
        ></el-input>
      </el-form-item>
      <el-form-item>
        <el-button type="primary" @click="handleSearch">搜索</el-button>
        <el-button @click="handleReset">重置</el-button>
      </el-form-item>
    </el-form>
    <div class="toolbar">
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
          :action="upload.url"
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
            type="primary"
            plain
            icon="el-icon-download"
            size="mini"
            @click="handleImport"
          >
            导入
          </el-button>

          <el-button
            type="warning"
            plain
            icon="el-icon-download"
            size="mini"
            @click="handleExport"
            >导出</el-button
          >
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
            @click="handleBatchDelete"
            >批量删除</el-button
          >
        </el-col>
      </el-row>
    </div>
    <!-- 表格 -->
    <el-table
      :data="configList"
      border
      ref="configTable"
      style="width: 100%"
      @selection-change="handleSelectionChange"
    >
      <el-table-column type="selection" width="55" />
      <el-table-column
        prop="protocolType"
        label="协议类型"
        :formatter="formatProtocolType"
      />
      <el-table-column prop="name" label="配置名称" />
      <el-table-column prop="template" label="创建模板" />
      <el-table-column label="操作" width="220">
        <template slot-scope="scope">
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

    <NorthBoundDialog
      ref="northBoundDialog"
      v-bind:visible="isDialogVisible"
      @update:visible="handleDialogClose"
      :configData="currentConfig"
      :edit-type="editType"
      @add="submitConfigAdd"
      @edit="submitConfigEdit"
    />
    <pagination
      v-show="pagination.total > 0"
      :total="pagination.total"
      :page.sync="pagination.page"
      :limit.sync="pagination.pageSize"
      @pagination="fetchConfigList"
    />
  </div>
</template>

<script>
import NorthBoundDialog from "../info/NorthBoundDialog.vue";
import { getToken } from "@/utils/auth";
import {
  listNorthBound,
  delNorthBound,
  addNorthBound,
  updateNorthBound,
} from "@/api/system/northbound";
import Pagination from "@/components/Pagination";
import { protocolTypes } from "@/constants/protocolTypes";

export default {
  components: { NorthBoundDialog, Pagination },
  data() {
    return {
      protocolTypes,
      // 显示搜索条件
      showSearch: true,
      editType: "add",
      searchForm: {
        protocolType: "",
        name: "",
        template: "",
      },
      upload: {
        open: false, // 是否显示弹出层
        title: "", // 弹出层标题
        isUploading: false, // 是否禁用上传
        headers: { Authorization: "Bearer " + getToken() }, // 设置上传的请求头部
        url: process.env.VUE_APP_BASE_API + "/config/import-configs", // 上传的地址
      },
      configList: [], // 表格数据
      selectedRows: [], // 已选中行
      pagination: {
        page: 1,
        pageSize: 10,
        total: 0,
      },
      isDialogVisible: false,
      dialogMode: "view", // 弹窗模式 "view" 或 "edit"
      currentConfig: {}, // 当前配置数据
    };
  },
  methods: {
    async fetchConfigList() {
      this.loading = true;
      const { page, pageSize } = this.pagination;
      // 调用接口获取数据
      const response = await listNorthBound({
        pageNum: page,
        pageSize: pageSize,
        ...this.searchForm,
      });
      const rows = Array.isArray(response.rows) ? response.rows : (Array.isArray(response.data) ? response.data : []);
      this.configList = rows;
      this.pagination.total = response.total;
      this.loading = false;
    },

    handleSearch() {
      this.pagination.page = 1; // 搜索时重置分页
      this.fetchConfigList();
    },
    handleReset() {
      this.searchForm = {
        protocolType: "",
        name: "",
        template: "",
      };
      this.handleSearch();
    },
    handleSelectionChange(selected) {
      this.selectedRows = selected;
    },
    async handleBatchDelete() {
      try {
        await this.$confirm("确定要删除选中的配置吗？", "提示", {
          type: "warning",
        });
        const ids = this.selectedRows.map((row) => row.id).join(",");
        await delNorthBound(ids);
        this.$message.success("删除成功");
        this.fetchConfigList();
        this.selectedRows = [];
      } catch (error) {
        if (error !== "cancel") {
          this.$message.error("删除失败: " + error.message);
        } else {
          this.$message.info("已取消删除");
        }
      }
    },
    onAdd() {
      this.editType = "add";
      this.resetCurrentConfig();
      this.isDialogVisible = true;
    },
    onView(row) {
      this.editType = "view";
      this.resetCurrentConfig(row);
      this.isDialogVisible = true;
    },
    onEdit(row) {
      this.editType = "edit";
      this.resetCurrentConfig(row);
      this.isDialogVisible = true;
    },

    handleDialogClose(val) {
      this.isDialogVisible = val;
      if (!val) {
        this.$refs.northBoundDialog.activeTab = "connectionConfig"; // 关闭时重置 tab
        this.resetCurrentConfig(); // 关闭时清空表单内容
      }
    },

    async submitConfigAdd(configData) {
      try {
        await addNorthBound(configData);
        this.$message.success("配置添加成功");
        this.isDialogVisible = false;
        this.pagination.page = Math.ceil(
          (this.pagination.total + 1) / this.pagination.pageSize
        ); // 跳到最后一页
        this.fetchConfigList();
      } catch (error) {
        this.$message.error("配置添加失败: " + error.message);
      }
    },

    async submitConfigEdit(configData) {
      try {
        await updateNorthBound(configData);
        this.$message.success("配置修改成功");
        this.isDialogVisible = false;
        this.fetchConfigList();
      } catch (error) {
        this.$message.error("配置修改失败: " + error.message);
      }
    },

    async onDelete(row) {
      await this.$confirm(`确认删除配置 ${row.name} 吗？`, "提示", {
        type: "warning",
      });
      await delNorthBound(row.id);
      this.$message.success("删除成功");
      this.fetchConfigList();
    },

    /** 导入按钮操作 */
    handleImport() {
      this.upload.title = "配置导入";
      this.upload.open = true;
    },
    /** 下载模板操作 */
    importTemplate() {
      this.download(
        "config/import-configs-file",
        {},
        `config_template_${new Date().getTime()}.xlsx`
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
    /** 导出操作 */
    handleExport() {
      this.download(
        "config/export-all-configs",
        {},
        `config_${new Date().getTime()}.xlsx`
      );
    },
    formatProtocolType(config) {
      return this.protocolTypes.find(
        (item) => item.value === config.protocolType
      )?.label;
    },
    resetCurrentConfig(row = {}) {
      this.currentConfig = {
        protocolType: "",
        name: "",
        template: "",
        ...row,
      };
    },
  },
  mounted() {
    this.fetchConfigList();
  },
};
</script>

<style scoped>
.northbound-management {
  padding: 20px;
}
.search-form {
  margin-bottom: 0px !important;
  padding-bottom: 0px !important;
}
.toolbar {
  display: flex;
  gap: 10px;
  margin-bottom: 15px;
}
</style>
