<template>
  <div class="device-management">
    <!-- 搜索栏和导出按钮在同一排 -->
    <el-form :inline="true" :model="searchForm" class="search-form">
      <el-form-item :label="$t('device.设备类型')">
        <el-select
          v-model="searchForm.dev_type"
          :placeholder="$t('device.请选择设备类型')"
          @change="fetchDeviceCodes"
        >
          <el-option
            v-for="item in deviceTypes"
            :key="item.value"
            :label="currentLocale === 'en' ? item.en_label : item.label"
            :value="item.value"
          />
        </el-select>
      </el-form-item>
      <el-form-item :label="$t('device.设备编号')">
        <el-select
          v-model="searchForm.dev_code"
          :placeholder="$t('device.请选择设备编号')"
        >
          <el-option
            v-for="device in deviceList"
            :key="device.device_code"
            :label="`${device.device_code}#${device.device_name}`"
            :value="device.device_code"
          />
        </el-select>
      </el-form-item>
      <el-form-item :label="$t('history.选择日期')">
        <el-date-picker
          v-model="searchForm.date"
          type="date"
          :placeholder="$t('history.选择日期')"
          :picker-options="datePickerOptions"
          @change="handleDateChange"
        />
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
            icon="el-icon-download"
            size="mini"
            :disabled="searchForm.dev_type === null || searchForm.date === null"
            @click="handleExport"
            >{{ $t("common.导出") }}</el-button
          >
          <el-tooltip
            :content="$t('history.导出所选时间当天的数据记录')"
            placement="top"
          >
            <i class="el-icon-question" style="margin-left: 5px"></i>
          </el-tooltip>
        </el-col>
      </el-row>
    </div>
    <!-- 表格 -->
    <el-table
      :data="savedDataList"
      border
      ref="deviceTable"
      style="width: 100%"
      v-loading="loading"
    >
      <el-table-column
        prop="devType"
        :label="$t('history.设备类型')"
        :formatter="formatDeviceType"
      />
      <el-table-column prop="devId" :label="$t('device.设备ID')" />
      <el-table-column prop="devName" :label="$t('device.设备名称')" />
      <el-table-column
        prop="pointName"
        :label="$t('device.测点名称')"
        :formatter="formatPointName"
      />
      <el-table-column prop="storageInterval" :label="$t('device.存储间隔')" />
      <el-table-column prop="timedValue" :label="$t('device.值')" />
      <el-table-column prop="createdTime" :label="$t('device.创建时间')" />
    </el-table>

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
import { listTimedData } from "@/api/system/history";
import { getDeviceListByType } from "@/api/system/device";
import Pagination from "@/components/Pagination";
import { deviceTypes, findDeviceTypeByValue } from "@/constants/deviceTypes";
import { Message, Loading } from "element-ui";
import i18n from "@/i18n"; // 引入i18n配置

export default {
  components: { Pagination },
  data() {
    const today = new Date();
    return {
      searchForm: {
        dev_code: null,
        dev_type: null,
        dev_name: null,
        date: today,
        start_time: Math.floor(
          new Date(
            today.getFullYear(),
            today.getMonth(),
            today.getDate(),
            0,
            0,
            0
          ).getTime() / 1000
        ),
        end_time: Math.floor(
          new Date(
            today.getFullYear(),
            today.getMonth(),
            today.getDate(),
            23,
            59,
            59
          ).getTime() / 1000
        ),
      },
      deviceTypes: deviceTypes,
      deviceList: [], // 设备编号列表
      savedDataList: [], // 表格数据
      pagination: {
        page: 1,
        pageSize: 10,
        total: 0,
      },
      datePickerOptions: {
        disabledDate(time) {
          return time.getTime() > today.getTime();
        },
      },
      loading: false, // 增加loading状态
    };
  },
  computed: {
    currentLocale() {
      return i18n.locale;
    },
  },
  methods: {
    formatPointName(row) {
      return this.currentLocale === "en" && row.pointEname
        ? row.pointEname
        : row.pointName;
    },
    async fetchDeviceList() {
      if (this.searchForm.dev_type === null || !this.searchForm.dev_code) {
        this.savedDataList = [];
        this.pagination.total = 0;
        return;
      }
      if (
        this.searchForm.start_time == null ||
        this.searchForm.end_time == null
      ) {
        Message.warning(this.$t("history.请选择完整查询日期"));
        this.savedDataList = [];
        this.pagination.total = 0;
        return;
      }
      this.loading = true;
      try {
        const d = this.searchForm.date || new Date();
        const dateStr = `${d.getFullYear()}-${String(d.getMonth() + 1).padStart(
          2,
          "0"
        )}-${String(d.getDate()).padStart(2, "0")}`;
        const picked = this.deviceList.find(
          (x) => x.device_code === this.searchForm.dev_code
        );
        const query = {
          dev_type: this.searchForm.dev_type,
          dev_code: this.searchForm.dev_code,
          dev_name: picked ? picked.device_name : "",
          date: dateStr,
          start_time: this.searchForm.start_time,
          end_time: this.searchForm.end_time,
          page_num: this.pagination.page,
          page_size: this.pagination.pageSize,
        };
        const response = await listTimedData(query);
        const pageData = response?.data || {};
        this.savedDataList = Array.isArray(pageData.rows) ? pageData.rows : [];
        this.pagination.total = Number(pageData.total || 0);
        this.searchForm.dev_name = picked ? picked.device_name : null;
      } catch (e) {
        this.savedDataList = [];
        this.pagination.total = 0;
        Message.error(this.$t("history.获取历史数据失败"));
      } finally {
        this.loading = false;
      }
    },

    async fetchDeviceCodes() {
      this.searchForm.dev_code = null; // 先将已选择的设备编号清空
      if (this.searchForm.dev_type !== null) {
        const response = await getDeviceListByType(this.searchForm.dev_type);
        this.deviceList = response.data.map((device) => ({
          device_code: device.device_code,
          device_name: device.device_name,
        }));
      } else {
        this.deviceList = [];
      }
    },

    handleSearch() {
      if (this.searchForm.dev_type === null) {
        Message.warning(this.$t("device.请选择设备类型"));
        return;
      }
      if (!this.searchForm.dev_code) {
        Message.warning(this.$t("device.请选择设备编号"));
        return;
      }
      this.pagination.page = 1; // 搜索时重置分页
      this.fetchDeviceList();
    },
    handleReset() {
      const today = new Date();
      this.searchForm = {
        dev_code: null,
        dev_type: null,
        date: today,
        start_time: Math.floor(
          new Date(
            today.getFullYear(),
            today.getMonth(),
            today.getDate(),
            0,
            0,
            0
          ).getTime() / 1000
        ),
        end_time: Math.floor(
          new Date(
            today.getFullYear(),
            today.getMonth(),
            today.getDate(),
            23,
            59,
            59
          ).getTime() / 1000
        ),
      };
      this.deviceList = [];
      this.handleSearch();
    },
    handleDateChange(date) {
      if (date) {
        const start = Math.floor(
          new Date(
            date.getFullYear(),
            date.getMonth(),
            date.getDate(),
            0,
            0,
            0
          ).getTime() / 1000
        );
        const end = Math.floor(
          new Date(
            date.getFullYear(),
            date.getMonth(),
            date.getDate(),
            23,
            59,
            59
          ).getTime() / 1000
        );

        this.searchForm.start_time = start;
        this.searchForm.end_time = end;
      } else {
        this.searchForm.start_time = null;
        this.searchForm.end_time = null;
      }
    },
    async handleExport() {
      const loadingInstance = Loading.service({
        lock: true,
        text: this.$t("upload.正在导出数据请稍候"),
        spinner: "el-icon-loading",
        background: "rgba(0, 0, 0, 0.7)",
      });

      try {
        const date = this.searchForm.date;
        const dateStr = `${date.getFullYear()}-${(date.getMonth() + 1)
          .toString()
          .padStart(2, "0")}-${date.getDate().toString().padStart(2, "0")}`;

        await this.download(
          "/history/export-timed-data",
          {
            date: dateStr,
            dev_code: this.searchForm.dev_code,
            dev_type: this.searchForm.dev_type,
            dev_name: this.searchForm.dev_name || "",
            start_time: this.searchForm.start_time,
            end_time: this.searchForm.end_time,
          },
          `${dateStr}.zip`
        );
        Message.success(this.$t("upload.数据导出成功"));
      } catch (error) {
        Message.error(this.$t("upload.数据导出失败，请重试"));
      } finally {
        loadingInstance.close();
      }
    },
    formatDeviceType(device) {
      const row = findDeviceTypeByValue(this.deviceTypes, device.devType);
      if (this.currentLocale === "en") return row?.en_label;
      return row?.label;
    },
  },
  mounted() {
    // 初始加载时不调用 fetchDeviceList
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
  display: flex;
  align-items: center;
}
.toolbar {
  display: flex;
  gap: 10px;
  margin-bottom: 15px;
}
</style>
