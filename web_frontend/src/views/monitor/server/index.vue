<template>
  <div class="app-container">
    <el-row>
      <el-col :span="24" class="card-box">
        <el-card>
          <div slot="header">
            <span
              ><i class="el-icon-receiving"></i>
              {{ $t("system.版本信息") }}</span
            >
          </div>
          <div class="el-table el-table--enable-row-hover el-table--medium">
            <table cellspacing="0" style="width: 100%">
              <thead>
                <tr>
                  <th class="el-table__cell el-table__cell is-leaf">
                    <div class="cell">ID</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">Name</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">CPLD Version</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">DSP CPU Version</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">SN</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">SCU Version</div>
                  </th>
                </tr>
              </thead>
              <tbody v-if="pcsSoftVersion">
                <tr v-for="(pcsDevice, index) in pcsSoftVersion" :key="index">
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ index + 1 }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ pcsDevice.name || "PCS1" }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ pcsDevice.cpld_version || "/" }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">
                      {{ pcsDevice.dsp_cpu1_version || "/" }}
                    </div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">
                      {{ pcsDevice.dsp_cpu2_version || "/" }}
                    </div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ pcsDevice.pcs_version || "/" }}</div>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </el-card>
      </el-col>
      <el-col :span="12" class="card-box">
        <el-card>
          <div slot="header">
            <span><i class="el-icon-cpu"></i> CPU</span>
          </div>
          <div class="el-table el-table--enable-row-hover el-table--medium">
            <table cellspacing="0" style="width: 100%">
              <thead>
                <tr>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.属性") }}</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.值") }}</div>
                  </th>
                </tr>
              </thead>
              <tbody>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.核心数") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.cpu">
                      {{ server.cpu.cpuNum }}
                    </div>
                  </td>
                </tr>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.用户使用率") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.cpu">
                      {{ server.cpu.used }}%
                    </div>
                  </td>
                </tr>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.系统使用率") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.cpu">
                      {{ server.cpu.sys }}%
                    </div>
                  </td>
                </tr>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.当前空闲率") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.cpu">
                      {{ server.cpu.free }}%
                    </div>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </el-card>
      </el-col>

      <el-col :span="12" class="card-box">
        <el-card>
          <div slot="header">
            <span><i class="el-icon-tickets"></i> {{ $t("system.内存") }}</span>
          </div>
          <div class="el-table el-table--enable-row-hover el-table--medium">
            <table cellspacing="0" style="width: 100%">
              <thead>
                <tr>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.属性") }}</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.内存") }}</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">Python</div>
                  </th>
                </tr>
              </thead>
              <tbody>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.总内存") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.mem">
                      {{ server.mem.total }}
                    </div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.py">
                      {{ server.py.total }}
                    </div>
                  </td>
                </tr>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.已用内存") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.mem">
                      {{ server.mem.used }}
                    </div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.py">
                      {{ server.py.used }}
                    </div>
                  </td>
                </tr>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.剩余内存") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.mem">
                      {{ server.mem.free }}
                    </div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.py">
                      {{ server.py.free }}
                    </div>
                  </td>
                </tr>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.使用率") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div
                      class="cell"
                      v-if="server.mem"
                      :class="{ 'text-danger': server.mem.usage > 80 }"
                    >
                      {{ server.mem.usage }}%
                    </div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div
                      class="cell"
                      v-if="server.py"
                      :class="{ 'text-danger': server.py.usage > 80 }"
                    >
                      {{ server.py.usage }}%
                    </div>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </el-card>
      </el-col>

      <el-col :span="24" class="card-box">
        <el-card>
          <div slot="header">
            <span
              ><i class="el-icon-monitor"></i>
              {{ $t("system.服务器信息") }}</span
            >
          </div>
          <div class="el-table el-table--enable-row-hover el-table--medium">
            <table cellspacing="0" style="width: 100%">
              <tbody>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.服务器名称") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.sys">
                      {{ server.sys.computerName }}
                    </div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.操作系统") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.sys">
                      {{ server.sys.osName }}
                    </div>
                  </td>
                </tr>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.服务器IP") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.sys">
                      {{ server.sys.computerIp }}
                    </div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.系统架构") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.sys">
                      {{ server.sys.osArch }}
                    </div>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </el-card>
      </el-col>

      <el-col :span="24" class="card-box">
        <el-card>
          <div slot="header">
            <span
              ><i class="el-icon-coffee-cup"></i>
              {{ $t("system.Python解释器信息") }}</span
            >
          </div>
          <div class="el-table el-table--enable-row-hover el-table--medium">
            <table cellspacing="0" style="width: 100%; table-layout: fixed">
              <tbody>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.Python名称") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.py">
                      {{ server.py.name }}
                    </div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.Python版本") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.py">
                      {{ server.py.version }}
                    </div>
                  </td>
                </tr>
                <tr>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.启动时间") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.py">
                      {{ server.py.startTime }}
                    </div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.运行时长") }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.py">
                      {{ server.py.runTime }}
                    </div>
                  </td>
                </tr>
                <tr>
                  <td colspan="1" class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.安装路径") }}</div>
                  </td>
                  <td colspan="3" class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.py">
                      {{ server.py.home }}
                    </div>
                  </td>
                </tr>
                <tr>
                  <td colspan="1" class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.项目路径") }}</div>
                  </td>
                  <td colspan="3" class="el-table__cell is-leaf">
                    <div class="cell" v-if="server.sys">
                      {{ server.sys.userDir }}
                    </div>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </el-card>
      </el-col>

      <el-col :span="24" class="card-box">
        <el-card>
          <div slot="header">
            <span
              ><i class="el-icon-receiving"></i>
              {{ $t("system.磁盘状态") }}</span
            >
          </div>
          <div class="el-table el-table--enable-row-hover el-table--medium">
            <table cellspacing="0" style="width: 100%">
              <thead>
                <tr>
                  <th class="el-table__cell el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.盘符路径") }}</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.文件系统") }}</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.盘符名称") }}</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.总大小") }}</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.可用大小") }}</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.已用大小") }}</div>
                  </th>
                  <th class="el-table__cell is-leaf">
                    <div class="cell">{{ $t("system.已用百分比") }}</div>
                  </th>
                </tr>
              </thead>
              <tbody v-if="server.sysFiles">
                <tr v-for="(sysFile, index) in server.sysFiles" :key="index">
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ sysFile.dirName }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ sysFile.sysTypeName }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ sysFile.typeName }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ sysFile.total }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ sysFile.free }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div class="cell">{{ sysFile.used }}</div>
                  </td>
                  <td class="el-table__cell is-leaf">
                    <div
                      class="cell"
                      :class="{ 'text-danger': sysFile.usage > 80 }"
                    >
                      {{ sysFile.usage }}%
                    </div>
                  </td>
                </tr>
              </tbody>
            </table>
          </div>
        </el-card>
      </el-col>
    </el-row>
  </div>
</template>

<script>
import { getServer, getPcsSoftVersion } from "@/api/monitor/server";

export default {
  name: "Server",
  data() {
    return {
      // 服务器信息
      server: [],
      // PCS软件版本信息
      pcsSoftVersion: [],
    };
  },
  created() {
    this.getList();
    this.getPcsSoftVersion();
    this.openLoading();
  },
  methods: {
    /** 查询服务器信息 */
    getList() {
      getServer().then((response) => {
        this.server = response.data;
        this.$modal.closeLoading();
      });
    },
    // 打开加载层
    openLoading() {
      this.$modal.loading(this.$t("system.正在加载服务监控数据，请稍候！"));
    },
    /** 查询PCS软件版本信息 */
    getPcsSoftVersion() {
      getPcsSoftVersion().then((response) => {
        this.pcsSoftVersion = response.data;
        this.$modal.closeLoading();
      });
    },
  },
};
</script>
