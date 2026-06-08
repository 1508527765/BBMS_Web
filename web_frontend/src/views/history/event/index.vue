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
          :disabled="!deviceList.length"
        >
          <el-option
            v-for="device in deviceList"
            :key="device.device_code"
            :label="`${device.device_code}#${device.device_name}`"
            :value="device.device_code"
          />
        </el-select>
      </el-form-item>
      <el-form-item :label="$t('history.选择日期范围')">
        <el-date-picker
          v-model="searchForm.dateRange"
          type="daterange"
          range-separator="-"
          :start-placeholder="$t('history.开始日期')"
          :end-placeholder="$t('history.结束日期')"
          :picker-options="datePickerOptions"
          @change="handleDateRangeChange"
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
            :disabled="!searchForm.dateRange || eventList.length === 0"
            @click="handleExport"
            >{{ $t("common.导出") }}</el-button
          >
          <el-tooltip
            :content="$t('upload.导出所选时间段内的数据记录')"
            placement="top"
          >
            <i class="el-icon-question" style="margin-left: 5px"></i>
          </el-tooltip>
        </el-col>
      </el-row>
    </div>
    <!-- 表格 -->
    <el-table
      :data="eventList"
      border
      ref="deviceTable"
      style="width: 100%"
      v-loading="loading"
    >
      <el-table-column
        prop="devType"
        :label="$t('device.设备类型')"
        :formatter="formatDeviceType"
      />
      <el-table-column prop="devId" :label="$t('device.设备ID')" />
      <el-table-column prop="devName" :label="$t('device.设备名称')" />
      <el-table-column
        prop="pointName"
        :label="$t('device.测点名称')"
        :formatter="formatPointName"
      />
      <el-table-column prop="faultsValue" :label="$t('device.值')" />
      <el-table-column prop="createdTime" :label="$t('device.创建时间')" />
    </el-table>

    <pagination
      v-show="pagination.total > 0"
      :total="pagination.total"
      :page.sync="pagination.page"
      :limit.sync="pagination.pageSize"
      @pagination="fetchEventList"
    />
  </div>
</template>

<script>
import { listEventData } from "@/api/system/history";
import { getDeviceListByType } from "@/api/system/device";
import Pagination from "@/components/Pagination";
import { deviceTypes, findDeviceTypeByValue } from "@/constants/deviceTypes";
import { Message, Loading } from "element-ui";
import i18n from "@/i18n"; // 引入i18n配置

export default {
  components: { Pagination },
  data() {
    const today = new Date();
    const maxDate = new Date();
    maxDate.setDate(today.getDate() - 180);
    return {
      searchForm: {
        dev_code: null,
        dev_type: null,
        dateRange: [today, today],
        start_time: Math.floor(
          new Date(today.setHours(0, 0, 0, 0)).getTime() / 1000
        ),
        end_time: Math.floor(
          new Date(today.setHours(23, 59, 59, 999)).getTime() / 1000
        ),
      },
      deviceTypes: deviceTypes,
      deviceList: [], // 设备编号列表
      eventList: [], // 表格数据
      pagination: {
        page: 1,
        pageSize: 10,
        total: 0,
      },
      datePickerOptions: {
        disabledDate(time) {
          return (
            time.getTime() < maxDate.getTime() ||
            time.getTime() > today.getTime()
          );
        },
      },
      loading: false, // 添加loading状态
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
    async fetchEventList() {
      this.loading = true;
      try {
        const query = {
          dev_code: this.searchForm.dev_code,
          dev_type: this.searchForm.dev_type,
          start_time: this.searchForm.start_time,
          end_time: this.searchForm.end_time,
          type: "event",
          page_num: this.pagination.page,
          page_size: this.pagination.pageSize,
        };
        const response = await listEventData(query);
        const pageData = response?.data || {};
        this.eventList = Array.isArray(pageData.rows) ? pageData.rows : [];
        this.pagination.total = Number(pageData.total || 0);
      } catch (e) {
        this.eventList = [];
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
      if (this.searchForm.dateRange && this.searchForm.dateRange.length === 2) {
        const [start, end] = this.searchForm.dateRange;
        const diffTime = Math.abs(end - start);
        const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24));
        if (diffDays > 31) {
          Message.warning(this.$t("history.选择的日期范围不能超过一个月"));
          return;
        }
      }
      this.pagination.page = 1; // 搜索时重置分页
      this.fetchEventList();
    },
    handleReset() {
      const today = new Date();
      this.searchForm = {
        dev_code: null,
        dev_type: null,
        dateRange: [today, today],
        start_time: Math.floor(
          new Date(today.setHours(0, 0, 0, 0)).getTime() / 1000
        ),
        end_time: Math.floor(
          new Date(today.setHours(23, 59, 59, 999)).getTime() / 1000
        ),
      };
      this.deviceList = [];
      this.handleSearch();
    },
    handleDateRangeChange(dates) {
      if (dates && dates.length === 2) {
        const [start, end] = dates;
        start.setHours(0, 0, 0, 0);
        end.setHours(23, 59, 59, 999);
        this.searchForm.start_time = Math.floor(start.getTime() / 1000);
        this.searchForm.end_time = Math.floor(end.getTime() / 1000);
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
        const startDate = this.searchForm.dateRange[0]
          ? this.searchForm.dateRange[0].toISOString().split("T")[0]
          : "no_date";
        const endDate = this.searchForm.dateRange[1]
          ? this.searchForm.dateRange[1].toISOString().split("T")[0]
          : "no_date";
        await this.download(
          "/history/export-event-data",
          {
            start_time: this.searchForm.start_time,
            end_time: this.searchForm.end_time,
            dev_code: this.searchForm.dev_code,
            dev_type: this.searchForm.dev_type,
            type: "event",
          },
          `event_data_${startDate}-${endDate}.csv`
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
    this.fetchEventList(); // 初始加载时调用 fetchEventList
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
