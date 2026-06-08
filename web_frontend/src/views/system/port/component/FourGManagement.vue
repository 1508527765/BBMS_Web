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
            @click="addFourG"
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
        <el-form-item :label="$t('system.拨号指令')">
          <el-input v-model="form.cmdContent" autocomplete="off"></el-input>
        </el-form-item>
        <el-form-item :label="$t('system.使能')">
          <el-switch
            v-model="form.enable"
            active-icon="el-icon-check"
            inactive-icon="el-icon-close"
          ></el-switch>
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
      :data="FourGList"
      style="margin-top: 20px; width: 100%"
      @selection-change="handleSelectionChange"
    >
      <el-table-column type="selection" width="55"></el-table-column>
      <el-table-column
        prop="name"
        :label="$t('system.配置名称')"
      ></el-table-column>
      <el-table-column
        prop="cmdContent"
        :label="$t('system.拨号指令')"
      ></el-table-column>
      <el-table-column
        prop="enable"
        :label="$t('system.使能')"
      ></el-table-column>

      <el-table-column :label="$t('common.操作')">
        <template #default="scope">
          <el-button
            type="primary"
            plain
            size="small"
            @click="editFourG(scope.row)"
          >
            {{ $t("common.编辑") }}
          </el-button>
          <el-button
            type="primary"
            plain
            size="small"
            @click="deleteFourG(scope.row)"
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
      @pagination="fetchFourGList"
    />
  </div>
</template>

<script>
import { listPort, delPort, addPort, updatePort } from "@/api/system/port";
export default {
  data() {
    return {
      FourGList: [],
      selectedRows: [],
      dialogVisible: false,
      dialogTitle: "",
      pageNum: 1,
      pageSize: 10,
      total: 0,
      form: {
        name: "4G配置",
        cmdContent: "",
        enable: "",
      },
    };
  },
  methods: {
    async batchDelete() {
      if (this.selectedRows.length) {
        const portIds = this.selectedRows.map((row) => row.id).join(",");
        await delPort(portIds);
        this.$message.success(this.$t("common.删除成功"));
        this.fetchFourGList();
      } else {
        this.$message.info(this.$t("common.请先选择要删除的项"));
      }
    },
    async addFourG() {
      this.dialogTitle = this.$t("system.新增4G");
      this.dialogVisible = true;
      this.form = {
        name: "4G配置",
        cmdContent: "",
        enable: "",
      };
    },
    async editFourG(row) {
      this.dialogTitle = this.$t("system.编辑4G");
      this.form = { ...row };
      this.dialogVisible = true;
    },
    handleSelectionChange(val) {
      this.selectedRows = val;
    },
    async submitForm() {
      this.form.type = 5;
      if (this.form.id) {
        await updatePort(this.form);
        this.$message.success(this.$t("common.更新成功"));
      } else {
        await addPort(this.form);
        this.$message.success(this.$t("common.添加成功"));
      }
      this.dialogVisible = false;
      this.fetchFourGList();
    },
    async fetchFourGList() {
      this.loading = true;
      const response = await listPort({
        type: 5,
        pageNum: this.pageNum,
        pageSize: this.pageSize,
      });
      this.FourGList = response.rows;
      this.total = response.total;
      this.loading = false;
    },
    async deleteFourG(row) {
      await delPort(row.id);
      this.$message.success(this.$t("common.删除成功"));
      this.fetchFourGList();
    },
  },
  mounted() {
    this.fetchFourGList();
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
