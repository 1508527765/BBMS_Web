<template>
  <div>
    <div class="toolbar">
      <el-row>
        <el-col :span="24">
          <el-button
            type="primary"
            plain
            icon="el-icon-plus"
            size="mini"
            @click="addCan"
            >{{ $t("common.新增") }}</el-button
          >
          <el-button
            type="danger"
            plain
            size="mini"
            icon="el-icon-delete"
            @click="batchDelete"
            :disabled="!selectedRows.length"
          >
            {{ $t("common.删除") }}
          </el-button>
        </el-col>
      </el-row>
    </div>
    <el-dialog :title="dialogTitle" :visible.sync="dialogVisible" width="30%">
      <el-form :model="form" label-width="150px" label-position="left">
        <el-form-item :label="$t('system.配置名称')">
          <el-input v-model="form.name" autocomplete="off" disabled></el-input>
        </el-form-item>
        <el-form-item :label="$t('system.CAN口号')">
          <el-input v-model="form.canId" autocomplete="off" disabled></el-input>
        </el-form-item>
        <el-form-item :label="$t('system.波特率')">
          <el-select
            v-model="form.baud"
            :placeholder="$t('system.请选择波特率')"
          >
            <el-option
              v-for="(label, value) in baudRates"
              :key="value"
              :label="label"
              :value="parseInt(value)"
            ></el-option>
          </el-select>
        </el-form-item>
        <el-form-item :label="$t('system.驱动路径')">
          <el-input v-model="form.address" autocomplete="off"></el-input>
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
      :data="canList"
      style="margin-top: 20px; width: 100%"
      @selection-change="handleSelectionChange"
    >
      <el-table-column type="selection" width="55"></el-table-column>
      <el-table-column
        prop="name"
        :label="$t('system.配置名称')"
      ></el-table-column>
      <el-table-column
        prop="canId"
        :label="$t('system.CAN口号')"
      ></el-table-column>
      <el-table-column prop="baud" :label="$t('system.波特率')">
        <template #default="scope">
          <span>{{ baudRates[scope.row.baud] }}</span>
        </template>
      </el-table-column>
      <el-table-column
        prop="address"
        :label="$t('system.驱动路径')"
      ></el-table-column>
      <el-table-column :label="$t('common.操作')">
        <template #default="scope">
          <el-button
            type="primary"
            plain
            size="small"
            @click="editCan(scope.row)"
            >{{ $t("common.编辑") }}</el-button
          >
          <el-button
            type="primary"
            plain
            size="small"
            @click="deleteCan(scope.row)"
            :disabled="scope.row.canId !== maxCanId"
            >{{ $t("common.删除") }}</el-button
          >
        </template>
      </el-table-column>
    </el-table>
    <pagination
      v-show="total > 0"
      :total="total"
      :page.sync="pageNum"
      :limit.sync="pageSize"
      @pagination="fetchCanList"
    />
  </div>
</template>

<script>
import { listPort, delPort, addPort, updatePort } from "@/api/system/port";
import { getToken } from "@/utils/auth";
export default {
  data() {
    return {
      canList: [],
      selectedRows: [],
      dialogVisible: false,
      dialogTitle: "",
      pageNum: 1,
      pageSize: 10,
      total: 0,
      form: {
        name: "",
        canId: "",
        baud: "",
        address: "",
      },
      // 枚举数据
      baudRates: {
        0: "125000", // kCan_125k
        1: "250000", // kCan_250k
        2: "500000", // kCan_500k
      },
      maxCanId: null,
    };
  },
  methods: {
    async batchDelete() {
      if (this.selectedRows.length) {
        const portIds = this.selectedRows.map((row) => row.id).join(",");
        await delPort(portIds);
        this.$message.success(this.$t("common.删除成功"));
        this.fetchCanList();
      } else {
        this.$message.info(this.$t("common.请先选择要删除的项"));
      }
    },
    addCan() {
      this.dialogTitle = this.$t("system.新增CAN");
      this.dialogVisible = true;
      const maxCanId = this.canList.reduce(
        (max, item) => Math.max(max, parseInt(item.canId)),
        -1
      );
      this.form = {
        name: `${this.$t("system.CAN口")}${maxCanId + 1}`,
        canId: (maxCanId + 1).toString(),
        baud: "",
        address: "",
      };
    },
    editCan(row) {
      this.dialogTitle = this.$t("system.编辑CAN");
      this.form = { ...row };
      this.dialogVisible = true;
    },
    handleSelectionChange(val) {
      this.selectedRows = val;
    },
    async submitForm() {
      this.form.type = 7;
      if (this.form.id) {
        await updatePort(this.form);
        this.$message.success(this.$t("common.更新成功"));
      } else {
        await addPort(this.form);
        this.$message.success(this.$t("common.添加成功"));
      }
      this.dialogVisible = false;
      this.fetchCanList();
    },
    async fetchCanList() {
      this.loading = true;
      const response = await listPort({
        type: 7,
        pageNum: this.pageNum,
        pageSize: this.pageSize,
      });
      this.canList = response.rows;
      this.total = response.total;
      this.loading = false;
      this.maxCanId = Math.max(
        ...this.canList.map((item) => parseInt(item.canId))
      );
    },
    async deleteCan(row) {
      await delPort(row.id);
      this.$message.success(this.$t("common.删除成功"));
      this.fetchCanList();
    },
  },
  mounted() {
    this.fetchCanList();
  },
};
</script>

<style scoped>
.toolbar {
  margin-top: 5px;
  display: flex;
  justify-content: space-between;
}
</style>
