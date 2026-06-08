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
            新增
          </el-button>
          <el-button
            type="danger"
            plain
            size="mini"
            icon="el-icon-delete"
            @click="batchDelete"
            :disabled="!selectedRows.length"
          >
            删除
          </el-button>
        </el-col>
      </el-row>
    </div>
    <el-dialog :title="dialogTitle" :visible.sync="dialogVisible" width="30%">
      <el-form :model="form" label-width="150px" label-position="left">
        <el-form-item label="配置名称">
          <el-input v-model="form.name" autocomplete="off" disabled></el-input>
        </el-form-item>
        <el-form-item label="wifi名称">
          <el-input v-model="form.wifiName" autocomplete="off"></el-input>
        </el-form-item>
        <el-form-item label="wifi密码">
          <el-input v-model="form.wifiPassword" autocomplete="off"></el-input>
        </el-form-item>
        <el-form-item label="使能">
          <el-switch
            v-model="form.enable"
            active-icon="el-icon-check"
            inactive-icon="el-icon-close"
          ></el-switch>
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="dialogVisible = false">取消</el-button>
          <el-button type="primary" @click="submitForm">确定</el-button>
        </span>
      </template>
    </el-dialog>
    <el-table
      :data="FourGList"
      style="margin-top: 20px; width: 100%"
      @selection-change="handleSelectionChange"
    >
      <el-table-column type="selection" width="55"></el-table-column>
      <el-table-column prop="name" label="配置名称"></el-table-column>
      <el-table-column prop="wifiName" label="wifi名称"></el-table-column>
      <el-table-column prop="wifiPassword" label="密码"></el-table-column>
      <el-table-column prop="enable" label="使能"></el-table-column>

      <el-table-column label="操作">
        <template #default="scope">
          <el-button
            type="primary"
            plain
            size="small"
            @click="editFourG(scope.row)"
            >编辑</el-button
          >
          <el-button
            type="primary"
            plain
            size="small"
            @click="deleteFourG(scope.row)"
            >删除</el-button
          >
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
        name: "Wifi配置",
        wifiName: "",
        wifiPassword: "",
        enable: "",
      },
    };
  },
  methods: {
    async batchDelete() {
      if (this.selectedRows.length) {
        const portIds = this.selectedRows.map((row) => row.id).join(",");
        await delPort(portIds);
        this.$message.success("删除成功！");
        this.fetchFourGList();
      } else {
        this.$message.info("请先选择要删除的项。");
      }
    },
    async addFourG() {
      this.dialogTitle = "新增Wifi";
      this.dialogVisible = true;
      this.form = {
        name: "Wifi配置",
        wifiName: "",
        wifiPassword: "",
        enable: "",
      };
    },
    async editFourG(row) {
      this.dialogTitle = "编辑Wifi";
      this.form = { ...row };
      this.dialogVisible = true;
    },
    handleSelectionChange(val) {
      this.selectedRows = val;
    },
    async submitForm() {
      this.form.type = 4;
      if (this.form.id) {
        await updatePort(this.form);
        this.$message.success("更新成功！");
      } else {
        await addPort(this.form);
        this.$message.success("添加成功！");
      }
      this.dialogVisible = false;
      this.fetchFourGList();
    },
    async fetchFourGList() {
      this.loading = true;
      const response = await listPort({
        type: 4,
        pageNum: this.pageNum,
        pageSize: this.pageSize,
      });
      this.FourGList = response.rows;
      this.total = response.total;
      this.loading = false;
    },
    async deleteFourG(row) {
      await delPort(row.id);
      this.$message.success("删除成功！");
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
