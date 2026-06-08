<template>
  <div style="margin-top: 10px; margin-left: 30px">
    <el-tabs v-model="activeTab" @tab-click="handleTabClick">
      <el-tab-pane :label="$t('system.串口列表管理')" name="serial">
        <SerialManagement v-if="activeTab === 'serial'" />
      </el-tab-pane>
      <el-tab-pane :label="$t('system.网口列表管理')" name="network">
        <NetworkManagement v-if="activeTab === 'network'" />
      </el-tab-pane>
      <el-tab-pane :label="$t('system.CAN管理')" name="can">
        <CANManagement v-if="activeTab === 'can'" />
      </el-tab-pane>
      <!-- <el-tab-pane :label="$t('system.Wi-Fi管理')" name="wifi">
        <WifiManagement v-if="activeTab === 'wifi'" />
      </el-tab-pane>
      <el-tab-pane :label="$t('system.4G管理')" name="fourG">
        <FourGManagement v-if="activeTab === 'fourG'" />
      </el-tab-pane> -->
    </el-tabs>
  </div>
</template>

<script>
import SerialManagement from "./component/SerialManagement.vue";
import NetworkManagement from "./component/NetworkManagement.vue";
import CANManagement from "./component/CANManagement.vue";
import WifiManagement from "./component/WifiManagement.vue";
import FourGManagement from "./component/FourGManagement.vue";

export default {
  components: {
    SerialManagement,
    NetworkManagement,
    CANManagement,
    WifiManagement,
    FourGManagement,
  },
  data() {
    return {
      activeTab: "serial",
    };
  },
  methods: {
    handleTabClick(tab) {
      this.activeTab = tab.name;
    },
  },
};
</script>
