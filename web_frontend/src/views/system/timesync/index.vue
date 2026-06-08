<template>
  <div style="margin-top: 10px; margin-left: 30px">
    <el-dialog :title="dialogTitle" :visible.sync="dialogVisible" width="30%">
      <el-form :model="form" label-width="150px" label-position="left">
        <el-form-item :label="$t('system.配置名称')">
          <el-input v-model="form.name" autocomplete="off"></el-input>
        </el-form-item>
        <el-form-item :label="$t('system.是否自动对时')">
          <el-radio-group v-model="form.isNtp">
            <el-radio :label="0">{{ $t("common.否") }}</el-radio>
            <el-radio :label="1">{{ $t("common.是") }}</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item :label="$t('system.NTP地址')">
          <el-input v-model="form.address" autocomplete="off"></el-input>
        </el-form-item>
        <el-form-item :label="$t('system.NTP端口号')">
          <el-input v-model="form.port" autocomplete="off"></el-input>
        </el-form-item>
        <el-form-item :label="$t('system.时间')">
          <el-time-picker
            v-model="form.manualTime"
            :placeholder="$t('system.选择时间')"
            format="HH:mm:ss"
            value-format="HH:mm:ss"
          >
          </el-time-picker>
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="dialogVisible = false">{{
            $t("common.取消")
          }}</el-button>
          <el-button type="primary" @click="submitForm">{{
            $t("common.确定")
          }}</el-button>
        </span>
      </template>
    </el-dialog>
    <el-table
      :data="RtcList"
      style="margin-top: 20px; width: 100%"
      @selection-change="handleSelectionChange"
    >
      <el-table-column type="selection" width="55"></el-table-column>
      <el-table-column
        prop="name"
        :label="$t('system.配置名称')"
      ></el-table-column>
      <el-table-column prop="isNtp" :label="$t('system.是否自动对时')">
        <template #default="scope">
          {{ scope.row.isNtp === 1 ? $t("common.是") : $t("common.否") }}
        </template>
      </el-table-column>
      <el-table-column
        prop="address"
        :label="$t('system.NTP地址')"
      ></el-table-column>
      <el-table-column
        prop="port"
        :label="$t('system.NTP端口号')"
      ></el-table-column>
      <el-table-column
        prop="manualTime"
        :label="$t('system.时间')"
      ></el-table-column>

      <el-table-column :label="$t('common.操作')">
        <template #default="scope">
          <el-button
            type="primary"
            plain
            size="small"
            @click="editRtc(scope.row)"
            >{{ $t("common.编辑") }}</el-button
          >
        </template>
      </el-table-column>
    </el-table>
  </div>
</template>

<script>
import { listPort, updatePort } from "@/api/system/port";
export default {
  data() {
    return {
      RtcList: [],
      selectedRows: [],
      dialogVisible: false,
      dialogTitle: "",
      pageNum: 1,
      pageSize: 10,
      total: 0,
      form: {
        name: "",
        isNtp: 0,
        address: "",
        port: "",
        manualTime: "",
      },
      loading: false,
    };
  },
  methods: {
    async addRtc() {
      this.dialogTitle = this.$t("system.新增对时");
      this.dialogVisible = true;
      this.form = {
        name: "",
        cmdContent: "",
        enable: "",
      };
    },
    async editRtc(row) {
      this.dialogTitle = this.$t("system.编辑对时");
      this.form = { ...row };
      this.dialogVisible = true;
    },
    handleSelectionChange(val) {
      this.selectedRows = val;
    },
    async submitForm() {
      this.form.type = 6;
      await updatePort(this.form);
      this.$message.success(this.$t("common.更新成功"));
      this.dialogVisible = false;
      this.fetchRtcList();
    },
    async fetchRtcList() {
      this.loading = true;
      const response = await listPort({
        type: 6,
        pageNum: 1,
        pageSize: 100,
      });
      this.RtcList = response.rows;
      this.total = response.total;
      this.loading = false;
    },
  },
  mounted() {
    this.fetchRtcList();
  },
};
</script>
