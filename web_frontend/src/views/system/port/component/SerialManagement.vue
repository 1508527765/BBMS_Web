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
            @click="addSerial"
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
        <el-form-item :label="$t('system.串口号')">
          <el-input
            v-model="form.uartId"
            autocomplete="off"
            disabled
          ></el-input>
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
        <el-form-item :label="$t('system.校验位')">
          <el-select
            v-model="form.parity"
            :placeholder="$t('system.请选择校验位')"
          >
            <el-option
              v-for="(label, value) in parityOptions"
              :key="value"
              :label="label"
              :value="parseInt(value)"
            ></el-option>
          </el-select>
        </el-form-item>
        <el-form-item :label="$t('system.数据位')">
          <el-select
            v-model="form.data"
            :placeholder="$t('system.请选择数据位')"
          >
            <el-option
              v-for="(label, value) in dataBits"
              :key="value"
              :label="label"
              :value="parseInt(value)"
            ></el-option>
          </el-select>
        </el-form-item>
        <el-form-item :label="$t('system.停止位')">
          <el-select
            v-model="form.stop"
            :placeholder="$t('system.请选择停止位')"
          >
            <el-option
              v-for="(label, value) in stopBits"
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
      :data="serialList"
      style="margin-top: 20px; width: 100%"
      @selection-change="handleSelectionChange"
    >
      <el-table-column type="selection" width="55"></el-table-column>
      <el-table-column
        prop="name"
        :label="$t('system.配置名称')"
      ></el-table-column>
      <el-table-column
        prop="uartId"
        :label="$t('system.串口号')"
      ></el-table-column>
      <el-table-column prop="baud" :label="$t('system.波特率')">
        <template #default="scope">
          <span>{{ baudRates[scope.row.baud] }}</span>
        </template>
      </el-table-column>
      <el-table-column prop="parity" :label="$t('system.校验位')">
        <template #default="scope">
          <span>{{ parityOptions[scope.row.parity] }}</span>
        </template>
      </el-table-column>
      <el-table-column prop="data" :label="$t('system.数据位')">
        <template #default="scope">
          <span>{{ dataBits[scope.row.data] }}</span>
        </template>
      </el-table-column>
      <el-table-column prop="stop" :label="$t('system.停止位')">
        <template #default="scope">
          <span>{{ stopBits[scope.row.stop] }}</span>
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
            @click="editSerial(scope.row)"
            >{{ $t("common.编辑") }}</el-button
          >
          <el-button
            type="primary"
            plain
            size="small"
            @click="deleteSerial(scope.row)"
            :disabled="scope.row.uartId !== maxUartId"
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
      @pagination="fetchSerialList"
    />
  </div>
</template>

<script>
import { listPort, delPort, addPort, updatePort } from "@/api/system/port";
export default {
  data() {
    return {
      serialList: [],
      selectedRows: [],
      dialogVisible: false,
      dialogTitle: "",
      pageNum: 1,
      pageSize: 10,
      total: 0,
      form: {
        name: "",
        uartId: "",
        baud: "",
        parity: "",
        data: "",
        stop: "",
        address: "",
      },
      // 枚举数据
      baudRates: {
        0: "115200",
        1: "57600",
        2: "38400",
        3: "19200",
        4: "9600",
        5: "4800",
        6: "2400",
        7: "1800",
        8: "1200",
        9: "600",
        10: "300",
        11: "200",
        12: "150",
      },
      parityOptions: {
        0: "None",
        1: "Odd",
        2: "Even",
        3: "Mark",
        4: "Space",
      },
      stopBits: {
        0: "1",
        1: "2",
      },
      dataBits: {
        0: "5",
        1: "6",
        2: "7",
        3: "8",
      },
      maxUartId: null,
    };
  },
  methods: {
    async batchDelete() {
      if (this.selectedRows.length) {
        const portIds = this.selectedRows.map((row) => row.id).join(",");
        await delPort(portIds);
        this.$message.success(this.$t("common.删除成功"));
        this.fetchSerialList();
      } else {
        this.$message.info(this.$t("common.请先选择要删除的项"));
      }
    },
    addSerial() {
      this.dialogTitle = this.$t("system.新增串口");
      this.dialogVisible = true;
      const maxSerialId = this.serialList.reduce(
        (max, item) => Math.max(max, parseInt(item.uartId)),
        -1
      );
      const newSerialId = maxSerialId + 1;
      this.form = {
        name: `${this.$t("system.串口")}${newSerialId}`,
        uartId: newSerialId.toString(),
        baud: 4, // 默认波特率9600
        parity: 0, // 默认校验位无校验
        data: 3, // 默认数据位8
        stop: 0, // 默认停止位1
        address: "",
      };
    },
    editSerial(row) {
      this.dialogTitle = this.$t("system.编辑串口");
      this.form = { ...row };
      this.dialogVisible = true;
    },
    handleSelectionChange(val) {
      this.selectedRows = val;
    },
    async submitForm() {
      this.form.type = 0;
      if (this.form.id) {
        await updatePort(this.form);
        this.$message.success(this.$t("common.更新成功"));
      } else {
        await addPort(this.form);
        this.$message.success(this.$t("common.添加成功"));
      }
      this.dialogVisible = false;
      this.fetchSerialList();
    },
    async fetchSerialList() {
      this.loading = true;
      const response = await listPort({
        type: 0,
        pageNum: this.pageNum,
        pageSize: this.pageSize,
      });
      this.serialList = response.rows;
      this.total = response.total;
      this.loading = false;
      this.maxUartId = Math.max(
        ...this.serialList.map((item) => parseInt(item.uartId))
      );
    },
    async deleteSerial(row) {
      await delPort(row.id);
      this.$message.success(this.$t("common.删除成功"));
      this.fetchSerialList();
    },
  },
  mounted() {
    this.fetchSerialList();
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
