class PointConstant:
    SCU_UPDATE_PROGRESS = 262
    DSP_CHANNEL_1_UPDATE_PROGRESS = 263
    DSP_CHANNEL_2_UPDATE_PROGRESS = 264
    DSP_UPDATE_PROGRESS_START = 265
    DSP_AUTO_SET_ADDR_RESULT_1 = 286  # can1编址结果
    DSP_AUTO_SET_ADDR_RESULT_2 = 287  # can2编址结果

class SignConstant:
    kSign_Start = 0
    kSign_SysReboot = kSign_Start    # 系统reboot标记
    kSign_SysSyncTime = 1            # 系统对时
    kSign_SysSetIp = 2               # 系统IP配置  
    kSign_SysUpdate = 3              # 本机系统软件升级
    kSign_DspSelectCanI = 4          # Can1通道升级信号，选择DSP1~DSP8，选择后bit0~bit7为1
    kSign_DspSelectCanII = 5         # Can2通道升级信号，选择DSP1~DSP8，选择后bit0~bit7为1
    kSign_DspSysCfgCanI = 6          # Can1通道 DSP系统配置信号，如果CAN1下面的哪一台配置了则 对应bit位置1，从bit0开始
    kSign_DspSysCfgCanII = 7         # Can2通道 DSP系统配置信号，如果CAN2下面的哪一台配置了则 对应bit位置1，从bit0开始
    kSign_DspRunCfgCanI = 8          # Can1通道 DSP运行配置信号，如果CAN1下面的哪一台配置了则 对应bit位置1，从bit0开始
    kSign_DspRunCfgCanII = 9         # Can2通道 DSP运行配置信号，如果CAN2下面的哪一台配置了则 对应bit位置1，从bit0开始
    kSign_DspProCfgCanI = 10         # Can1通道 DSP保护配置信号，如果CAN1下面的哪一台配置了则 对应bit位置1，从bit0开始
    kSign_DspProCfgCanII = 11        # Can2通道 DSP保护配置信号，如果CAN2下面的哪一台配置了则 对应bit位置1，从bit0开始
    kSign_DspAdvanceCfg = 12         # DSP高级配置标志
    kSign_DspAutoSetAddr = 13        # can通道的自动编制，低八位为1表示can1自动编制按钮点击，高八位为1表示can1自动编制按钮点击
    kSign_DspOnSwitch = 14          # DSP开机配置标志，如果配置了某台DSP开机，则对应bit位置1，从bit0开始
    kSign_DspOffSwitch = 15         # DSP关机机配置标志，如果配置了某台DSP关机，则对应bit位置1，从bit0开始
    kSign_DspResetReboot = 16       # DSP复位配置标志，如果配置了某台DSP复位，则对应bit位置1，从bit0开始
    kSign_DiDoLogicConfig = 17        # DIDO策略配置参数
    kSign_ExportCurDayLog = 18        # 导出当前日志标志
    kSign_Rev40 = kSign_Start + 39
    kSign_End = 40