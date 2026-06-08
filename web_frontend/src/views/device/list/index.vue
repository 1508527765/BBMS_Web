<template>
  <div class="device-management">
    <!-- 搜索栏 -->
    <el-form :inline="true" :model="searchForm" class="search-form">
      <el-form-item :label="$t('device.设备编号')">
        <el-input
          v-model="searchForm.code"
          :placeholder="$t('device.输入设备编号')"
        ></el-input>
      </el-form-item>

      <el-form-item :label="$t('device.设备类型')">
        <el-select
          v-model="searchForm.type"
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
      <el-form-item :label="$t('device.设备名称')">
        <el-input
          v-model="searchForm.name"
          :placeholder="$t('device.请输入设备名称')"
        ></el-input>
      </el-form-item>
      <el-form-item :label="$t('device.协议类型')">
        <el-select
          v-model="searchForm.protocolType"
          :placeholder="$t('device.请选择协议类型')"
        >
          <el-option
            v-for="item in protocolTypes"
            :key="item.value"
            :label="currentLocale === 'en' ? item.en_label : item.label"
            :value="item.value"
          />
        </el-select>
      </el-form-item>
      <el-form-item>
        <el-button type="primary" @click="handleSearch">{{
          $t("common.搜索")
        }}</el-button>
        <el-button @click="handleReset">{{ $t("common.重置") }}</el-button>
      </el-form-item>
    </el-form>
    <div class="toolbar">
      <el-row>
        <el-col :span="24">
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
            @click="handleBatchDelete"
            >{{ $t("common.批量删除") }}</el-button
          >
        </el-col>
      </el-row>
    </div>
    <!-- 表格 -->
    <el-table
      :data="deviceList"
      border
      ref="deviceTable"
      style="width: 100%"
      @selection-change="handleSelectionChange"
    >
      <el-table-column type="selection" width="55" :selectable="rowSelectable" />
      <el-table-column prop="name" :label="$t('device.设备名称')" />
      <el-table-column prop="code" :label="$t('device.设备编号')" />
      <el-table-column
        prop="type"
        :label="$t('device.设备类型')"
        :formatter="formatDeviceType"
      />
      <el-table-column
        prop="protocolType"
        :label="$t('device.协议类型')"
        :formatter="formatProtocolType"
      />
      <el-table-column prop="templateId" :label="$t('device.模板id')" />
      <el-table-column :label="$t('common.操作')" width="220">
        <template slot-scope="scope">
          <el-button size="mini" @click="onView(scope.row)">{{
            $t("common.查看")
          }}</el-button>
          <el-button size="mini" type="primary" @click="onEdit(scope.row)">{{
            $t("common.编辑")
          }}</el-button>
          <el-button size="mini" type="danger" :disabled="isLocalSystemRow(scope.row)" @click="onDelete(scope.row)">{{
            $t("common.删除")
          }}</el-button>
        </template>
      </el-table-column>
    </el-table>

    <DeviceFormDialog
      ref="deviceFormDialog"
      v-bind:visible="isDialogVisible"
      @update:visible="handleDialogClose"
      :deviceData="currentDevice"
      :edit-type="editType"
      @add="submitDeviceAdd"
      @edit="submitDeviceEdit"
    />
    <pagination
      v-show="pagination.total > 0"
      :total="pagination.total"
      :page.sync="pagination.page"
      :limit.sync="pagination.pageSize"
      @pagination="fetchDeviceList"
    />
  </div>
</template>

<script>
import DeviceFormDialog from "../info/DeviceFormDialog.vue";
import { getToken } from "@/utils/auth";
import {
  listDevice,
  delDevice,
  addDevice,
  updateDevice,
} from "@/api/system/device";
import Pagination from "@/components/Pagination";
import { protocolTypes } from "@/constants/protocolTypes";
import { deviceTypes, findDeviceTypeByValue } from "@/constants/deviceTypes";
import i18n from "@/i18n"; // 引入i18n配置

export default {
  components: { DeviceFormDialog, Pagination },
  data() {
    return {
      protocolTypes,
      // 显示搜索条件
      showSearch: true,
      editType: "add",
      searchForm: {
        code: "",
        name: "",
        type: "",
        protocolType: "",
      },
      upload: {
        open: false, // 是否显示弹出层
        title: "", // 弹出层标题
        isUploading: false, // 是否禁用上传
        updateSupport: false, // 是否更新已经存在的用户数据
        headers: { Authorization: "Bearer " + getToken() }, // 设置上传的请求头部
        url: process.env.VUE_APP_BASE_API + "/device/import-devices", // 上传的地址
      },
      deviceTypes: deviceTypes,
      deviceList: [], // 表格数据
      selectedRows: [], // 已选中行
      pagination: {
        page: 1,
        pageSize: 10,
        total: 0,
      },
      isDialogVisible: false,
      dialogMode: "view", // 弹窗模式 "view" 或 "edit"
      currentDevice: {}, // 当前设备数据
    };
  },
  computed: {
    currentLocale() {
      return i18n.locale;
    },
  },
  watch: {
    currentLocale() {
      this.fetchDeviceList();
    },
  },
  methods: {
    async fetchDeviceList() {
      this.loading = true;
      try {
        const query = {
          page_num: this.pagination.page,
          page_size: this.pagination.pageSize,
          code: this.searchForm.code || undefined,
          name: this.searchForm.name || undefined,
          type: this.searchForm.type === "" ? undefined : this.searchForm.type,
          protocol_type:
            this.searchForm.protocolType === "" ? undefined : this.searchForm.protocolType,
        };
        const res = await listDevice(query);
        this.deviceList = Array.isArray(res?.rows) ? res.rows : [];
        this.pagination.total = Number(res?.total || 0);
      } finally {
        this.loading = false;
      }
    },

    handleSearch() {
      this.pagination.page = 1; // 搜索时重置分页
      this.fetchDeviceList();
    },
    handleReset() {
      this.searchForm = {
        code: "",
        name: "",
        type: "",
        protocolType: "",
      };
      this.handleSearch();
    },
    handleSelectionChange(selected) {
      this.selectedRows = selected;
    },
    async handleBatchDelete() {
      try {
        await this.$confirm(
          this.$t("device.确定要删除选中的设备吗？"),
          this.$t("common.提示"),
          {
            type: "warning",
          }
        );
        const ids = this.selectedRows
          .filter((row) => !this.isLocalSystemRow(row))
          .map((row) => row.id)
          .join(",");
        if (!ids) {
          this.$message.warning(this.$t("device.localSystemNoDelete"));
          return;
        }
        await delDevice(ids);
        this.$message.success(this.$t("device.删除成功"));
        this.fetchDeviceList();
        this.selectedRows = [];
      } catch (error) {
        if (error !== "cancel") {
          this.$message.error(
            this.$t("device.删除失败") + ": " + error.message
          );
        } else {
          this.$message.info(this.$t("device.已取消删除"));
        }
      }
    },
    onAdd() {
      this.editType = "add";
      this.resetCurrentDevice();
      this.isDialogVisible = true;
    },
    onView(row) {
      this.editType = "view";
      this.resetCurrentDevice(row);
      this.isDialogVisible = true;
    },
    onEdit(row) {
      this.editType = "edit";
      this.resetCurrentDevice(row);
      this.isDialogVisible = true;
    },

    handleDialogClose(val) {
      this.isDialogVisible = val;
      if (!val) {
        this.$refs.deviceFormDialog.activeTab = "deviceConfig"; // 关闭时重置 tab
        this.resetCurrentDevice(); // 关闭时清空表单内容
      }
    },

    async submitDeviceAdd(deviceData) {
      try {
        await addDevice(deviceData);
        this.$message.success(this.$t("device.设备添加成功"));
        this.isDialogVisible = false;
        this.pagination.page = Math.ceil(
          (this.pagination.total + 1) / this.pagination.pageSize
        ); // 跳到最后一页
        this.fetchDeviceList();
      } catch (error) {
        this.$message.error(
          this.$t("device.设备添加失败") + ": " + error.message
        );
      }
    },

    async submitDeviceEdit(deviceData) {
      try {
        await updateDevice(deviceData);
        this.$message.success(this.$t("device.设备修改成功"));
        this.isDialogVisible = false;
        this.fetchDeviceList();
      } catch (error) {
        this.$message.error(
          this.$t("device.设备修改失败") + ": " + error.message
        );
      }
    },

    isLocalSystemRow(row) {
      if (!row) return false;
      if (Number(row.id) === 1) return true;
      if (Number(row.type) === 0) return true;
      return false;
    },
    rowSelectable(row) {
      return !this.isLocalSystemRow(row);
    },

    async onDelete(row) {
      if (this.isLocalSystemRow(row)) {
        this.$message.warning(this.$t("device.localSystemNoDelete"));
        return;
      }
      await this.$confirm(
        this.$t("device.确认删除设备") + ` ${row.name} ` + this.$t("device.吗"),
        this.$t("common.提示"),
        {
          type: "warning",
        }
      );
      await delDevice(row.id);
      this.$message.success(this.$t("device.删除成功"));
      this.fetchDeviceList();
    },

    formatDeviceType(device) {
      const row = findDeviceTypeByValue(this.deviceTypes, device.type);
      if (this.currentLocale === "en") return row?.en_label;
      return row?.label;
    },
    formatProtocolType(device) {
      if (this.currentLocale === "en") {
        return this.protocolTypes.find(
          (item) => item.value === device.protocolType
        )?.en_label;
      } else {
        return this.protocolTypes.find(
          (item) => item.value === device.protocolType
        )?.label;
      }
    },
    resetCurrentDevice(row = {}) {
      this.currentDevice = {
        name: "",
        type: "",
        protocolType: "",
        templateId: "",
        remark: "",
        uartId: "",
        baud: 0,
        parity: 0,
        data: 0,
        stop: 0,
        address: "",
        ip: "",
        port: 0,
        timeout: 0,
        uId: 0,
        canId: "",
        ...row,
      };
    },
  },
  mounted() {
    this.fetchDeviceList();
  },
};
</script>

<style scoped>
.device-management {
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
