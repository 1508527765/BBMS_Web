<template>
  <div class="config-page">
    <div class="toolbar">
      <el-row>
        <el-button
          type="primary"
          plain
          icon="el-icon-plus"
          size="mini"
          @click="onAdd"
        >
          {{ $t("common.新增") }}
        </el-button>
        <el-button
          type="info"
          plain
          size="mini"
          icon="el-icon-refresh"
          @click="refresh"
        >
          {{ $t("common.刷新") }}
        </el-button>
      </el-row>
    </div>
    <!-- DI 类型表格 -->
    <div>
      <el-divider content-position="left">DI</el-divider>
      <el-table :data="diData" border>
        <el-table-column prop="seq" :label="$t('device.序号')" width="50" />
        <el-table-column
          prop="type"
          :label="$t('device.类型')"
          width="80"
          :formatter="typeFormatter"
        />
        <el-table-column
          prop="cabinetCode"
          :label="$t('device.柜子编号')"
          width="120"
        />
        <el-table-column prop="strategySeq" :label="$t('device.策略序号')" />
        <el-table-column
          :prop="currentLocale === 'en' ? 'strategyEname' : 'strategyName'"
          :label="$t('device.策略名称')"
        />
        <el-table-column :label="$t('device.操作')" width="200">
          <template slot-scope="scope">
            <el-button size="mini" @click="editConfig(scope.row)">{{
              $t("common.编辑")
            }}</el-button>
            <el-button
              size="mini"
              type="danger"
              :disabled="!isLastItem(scope.row, diData)"
              @click="confirmDelete(scope.row)"
              >{{ $t("common.删除") }}</el-button
            >
          </template>
        </el-table-column>
      </el-table>
    </div>

    <!-- DO 类型表格 -->
    <div style="margin-top: 20px">
      <el-divider content-position="left">DO</el-divider>
      <el-table :data="doData" border>
        <el-table-column prop="seq" :label="$t('device.序号')" width="50" />
        <el-table-column
          prop="type"
          :label="$t('device.类型')"
          width="80"
          :formatter="typeFormatter"
        />
        <el-table-column
          prop="cabinetCode"
          :label="$t('device.柜子编号')"
          width="120"
        />
        <el-table-column prop="strategySeq" :label="$t('device.策略序号')" />
        <el-table-column
          :prop="currentLocale === 'en' ? 'strategyEname' : 'strategyName'"
          :label="$t('device.策略名称')"
        />
        <el-table-column :label="$t('device.操作')" width="200">
          <template slot-scope="scope">
            <el-button size="mini" @click="editConfig(scope.row)">{{
              $t("common.编辑")
            }}</el-button>
            <el-button
              size="mini"
              type="danger"
              :disabled="!isLastItem(scope.row, doData)"
              @click="confirmDelete(scope.row)"
              >{{ $t("common.删除") }}</el-button
            >
          </template>
        </el-table-column>
      </el-table>
    </div>

    <!-- 弹窗 -->
    <el-dialog :visible.sync="dialogVisible" :title="$t('device.配置详情')">
      <el-form :model="form">
        <el-form-item :label="$t('device.类型')" :required="true">
          <el-select
            v-model="form.type"
            :placeholder="$t('device.请选择类型')"
            @change="fetchStrategies"
          >
            <el-option
              label="DI"
              value="0"
              :disabled="diData.length >= diMaxCount"
            >
              DI
            </el-option>
            <el-option
              label="DO"
              value="1"
              :disabled="doData.length >= doMaxCount"
            >
              DO
            </el-option>
          </el-select>
          <span v-if="limitMessage" style="color: red; margin-left: 20px">
            *{{ limitMessage }}
          </span>
        </el-form-item>
        <el-form-item :label="$t('device.柜子编号')" :required="true">
          <el-select
            v-model="form.cabinetCode"
            :placeholder="$t('device.请选择柜子编号')"
          >
            <el-option
              v-for="cabinet in cabinets"
              :key="cabinet.value"
              :label="cabinet.label"
              :value="cabinet.value"
            ></el-option>
          </el-select>
        </el-form-item>
        <el-form-item :label="$t('device.策略序号')" :required="true">
          <el-select
            v-model="form.strategySeq"
            :placeholder="$t('device.请选择策略序号')"
          >
            <el-option
              v-for="strategy in strategies"
              :key="strategy.id"
              :label="
                currentLocale === 'en' && strategy.strategyEname
                  ? strategy.strategyEname
                  : strategy.strategyName
              "
              :value="strategy.strategySeq"
            ></el-option>
          </el-select>
        </el-form-item>
      </el-form>
      <div slot="footer" class="dialog-footer">
        <el-button @click="dialogVisible = false">{{
          $t("common.取消")
        }}</el-button>
        <el-button
          type="primary"
          @click="saveConfig"
          :disabled="
            (form.type === '0' && diData.length >= diMaxCount) ||
            (form.type === '1' && doData.length >= doMaxCount)
          "
        >
          {{ $t("common.保存") }}
        </el-button>
      </div>
    </el-dialog>
  </div>
</template>

<script>
import {
  listLogicList,
  listDrynode,
  updateDrynode,
  deleteDrynode,
  addDrynode,
} from "@/api/system/drynode";
import { getToken } from "@/utils/auth";
import i18n from "@/i18n";

export default {
  data() {
    return {
      currentLocale: i18n.locale,
      upload: {
        open: false, // 是否显示弹出层
        title: "", // 弹出层标题
        isUploading: false, // 是否禁用上传
        updateSupport: false, // 是否更新已经存在的用户数据
        headers: { Authorization: "Bearer " + getToken() }, // 设置上传的请求头部
        url: process.env.VUE_APP_BASE_API + "/drynode/import-drynodes", // 上传的地址
      },
      // 表格数据
      diData: [],
      doData: [],
      // 弹窗数据
      dialogVisible: false,
      form: {
        type: "",
        cabinetCode: "",
        strategySeq: "",
      },
      cabinets: [
        { label: "1", value: "1" },
        { label: "2", value: "2" },
        { label: "3", value: "3" },
        { label: "4", value: "4" },
        { label: "5", value: "5" },
        { label: "6", value: "6" },
        { label: "7", value: "7" },
        { label: "8", value: "8" },
      ],
      strategies: [],
      diMaxCount: 85,
      doMaxCount: 22,
    };
  },
  computed: {
    limitMessage() {
      return this.diData.length >= this.diMaxCount &&
        this.doData.length >= this.doMaxCount
        ? this.$t("device.DI/DO数量已达到系统允许的最大值")
        : this.diData.length >= this.diMaxCount
        ? this.$t("device.DI数量已达到系统允许的最大值")
        : this.doData.length >= this.doMaxCount
        ? this.$t("device.DO数量已达到系统允许的最大值")
        : "";
    },
  },
  methods: {
    typeFormatter(row) {
      return row.type === "0" ? "DI" : "DO";
    },
    onAdd() {
      this.dialogVisible = true;
      this.form = { type: "", cabinetCode: "", strategySeq: "" }; // 清空表单
    },
    editConfig(row) {
      this.form = { ...row }; // 填充表单
      this.dialogVisible = true;
      this.fetchStrategies(row.type);
    },
    saveConfig() {
      if (!this.form.type || !this.form.cabinetCode || !this.form.strategySeq) {
        this.$message.error(this.$t("device.请填写完整信息"));
        return;
      }
      // 保存逻辑
      const apiMethod = this.form.seq ? updateDrynode : addDrynode;
      apiMethod(this.form).then(() => {
        this.refresh();
        this.dialogVisible = false;
      });
    },
    confirmDelete(row) {
      this.$confirm(
        this.$t("device.此操作将永久删除该配置, 是否继续"),
        this.$t("device.提示"),
        {
          confirmButtonText: this.$t("device.确定"),
          cancelButtonText: this.$t("device.取消"),
          type: "warning",
        }
      )
        .then(() => {
          this.deleteConfig(row);
        })
        .catch(() => {
          this.$message({
            type: "info",
            message: this.$t("device.已取消删除"),
          });
        });
    },
    deleteConfig(row) {
      deleteDrynode(row.id).then(() => {
        this.refresh();
      });
    },
    refresh() {
      listDrynode().then((response) => {
        this.diData = response.rows.DI;
        this.doData = response.rows.DO;
      });
    },
    fetchStrategies(type) {
      const typeStr = type === "0" ? "DI" : "DO";
      listLogicList({ type: typeStr }).then((response) => {
        this.strategies = response.rows;
      });
    },
    isLastItem(row, data) {
      return row.seq === Math.max(...data.map((item) => item.seq));
    },
  },
  created() {
    this.refresh();
  },
};
</script>

<style scoped>
.config-page {
  padding: 20px;
}
</style>
