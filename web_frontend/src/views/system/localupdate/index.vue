<template>
  <div class="upgrade-container">
    <div class="card-container">
      <!-- 本机升级 -->
      <div
        class="card card-upgrade"
        :class="{ 'card-active': debUploaded }"
        style="height: auto; min-height: 400px"
      >
        <h3 class="card-title">{{ $t("system.本机升级") }}</h3>
        <p class="card-desc">
          {{ $i18n.locale === 'en' ? 'Upgrade BBMS controller system and application (deb package).' : $t('system.本机升级说明') }}
        </p>

        <div class="action" style="margin-bottom: 24px">
          <el-button
            type="primary"
            @click="debFileDialogVisible = true"
            :disabled="debUpgrading"
            >{{ $t("system.开始升级") }}</el-button
          >
        </div>

        <el-dialog
          :title="$t('system.上传Deb升级包')"
          :visible.sync="debFileDialogVisible"
          width="400px"
          append-to-body
        >
          <el-upload
            ref="uploadDeb"
            :limit="1"
            accept=".deb"
            :headers="{ Authorization: 'Bearer ' + token }"
            :action="uploadLocalUpdateUrl"
            :disabled="debUpgrading"
            :on-progress="handleDebUploadProgress"
            :on-success="handleDebUploadSuccess"
            :on-error="handleDebUploadError"
            :before-upload="beforeDebUpload"
            :on-change="handleDebFileChange"
            :auto-upload="false"
            drag
            class="upload-area"
          >
            <i class="el-icon-upload"></i>
            <div class="el-upload__text">
              {{ $t("upload.将文件拖到此处，或点击上传") }}
            </div>
            <div class="el-upload__tip text-center" slot="tip">
              <span>{{ $t("upload.仅允许上传Deb格式文件") }}</span>
            </div>
          </el-upload>
          <div slot="footer" class="dialog-footer">
            <el-button
              type="primary"
              @click="startDebUpgrade"
              :disabled="debUpgrading"
              >{{ $t("common.开始导入") }}</el-button
            >
            <el-button
              @click="debFileDialogVisible = false"
              :disabled="debUpgrading"
              >{{ $t("common.取消") }}</el-button
            >
          </div>
        </el-dialog>

        <div v-if="debUpgrading" style="margin-top: 20px">
          <el-progress :percentage="debProgress"></el-progress>
        </div>
      </div>

      <!-- DSP升级 -->
      <div class="card card-upgrade" :class="{ 'card-active': hexUploaded }">
        <h3 class="card-title">{{ $t("system.DSP升级") }}</h3>
        <p class="card-desc">
          {{
            ($i18n.locale || "").toLowerCase().indexOf("en") === 0
              ? "Upgrade PCS controller firmware for selected channels and units (hex package)."
              : $t("system.DSP升级说明")
          }}
        </p>

        <div class="dsp-channels">
          <div
            v-for="(channel, idx) in dspChannels"
            :key="idx"
            class="dsp-channel"
          >
            <el-checkbox
              v-model="channel.selected"
              :label="channel.name"
              class="channel-checkbox"
              :disabled="dspUpgrading"
              @change="handleChannelSelect(channel)"
            >
              {{ channel.name }}
            </el-checkbox>

            <div class="dsp-list">
              <el-checkbox
                v-for="(dsp, index) in channel.dsps"
                :key="index"
                :label="dsp.name"
                v-model="dsp.selected"
                :disabled="!channel.selected || dspUpgrading"
                class="dsp-checkbox"
              >
                {{ dsp.name }}
              </el-checkbox>
            </div>
          </div>
        </div>

        <div class="section">
          <label class="section-label">{{ $t("system.升级方式") }}</label>
          <el-radio-group
            v-model="dspUpgradeMode"
            :disabled="dspUpgrading || dspUpgradeMode === 'broadcast'"
          >
            <el-radio label="broadcast" :disabled="true">{{
              $t("system.广播升级")
            }}</el-radio>
            <el-radio label="sequential">{{ $t("system.依次升级") }}</el-radio>
          </el-radio-group>
        </div>

        <div class="section">
          <label class="section-label">{{
            $t("system.选择升级目标CPU")
          }}</label>
          <el-radio-group v-model="selectedCpu" :disabled="dspUpgrading">
            <el-radio :label="1">CPU1</el-radio>
            <el-radio :label="2">CPU2</el-radio>
          </el-radio-group>
        </div>

        <div class="action">
          <el-button
            type="primary"
            @click="hexFileDialogVisible = true"
            :disabled="!isAnyDspSelected || dspUpgrading"
          >
            {{ $t("system.开始升级") }}
          </el-button>
        </div>

        <!-- 升级弹窗 -->
        <el-dialog
          :title="$t('system.上传Hex升级包')"
          :visible.sync="hexFileDialogVisible"
          width="400px"
          append-to-body
        >
          <el-upload
            ref="uploadHex"
            :limit="1"
            accept=".hex"
            :headers="{ Authorization: 'Bearer ' + token }"
            :action="`${uploadDspUpdateUrl}?cpu=${selectedCpu}`"
            :disabled="dspUpgrading"
            :on-progress="handleHexUploadProgress"
            :on-success="handleHexUploadSuccess"
            :on-error="handleHexUploadError"
            :before-upload="beforeHexUpload"
            :on-change="handleHexFileChange"
            :auto-upload="false"
            drag
          >
            <i class="el-icon-upload"></i>
            <div class="el-upload__text">
              {{ $t("upload.将文件拖到此处，或点击上传") }}
            </div>
            <div class="el-upload__tip text-center" slot="tip">
              <span>{{ $t("upload.仅允许上传Hex格式文件") }}</span>
            </div>
          </el-upload>
          <div slot="footer" class="dialog-footer">
            <el-button
              type="primary"
              @click="startDspUpgrade"
              :disabled="dspUpgrading"
              >{{ $t("common.开始导入") }}</el-button
            >
            <el-button
              @click="hexFileDialogVisible = false"
              :disabled="dspUpgrading"
              >{{ $t("common.取消") }}</el-button
            >
          </div>
        </el-dialog>

        <!-- 升级进度 -->
        <div v-if="dspUpgrading" class="upgrade-progress">
          <template v-if="dspUpgradeMode === 'broadcast'">
            <div v-for="channel in selectedChannels" :key="channel">
              <h4 class="progress-title">
                {{ $t("system.通道") }} {{ channel }}
                {{ $t("system.升级进度") }}
              </h4>
              <el-progress :percentage="channelProgress[channel]"></el-progress>
            </div>
          </template>
          <template v-else-if="dspUpgradeMode === 'sequential'">
            <div v-for="(dsp, index) in sequentialUpgradeQueue" :key="index">
              <h4 class="progress-title">
                {{ $t("system.正在升级") }}: {{ dsp.channelName }} -
                {{ dsp.dspName }}
              </h4>
              <el-progress
                :percentage="dsp.progress"
                :status="dsp.progress >= 100 ? 'success' : 'active'"
              />
            </div>
          </template>
        </div>
      </div>

      <!-- DSP编址功能 -->
      <div class="card card-upgrade">
        <h3 class="card-title">{{ $t("system.DSP编址功能") }}</h3>
        <div class="section">
          <el-checkbox-group
            v-model="selectedAddressingChannels"
            :disabled="addressing"
          >
            <el-checkbox :label="1">{{
              $t("system.can0通道编址")
            }}</el-checkbox>
            <el-checkbox :label="2">{{
              $t("system.can1通道编址")
            }}</el-checkbox>
          </el-checkbox-group>
        </div>
        <div class="action">
          <el-button
            type="primary"
            @click="handleDspAddressing"
            :disabled="addressing || selectedAddressingChannels.length === 0"
            :loading="addressing"
          >
            {{ addressing ? $t("system.编址下发中") : $t("system.确认编址") }}
          </el-button>
        </div>
      </div>
    </div>

    <!-- 升级完成弹窗 -->
    <!-- <el-dialog
      :title="$t('system.升级完成')"
      :visible.sync="upgradeDialogVisible"
    >
      <span>{{ $t("system.升级已完成本机需要重启后生效") }}</span>
      <span slot="footer">
        <el-button @click="handleRebootLater">{{
          $t("system.稍后重启")
        }}</el-button>
        <el-button type="primary" @click="handleRebootNow">{{
          $t("system.立即重启")
        }}</el-button>
      </span>
    </el-dialog> -->

    <!-- DSP升级成功弹窗 -->
    <el-dialog
      :title="$t('system.DSP升级成功')"
      :visible.sync="dspUpgradeSuccessDialogVisible"
    >
      <span>{{ $t("system.DSP升级已成功完成") }}</span>
      <span slot="footer">
        <el-button type="primary" @click="handleDspUpgradeSuccessConfirm">{{
          $t("common.确认")
        }}</el-button>
      </span>
    </el-dialog>

    <!-- DSP升级失败弹窗 -->
    <el-dialog
      :title="$t('system.DSP升级失败')"
      :visible.sync="dspUpgradeFailureDialogVisible"
      width="40%"
      :style="{ top: '20vh' }"
    >
      <div
        style="
          font-size: 16px;
          color: #ff4d4f;
          font-weight: bold;
          margin-bottom: 10px;
        "
      >
        <i class="el-icon-error" style="margin-right: 5px"></i>
        {{ $t("system.DSP升级失败") }}
      </div>
      <div style="font-size: 14px; color: #333">
        <span v-if="dspFailureDetails"
          ><strong> DSP {{ dspFailureDetails.code }}</strong
          >&nbsp;&nbsp;&nbsp; {{ $t("system.错误码") }}:
          <strong>{{ getErrorMessage(dspFailureDetails.errorCode) }}</strong>
        </span>
        <span v-else> {{ $t("system.DSP升级失败请重试") }} </span>
      </div>
      <span slot="footer">
        <el-button type="primary" @click="handleDspUpgradeFailureConfirm">{{
          $t("common.确认")
        }}</el-button>
      </span>
    </el-dialog>

    <!-- DSP编址成功弹窗 -->
    <el-dialog
      :title="$t('system.DSP编址成功')"
      :visible.sync="dspAddressingSuccessDialogVisible"
      width="30%"
      :style="{ top: '20vh' }"
    >
      <span
        >{{ $t("system.通道") }}{{ successChannels.join(", ")
        }}{{ $t("system.编址已成功完成") }}</span
      >
      <span slot="footer">
        <el-button
          type="primary"
          @click="dspAddressingSuccessDialogVisible = false"
          >{{ $t("common.确认") }}</el-button
        >
      </span>
    </el-dialog>

    <!-- DSP编址失败弹窗 -->
    <el-dialog
      :title="$t('system.DSP编址失败')"
      :visible.sync="dspAddressingFailureDialogVisible"
      width="30%"
      :style="{ top: '20vh' }"
    >
      <span v-if="failedChannels.length === 0">{{
        $t("system.DSP编址失败请重试")
      }}</span>
      <span v-else
        >{{ $t("system.通道") }}{{ failedChannels.join(", ")
        }}{{ $t("system.编址失败请重试") }}</span
      >
      <span slot="footer">
        <el-button
          type="primary"
          @click="dspAddressingFailureDialogVisible = false"
          >{{ $t("common.确认") }}</el-button
        >
      </span>
    </el-dialog>
  </div>
</template>

<script>
import {
  confirmLocalUpgrade,
  getLocalUpgradeProgress,
  confirmDspUpgrade,
  getDspUpgradeProgress,
  rebootAfterLocalUpgrade,
  setDspAddressing,
  getDspAddressing,
} from "@/api/system/localupdate";
import { getToken } from "@/utils/auth";
import { fetchDspDevices } from "@/api/system/controlstrategy";

export default {
  data() {
    return {
      selectedDSPs: [[], []], // 修改为二维数组
      dspDevices: {}, // 存储DSP设备信息
      uploadDspUpdateUrl:
        process.env.VUE_APP_BASE_API + "/localupdate/upload/dsp",
      uploadLocalUpdateUrl:
        process.env.VUE_APP_BASE_API + "/localupdate/upload/local",
      debUploaded: false,
      debUpgrading: false,
      debProgress: 0,
      hexUploaded: false,
      dspUpgrading: false,
      dspProgress: 0,
      channel1Progress: 0,
      channel2Progress: 0,
      dspUpgradeMode: "sequential", // 默认设置为依次升级
      upgradeDialogVisible: false,
      dspUpgradeSuccessDialogVisible: false, // DSP升级成功弹窗可见性
      dspUpgradeFailureDialogVisible: false, // DSP升级失败弹窗可见性
      dspFailureDetails: null, // DSP升级失败详情
      debFileDialogVisible: false,
      hexFileDialogVisible: false,
      selectedChannel: [],
      channelProgress: { 1: 0, 2: 0 }, // 广播模式下各通道进度
      sequentialUpgradeQueue: [], // 串行模式下的升级队列
      currentSequentialIndex: 0, // 串行模式当前升级的DSP索引
      dspChannels: [], // 动态加载DSP通道
      token: getToken(),
      selectedCpu: 1, // 默认值
      selectedAddressingChannels: [], // 选中的编址通道
      addressing: false, // 是否正在编址
      dspAddressingSuccessDialogVisible: false, // DSP编址成功弹窗可见性
      dspAddressingFailureDialogVisible: false, // DSP编址失败弹窗可见性
      successChannels: [], // 成功的通道
      failedChannels: [], // 失败的通道
    };
  },
  computed: {
    isAnyDspSelected() {
      return this.dspChannels.some((channel) =>
        channel.dsps.some((dsp) => dsp.selected)
      );
    },
    selectedChannels() {
      return this.dspChannels
        .filter((channel) => channel.selected)
        .map((channel) => channel.value);
    },
    selectedDsps() {
      const selected = [];
      this.dspChannels.forEach((channel) => {
        if (channel.selected) {
          channel.dsps.forEach((dsp) => {
            if (dsp.selected) {
              selected.push({
                channel: channel.value,
                channelName: channel.name,
                dspName: dsp.name,
                dspCode: dsp.code,
                progress: 0,
              });
            }
          });
        }
      });
      return selected;
    },
  },
  mounted() {
    this.fetchDspCount(); // 页面加载时获取DSP通道与设备
  },
  methods: {
    handleChannelSelect(selectedChannel) {
      this.dspChannels.forEach((channel) => {
        if (channel !== selectedChannel) {
          channel.selected = false;
          channel.dsps.forEach((dsp) => {
            dsp.selected = false; // 清空之前选择的DSP
          });
        }
      });
      selectedChannel.selected = true;
    },
    handleDspAddressing() {
      if (this.selectedAddressingChannels.length === 0) {
        this.$message.warning(this.$t("system.请选择要编址的通道"));
        return;
      }

      this.addressing = true;
      const channels = this.selectedAddressingChannels.join(",");

      setDspAddressing(channels)
        .then(() => {
          let attempts = 0;
          const maxAttempts = 10; // 20秒 / 2秒 = 10次

          const checkStatus = () => {
            getDspAddressing(channels)
              .then((response) => {
                if (response.code === 200 && response.data.result === 1) {
                  this.addressing = false;
                  this.dspAddressingSuccessDialogVisible = true;
                  this.successChannels = response.data.success_channels;
                  this.selectedAddressingChannels = []; // 清空编址选择
                } else {
                  attempts++;
                  if (attempts < maxAttempts) {
                    setTimeout(checkStatus, 2000);
                  } else {
                    this.addressing = false;
                    this.failedChannels = response.data.failed_channels;
                    this.dspAddressingFailureDialogVisible = true;
                  }
                }
              })
              .catch(() => {
                attempts++;
                if (attempts < maxAttempts) {
                  setTimeout(checkStatus, 2000);
                } else {
                  this.addressing = false;
                  this.dspAddressingFailureDialogVisible = true;
                }
              });
          };

          setTimeout(checkStatus, 2000);
        })
        .catch(() => {
          this.addressing = false;
          this.dspAddressingFailureDialogVisible = true;
        });
    },
    async fetchDspCount() {
      try {
        const dspDict = await fetchDspDevices();
        this.dspDevices = dspDict.data; // 存储DSP设备信息

        // 按 BBMS 项目语义重命名：
        // - 通道 0：RBMS 簇级 BMS 升级（RBMS-01 ~ RBMS-xx）
        // - 通道 1：PCS 升级（PCS-01 ~ PCS-xx）
        this.dspChannels = Object.entries(this.dspDevices).map(
          ([channel, dsps]) => {
            const ch = parseInt(channel);
            let channelName = "";
            if (ch === 0) {
              channelName = "RBMS (CAN0)";
            } else if (ch === 1) {
              channelName = "PCS (CAN1)";
            } else {
              channelName = this.$t("system.通道") + ch;
            }

            return {
              name: channelName,
              value: ch + 1,
              selected: false,
              dsps: dsps.map((dspId, idx) => {
                const idNum = parseInt(dspId);
                // RBMS 通道命名为 RBMS-xx，其它通道保持 PCS-xx
                const isRbmsChannel = ch === 0;
                const labelPrefix = isRbmsChannel ? "RBMS" : "PCS";
                const displayId = Number.isNaN(idNum) ? idx + 1 : idNum;
                return {
                  name: `${labelPrefix}-${displayId.toString().padStart(2, "0")}`,
                  selected: false,
                  progress: 0,
                  code: dspId, // 实际下发仍用原始 DSP 编号
                };
              }),
            };
          }
        );
      } catch (err) {
        this.$message.error(this.$t("system.获取DSP数量失败"));
      }
    },
    beforeDebUpload(file) {
      const isDeb = file.name.endsWith(".deb");
      if (!isDeb) this.$message.error(this.$t("system.只能上传deb文件"));
      return isDeb;
    },
    handleDebFileChange(file) {
      this.debFileDialogVisible = true;
    },
    handleDebUploadProgress(event, file, fileList) {
      this.debUpgrading = true;
    },

    startDebUpgrade() {
      if (this.$refs.uploadDeb && this.$refs.uploadDeb.uploadFiles.length) {
        // 手动触发上传
        this.$refs.uploadDeb.submit();
        this.debFileDialogVisible = false;
      } else {
        this.$message.error(this.$t("system.请先选择一个deb文件"));
      }
    },

    handleDebUploadSuccess(response, file, fileList) {
      if (response.code === 200) {
        this.$confirm(
          this.$t("system.文件上传成功，是否开始升级？"),
          this.$t("common.确认"),
          {
            confirmButtonText: this.$t("system.开始升级"),
            cancelButtonText: this.$t("common.取消"),
            type: "success",
          }
        )
          .then(() => {
            this.debFileDialogVisible = false; // 关闭弹窗
            confirmLocalUpgrade()
              .then(() => {
                this.debUpgrading = true;
                this.debProgress = 0;
                const interval = setInterval(() => {
                  getLocalUpgradeProgress().then((response) => {
                    this.debProgress = response.data.progress;
                    if (this.debProgress >= 100) {
                      clearInterval(interval);
                      // 直接发送重启指令
                      rebootAfterLocalUpgrade()
                        .then((response) => {
                          if (response.code == 200) {
                            this.$message({
                              type: "success",
                              message: this.$t("system.重启中"),
                            });
                            setTimeout(() => {
                              location.reload();
                            }, 3000);
                          } else {
                            this.$message.error(
                              this.$t("system.重启失败，请重试")
                            );
                          }
                        })
                        .catch(() => {
                          this.$message.error(
                            this.$t("system.重启请求失败，请检查网络连接")
                          );
                        });
                    }
                  });
                }, 1000);
              })
              .catch(() => {
                this.$message.error(this.$t("system.确认升级失败"));
              });
          })
          .catch(() => {
            this.$message.info(this.$t("system.已取消升级"));
          });
      } else {
        this.$alert(
          (response.msg || this.$t("system.文件上传失败，请重新上传")) +
            "," +
            this.$t("system."),
          this.$t("system.错误"),
          {
            confirmButtonText: this.$t("system.确定"),
          }
        );
        this.$refs.uploadDeb.clearFiles();
      }
    },
    handleDebUploadError(err, file, fileList) {
      this.$alert(
        this.$t("system.文件上传失败，请重新上传"),
        this.$t("system.错误"),
        {
          confirmButtonText: this.$t("system.确定"),
        }
      );
      this.$refs.uploadDeb.clearFiles();
    },
    beforeHexUpload(file) {
      const isHex = file.name.endsWith(".hex");
      if (!isHex) this.$message.error(this.$t("system.只能上传hex文件"));
      return isHex;
    },
    handleHexFileChange(file) {
      this.hexFileDialogVisible = true;
    },
    handleHexUploadProgress(event, file, fileList) {
      this.dspUpgrading = true;
    },

    handleHexUploadError(err, file, fileList) {
      this.$alert(
        this.$t("system.文件上传失败，请重新上传"),
        this.$t("system.错误"),
        {
          confirmButtonText: this.$t("system.确定"),
        }
      );
      this.$refs.uploadHex.clearFiles();
    },
    startDspUpgrade() {
      if (this.$refs.uploadHex && this.$refs.uploadHex.uploadFiles.length) {
        // 手动触发上传
        this.$refs.uploadHex.submit();
        this.hexFileDialogVisible = false;
      } else {
        this.$message.error(this.$t("system.请先选择一个hex文件"));
      }
    },
    handleRebootNow() {
      rebootAfterLocalUpgrade()
        .then((response) => {
          if (response.code == 200) {
            this.$message({
              type: "success",
              message: this.$t("system.重启中"),
            });
            this.upgradeDialogVisible = false;
            setTimeout(() => {
              this.$message({
                type: "success",
                message: this.$t("system.重启成功"),
              });
              location.reload();
            }, 3000);
          } else {
            this.$message.error(this.$t("system.重启失败，请重试"));
          }
        })
        .catch(() => {
          this.$message.error(this.$t("system.重启请求失败，请检查网络连接"));
        });
    },
    handleRebootLater() {
      location.reload();
    },
    handleHexUploadSuccess(response, file, fileList) {
      if (response.code === 200) {
        this.$confirm(
          this.$t("system.文件上传成功，是否开始升级？"),
          this.$t("common.确认"),
          {
            confirmButtonText: this.$t("system.开始升级"),
            cancelButtonText: this.$t("common.取消"),
            type: "success",
          }
        )
          .then(() => {
            this.hexFileDialogVisible = false;
            const dspParam = {
              mode: this.dspUpgradeMode,
              channel: this.selectedChannels,
              cpu: this.selectedCpu,
              channel1_count: this.dspChannels[0].dsps.length,
              channel2_count: this.dspChannels[1].dsps.length,
              channel1_dsps: this.dspChannels[0].selected
                ? this.dspChannels[0].dsps
                    .map((dsp, index) => (dsp.selected ? index + 1 : null))
                    .filter((index) => index !== null)
                : [],
              channel2_dsps: this.dspChannels[1].selected
                ? this.dspChannels[1].dsps
                    .map((dsp) => (dsp.selected ? dsp.code : null))
                    .filter((code) => code !== null)
                : [],
            };

            confirmDspUpgrade(dspParam)
              .then(() => {
                this.dspUpgrading = true;

                if (this.dspUpgradeMode === "broadcast") {
                  // 广播模式 - 按通道显示进度
                  this.startBroadcastUpgrade();
                } else {
                  // 串行模式 - 按DSP顺序显示进度
                  this.startSequentialUpgrade();
                }
              })
              .catch(() => {
                this.$message.error(this.$t("system.确认升级失败"));
              });
          })
          .catch(() => {
            this.$message.info(this.$t("system.已取消升级"));
          });
      } else {
        this.$alert(
          response.msg || this.$t("system.文件上传失败"),
          this.$t("system.错误"),
          {
            confirmButtonText: this.$t("system.确定"),
          }
        );
        this.$refs.uploadHex.clearFiles();
      }
    },

    // startBroadcastUpgrade() {
    //   // 初始化通道进度
    //   this.channelProgress = { 1: 0, 2: 0 };

    //   const checkProgress = () => {
    //     const progressParam = {
    //       mode: "broadcast",
    //       channel: this.selectedChannels,
    //     };

    //     getDspUpgradeProgress(progressParam)
    //       .then((response) => {
    //         if (response.code === 200) {
    //           // 更新各通道进度
    //           this.selectedChannels.forEach((channel) => {
    //             const progress = response.data[channel] || 0;
    //             if (progress < 0) {
    //               this.dspUpgrading = false;
    //               this.dspFailureDetails = {
    //                 code: response.data.dspCode,
    //                 errorCode: progress,
    //               };
    //               this.dspUpgradeFailureDialogVisible = true; // 显示失败弹窗
    //               return;
    //             }
    //             this.channelProgress[channel] = progress;
    //           });

    //           // 检查是否全部完成
    //           const allCompleted = this.selectedChannels.every(
    //             (channel) => this.channelProgress[channel] >= 100
    //           );

    //           if (!allCompleted) {
    //             setTimeout(checkProgress, 1000);
    //           } else {
    //             this.dspUpgradeSuccessDialogVisible = true; // 显示DSP升级成功弹窗
    //             this.dspUpgrading = false;
    //           }
    //         }
    //       })
    //       .catch(() => {
    //         this.$message.error("获取升级进度失败");
    //         this.dspUpgrading = false;
    //       });
    //   };

    //   checkProgress();
    // },

    startSequentialUpgrade() {
      // 初始化升级队列
      this.sequentialUpgradeQueue = this.selectedDsps;
      this.currentSequentialIndex = 0;

      // 开始第一个DSP升级
      this.upgradeNextDsp();
    },

    upgradeNextDsp() {
      if (this.currentSequentialIndex >= this.sequentialUpgradeQueue.length) {
        this.dspUpgradeSuccessDialogVisible = true; // 显示DSP升级成功弹窗
        this.dspUpgrading = false;
        return;
      }

      const currentDsp =
        this.sequentialUpgradeQueue[this.currentSequentialIndex];
      currentDsp.progress = 0;

      const checkProgress = () => {
        const progressParam = {
          mode: "sequential",
          channel: [currentDsp.channel],
          dsp_id: currentDsp.dspCode,
        };

        getDspUpgradeProgress(progressParam)
          .then((response) => {
            if (response.code === 200) {
              const progress = response.data[currentDsp.channel] || 0;
              if (progress < 0) {
                this.dspUpgrading = false;
                this.dspFailureDetails = {
                  code: currentDsp.dspCode,
                  errorCode: progress,
                };
                this.dspUpgradeFailureDialogVisible = true; // 显示失败弹窗
                return;
              }
              currentDsp.progress = progress;

              if (currentDsp.progress < 100) {
                setTimeout(checkProgress, 1000);
              } else {
                // 当前DSP升级完成，开始下一个
                this.currentSequentialIndex++;
                this.upgradeNextDsp();
              }
            }
          })
          .catch(() => {
            this.$message.error(
              this.$t("system.获取") +
                currentDsp.dspName +
                this.$t("system.升级进度失败")
            );
            this.dspUpgrading = false;
          });
      };

      checkProgress();
    },

    handleDspUpgradeSuccessConfirm() {
      this.dspUpgradeSuccessDialogVisible = false;
      location.reload();
    },

    handleDspUpgradeFailureConfirm() {
      this.dspUpgradeFailureDialogVisible = false;
      location.reload();
    },
    getErrorMessage(errorCode) {
      const errorMessages = {
        1: "ERR_FRAME_HEADER_ASSIGN_FAILED",
        2: "ERR_DSP_NOT_INIT",
        3: "ERR_HEX_NOT_FOUND",
        4: "ERR_HEX_TO_BIN",
        5: "ERR_1807_CMD_FAILED",
        6: "ERR_1A06_RESP_FAILED",
        7: "ERR_UPGRADE_CONDITION_TIMEOUT",
        8: "ERR_ENTER_BOOT_MODE_TIMEOUT",
        9: "ERR_FLASH_ERASE_TIMEOUT",
        0: "ERR_START_TRANSFER_TIMEOUT",
        11: "ERR_TRANSFER_DATA_TIMEOUT",
        12: "ERR_END_TRANSFER_TIMEOUT",
        13: "ERR_VERIFY_CRC_TIMEOUT",
        14: "ERR_REBOOT_TIMEOUT",
        15: "ERR_CAN_SEND",
        16: "ERR_CAN_RECV",
        17: "ERR_MEMORY",
      };
      return errorMessages[-errorCode] || "UNKNOWN ERROR";
    },
  },
};
</script>

<style scoped>
.upgrade-container {
  display: flex;
  justify-content: center;
  padding: 20px;
  background: #f5f7fa;
  min-height: 100vh; /* 确保背景色覆盖全屏 */
}
.card-container {
  display: flex;
  gap: 20px;
}
.card {
  background: white;
  padding: 20px;
  border-radius: 12px;
  box-shadow: 0 2px 8px rgba(0, 0, 0, 0.1);
  width: 400px;
  transition: box-shadow 0.3s;
}
.card-active {
  box-shadow: 0 4px 16px rgba(0, 0, 0, 0.2);
}

.card-upgrade {
  height: auto;
  padding: 24px;
  font-family: "Helvetica Neue", Helvetica, Arial, sans-serif;
  background: #fff;
  border-radius: 16px;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.06);
}
.card-title {
  font-size: 20px;
  font-weight: bold;
  margin-bottom: 24px;
}
.dsp-channels {
  display: flex;
  flex-direction: column;
  gap: 16px;
  margin-bottom: 24px;
}
.dsp-channel {
  border: 1px solid #e5e7eb;
  padding: 16px;
  border-radius: 8px;
}
.channel-checkbox {
  font-weight: 600;
  font-size: 16px;
  margin-bottom: 8px;
  display: block;
}
.dsp-list {
  padding-left: 20px;
  display: flex;
  flex-wrap: wrap;
  gap: 12px;
  margin-top: 8px;
}
.dsp-checkbox {
  font-size: 14px;
  margin: 0;
}
.section {
  margin-bottom: 20px;
}
.section-label {
  display: block;
  font-weight: 600;
  margin-bottom: 8px;
}
.action {
  margin-bottom: 20px;
}
.progress-title {
  font-size: 14px;
  color: gray;
  margin-bottom: 8px;
}
.upgrade-progress {
  margin-top: 24px;
}
</style>
