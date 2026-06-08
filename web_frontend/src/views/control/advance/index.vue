<template>
  <div style="margin-top: 25px; max-width: 90%; margin-left: 30px">
    <!-- 弹窗 - 高级配置1  (弹窗放宽，标签全部能放一行) -->
    <el-dialog :title="dialogTitle" :visible.sync="dialogVisible" width="700px">
      <el-form :model="form" label-width="250px" label-position="left">
        <el-form-item :label="$t('control.本地远程')">
          <el-radio-group v-model="form.remote">
            <el-radio :label="0">{{ $t("control.远程") }}</el-radio>
            <el-radio :label="1">{{ $t("control.本地") }}</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item :label="$t('control.充放电方向')">
          <el-radio-group v-model="form.direction">
            <el-radio :label="1">{{ $t("control.放电为正") }}</el-radio>
            <el-radio :label="-1">{{ $t("control.放电为负") }}</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item :label="$t('control.分配方式')">
          <el-radio-group v-model="form.dist_type">
            <el-radio :label="0">{{ $t("control.soc均衡分配") }}</el-radio>
            <el-radio :label="1">{{ $t("control.平均分配") }}</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item :label="$t('control.开关机')">
          <el-radio-group v-model="form.power_on_off">
            <el-radio :label="0">{{ $t("control.关机") }}</el-radio>
            <el-radio :label="1">{{ $t("control.开机") }}</el-radio>
          </el-radio-group>
        </el-form-item>
        <el-form-item :label="$t('control.有功功率目标值(kW)')">
          <el-input-number
            v-model="form.active_power"
            :controls="false"
            style="width: 100%"
          />
        </el-form-item>
        <el-form-item :label="$t('control.无功功率目标值(kVar)')">
          <el-input-number
            v-model="form.reactive_power"
            :controls="false"
            style="width: 100%"
          />
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="dialogVisible = false">
            {{ $t("control.取消") }}
          </el-button>
          <el-button type="primary" @click="submitForm">
            {{ $t("control.确定") }}
          </el-button>
        </span>
      </template>
    </el-dialog>

    <!-- Battery Stack 编辑弹窗 -->
    <el-dialog
      :title="$t('control.编辑') + ' Battery Stack'"
      :visible.sync="batteryStackEditVisible"
      width="450px"
    >
      <el-form
        :model="batteryStackEditForm"
        label-width="180px"
        label-position="left"
      >
        <el-form-item :label="$t('control.电池堆')">
          <el-input-number
            v-model="batteryStackEditForm.BatteryStack"
            :min="1"
            :controls="false"
            style="width: 100px"
          ></el-input-number>
        </el-form-item>
        <el-form-item :label="$t('control.起始PCS编号')">
          <el-input-number
            v-model="batteryStackEditForm.StartPCSNumber"
            :min="1"
            :controls="false"
            style="width: 120px"
          ></el-input-number>
        </el-form-item>
        <el-form-item :label="$t('control.结束PCS编号')">
          <el-input-number
            v-model="batteryStackEditForm.EndPCSNumber"
            :min="batteryStackEditForm.StartPCSNumber"
            :controls="false"
            style="width: 120px"
          ></el-input-number>
        </el-form-item>
      </el-form>
      <template #footer>
        <span class="dialog-footer">
          <el-button @click="batteryStackEditVisible = false">
            {{ $t("control.取消") }}
          </el-button>
          <el-button type="primary" @click="confirmEditBatteryStackItem">
            {{ $t("control.确定") }}
          </el-button>
        </span>
      </template>
    </el-dialog>

    <!-- 高级配置1 -->
    <el-card class="advanced-settings-card" style="margin-bottom: 20px">
      <div
        slot="header"
        style="
          display: flex;
          justify-content: space-between;
          align-items: center;
        "
      >
        <span>{{ advancedConfig1.name }}</span>
      </div>

      <!-- 去掉表头颜色，表格列放宽 -->
      <el-table :data="[advancedConfig1]" style="width: 100%">
        <el-table-column
          prop="name"
          :label="$t('control.配置名称')"
          width="200"
        />
        <el-table-column
          prop="remote"
          :label="$t('control.本地远程')"
          width="140"
        >
          <template #default="scope">
            {{
              scope.row.remote === 1 ? $t("control.本地") : $t("control.远程")
            }}
          </template>
        </el-table-column>
        <el-table-column
          prop="direction"
          :label="$t('control.充放电方向')"
          width="200"
        >
          <template #default="scope">
            {{
              scope.row.direction === 1
                ? $t("control.放电为正")
                : $t("control.放电为负")
            }}
          </template>
        </el-table-column>
        <el-table-column
          prop="dist_type"
          :label="$t('control.分配方式')"
          width="160"
        >
          <template #default="scope">
            {{
              scope.row.dist_type === 1
                ? $t("control.平均分配")
                : $t("control.soc均衡分配")
            }}
          </template>
        </el-table-column>
        <el-table-column
          prop="power_on_off"
          :label="$t('control.开关机')"
          width="140"
        >
          <template #default="scope">
            {{
              scope.row.power_on_off === 1
                ? $t("control.开机")
                : $t("control.关机")
            }}
          </template>
        </el-table-column>
        <el-table-column
          prop="active_power"
          :label="$t('control.有功功率目标值(kW)')"
          width="250"
        />
        <el-table-column
          prop="reactive_power"
          :label="$t('control.无功功率目标值(kVar)')"
          width="250"
        />
        <el-table-column :label="$t('control.操作')" width="120">
          <template #default="scope">
            <el-button
              type="primary"
              plain
              size="small"
              @click="editAdvancedData(scope.row)"
            >
              {{ $t("control.编辑") }}
            </el-button>
          </template>
        </el-table-column>
      </el-table>
    </el-card>

    <!-- Control Architecture -->
    <el-card class="control-architecture-card" style="margin-bottom: 20px">
      <div
        slot="header"
        style="
          display: flex;
          justify-content: space-between;
          align-items: center;
        "
      >
        <span>{{ controlArchitecture.name }}</span>
        <el-button type="primary" size="small" @click="saveControlArchitecture">
          {{ $t("common.保存") }}
        </el-button>
      </div>

      <!-- 集中式/分布式按钮风格与PCS相同，互斥样式 -->
      <div style="padding: 20px">
        <el-radio-group
          v-model="controlArchitectureValue"
          @change="handleControlArchChange"
        >
          <el-radio :label="0" style="margin-right: 30px">
            {{ $t("control.集中式") }}
          </el-radio>
          <el-radio :label="1">
            {{ $t("control.分布式") }}
          </el-radio>
        </el-radio-group>
      </div>
    </el-card>

    <!-- Battery Stack E-Stop -->
    <el-card class="battery-stack-card" style="margin-bottom: 20px">
      <div
        slot="header"
        style="
          display: flex;
          justify-content: space-between;
          align-items: center;
        "
      >
        <span>{{ batteryStackEStop.name }}</span>
        <div>
          <el-button type="primary" size="small" @click="addBatteryStackItem">
            {{ $t("common.新增") }}
          </el-button>
          <el-button
            type="primary"
            size="small"
            @click="saveBatteryStack"
            style="margin-left: 10px"
          >
            {{ $t("common.保存") }}
          </el-button>
        </div>
      </div>

      <!-- 去掉表头颜色，表格列放宽 -->
      <el-table :data="batteryStackItems" style="width: 100%">
        <el-table-column
          prop="BatteryStack"
          :label="$t('control.电池堆')"
          width="200"
        />
        <el-table-column
          prop="StartPCSNumber"
          :label="$t('control.起始PCS编号')"
          width="220"
        />
        <el-table-column
          prop="EndPCSNumber"
          :label="$t('control.结束PCS编号')"
          width="220"
        />
        <el-table-column :label="$t('control.操作')" width="200">
          <template #default="scope">
            <el-button
              type="primary"
              plain
              size="small"
              @click="editBatteryStackItem(scope.$index)"
            >
              {{ $t("control.编辑") }}
            </el-button>
            <el-button
              type="danger"
              plain
              size="small"
              style="margin-left: 8px"
              @click="removeBatteryStackItem(scope.$index)"
            >
              {{ $t("common.删除") }}
            </el-button>
          </template>
        </el-table-column>
      </el-table>
    </el-card>

    <!-- PCS 操作表单 -->
    <el-card
      class="pcs-card"
      style="
        margin-top: 20px;
        width: 100%;
        max-height: 600px;
        margin-left: 0;
        overflow-y: auto;
      "
    >
      <div
        slot="header"
        style="
          display: flex;
          justify-content: space-between;
          align-items: center;
        "
      >
        <span>{{ $t("control.PCS控制配置") }}</span>
        <el-button type="primary" @click="handlePcsSave">
          {{ $t("common.保存") }}
        </el-button>
      </div>

      <el-form label-width="100px">
        <el-row :gutter="20">
          <el-col :span="8" v-for="(pcs, index) in pcsList" :key="pcs.dsp_id">
            <el-form-item :label="'PCS ' + pcs.dsp_id">
              <el-radio-group v-model="pcsFormMap[pcs.dsp_id]">
                <el-radio :label="'on'">{{ $t("control.开机") }}</el-radio>
                <el-radio :label="'off'">{{ $t("control.关机") }}</el-radio>
                <el-radio :label="'reset'">{{ $t("control.复位") }}</el-radio>
                <el-radio :label="'none'">{{ $t("common.无") }}</el-radio>
              </el-radio-group>
            </el-form-item>
          </el-col>
        </el-row>
      </el-form>
    </el-card>
  </div>
</template>

<script>
import {
  fetchAdvancedConfigs,
  saveAdvancedConfigs,
  getPcsList,
  savePcsConfigList,
  saveControlArchitectureConfig,
  saveBatteryStackConfig,
} from "@/api/system/controlstrategy";

export default {
  data() {
    return {
      // 拆分后的三个配置
      advancedConfig1: {},
      controlArchitecture: {},
      batteryStackEStop: {},

      // 控制架构的值
      controlArchitectureValue: 0,

      // 电池堆栈项目
      batteryStackItems: [],

      // Battery Stack 编辑相关
      batteryStackEditVisible: false,
      batteryStackEditForm: {
        BatteryStack: 1,
        StartPCSNumber: 1,
        EndPCSNumber: 1,
      },
      batteryStackEditIndex: null,

      selectedRows: [],
      dialogVisible: false,
      dialogTitle: "",
      pageNum: 1,
      pageSize: 10,
      form: {
        name: "",
        remote: 0,
        direction: 1,
        dist_type: 0,
        power_on_off: 0,
        active_power: 0,
        reactive_power: 0,
      },
      pcsList: [],
      pcsFormMap: {}, // { dspId: 'on' | 'off' | 'reset' | 'none' }
      loading: false,
    };
  },
  methods: {
    async editAdvancedData(row) {
      this.dialogTitle = `${this.$t("control.编辑配置")} - ${row.name}`;
      this.form = { ...row };
      this.dialogVisible = true;
    },

    async submitForm() {
      this.form.type = 1; // 类型1对应高级配置1

      // 构造json字符串参数
      const configData = {
        id: this.form.id,
        type: this.form.type,
        name: this.form.name,
        content: JSON.stringify({
          remote: this.form.remote,
          direction: this.form.direction,
          dist_type: this.form.dist_type,
          power_on_off: this.form.power_on_off,
          active_power: this.form.active_power,
          reactive_power: this.form.reactive_power,
        }),
      };

      await saveAdvancedConfigs(configData);
      this.$message.success(this.$t("common.更新成功"));
      this.dialogVisible = false;
      this.fetchAdvancedDataList();
    },

    async fetchAdvancedDataList() {
      this.loading = true;
      const response = await fetchAdvancedConfigs();
      const configs = response.data || [];

      // 分离三个不同的配置
      configs.forEach((config) => {
        switch (config.type) {
          case 1: // 高级配置1
            this.advancedConfig1 = config;
            break;
          case 2: // Control Architecture
            this.controlArchitecture = config;

            // 必定只有这两种情况，不再考虑其它特殊情况
            // 取 Centralized 和 Distributed 字段
            let centralized = 0;
            let distributed = 1;

            if (config.content) {
              let content;
              if (typeof config.content === "string") {
                try {
                  content = JSON.parse(config.content);
                } catch (e) {
                  content = {};
                }
              } else {
                content = config.content;
              }
              if (
                typeof content.Centralized === "number" &&
                typeof content.Distributed === "number"
              ) {
                centralized = content.Centralized;
                distributed = content.Distributed;
              } else if (
                typeof config.Centralized === "number" &&
                typeof config.Distributed === "number"
              ) {
                centralized = config.Centralized;
                distributed = config.Distributed;
              }
            } else if (
              typeof config.Centralized === "number" &&
              typeof config.Distributed === "number"
            ) {
              centralized = config.Centralized;
              distributed = config.Distributed;
            }

            // Centralized=1,Distributed=0时选集中式（0），=0,1时选分布式（1）
            this.controlArchitectureValue = centralized === 1 ? 0 : 1;
            this.controlArchitecture["Centralized"] = centralized;
            this.controlArchitecture["Distributed"] = distributed;
            break;
          case 3: // Battery Stack E-Stop
            this.batteryStackEStop = config;
            this.batteryStackItems = config.items || [];
            break;
        }
      });

      this.loading = false;
    },

    // Control Architecture 相关方法
    handleControlArchChange(value) {
      this.controlArchitectureValue = value;
      // 必定互斥，直接赋值
      this.controlArchitecture["Centralized"] = value === 0 ? 1 : 0;
      this.controlArchitecture["Distributed"] = value === 1 ? 1 : 0;
    },

    async saveControlArchitecture() {
      try {
        // 必定互斥
        const contentData = {
          Centralized: this.controlArchitectureValue === 0 ? 1 : 0,
          Distributed: this.controlArchitectureValue === 1 ? 1 : 0,
        };

        const configData = {
          id: this.controlArchitecture.id,
          type: this.controlArchitecture.type,
          name: this.controlArchitecture.name,
          content: JSON.stringify(contentData),
        };

        await saveAdvancedConfigs(configData);
        this.$message.success(this.$t("common.保存成功"));
      } catch (error) {
        console.error("保存Control Architecture失败:", error);
        this.$message.error(this.$t("common.保存失败"));
      }
    },

    // Battery Stack E-Stop 相关方法
    addBatteryStackItem() {
      const newItem = {
        BatteryStack: this.batteryStackItems.length + 1,
        StartPCSNumber: 1,
        EndPCSNumber: 6,
      };
      this.batteryStackItems.push(newItem);
    },

    editBatteryStackItem(index) {
      // 使用表单弹窗编辑 Battery Stack 行
      const item = this.batteryStackItems[index];
      this.batteryStackEditIndex = index;
      this.batteryStackEditForm = { ...item }; // 深拷贝
      this.batteryStackEditVisible = true;
    },

    confirmEditBatteryStackItem() {
      // 保存编辑（表单验证）
      const form = this.batteryStackEditForm;
      if (
        form.StartPCSNumber > form.EndPCSNumber ||
        form.BatteryStack <= 0 ||
        form.StartPCSNumber <= 0 ||
        form.EndPCSNumber <= 0
      ) {
        this.$message.error(this.$t("common.请输入有效数据"));
        return;
      }
      this.batteryStackItems.splice(this.batteryStackEditIndex, 1, { ...form });
      this.batteryStackEditVisible = false;
    },

    removeBatteryStackItem(index) {
      this.$confirm(
        this.$t("control.确认删除该配置项吗？"),
        this.$t("common.提示"),
        {
          type: "warning",
        }
      )
        .then(() => {
          this.batteryStackItems.splice(index, 1);
          this.$message.success(this.$t("common.删除成功"));
        })
        .catch(() => {});
    },

    async saveBatteryStack() {
      // 更新数据
      this.batteryStackEStop.items = this.batteryStackItems;

      try {
        // 构造json字符串参数
        const configData = {
          id: this.batteryStackEStop.id,
          type: this.batteryStackEStop.type,
          name: this.batteryStackEStop.name,
          content: JSON.stringify(this.batteryStackItems),
        };
        await saveAdvancedConfigs(configData);
        this.$message.success(this.$t("common.保存成功"));
      } catch (error) {
        this.$message.error(this.$t("common.保存失败"));
      }
    },

    async fetchPcsList() {
      const res = await getPcsList();
      this.pcsList = res.data || [];
      this.pcsList.forEach((pcs) => {
        const action = pcs.on
          ? "on"
          : pcs.off
          ? "off"
          : pcs.reset
          ? "reset"
          : "none";
        this.$set(this.pcsFormMap, pcs.dsp_id, action);
      });
    },

    async handlePcsSave() {
      const data = this.pcsList.map((pcs) => ({
        pcs_id: pcs.dsp_id,
        action: this.pcsFormMap[pcs.dsp_id],
      }));

      try {
        await savePcsConfigList(data);
        this.$message.success(this.$t("control.下发成功"));
        this.fetchPcsList(); // 刷新界面
      } catch (err) {
        this.$message.error(this.$t("control.下发失败"));
      }
    },
  },

  mounted() {
    this.fetchAdvancedDataList();
    this.fetchPcsList();
  },
};
</script>

<style scoped>
.advanced-config-container {
  margin-top: 25px;
  max-width: 90%;
  margin-left: 30px;
}

.advanced-config-header {
  background-color: #f5f7fa;
  padding: 10px;
  font-weight: bold;
  border: 1px solid #dcdfe6;
  border-radius: 4px;
  margin-bottom: 10px;
}

.header-title {
  color: #409eff;
}
</style>
