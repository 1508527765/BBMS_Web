<template>
  <div class="topology-container">
    <div class="toolbar">
      <el-dialog
        :title="upload.title"
        :visible.sync="upload.open"
        width="400px"
        append-to-body
        :modal-append-to-body="false"
        :center="true"
      >
        <el-upload
          ref="upload"
          :limit="1"
          accept=".xlsx, .xls"
          :headers="upload.headers"
          :action="upload.url"
          :disabled="upload.isUploading"
          :on-progress="handleFileUploadProgress"
          :on-success="handleFileSuccess"
          :auto-upload="false"
          drag
        >
          <i class="el-icon-upload"></i>
          <div class="el-upload__text">
            {{ $t("upload.将文件拖到此处，或点击上传") }}
          </div>
          <div class="el-upload__tip text-center" slot="tip">
            <span>{{ $t("upload.仅允许导入xls、xlsx格式文件。") }}</span>
            <el-link
              type="primary"
              :underline="false"
              style="font-size: 12px; vertical-align: baseline"
              @click="importTemplate"
              >{{ $t("upload.下载模板") }}</el-link
            >
          </div>
        </el-upload>
        <div slot="footer" class="dialog-footer">
          <el-button type="primary" @click="submitFileForm">{{
            $t("upload.确 定")
          }}</el-button>
          <el-button @click="upload.open = false">{{
            $t("upload.取 消")
          }}</el-button>
        </div>
      </el-dialog>
      <el-row>
        <el-col :span="24">
          <el-button
            type="success"
            plain
            icon="el-icon-upload"
            size="mini"
            @click="handleImport"
          >
            {{ $t("common.导入") }}
          </el-button>

          <el-button
            type="warning"
            plain
            icon="el-icon-download"
            size="mini"
            @click="handleExport"
            >{{ $t("common.导出") }}</el-button
          >
          <el-button
            type="primary"
            plain
            icon="el-icon-plus"
            size="mini"
            @click="handleAddNode"
            >{{ $t("common.新增") }}</el-button
          >
          <el-button
            type="info"
            plain
            icon="el-icon-sort"
            size="mini"
            @click="toggleExpandAll"
            >{{ $t("common.展开/折叠") }}</el-button
          >
        </el-col>
      </el-row>
    </div>
    <el-table
      v-if="refreshTable"
      :data="treeData"
      row-key="id"
      :tree-props="{ children: 'children', hasChildren: 'hasChildren' }"
      :default-expand-all="isExpandAll"
      class="tree-style"
    >
      <el-table-column
        prop="type"
        :label="$t('device.节点类型')"
      ></el-table-column>
      <el-table-column
        prop="label"
        :label="$t('device.节点名称')"
        :formatter="formatNodeLabel"
      ></el-table-column>
      <el-table-column
        prop="deviceType"
        :label="$t('device.设备类型')"
        :formatter="formatDeviceType"
      ></el-table-column>
      <el-table-column :label="$t('common.操作')" width="180">
        <template slot-scope="scope">
          <el-button
            size="mini"
            type="text"
            @click="handleEditNode(scope.row)"
            >{{ $t("common.编辑") }}</el-button
          >
          <el-button
            size="mini"
            type="text"
            @click="handleDeleteNode(scope.row)"
            :disabled="isLocalSystemTopologyNode(scope.row)"
            >{{ $t("common.删除") }}</el-button
          >
        </template>
      </el-table-column>
    </el-table>

    <!-- 新增/编辑模态框 -->
    <el-dialog
      :title="dialogTitle"
      :visible.sync="dialogVisible"
      width="30%"
      :center="true"
    >
      <el-form :model="form" label-width="120px">
        <el-form-item :label="$t('device.节点类型')">
          <el-select
            v-model="form.type"
            :placeholder="$t('device.选择节点类型')"
            @change="handleTypeChange"
          >
            <el-option
              :label="$t('device.虚拟节点')"
              value="virtual"
            ></el-option>
            <el-option
              :label="$t('device.设备节点')"
              value="device"
            ></el-option>
          </el-select>
        </el-form-item>
        <el-form-item
          :label="$t('device.节点名称')"
          v-if="form.type === 'virtual'"
        >
          <el-input
            v-model="form.name"
            :placeholder="$t('device.请输入节点名称')"
          ></el-input>
        </el-form-item>
        <el-form-item
          :label="$t('device.虚拟设备类型')"
          v-if="form.type === 'virtual'"
          required
        >
          <el-select
            v-model="form.virtualDevType"
            :placeholder="$t('device.选择虚拟设备类型')"
            style="width: 100%"
          >
            <el-option
              v-for="dt in virtualDeviceTypeOptions"
              :key="'vdt-' + dt.value"
              :label="currentLocale === 'en' ? (dt.en_label || dt.label) : dt.label"
              :value="dt.value"
            />
          </el-select>
        </el-form-item>
        <el-form-item
          :label="$t('device.选择设备')"
          v-if="form.type === 'device'"
        >
          <el-select v-model="form.devId" :placeholder="$t('device.选择设备')">
            <el-option
              v-for="device in availableDeviceList"
              :key="device.id"
              :label="currentLocale === 'en' ? device.name : device.name"
              :value="device.id"
            ></el-option>
          </el-select>
        </el-form-item>
        <el-form-item :label="$t('device.上级节点')">
          <el-select
            v-model="form.parentId"
            :placeholder="$t('device.选择上级节点')"
          >
            <el-option :label="$t('common.无')" value=""></el-option>
            <el-option
              v-for="node in flatTreeData"
              :key="node.id"
              :label="node.label"
              :value="node.id"
            ></el-option>
          </el-select>
        </el-form-item>
      </el-form>
      <div slot="footer">
        <el-button @click="dialogVisible = false">{{
          $t("common.取消")
        }}</el-button>
        <el-button type="primary" @click="saveNode">{{
          $t("common.保存")
        }}</el-button>
      </div>
    </el-dialog>

    <!-- 删除确认弹框 -->
    <el-dialog
      :title="$t('common.提示')"
      :visible.sync="deleteDialogVisible"
      width="30%"
      :center="true"
    >
      <p>{{ $t("device.确认删除该节点及其所有子节点？") }}</p>
      <div slot="footer">
        <el-button @click="deleteDialogVisible = false">{{
          $t("common.取消")
        }}</el-button>
        <el-button type="danger" @click="confirmDeleteNode">{{
          $t("common.确认")
        }}</el-button>
      </div>
    </el-dialog>
  </div>
</template>

<script>
import {
  listTopology,
  addTopology,
  updateTopology,
  deleteTopology,
} from "@/api/system/topology";
import { listDevice } from "@/api/system/device";
import { getToken } from "@/utils/auth";
import { deviceTypes, findDeviceTypeByValue } from "@/constants/deviceTypes";
import i18n from "@/i18n"; // 引入i18n配置

export default {
  constants: {
    STORAGE_KEY: "bbms_topology_config_v1",
  },
  data() {
    return {
      upload: {
        open: false, // 是否显示弹出层
        title: "", // 弹出层标题
        isUploading: false, // 是否禁用上传
        updateSupport: false, // 是否更新已经存在的用户数据
        headers: { Authorization: "Bearer " + getToken() }, // 设置上传的请求头部
        url: process.env.VUE_APP_BASE_API + "/topology/import-topologies", // 上传的地址
      },
      treeData: [], // 树形结构数据
      isExpandAll: true, // 初始化默认全部展开
      refreshTable: true, // 重新渲染表格状态
      deviceTypes: deviceTypes,
      defaultProps: {
        children: "children",
        label: "label",
      },
      deviceList: [], // 设备列表
      dialogVisible: false,
      dialogTitle: "",
      deleteDialogVisible: false,
      currentNode: null,
      form: {
        id: "",
        type: "",
        name: "",
        devId: "",
        parentId: "",
        virtualDevType: null,
      },
    };
  },
  computed: {
    currentLocale() {
      return i18n.locale;
    },
    availableDeviceList() {
      const selectedDeviceIds = [];
      const collectDeviceIds = (nodes) => {
        nodes.forEach((node) => {
          if (node.type === "device") {
            selectedDeviceIds.push(parseInt(node.devId, 10));
          }
          if (node.children) {
            collectDeviceIds(node.children);
          }
        });
      };
      collectDeviceIds(this.treeData);
      return this.deviceList.filter(
        (device) => !selectedDeviceIds.includes(device.id)
      );
    },
    /** 虚拟节点可选设备类型（不含本机 0） */
    virtualDeviceTypeOptions() {
      return (this.deviceTypes || []).filter((dt) => Number(dt.value) !== 0);
    },
    flatTreeData() {
      const result = [];
      const flatten = (nodes) => {
        nodes.forEach((node) => {
          result.push(node);
          if (node.children) {
            flatten(node.children);
          }
        });
      };
      flatten(this.treeData);
      return result;
    },
  },
  methods: {
    storageKey() {
      return "bbms_topology_config_v1";
    },
    handleAddNode() {
      this.dialogTitle = this.$t("device.新增节点");
      this.dialogVisible = true;
      this.form = { id: "", type: "", name: "", devId: "", parentId: "", virtualDevType: null };
    },
    handleEditNode(node) {
      this.dialogTitle = this.$t("device.编辑节点");
      this.dialogVisible = true;
      this.fetchDeviceList().then(() => {
        if (node.devId) {
          const device = this.deviceList.find((d) => d.id == node.devId);
          if (device) {
            this.form.devId = device.id;
          }
        }
      });
      this.form.id = node.id;
      this.form.type = node.type;
      this.form.name = node.name;
      this.form.devId = node.devId;
      this.form.virtualDevType =
        node.virtualDevType !== undefined && node.virtualDevType !== null
          ? Number(node.virtualDevType)
          : node.type === "virtual" && node.deviceType !== undefined && node.deviceType !== null
            ? Number(node.deviceType)
            : null;
      this.form.parentId = node.parentId;
      const parentNode = this.findNodeById(node.parentId, this.treeData);
      if (parentNode) {
        this.form.parentId = parentNode.id;
      }
    },
    handleDeleteNode(node) {
      if (this.isLocalSystemTopologyNode(node)) {
        this.$message.warning(this.$t('device.localSystemTopologyNoDelete'));
        return;
      }
      this.currentNode = node;
      this.deleteDialogVisible = true;
    },
    handleTypeChange(value) {
      if (value === "device") {
        this.form.name = ""; // 清空节点名称
        this.fetchDeviceList();
      } else {
        this.form.devId = "";
        if (this.form.virtualDevType === null || this.form.virtualDevType === "") {
          this.form.virtualDevType = 5;
        }
      }
    },
    async saveNode() {
      const parentId = this.form.parentId ? parseInt(this.form.parentId, 10) : null;
      const menuTree = this.form.type === "virtual" ? 0 : 1;
      const originalNode = this.form.id ? this.findNodeById(this.form.id, this.treeData) : null;
      const selectedDevice = this.form.devId
        ? this.deviceList.find((d) => String(d.id) === String(this.form.devId))
        : null;
      const deviceType = selectedDevice ? Number(selectedDevice.type) : null;
      const validateMessage = this.validateTopologyPlacement({
        menuTree,
        parentId,
        deviceType,
        originalNode,
      });
      if (validateMessage) {
        this.$message.warning(validateMessage);
        return;
      }
      if (menuTree === 0 && !String(this.form.name || "").trim()) {
        this.$message.warning("请输入虚拟节点名称");
        return;
      }
      if (
        menuTree === 0 &&
        (this.form.virtualDevType === null ||
          this.form.virtualDevType === "" ||
          Number.isNaN(Number(this.form.virtualDevType)))
      ) {
        this.$message.warning(this.$t("device.请选择虚拟设备类型"));
        return;
      }
      if (menuTree === 1 && !this.form.devId) {
        this.$message.warning("请选择设备");
        return;
      }

      const payload = {
        parentId,
        menuTree,
        name: menuTree === 0 ? String(this.form.name || "").trim() : (selectedDevice?.name || (originalNode && originalNode.name) || ""),
        devId: menuTree === 1 ? Number(this.form.devId) : null,
        virtualDevType: menuTree === 0 ? Number(this.form.virtualDevType) : null,
      };
      if (this.form.id) payload.id = Number(this.form.id);

      try {
        if (this.form.id) {
          await updateTopology(payload);
        } else {
          await addTopology(payload);
        }
        this.dialogVisible = false;
        await this.fetchDeviceList();
        await this.getList();
        this.$message.success(this.$t("common.操作成功"));
      } catch (e) {
        this.$message.error((e && e.msg) || (e && e.message) || this.$t("common.操作失败"));
      }
    },
    refreshTopologyList() {
      this.getList();
    },
    async confirmDeleteNode() {
      if (!this.currentNode || !this.currentNode.id) {
        this.deleteDialogVisible = false;
        return;
      }
      try {
        await deleteTopology(this.currentNode.id);
        this.deleteDialogVisible = false;
        await this.fetchDeviceList();
        await this.getList();
        this.$message.success(this.$t("common.删除成功"));
      } catch (e) {
        this.$message.error((e && e.msg) || (e && e.message) || this.$t("common.删除失败"));
      }
    },
    deleteNodeById(id, nodes) {
      for (let i = 0; i < nodes.length; i++) {
        if (nodes[i].id === id) {
          nodes.splice(i, 1);
          return;
        } else if (nodes[i].children) {
          this.deleteNodeById(id, nodes[i].children);
        }
      }
    },
    findNodeById(id, nodes) {
      for (const node of nodes) {
        if (node.id == id) return node;
        if (node.children) {
          const result = this.findNodeById(id, node.children);
          if (result) return result;
        }
      }
      return null;
    },
    formatDeviceType(row) {
      const dt =
        row && row.type === "virtual"
          ? row.deviceType != null
            ? row.deviceType
            : row.virtualDevType
          : row.deviceType;
      const hit = findDeviceTypeByValue(this.deviceTypes, dt);
      if (!hit) return dt != null ? String(dt) : "--";
      if (this.currentLocale === "en") return hit.en_label || hit.label;
      return hit.label;
    },
    formatNodeLabel(row) {
      const label = String((row && (row.label || row.name)) || "");
      const lower = label.toLowerCase();
      if (row && row.type === "device" && Number(row.deviceType) === 0) {
        return this.currentLocale === "en" ? "Local system" : this.$t("device.localSystemName");
      }
      if (row && row.type === "virtual" && (row.deviceType === 17 || lower.includes("bbms"))) {
        return this.currentLocale === "en" ? "Virtual BBMS" : "虚拟BBMS";
      }
      if (row && row.type === "device" && (Number(row.deviceType) === 6 || lower.includes("ac meter") || label.includes("交流电表"))) {
        return this.currentLocale === "en" ? "AC Meter" : "交流电表";
      }
      return label;
    },
    isLocalSystemTopologyNode(row) {
      return row && row.type === "device" && Number(row.deviceType) === 0;
    },
    isVirtualBbmsNode(row) {
      if (!row || row.type !== "virtual") return false;
      const label = String((row.label || row.name || "")).toLowerCase();
      return label.includes("bbms");
    },
    parentIsStackContainer(parentNode) {
      if (!parentNode) return false;
      if (parentNode.type === "device") return Number(parentNode.deviceType) === 1;
      if (parentNode.type === "virtual") {
        const vdt =
          parentNode.virtualDevType !== undefined && parentNode.virtualDevType !== null
            ? Number(parentNode.virtualDevType)
            : Number(parentNode.deviceType);
        if (vdt === 1) return true;
        const label = String((parentNode.label || parentNode.name) || "").toLowerCase();
        return /virtual\s*bbms|虚拟\s*bbms|堆控|stack\s*bms/.test(label);
      }
      return false;
    },
    validateTopologyPlacement({ menuTree, parentId, deviceType, originalNode }) {
      if (menuTree === 0) return "";
      if (deviceType === null || Number.isNaN(deviceType)) return this.$t("device.selectDeviceFirst");

      const parentNode = parentId ? this.findNodeById(parentId, this.treeData) : null;
      const editingNodeId = originalNode && originalNode.id ? String(originalNode.id) : "";

      if (deviceType === 0) {
        if (parentId != null && parentId !== "") return this.$t("device.localSystemMustRoot");
        const exists = this.flatTreeData.some((n) => {
          if (!n || n.type !== "device") return false;
          if (String(n.id) === editingNodeId) return false;
          return Number(n.deviceType) === 0;
        });
        if (exists) return this.$t("device.localSystemOnlyOne");
        return "";
      }
      if (deviceType === 1) {
        if (!parentNode) return this.$t("device.stackNeedParent");
        if (parentNode.type !== "device" || Number(parentNode.deviceType) !== 0) {
          return this.$t("device.stackOnlyUnderLocal");
        }
        return "";
      }
      if (!parentNode || !this.parentIsStackContainer(parentNode)) {
        return this.$t("device.onlyUnderStack");
      }
      return "";
    },
    async fetchDeviceList() {
      const res = await listDevice();
      this.deviceList = Array.isArray(res?.rows) ? res.rows : [];
    },
    async getList() {
      const res = await listTopology();
      const rows = Array.isArray(res?.data) ? res.data : [];
      this.treeData = rows;
      this.applyExpandAll();
    },
    /** 树表数据加载后展开全部层级（default-expand-all 仅在表格重渲时生效） */
    applyExpandAll() {
      this.isExpandAll = true;
      this.refreshTable = false;
      this.$nextTick(() => {
        this.refreshTable = true;
      });
    },
    toggleExpandAll() {
      this.refreshTable = false;
      this.isExpandAll = !this.isExpandAll;
      this.$nextTick(() => {
        this.refreshTable = true;
      });
    },

    /** 导入按钮操作 */
    handleImport() {
      this.upload.title = this.$t("device.设备拓扑导入");
      this.upload.open = true;
    },
    /** 下载模板操作 */
    importTemplate() {
      this.download(
        "topology/import-topologies-file",
        {},
        `topology_template_${new Date().getTime()}.xlsx`
      );
    },
    /** 文件上传中处理 */
    handleFileUploadProgress(event, file, fileList) {
      this.upload.isUploading = true;
    },
    /** 文件上传成功处理 */
    handleFileSuccess(response, file, fileList) {
      this.upload.open = false;
      this.upload.isUploading = false;
      this.$refs.upload.clearFiles();
      this.$alert(
        "<div style='overflow: auto;overflow-x: hidden;max-height: 70vh;padding: 10px 20px 0;'>" +
          response.msg +
          "</div>",
        this.$t("upload.导入结果"),
        { dangerouslyUseHTMLString: true }
      );
      this.getList();
    },
    /** 提交上传文件 */
    submitFileForm() {
      this.$refs.upload.submit();
    },
    /** 导出操作 */
    handleExport() {
      this.download(
        "topology/export-all-topologies",
        {},
        `topology_${new Date().getTime()}.xlsx`
      );
    },
  },
  async mounted() {
    await this.fetchDeviceList();
    await this.getList();
  },
};
</script>

<style>
/* 样式根据需要调整 */
.topology-container {
  padding: 20px;
}

.button-group {
  margin-bottom: 20px;
}

.tree-style {
  margin-top: 20px;
}
</style>
