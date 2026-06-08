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
            @click="addNetwork"
          >
            {{ $t("common.新增") }}
          </el-button>
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
        <el-form-item :label="$t('system.网口号')">
          <el-input v-model="form.netId" autocomplete="off" disabled></el-input>
        </el-form-item>
        <el-form-item :label="$t('system.是否DHCP')">
          <el-select
            v-model="form.isDHCP"
            :placeholder="$t('system.请选择是否DHCP')"
          >
            <el-option :label="$t('common.是')" value="1"></el-option>
            <el-option :label="$t('common.否')" value="0"></el-option>
          </el-select>
        </el-form-item>
        <el-form-item :label="$t('system.IP地址')">
          <el-input v-model="form.ip" autocomplete="off"></el-input>
        </el-form-item>
        <el-form-item :label="$t('system.子网掩码')">
          <el-input v-model="form.mask" autocomplete="off"></el-input>
        </el-form-item>
        <el-form-item :label="$t('system.网关')">
          <el-input v-model="form.gateway" autocomplete="off"></el-input>
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
      :data="networkList"
      style="margin-top: 20px; width: 100%"
      @selection-change="handleSelectionChange"
    >
      <el-table-column type="selection" width="55"></el-table-column>
      <el-table-column
        prop="name"
        :label="$t('system.配置名称')"
      ></el-table-column>
      <el-table-column
        prop="netId"
        :label="$t('system.网口号')"
      ></el-table-column>
      <el-table-column prop="isDHCP" :label="$t('system.是否DHCP')">
        <template #default="scope">
          <span>{{
            scope.row.isDHCP === "1" ? $t("common.是") : $t("common.否")
          }}</span>
        </template>
      </el-table-column>
      <el-table-column prop="ip" :label="$t('system.IP地址')"></el-table-column>
      <el-table-column
        prop="mask"
        :label="$t('system.子网掩码')"
      ></el-table-column>
      <el-table-column
        prop="gateway"
        :label="$t('system.网关')"
      ></el-table-column>
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
            @click="editNetwork(scope.row)"
          >
            {{ $t("common.编辑") }}
          </el-button>
          <el-button
            type="primary"
            plain
            size="small"
            @click="deleteNetwork(scope.row)"
            :disabled="scope.row.netId !== maxNetId"
          >
            {{ $t("common.删除") }}
          </el-button>
        </template>
      </el-table-column>
    </el-table>
    <pagination
      v-show="total > 0"
      :total="total"
      :page.sync="pageNum"
      :limit.sync="pageSize"
      @pagination="fetchNetworkList"
    />
  </div>
</template>

<script>
import { listPort, delPort, addPort, updatePort } from "@/api/system/port";
export default {
  data() {
    return {
      networkList: [],
      selectedRows: [],
      dialogVisible: false,
      dialogTitle: "",
      pageNum: 1,
      pageSize: 10,
      total: 0,
      form: {
        name: "",
        netId: "",
        isDHCP: "",
        ip: "",
        mask: "",
        gateway: "",
        address: "",
      },
      maxNetId: null,
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
    };
  },
  methods: {
    async batchDelete() {
      if (this.selectedRows.length) {
        const portIds = this.selectedRows.map((row) => row.id).join(",");
        await delPort(portIds);
        this.$message.success(this.$t("common.删除成功"));
        this.fetchNetworkList();
      } else {
        this.$message.info(this.$t("common.请先选择要删除的项"));
      }
    },
    async addNetwork() {
      this.dialogTitle = this.$t("system.新增网口");
      this.dialogVisible = true;
      const maxNetId = this.networkList.reduce(
        (max, item) => Math.max(max, parseInt(item.netId)),
        -1
      );
      const newNetId = maxNetId + 1;
      this.form = {
        name: `${this.$t("system.网口")}${newNetId}`,
        netId: newNetId.toString(),
        isDHCP: "",
        ip: "",
        mask: "",
        gateway: "",
        address: "",
      };
    },
    async editNetwork(row) {
      this.dialogTitle = this.$t("system.编辑网口");
      this.form = { ...row };
      this.dialogVisible = true;
    },
    handleSelectionChange(val) {
      this.selectedRows = val;
    },
    async submitForm() {
      this.form.type = 1;
      if (this.form.id) {
        await updatePort(this.form);
        this.$message.success(this.$t("common.更新成功"));
      } else {
        await addPort(this.form);
        this.$message.success(this.$t("common.添加成功"));
      }
      this.dialogVisible = false;
      this.fetchNetworkList();
    },
    async fetchNetworkList() {
      this.loading = true;
      const response = await listPort({
        type: 1,
        pageNum: this.pageNum,
        pageSize: this.pageSize,
      });
      this.networkList = response.rows;
      this.total = response.total;
      this.loading = false;
      this.maxNetId = Math.max(
        ...this.networkList.map((item) => parseInt(item.netId))
      );
    },
    async deleteNetwork(row) {
      await delPort(row.id);
      this.$message.success(this.$t("common.删除成功"));
      this.fetchNetworkList();
    },
  },
  mounted() {
    this.fetchNetworkList();
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
