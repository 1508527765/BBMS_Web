// clang-format off
/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            kit_model.h
 * @brief           数据存储模块源文件
 * @author          gary
 * @date            2025-07-10
 * @remark
 *****************************************************************************/
#ifndef KIT_MODEL_H
#define KIT_MODEL_H

#include "kit_data.h"

#define DEV_IS_ONLINE 0

typedef struct
{
    uint16_t devNum;
    uint16_t devPointNum;
} devModeAttr_t;

// 接入所有设备类型数量-用数据替换方便获取
extern uint16_t gStDevTypeNum[kDev_Type_End];
extern uint16_t gStDevTypePointNum[kDev_Type_End];
extern devModeAttr_t gStModeInfo[kDev_Type_End];

//-------------------------本机信息------------------------------
typedef enum
{
    kCon_Start = 0,                      // 开始
    kCon_Online = kCon_Start,            // 本机是否在线
    kCon_Status_Start,                   // 状态
    kCon_Status_1 = kCon_Status_Start,   // 状态-1
    kCon_Status_20 = kCon_Status_1 + 19, // 状态-20
    kCon_Status_End,                     // 结束索引21，这个不要改动

    // 补充DI3-40状态
    kCon_Status_DI_Start = kCon_Status_End,  // 状态-数字输入40预定义40个DI
    kCon_Status_DI1 = kCon_Status_DI_Start,  // DI1状态
    kCon_Status_DI40 = kCon_Status_DI1 + 39, // DI40状态
    kCon_Status_DI_End,

    kCon_Status_DO_Start = kCon_Status_DI_End, // 状态-数字输出40预定义40个DO
    kCon_Status_DO1 = kCon_Status_DO_Start,    // DO1状态
    kCon_Status_DO40 = kCon_Status_DO1 + 39,   // DO40状态
    kCon_Status_DO_End,                        // 数字输出结束标识

    kCon_Yc_Data_Start = kCon_Status_DO_End,
    kCon_CpuTotalUsage = kCon_Yc_Data_Start, // CPU总利用率
    kCon_ProcessCpuUsage,                    // 当前进程CPU占用
    kCon_MemTotalSize,                       // 内存总大小
    kCon_MemUsageRate,                       // 内存占用率
    kCon_ProcessMemUsage,                    // 当前进程内存占用
    kCon_netStatus,                          // 网络状态 1:连接外网，0:未连接外网
    kCon_RunHeartCount,                      // 运行心跳 自增
    kCon_TestCount,                          // 测试次数
    kCon_UpgradeConPercent,                  // 升级控制器进度
    kCon_YCData50 = kCon_Yc_Data_Start + 49,
    kCon_YCDataEnd,

    /*接入本控制器的设备数量*/
    kCon_DevNumStart = kCon_YCDataEnd, // 设备数量开始索引:151
    kCon_Num = kCon_DevNumStart,       // 本机数量
    kCon_BBMSNum,                      // BMS堆控数量
    kCon_RBMSNum,                      // BMS簇控制器数量
    kCon_PCSNum,                       // PCS 类型数量
    kCon_TransferNum,                  // 变压器数量
    kCon_LqdNum,                       // 空调/液冷机组数量
    kCon_MeterNum,                     // 电表设备数量
    kCon_UPSNum,                       // UPS设备数量
    kCon_FireDeviceNum,                // 消防设备数量
    kCon_dehumidifierNum,              // 除湿机数量
    kCon_TempSensorNum,                // 温度传感器数量
    kCon_HumiditySensorNum,            // 湿度传感器数量
    kCon_WaterSensorNum,               // 水浸传感器数量
    kCon_SmokeSensorNum,               // 烟雾传感器数量
    kCon_OtherSensor1Num,              // 其他传感器1数量
    kCon_OtherSensor2Num,              // 其他传感器2数量
    kCon_OtherSensor3Num,              // 其他传感器3数量
    kCon_Reserve_17,                   // 预留数量
    kCon_Reserve_18,                   // 预留数量
    kCon_Reserve_19,                   // 预留数量
    kCon_Reserve_20,                   // 预留数量
    kCon_Reserve_21,                   // 预留数量
    kCon_Reserve_22,                   // 预留数量
    kCon_Reserve_23,                   // 预留数量
    kCon_Reserve_24,                   // 预留数量
    kCon_Reserve_25,                   // 预留数量
    kCon_Reserve_26,                   // 预留数量
    kCon_Reserve_27,                   // 预留数量
    kCon_Reserve_28,                   // 预留数量
    kCon_Reserve_29,                   // 预留数量
    kCon_DevNumEnd,                    // 设备数量结束索引:181

    // 预留点位到200
    kCon_Reserve_Start = kCon_DevNumEnd,         // 控制预留开始
    kCon_Reserve_Yc_1 = kCon_Reserve_Start,      // 预留Yc数据开始
    kCon_Reserve_Yc_20 = kCon_Reserve_Yc_1 + 19, // 预留Yc数据结束索引:20
    kCon_Reserve_YcData_End,                     // 预留Yc数据结束 索引201

    /*控制类数据：DO信号 */
    kCon_Do_Ctrl_Start = kCon_Reserve_YcData_End, // DO信号从201-240
    kCon_Do1_Ctrl = kCon_Do_Ctrl_Start,           // DO1输出
    kCon_Do40_Ctrl = kCon_Do_Ctrl_Start + 39,     // DO40输出
    kCon_Do_Ctrl_End,                             // DO信号结束 索引值241

    /*添加预留即可*/
    kCon_Ctrl_Reserve_Start = kCon_Do_Ctrl_End, // 控制预留开始
    kCon_CtrlEnable = kCon_Ctrl_Reserve_Start,  // 控制详细日志打印开关 0:日志输出不使能；1:日志输出使能
    kCon_LogicLog_Enable,                       // 保护日志开关
    kCon_TcpLog_Enable,                         // 网口通讯报文日志开关
    kCon_UartLog_Enable,                        // 串口通讯报文日志开关

    /*预留数据244~299*/

    kCon_DataEnd = 300,
} devConSelfInfo_E;

// BBMS(电池管理系统)测点枚举 - 完整系统信息
typedef enum
{
    kBbms_Data_Start,
    kBbms_Online = kBbms_Data_Start,
    kBbms_BatSt,                 // Bank Status Bank工作状态
    kBbms_ChSt,                  // Bank Charging Status Bank充放电状态
    kBbms_SysOperSt,             // Bank System Operating Status Bank系统运行状态
    kBbms_PmtRkFltNum,           // Min number of Racks Allowed to Run 允许运行的最小Rack数量
    kBbms_SetInverterSt,         // Set Inverter Status 设置逆变器状态
    kBbms_RackFltLvl1Nbr,        // Positions of Rack with Emergency Fault  存在严重故障的Rack位置
    kBbms_CntrSt,                // Bank Contactor Status Bank接触器状态
    kBbms_SOC,                   // System SOC (%)  系统SOC (%)
    kBbms_SOH,                   // System SOH (%)  系统SOH (%)
    kBbms_MaxChrgE,              // SOE for Charge (kWh)  系统可充能量 (kWh)
    kBbms_MaxDisChrgE,           // SOE for Discharge (kWh)  系统可放能量 (kWh)
    kBbms_MaxContChrgPower,      // Const Charge Power Limit (kW)  持续充电功率限制 (kW)
    kBbms_Max60sChrgPower,       // Pulse Charge Power Limit (kW)  脉冲充电功率限制 (kW)
    kBbms_MaxContDisChrgPower,   // Const Discharge Power Limit (kW)  持续放电功率限制 (kW)
    kBbms_Max60sDisChrgPower,    // Pulse Discharge Power Limit (kW)  脉冲放电功率限制 (kW)
    kBbms_MaxContChrgCur,        // Const Charge Curr Limit (A)  持续充电电流限制 (A)
    kBbms_Max60sChrgCur,         // Pulse Charge Curr Limit (A)  脉冲充电电流限制 (A)
    kBbms_MaxContDisChrgCur,     // Const Discharge Curr Limit (A)  持续放电电流限制 (A)
    kBbms_Max60sDisChrgCur,      // Pulse Discharge Curr Limit (A)  脉冲放电电流限制 (A)
    kBbms_HumSensorRH,           // Bank Humidity (RH)  Bank湿度 (RH)
    kBbms_TmpSensorRH,           // Bank Temp (℃)  Bank温度 (℃)
    kBbms_WtrSt,                 // Bank Water Sensor Status  Bank水浸状态
    kBbms_StrCtrlHb,             // Bank BMS Heartbeat Bank BMS心跳信号
    kBbms_DCBusV,                // DC Bus Voltage (V) 母线电压 (V)
    kBbms_TotalDCCur,            // DC Bus Current (A) 母线电流 (A)
    kBbms_TotalPower,            // Total Power (kW) 总功率 (kW)
    kBbms_MaxRackCur,            // Max Rack Current (A) 最大Rack电流 (A)
    kBbms_MaxCurRackNr,          // Max Current Rack Position 最大电流Rack位置
    kBbms_MinRackCur,            // Min Rack Current (A) 最小Rack电流 (A)
    kBbms_MinCurRackNr,          // Min Current Rack Position 最小电流Rack位置
    kBbms_MaxCellV,              // Max Cell Voltage (mV) 最大电芯电压 (mV)
    kBbms_MaxVCellRackNr,        // Max Cell Voltage Rack Position 最大电芯电压Rack位置
    kBbms_MaxVCellTrayNr,        // Max Cell Voltage Module Position 最大电芯电压模组位置
    kBbms_MaxVCellNr,            // Max Cell Voltage Module Inner Position 最大电芯电压模组内位置
    kBbms_MinCellV,              // Min Cell Voltage (mV) 最小电芯电压 (mV)
    kBbms_MinVCellRackNr,        // Min Cell Voltage Rack Position 最小电芯电压Rack位置
    kBbms_MinVCellTrayNr,        // Min Cell Voltage Module Position 最小电芯电压模组位置
    kBbms_MinVCellNr,            // Min Cell Voltage Position 最小电芯电压模组内位置
    kBbms_AvgCellV,              // Average Cell Voltage (mV) 平均电芯电压 (mV)
    kBbms_MaxHvBoxTRackNr,       // Max Hv Box Temp Rack Position 最高高压箱温度Rack位置
    kBbms_MaxTCellRackNr,        // Max Cell Temp Rack Position 最高电芯温度Rack位置
    kBbms_MaxTCellTrayNr,        // Max Cell Temp Module Position 最高电芯温度模组位置
    kBbms_MaxTCellNr,            // Max Cell Temp Module Inner Position  最高电芯温度模组内位置
    kBbms_MinHvBoxTRackNr,       // Min Hv Box Temp Rack Position 最低高压箱温度Rack位置
    kBbms_MinTCellRackNr,        // Min Cell Temp Rack Position 最低电芯温度Rack位置
    kBbms_MinTCellTrayNr,        // Min Cell Temp Module Position 最低电芯温度模组位置
    kBbms_MinTCellNr,            // Min Cell Temp Module Inner Position  最低电芯温度模组内位置
    kBbms_ProhUpgradeFlg,        // Prohibited Upgrade Flag 禁止升级标志位
    kBbms_BMSMaxFltLevel,        // System Fault Level 系统故障等级
    kBbms_IDLearnSt,             // Bank ID Learn Status  Bank ID编码状态
    kBbms_IDLearnFaiLocationNbr, // Bank ID Learn Fail Position Bank ID编码失败位置
    kBbms_IsoRPos,               // Positive to GND Insulation Resistance (kOhm) 正对地绝缘阻值 (kOhm)
    kBbms_IsoRNeg,               // Negative to GND Insulation Resistance (kOhm) 负对地绝缘阻值 (kOhm)
    kBbms_AccuChrgC,             // Accu Charge Cap (Ah) 累计充电容量 (Ah)
    kBbms_AccuChrgE,             // Accu Charge Eng (kWh) 累计充电能量 (kWh)
    kBbms_AccuDisChrgC,          // Accu Discharge Cap (Ah) 累计放电容量 (Ah)
    kBbms_AccuDisChrgE,          // Accu Discharge Eng (kWh) 累计放电能量 (kWh)
    kBbms_LstChEkWh,             // Bank Last Charge Energy (kWh) 最近一次堆充电能量 (kWh)
    kBbms_LstDchEkWh,            // Bank Last Discharge Energy (kWh) 最近一次堆放电能量 (kWh)
    kBbms_RackEnaNbr,            // Rack Enable Number Rack使能数量
    kBbms_RackOnlineNbr,         // Rack Online Number Rack在线数量
    kBbms_ConFail,               // HV Connect Fail Reason 高压连接失败原因
    kBbms_MaxChrgCap,            // Remaining Charge Cap (Ah) 可充容量 (Ah)
    kBbms_MaxDisChrgCap,         // Remaining Discharge Cap (Ah) 可放容量 (Ah)
    kBbms_MaxHvBoxTDegC,         // Max Hv Box Temp (℃) 最高高压箱温度 (℃)
    kBbms_MinHvBoxTDegC,         // Min Hv Box Temp (℃) 最低高压箱温度 (℃)
    kBbms_MaxCellT,              // Max Cell Temp (℃)  最高电芯温度 (℃)
    kBbms_MinCellT,              // Min Cell Temp (℃)  最低电芯温度 (℃)
    kBbms_AvgCellT,              // Average Cell Temp (℃)  平均电芯温度 (℃)
    kBbms_MaxPoleT,              // Max Pole Temp (℃) 最高极柱温度 (℃)
    kBbms_MinPoleT,              // Min Pole Temp (℃) 最低极柱温度 (℃)
    kBbms_MaxPoleTRackNr,        // Max Pole Temp Rack Position 最高极柱温度Rack位置
    kBbms_MaxPoleTMdulNr,        // Max Pole Temp Module Position 最高极柱温度模组位置
    kBbms_MaxPoleTMdulInnerNr,   // Max Pole Temp Module Inner Position  最高极柱温度模组内位置
    kBbms_MinPoleTRackNr,        // Min Pole Temp Rack Position 最低极柱温度Rack位置
    kBbms_MinPoleTMdulNr,        // Min Pole Temp Module Position 最低极柱温度模组位置
    kBbms_MinPoleTMdulInnerNr,   // Min Pole Temp Module Inner Position  最低极柱温度模组内位置  77
    kBbms_BBMSNo,                // Bank Number 堆编号
    kBbms_StartReConDectFlg,     // Rack Anti-Reverse Connection Detection Start Flag 簇防反接检测开始标志位
    kBbms_SumInfo_Byte128Reserved, // Reserved 预留占位 (BBMS_SumInfo byte128 bit5-7)
    kBbms_ReverConRackFailNbr,   // HV Box And Busbar Cabinet reverse connection cluster 高压盒与汇流柜反接簇编号

    //// BBMS_A_Selfdr - 读取-自放电率演变信息
    kBbms_A_Selfdr_Start,                          // 自放电率信息开始枚举
    kBbms_SbEMCR_RTCnCMTimeVldFlg = kBbms_A_Selfdr_Start,                  // RTC时间与生产日期有效性标志
    kBbms_ScEMCR_CellUsedMonth,                     // 电池使用时长(month)
    kBbms_ScEMCR_CellDischargeRatePct,              // 电池自放电率(%)
    kBbms_A_Selfdr_End,                              // 自放电率信息结束枚举

    kBbms_Reserved_End = 100,               // Reserved 预留占位结束

    //BBMS故障列表
    kBbms_FaultList_Start,             // Fault List 故障列表  25字节，共计200个故障位，web解析详细的数据
    kBbms_Flt_1 = kBbms_FaultList_Start, // Fault 1 故障1
    kBbms_Flt_25 = kBbms_Flt_1 + 24,        // Fault 20 故障20
    kBbms_FaultList_End,                    // 故障列表结束

    kBbms_ACore_FaultList_Start = kBbms_FaultList_End,              // A核障列表开始
    kBbms_ACore_FaultList_1 = kBbms_ACore_FaultList_Start,          // A核障1
    kBbms_ACore_FaultList_25 = kBbms_ACore_FaultList_Start + 24,             // A核障列表结束
    kBbms_ACore_FaultList_End, // A核障列表结束

    //BBMS故障越限次数->需要汇总rbms数据
    kBbms_BBMS_FltOvTiNbr_Start = kBbms_ACore_FaultList_End,            // BBMS故障越限次数统计开始
    kBbms_BBMS_FltOvTiNbr_0 = kBbms_BBMS_FltOvTiNbr_Start,              // Rack故障越限次数统计开始
    kBbms_BBMS_FltOvTiNbr_200 = kBbms_BBMS_FltOvTiNbr_Start + 200,      // BBMS故障越限次数统计结束
    kBbms_BBMS_FltOvTiNbr_End, // BBMS故障越限次数统计结束
    // BBMS_A_SOCInfo - 单芯计算的SOC以及各簇SOC结果
    kBbms_A_SOCInfo_Start,                          // 单芯SOC信息开始枚举
    // 等间隔位置电芯SOC数组[16]
    kBbms_SaSOCB_BankCellSOCPctxT_0,                // 等间隔位置电芯SOC[0] (%)
    kBbms_SaSOCB_BankCellSOCPctxT_15 = kBbms_SaSOCB_BankCellSOCPctxT_0 + 15,               // 等间隔位置电芯SOC[15] (%)
    // 等间隔位置电芯SOC状态数组[16]
    kBbms_SaSOCB_BankCellSOCStatexT_0,              // 等间隔位置电芯SOC状态[0] (Nbr)
    kBbms_SaSOCB_BankCellSOCStatexT_15 = kBbms_SaSOCB_BankCellSOCStatexT_0 + 15,             // 等间隔位置电芯SOC状态[15] (Nbr)
    // Rack实时SOC数组[最大12]
    kBbms_SaSOCB_RackRealSOCPct_0,                  // Rack实时SOC[0] (%)
    kBbms_SaSOCB_RackRealSOCPct_11 = kBbms_SaSOCB_RackRealSOCPct_0 + 11,                 // Rack实时SOC[11] (%)
    // Rack最大SOC数组[最大12]
    kBbms_SaSOCB_RackMaxSOCPct_0,                   // Rack最大SOC[0] (%)
    kBbms_SaSOCB_RackMaxSOCPct_11 = kBbms_SaSOCB_RackMaxSOCPct_0 + 11,                 // Rack最大SOC[11] (%)
    // Rack最小SOC数组[最大12]
    kBbms_SaSOCB_RackMinSOCPct_0,                   // Rack最小SOC[0] (%)
    kBbms_SaSOCB_RackMinSOCPct_11 = kBbms_SaSOCB_RackMinSOCPct_0 + 11,                 // Rack最小SOC[11] (%)
    kBbms_A_SOCInfo_End,                             // 单芯SOC信息结束枚举


    /* 单芯计算的SOH以及各簇SOH结果 */
    kBbms_A_SOHInfo_Start = kBbms_A_SOCInfo_End,   // 单芯SOH信息开始枚举
    // 等间隔位置电芯容量数组[16]
    kBbms_SaSOHB_BankCellCapAhxT_0,                // 等间隔位置电芯容量[0] (Ah)
    kBbms_SaSOHB_BankCellCapAhxT_15 = kBbms_SaSOHB_BankCellCapAhxT_0 + 15,
    // 等间隔位置电芯DFCL容量数组[16]
    kBbms_SaSOHB_DFCLCapResultAhxT_0,               // 等间隔位置电芯DFCL容量[0] (Ah)
    kBbms_SaSOHB_DFCLCapResultAhxT_15 = kBbms_SaSOHB_DFCLCapResultAhxT_0 + 15,
    // 等间隔位置电芯MFCL容量数组[16]
    kBbms_SaSOHB_MFCLCapResultAhxT_0,               // 等间隔位置电芯MFCL容量[0] (Ah)
    kBbms_SaSOHB_MFCLCapResultAhxT_15 = kBbms_SaSOHB_MFCLCapResultAhxT_0 + 15,
    // Rack实时容量数组[最大12]
    kBbms_SaSOHB_RackRealCapAh_0,                   // Rack实时容量[0] (Ah)
    kBbms_SaSOHB_RackRealCapAh_11 = kBbms_SaSOHB_RackRealCapAh_0 + 11,
    // Rack最大容量数组[最大12]
    kBbms_SaSOHB_RackMaxCapAh_0,                    // Rack最大容量[0] (Ah)
    kBbms_SaSOHB_RackMaxCapAh_11 = kBbms_SaSOHB_RackMaxCapAh_0 + 11,
    // Rack最小容量数组[最大12]
    kBbms_SaSOHB_RackMinCapAh_0,                    // Rack最小容量[0] (Ah)
    kBbms_SaSOHB_RackMinCapAh_11 = kBbms_SaSOHB_RackMinCapAh_0 + 11,
    kBbms_A_SOHInfo_End,                            // 单芯SOH信息结束枚举

    
    /*热管理信息 */ 
    kBbms_TmsSumInfo_Start = kBbms_A_SOHInfo_End,      // 热管理信息开始枚举
    kBbms_BBMS_WtrTmpCmd = kBbms_TmsSumInfo_Start,    // Setting Temp (℃) 设定温度(℃)
    kBbms_BBMS_cellTmpMax,                            // Max Cell Temp (℃) 电芯最高温度(℃)
    kBbms_BBMS_cellTmpMin,                            // Min Cell Temp (℃) 电芯最低温度(℃)
    kBbms_BBMS_cellTmpAvg,                            // Average Cell Temp (℃) 电芯平均温度(℃)
    kBbms_TMS_OutWtrTmp,                              // Outlet Water Temp (℃) 出水温度(℃)
    kBbms_TMS_BackWtrTmp,                             // Inlet Water Temp (℃) 进水温度(℃)
    kBbms_TMS_EnvTmp,                                 // Environment Temp (℃) 环境温度(℃)
    kBbms_TMS_InWtrP,                                 // Inlet Water Pressure (Bar) 进水水压(Bar)
    kBbms_TMS_OutWtrP,                                // Outlet Water Pressure  (Bar) 出水水压(Bar)
    kBbms_TMS_BBMSFltLevel,                           // TMS Fault Level TMS 故障等级
    kBbms_TMS_BBMSFltCode,                            // TMS Fault Code TMS 故障码
    kBbms_BMS_TMSReqMode,                             // BMS Request Mode BMS请求模式
    kBbms_TMS_BMSMode,                                // TMS Working Mode TMS工作模式
    kBbms_TMSNo,                                      // TMS Number TMS编号
    kBbms_TMSCoolState,                               // TMS Cool Status TMS制冷状态
    kBbms_TmsInfo_Reserver = kBbms_TmsSumInfo_Start + 20, // 热管理信息保留位1
    kBbms_TmsSumInfo_End,                             // 热管理信息结束枚举

    //ParaThr_TMS 参数阈值由上位机设置后反写
    kBbms_ParaThr_TMS_Start = kBbms_TmsSumInfo_End, // 参数阈值开始枚举
    kBbms_CcTHMC_EntHeatMinTemLpLmt = kBbms_ParaThr_TMS_Start,    // 制热启动温度最小电芯温度阈值
    kBbms_CcTHMC_EntHeatAvgTemLpLmt,    // 制热启动温度平均电芯温度阈值
    kBbms_CcTHMC_ExtHeatMinTemLowLmt,   // 制热停止温度最小电芯温度阈值
    kBbms_CcTHMC_ExtHeatAvgTemLowLmt,   // 制热停止温度平均电芯温度阈值
    kBbms_CcTHMC_EntCoolMaxTemLowLimtA, // 制冷A启动温度最大电芯温度阈值
    kBbms_CcTHMC_EntCoolAvgTemLowLimtA, // 制冷A启动温度平均电芯温度阈值
    kBbms_CcTHMC_ExtCoolMaxTemUpLimtA,  // 制冷A停止温度最大电芯温度阈值
    kBbms_CcTHMC_ExtCoolAvgTemUpLimtA,  // 制冷A停止温度平均电芯温度阈值
    kBbms_CcTHMC_Reserved = kBbms_ParaThr_TMS_Start + 9,      // 预留
    kBbms_ParaThr_TMS_End,               // 参数阈值结束枚举


    //控制指令存储HMI Control HMI 控制指令
    kBbms_Hmi_Control_Start = kBbms_ParaThr_TMS_End,             // Control 控制指令 7字节，共计14个控制位，web解析详细的数据
    kBbms_Hmi_AlmRst = kBbms_Hmi_Control_Start,                     // 清除所有故障信息
    kBbms_Hmi_SetEnalso,                 // 使能/禁止绝缘检测
    kBbms_Hmi_Reserved_4_5,              // 预留位
    kBbms_Hmi_SetEna,                    // 使能/禁止电池串
    kBbms_Hmi_EPORst,                    // EPO复位
    kBbms_Hmi_EnalsoManualCtrl,          // 绝缘手动控制
    kBbms_Hmi_SetCon,                    // 电池串连接控制
    kBbms_Hmi_IDLearnManualCtrl,         // ID手动编码
    kBbms_Hmi_AutoHvUpConf,              // 自动上高压设置
    kBbms_Hmi_IDLearnDoCtrl,             // 自动编码DO控制
    kBbms_Hmi_PmtRkFltNum,               // 最小允许Rack数量
    kBbms_Hmi_PCSComDiagConf,            // PCS通讯诊断
    kBbms_Hmi_LocRemCtrl,                // 控制模式
    kBbms_Hmi_Reserved_3_7,              // 预留
    kBbms_Hmi_Reserved = kBbms_Hmi_Control_Start + 19,              // 预留，kBbms_Hmi_Control_Start共计20个点位
    kBbms_Hmi_Control_End,             // Control 控制指令结束

    //上位机控制TMS
    kBbms_Hmi_TMSCtrlWord_Start = kBbms_Hmi_Control_End,             // TMSCtrlWord TMS控制指令 4字节，共计12个控制位，web解析详细的数据    kBbms_Hmi_Tms_Control_Start,       // Control 控制指令 4字节，共计4个控制位，web解析详细的数据
    kBbms_Hmi_TMSManCtrlMode,          // 强控TMS工作模式
    kBbms_Hmi_TMSManCtrlTempDegC,      // 强控TMS工作温度
    kBbms_Hmi_TMSManCtrlEnaFlg,        // 强控TMS使能信号
    kBbms_Hmi_TMSNo,                   // TMS编号
    kBbms_Hmi_TMSCtrlReserved = kBbms_Hmi_TMSCtrlWord_Start + 10,          // 预留
    kBbms_Hmi_TMSCtrlWord_End,         // TMSCtrlWord TMS控制指令结束


    /*HMI三级板DO控制指令（灯）共计1个字节，共计3个控制位，web解析详细的数据*/
    kBbms_Hmi_DO_Control_Start,       // Control 控制指令 1字节，共计3个控制位，web解析详细的数据
    kBbms_Hmi_DOManCtrlMode,          // 强控DO工作模式
    kBbms_Hmi_DOManCtrlReserved = kBbms_Hmi_DO_Control_Start + 7,      // 预留
    kBbms_Hmi_DO_Control_End,         // Control 控制指令结束

    //ParaThr_TMS 设置参数阈值-- 这个位置不用，与上面重复了，这里保留着，方便后续扩展
    kBbms_Crtl_TMS_Start = kBbms_Hmi_DO_Control_End, // 参数阈值开始枚举
    // kBbms_Crtl_TMS_EntHeatMinTemLpLmt = kBbms_Crtl_TMS_Start,    // 制热启动温度最小电芯温度阈值
    // kBbms_Crtl_TMS_EntHeatAvgTemLpLmt,    // 制热启动温度平均电芯温度阈值
    // kBbms_Crtl_TMS_ExtHeatMinTemLowLmt,   // 制热停止温度最小电芯温度阈值
    // kBbms_Crtl_TMS_ExtHeatAvgTemLowLmt,   // 制热停止温度平均电芯温度阈值
    // kBbms_Crtl_TMS_EntCoolMaxTemLowLimtA, // 制冷A启动温度最大电芯温度阈值
    // kBbms_Crtl_TMS_EntCoolAvgTemLowLimtA, // 制冷A启动温度平均电芯温度阈值
    // kBbms_Crtl_TMS_ExtCoolMaxTemUpLimtA,  // 制冷A停止温度最大电芯温度阈值
    // kBbms_Crtl_TMS_ExtCoolAvgTemUpLimtA,  // 制冷A停止温度平均电芯温度阈值
    kBbms_Crtl_TMS_Reserved = kBbms_Crtl_TMS_Start + 9,      // 预留
    kBbms_Crtl_TMS_End,               // 参数阈值结束枚举


    kBbms_Hmi_BBMSDO_Start  = kBbms_Crtl_TMS_End,                  // HMI三级板DO控制命令开始枚举，共12个控制位
    kBbms_HMI_BBMSDO0CtrlFlg,                // DO0控制
    kBbms_HMI_BBMSDO1CtrlFlg,                // DO1控制
    kBbms_HMI_BBMSDO2CtrlFlg,                // DO2控制
    kBbms_HMI_BBMSDO3CtrlFlg,                // DO3控制
    kBbms_HMI_BBMSDO4CtrlFlg,                // DO4控制
    kBbms_HMI_BBMSDO5CtrlFlg,                // DO5控制
    kBbms_HMI_BBMSDO6CtrlFlg,                // DO6控制
    kBbms_HMI_BBMSDO7CtrlFlg,                // DO7控制
    kBbms_HMI_BBMSDO8CtrlFlg,                // DO8控制
    kBbms_HMI_BBMSDO9CtrlFlg,                // DO9控制
    kBbms_HMI_BBMSDO10CtrlFlg,               // DO10控制
    kBbms_HMI_BBMSDO11CtrlFlg,               // DO11控制
    kBbms_Hmi_BBMSDO_Reserved = kBbms_Hmi_BBMSDO_Start + 19,      // 预留
    kBbms_Hmi_BBMSDO_End,                  // HMI三级板DO控制命令结束枚举


    //写入-Bank故障累积次数标定
    kBbms_Hmi_BankErrorCntSet_Start,                // Bank Error Level 银行错误等级 0: 默认，不标定1:标定故障越限次数数组
    kBbms_HMI_RackFltHistInfoIndicator,            // 标定故障超限次数数组指示
    kBbms_HMI_RackAllFltHistAccuCaliVal_0,
    kBbms_HMI_RackAllFltHistAccuCaliVal_200 = kBbms_Hmi_BankErrorCntSet_Start + 200,          // Rack故障累积次数标定值,共计200个
    kBbms_Hmi_BankErrorCntSet_End,                  // Bank Error Level 结束枚举


    //故障使能数组,读取和写入都是该位置
    kBbms_Hmi_FaultEnaFlg_Start,                // 故障使能数组开始枚举
    kBbms_HMI_FaultEnaFlg_1 = kBbms_Hmi_FaultEnaFlg_Start,                // 故障使能数组0
    kBbms_HMI_FaultEnaFlg_25 = kBbms_Hmi_FaultEnaFlg_Start + 25,          // 故障使能数组200,共计200个
    kBbms_Hmi_FaultEnaFlg_End,                  // 故障使能数组结束枚举


    //BBMS_A_CtlWord - 控制字 cmdGroup: 0x01, CmdId: 14 周期性上送
    kBbms_A_CtlWord_Start = kBbms_Hmi_FaultEnaFlg_End,                // 控制字开始枚举
    kBbms_A_EMSCtrlPowerCmd = kBbms_A_CtlWord_Start,          // EMS Control Power-up CmdEMS控制上下高压指令
    kBbms_A_CtlWord_Reserved = kBbms_A_CtlWord_Start + 9,          // 预留
    kBbms_A_CtlWord_End,                  // 控制字结束枚举

    kBbms_Data_End,
} devBBMSPoint_e;

// RBMS设备模型
typedef enum
{
    kRbms_Data_Start = 0,
    kRbms_Online_State = kRbms_Data_Start, // Online State 在线状态
    kRbms_SumInfo_Start,
    kRbms_St = kRbms_SumInfo_Start, // Battery String Status
    kRbms_ChaSt,                    // Rack Charging Status Rack充放电状态
    kRbms_SysOperSt,                // Rack Operating Status Rack运行状态
    kRbms_ConSt,                    // Contactor Status 继电器/干接点状态
    kRbms_ConFail,                  // HV Connect Fail Reason 高压连接失败原因
    kRbms_V,                        // Battery String Voltage (V) 电池串电压 (V)
    kRbms_DCBusV,                   // DC Bus Voltage (V) 总线电压 (V)
    kRbms_A_HighAccu,               // DC Bus Current (A) (High accu) 总线电流 (A) (高精度)
    kRbms_IsoR,                     // Insulation Resistance (kOhm) 绝缘阻值 (kOhm)
    kRbms_HvBoxMaxTemp,             // Max HV Box Temperature (℃) 最高高压箱温度 (℃)
    kRbms_HvBoxMinTemp,             // Min HV Box Temperature (℃) 最低高压箱温度 (℃)
    kRbms_Reserved3,                // Reserved3 预留占位3
    kRbms_LowPowSply,               // Low Power Supply Voltage (mV) 低压供电电压 (mV)
    kRbms_RlyCtrlSeqStState,        // Relay Control Status 继电器控制状态
    kRbms_ReverConDetecOpenFlg,     // Reverse Connection Detect Open Flag 反接检测开路标志
    kRbms_Reserved,                 // Reserved 预留占位 (3 bit)
    kRbms_Reserved1,                // Reserved1 预留占位1
    kRbms_HisSleepTiWrMin,          // Rack History Sleep Time (min) 历史休眠时间 (min)
    kRbms_HisRunTiWrMin,            // Rack History Run Time (min) 历史运行时间 (min)
    kRbms_LstSleepDurTiMin,         // Last Power-down Sleep Time (min) 上次下电休眠时间 (min)
    kRbms_IDLearnCodeSt,            // Rack ID Learn Status Rack ID编码状态
    kRbms_IsoRPos,                  // Positive to GND Insulation Resistance (kOhm) 正对地绝缘阻值 (kOhm)
    kRbms_IsoRNeg,                  // Negative to GND Insulation Resistance (kOhm) 负对地绝缘阻值 (kOhm)
    kRbms_CmdInhibitFlg,            // Insulation Disable Flag 绝缘禁止标志位
    kRbms_IslnHvVldNbr,             // Insulation High Voltage Validity Status 绝缘高压有效性状态
    kRbms_IslnInhibitNbr,           // Insulation Prohibition Reason 绝缘禁止原因
    kRbms_CellTMUXFaiIDNbr_Start,   // Temp Sampling MUX Fail Position 温度采样MUX失效位置
    kRbms_CellTMUXFaiIDNbr_Byte_1 = kRbms_CellTMUXFaiIDNbr_Start,
    kRbms_CellTMUXFaiIDNbr_End = kRbms_CellTMUXFaiIDNbr_Byte_1 + 64,
    kRbms_SoC = kRbms_CellTMUXFaiIDNbr_End, // Rack SOC (%) Rack SOC（%）
    kRbms_SoH,                      // Rack SOH (%) Rack SOH（%）
    kRbms_SoEC,                     // Rack SOE for Charge (kWh) Rack可充能量 (kWh)
    kRbms_SoED,                     // Rack SOE for Discharge (kWh) Rack可放能量 (kWh)
    kRbms_SoFPulseChrgCurr,         // Pulse Charge Curr Limit (A) 脉冲充电电流限制 (A)
    kRbms_SoFPulseDisChrgCurr,      // Pulse Discharge Curr Limit (A) 脉冲放电电流限制 (A)
    kRbms_SoFPulseChrgPwr,          // Pulse Charge Power Limit (kW) 脉冲充电功率限制 (kW)
    kRbms_SoFPulseDisChrgPwr,       // Pulse Discharge Power Limit (kW) 脉冲放电功率限制 (kW)
    kRbms_SoFContChrgCurr,          // Const Charge Curr Limit (A) 持续充电电流限制 (A)
    kRbms_SoFContDisChrgCurr,       // Const Discharge Curr Limit (A) 持续放电电流限制 (A)
    kRbms_SoFContChrgPwr,           // Const Charge Power Limit (kW) 持续充电功率限制 (kW)
    kRbms_SoFContDisChrgPwr,        // Const Discharge Power Limit (kW) 持续放电功率限制 (kW)
    kRbms_CellVMax,                 // Max Cell Voltage (mV) 最大电芯电压 (mV)
    kRbms_CellVMaxPstn,             // Max Cell Voltage Position 最大电芯电压位置
    kRbms_CellVMin,                 // Min Cell Voltage (mV) 最小电芯电压 (mV)
    kRbms_CellVMinPstn,             // Min Cell Voltage Position 最小电芯电压位置
    kRbms_CellVAvg,                 // Average Cell Voltage (mV) 平均电芯电压 (mV)
    kRbms_ModTmpMax,                // Max Module Temp (℃) 最高电芯温度 (℃)
    kRbms_Reserved4,                // Reserved4 预留占位4
    kRbms_ModTmpMin,                // Min Cell Temp (℃) 最低电芯温度 (℃)
    kRbms_MCURstRsnFltNbr,          // Fault Causes Leading to MCU Reset 导致MCU复位的故障原因
    kRbms_ModTmpAvg,                // Average Cell Temp (℃) 平均电芯温度 (℃)
    kRbms_MaxPoleTDegC,             // Max Pole Temp (℃) 最高极柱温度 (℃)
    kRbms_MaxPoleTIdxNbr,           // Max Pole Temp Position 最高极柱温度位置
    kRbms_MinPoleTDegC,             // Min Pole Temp (℃) 最低极柱温度 (℃)
    kRbms_MinPoleTIdxNbr,           // Min Pole Temp Position 最低极柱温度位置
    kRbms_MaxPoleTMdulIdxNbr,       // Max Pole Temp Module Position 最高极柱温度模组位置
    kRbms_MinPoleTMdulIdxNbr,       // Min Pole Temp Module Position 最低极柱温度模组位置
    kRbms_AvgPoleTDegC,             // Average Pole Temp (℃) 平均极柱温度 (℃)
    kRbms_CommType,                 // Communication Type 通讯类型
    kRbms_StrCtrlHb,                // Rack BMS Heartbeat Rack BMS心跳信号
    kRbms_ResChAvlCapAh,            // Remaining Charge Cap (Ah) 可充容量 (Ah)
    kRbms_ResDischAvlCapAh,         // Remaining Discharge Cap (Ah) 可放容量 (Ah)
    kRbms_LstFullChCapAh,           // Rack Last Full-Charge Cap (Ah) 最近一次簇满充容量 (Ah)
    kRbms_LstFullDischCapAh,        // Rack Last Full-Discharge Cap (Ah) 最近一次簇满放容量 (Ah)
    kRbms_CycNbr,                   // Full Equiv Ch-Disch Cycles 等效完整充放电循环次数
    kRbms_MaxPoleTMdulInnerIdxNbr,  // Max Pole Temp Module Inner Position 最高极柱温度模组内位置
    kRbms_MinPoleTMdulInnerIdxNbr,  // Min Pole Temp Module Inner Position 最低极柱温度模组内位置
    kRbms_AFEChipTmpMax,            // Max AFE Chip Temperature (℃) 最高AFE芯片温度 (℃)
    kRbms_AFEChipTmpMin,            // Min AFE Temperature (℃) 最低AFE温度 (℃)
    kRbms_MCURstRsnNbr,             // MCU Reset Source MCU复位源
    kRbms_GenlVldFlg,               // RBMS Sampling Validity Flag RBMS采样有效性标志
    kRbms_IslnFinalBatV,            // Total Insulation Voltage (V) 绝缘总压 (V)
    kRbms_IslnFinalPosV,            // Insulation Positive to GND Voltage (V) 绝缘正对地电压 (V)
    kRbms_IslnFinalNegV,            // Insulation Negative to GND Voltage (V) 绝缘负对地电压 (V)
    kRbms_IslnMeastStState,         // Insulation Measure Status 绝缘测量状态
    kRbms_LstChEkWh,                // Rack Last Charge Energy (kWh) 最近一次簇充电能量 (kWh)
    kRbms_LstDchEkWh,               // Rack Last Discharge Energy (kWh) 最近一次簇放电能量 (kWh)
    kRbms_BalBdTmpMax,              // Max Balancing Board Temp (℃) 最高均衡板温 (℃)
    kRbms_RmngCellBalTiMax,         // Max Balancing Remain Time (min) 最大剩余均衡时间 (min)
    kRbms_RealSysSOHC,              // Rack Real SOH (%) Rack真实SOH (%)
    kRbms_RealSysCapAh,             // Rack Max Available Capacity (Ah) 电池簇最大可用容量 (Ah)
    kRbms_RealCellCapAh,            // Min Cell Real Capacity (Ah) 最小电芯真实容量 (Ah)
    kRbms_SysMaxSOC,                // RBMS Max SOC (%) RBMS最大SOC (%)
    kRbms_SysMinSOC,                // RBMS Min SOC (%) RBMS最小SOC (%)
    kRbms_RealSysSOCPct,            // Rack Real SOC (%) Rack真实SOC (%)
    kRbms_AccuDisChrgAh2,           // Accu Discharge Cap (Ah) 累计放电容量 (Ah)
    kRbms_AccuDisChrgkWh2,          // Accu Discharge Eng (kWh) 累计放电能量 (kWh)
    kRbms_AccuChrgAh2,              // Accu Charge Cap (Ah) 累计充电容量 (Ah)
    kRbms_AccuChrgkWh2,             // Accu Charge Eng (kWh) 累计充电能量 (kWh)
    kRbms_BMSMaxFltLevel,           // Rack Fault Level Rack故障等级
    kRbms_MaxCellVMdulTDegC,        // Max Voltage Cell Temp (℃) 最大电压电芯对应温度 (℃)
    kRbms_MinCellVMdulTDegC,        // Min Voltage Cell Temp (℃) 最小电压电芯对应温度 (℃)
    kRbms_MaxCellVMdulTVldFlg,      // Max Voltage Cell Temp Validity 最大电压电芯温度有效性
    kRbms_MinCellVMdulTVldFlg,      // Min Voltage Cell Temp Validity 最小电压电芯温度有效性
    kRbms_A,                        // DC Bus Current (A) 总线电流 (A)
    kRbms_A1,                       // Rack Current of Sensor 1 (A) 传感器1的簇电流 (A)
    kRbms_A2,                       // Rack Current of Sensor 2 (A) 传感器2的簇电流 (A)
    kRbms_BalBdTmpMin,              // Min Balancing Board Temp (℃) 最低均衡板温 (℃)
    kRbms_PreChrgRT,                // Precharge Resistor Temp (℃) 预充电阻温度 (℃)
    kRbms_RlyCtrlCmd,               // Relay Control Cmd 继电器控制命令
    kRbms_FaultEvt,                 // Fault Event 故障信息
    kRbms_AttachState,              // Attachment Status 附件状态
    kRbms_Reserved2,                // Reserved2 预留占位2
    kRbms_MaxCellVMdulIdxNbr,       // Max Cell Voltage Module Position 最高电芯电压模组位置
    kRbms_MinCellVMdulIdxNbr,       // Min Cell Voltage Module Position 最低电芯电压模组位置
    kRbms_MaxTMdulIdxNbr,           // Max Cell Temp Module Position 最高电芯温度模组位置
    kRbms_MinTMdulIdxNbr,           // Min Cell Temp Module Position 最低电芯温度模组位置
    kRbms_MaxCellVMdulInnerIdxNbr,  // Max Cell Voltage Module Inner Position 最高电芯电压模组内位置
    kRbms_MinCellVMdulInnerIdxNbr,  // Min Cell Voltage Module Inner Position 最低电芯电压模组内位置
    kRbms_MaxTMdulInnerIdxNbr,      // Max Cell Temp Module Inner Position 最高电芯温度模组内位置
    kRbms_MinTMdulInnerIdxNbr,      // Min Cell Temp Module Inner Position 最低电芯温度模组内位置
    kRbms_DaisyChain1LastNode,      // AFE Daisy Chain Master Chain Nodes AFE菊花链主链节点数
    kRbms_DaisyChain2LastNode,      // AFE Daisy Chain Slave Chain Nodes AFE菊花链副链节点数
    kRbms_ModTmpMaxPstn,            // Max Cell Temp Position 最高电芯温度位置
    kRbms_ModTmpMinPstn,            // Min Module Temp Position 最低电芯温度位置
    kRbms_AccuDisChrgAh,            // Accu Discharge Cap (Ah) (Low accu) 累计放电容量 (Ah) (低精度)
    kRbms_AccuDisChrgkWh,           // Accu Discharge Eng (kWh) (Low accu) 累计放电能量 (kWh) (低精度)
    kRbms_AccuChrgAh,               // Accu Charge Cap (Ah) (Low accu) 累计充电容量 (Ah) (低精度)
    kRbms_AccuChrgkWh,              // Accu Charge Eng (kWh) (Low accu) 累计充电能量 (kWh) (低精度)
    kRbms_LstChCapAh,               // Rack Last Charge Cap (Ah) 最近一次簇充电容量 (Ah)
    kRbms_LstDchCapAh,              // Rack Last Discharge Cap (Ah) 最近一次簇放电容量 (Ah)
    kRbms_FltSOPDischCoeffPct,      // Fault-based Discharge Power Limit Coefficient (%) 基于故障的放电功率限制系数
    kRbms_FltSOPChCoeffPct,         // Fault-based Charge Power Limit Coefficient (%) 基于故障的充电功率限制系数
    kRbms_CurrSnsrPowSplyVoltmV,    // Current Sensor Power Supply Volt (mV) 电流传感器供电电压 (mV)
    kRbms_CtlBoxT0DegC,             // HV Box Temperature0 (℃) 高压箱温度T0 (℃)
    kRbms_CtlBoxT1DegC,             // HV Box Temperature1 (℃) 高压箱温度T1 (℃)
    kRbms_CtlBoxT2DegC,             // HV Box Temperature2 (℃) 高压箱温度T2 (℃)
    kRbms_CtlBoxT3DegC,             // HV Box Temperature3 (℃) 高压箱温度T3 (℃)
    kRbms_PCSChVoltLim,             // Charge Voltage Limit to PCS (V) 发送至PCS的充电限制电压 (V)
    kRbms_PCSDchVoltLim,            // Discharge Voltage Limit to PCS 发送至PCS的放电限制电压 (V)
    kRbms_PCSSOP,                   // SOP to PCS (kW) 发送至PCS的SOP (kW)
    kRbms_PCSBMSSt,                 // BMS Status to PCS 发送至PCS的BMS状态

    kRbms_SumInfo_Reserved_Start,
    kRbms_SumInfo_Reserved_1 = kRbms_SumInfo_Reserved_Start,
    kRbms_SumInfo_Reserved_20 = kRbms_SumInfo_Reserved_1 + 20,
    kRbms_SumInfo_Reserved_End,

    /*电芯均衡状态（最多416个），共计52个枚举，一个枚举表示8个状态*/
    kRbms_CellBalStatus_Start = kRbms_SumInfo_Reserved_End,
    kRbms_CellBalStatus1_8 = kRbms_CellBalStatus_Start, // Cell Balancing Status 1~8 电芯均衡状态1~8个
    kRbms_CellBalStatus_409_416 = kRbms_CellBalStatus1_8 + 51,
    kRbms_CellBalStatus_End,

    /*Cell Self Discharge Rate (up to 416 cells)电芯自放电率（最多416个）,共计416个枚举*/
    kRbms_CellSdrate_Start = kRbms_CellBalStatus_End,
    kRbms_CellSdrate_1 = kRbms_CellSdrate_Start,     // Cell Self Discharge Rate 电芯1自放电率
    kRbms_CellSdrate_416 = kRbms_CellSdrate_1 + 415, // Cell Self Discharge Rate 电芯416自放电率
    kRbms_CellSdrate_End,

    // RBMS Fault Array
    // RBMS故障数组,共计200个bit，每个bit表示一个故障，用25个字节表示200个故障状态
    kRbms_Fault_Start = kRbms_CellSdrate_End,      // Fault Array 故障数组
    kRbms_Fault_Byte_1 = kRbms_Fault_Start,        // Fault Array 故障数组字节1
    kRbms_Fault_Byte_25 = kRbms_Fault_Byte_1 + 24, // Fault Array 故障数组字节2
    kRbms_Fault_End,

    /*电芯电压*/
    kRbms_CellVoltInfo_Start = kRbms_Fault_End, // Cell Voltage 电芯电压

    kRbms_CellVolt_Validity_Start = kRbms_CellVoltInfo_Start, // 电压有效性，共计52个字节，一个字节表示8个电芯电压有效性，共计416个电芯电压有效性
    kRbms_CellVolt_Validity_End = kRbms_CellVolt_Validity_Start + 52,

    kRbms_CellVolt_Start = kRbms_CellVolt_Validity_End, // 每个电芯电芯电压，共计416个电芯电压
    kRbms_CellVolt_End = kRbms_CellVolt_Start + 416,    // 电芯电压结束枚举

    kRbms_AFEVolt_Start = kRbms_CellVolt_End,     // 每个afe总压，最多32个afe，共计32个枚举
    kRbms_AFEVolt_End = kRbms_AFEVolt_Start + 32, // AFE电压结束枚举

    kRbms_CellVoltInfo_End = kRbms_AFEVolt_End, // Cell Voltage 电芯电压结束枚举

    /*温度类信息*/
    kRbms_TempInfo_Start = kRbms_CellVoltInfo_End,             // Cell Temperature 电芯温度
    kRbms_TempInfo_Cell_Start = kRbms_TempInfo_Start,          // Cell Temperature 电芯温度
    kRbms_TempInfo_Cell_End = kRbms_TempInfo_Cell_Start + 416, // Cell Temperature 电芯温度结束枚举

    /*极柱温度*/
    kRbms_TempInfo_PoleTemp_Start = kRbms_TempInfo_Cell_End,           // 极柱温度（最多128个）
    kRbms_TempInfo_PoleTemp_End = kRbms_TempInfo_PoleTemp_Start + 128, // 极柱温度结束枚举

    /*Pack正负极连接件温度（最多16个）*/
    kRbms_TempInfo_PackPosNegTemp_Start = kRbms_TempInfo_PoleTemp_End,            // Pack正负极连接件温度（最多16个）
    kRbms_TempInfo_PackPosNegTemp_End = kRbms_TempInfo_PackPosNegTemp_Start + 16, // Pack正负极连接件温度结束枚举

    /*均衡板温有效性（最多32个,每个bit表示一个均衡板温有效性）*/
    kRbms_TempInfo_BalBoardTempValidity_Start = kRbms_TempInfo_PackPosNegTemp_End,           // 均衡板温有效性（最多32个）
    kRbms_TempInfo_BalBoardTempValidity_End = kRbms_TempInfo_BalBoardTempValidity_Start + 4, // 均衡板温有效性结束枚举

    /*均衡板温（最多32个）*/
    kRbms_TempInfo_BalBoardTemp_Start = kRbms_TempInfo_BalBoardTempValidity_End, // 均衡板温（最多32个）
    kRbms_TempInfo_BalBoardTemp_End = kRbms_TempInfo_BalBoardTemp_Start + 32,    // 均衡板温结束枚举

    kRbms_TempInfo_End = kRbms_TempInfo_BalBoardTemp_End, // 温度类信息结束枚举

    /*热管理信息 */
    kRbms_TmsSumInfo_Start = kRbms_TempInfo_End,      // 热管理信息开始枚举
    kRbms_BBMS_WtrTmpCmd = kRbms_TmsSumInfo_Start,    // Setting Temp (℃) 设定温度(℃)
    kRbms_BBMS_cellTmpMax,                            // Max Cell Temp (℃) 电芯最高温度(℃)
    kRbms_BBMS_cellTmpMin,                            // Min Cell Temp (℃) 电芯最低温度(℃)
    kRbms_BBMS_cellTmpAvg,                            // Average Cell Temp (℃) 电芯平均温度(℃)
    kRbms_TMS_OutWtrTmp,                              // Outlet Water Temp (℃) 出水温度(℃)
    kRbms_TMS_BackWtrTmp,                             // Inlet Water Temp (℃) 进水温度(℃)
    kRbms_TMS_EnvTmp,                                 // Environment Temp (℃) 环境温度(℃)
    kRbms_TMS_InWtrP,                                 // Inlet Water Pressure (Bar) 进水水压(Bar)
    kRbms_TMS_OutWtrP,                                // Outlet Water Pressure  (Bar) 出水水压(Bar)
    kRbms_TMS_BBMSFltLevel,                           // TMS Fault Level TMS 故障等级
    kRbms_TMS_BBMSFltCode,                            // TMS Fault Code TMS 故障码
    kRbms_BMS_TMSReqMode,                             // BMS Request Mode BMS请求模式
    kRbms_TMS_BMSMode,                                // TMS Working Mode TMS工作模式
    kRbms_TMSNo,                                      // TMS Number TMS编号
    kRbms_TMSCoolState,                               // TMS Cool Status TMS制冷状态
    kRbms_TmsInfo_Reserver = kRbms_TMSCoolState + 10, // 热管理信息保留位1
    kRbms_TmsSumInfo_End,                             // 热管理信息结束枚举

    /*SOX算法调试的输入数据，包含电流、电压、温度等*/
    kRbms_SoxInput_Start = kRbms_TmsSumInfo_End, // SOX算法调试的输入数据开始枚举
    kRbms_ScSGPC_BatIA = kRbms_SoxInput_Start,                          // Rack Current (A) 簇电流(A)
    kRbms_ScSGPC_MaxCellVmV,                     // Max Cell Voltage (mV) 最大电芯电压 (mV)
    kRbms_ScSGPC_MinCellVmV,                     // Min Cell Voltage (mV) 最小电芯电压 (mV)
    kRbms_ScSGPC_MaxCellTDegC,                   // Max Cell Temp (℃) 最高电芯温度 (℃)
    kRbms_ScSGPC_MinCellTDegC,                   // Min Cell Temp (℃) 最低电芯温度 (℃)
    kRbms_ScSGPC_AvgCellTDegC,                   // Average Cell Temp (℃) 平均电芯温度 (℃)
    kRbms_ScCSPC_FullChEnaNbr,                   // Full Charge Correction Flag 满充修正标志位
    kRbms_ScCSPC_FullDischEnaNbr,                // Full Discharge Correction Flag 满放修正标志位
    kRbms_ScHIST_HistAccuChCapAh,                // Accumulated Charge Capacity (Ah) 累计充电容量 (Ah)
    kRbms_ScHIST_HistAccuDischCapAh,             // Accumulated Discharge Capacity (Ah) 累计放电容量 (Ah)
    kRbms_ScSGPC_BatIStNbr,                      // Rack Charging Status Rack充放电状态
    kRbms_ScSGPC_LstSleepDurTiMin,               // Last Power-down Sleep Time (min) 上次下电休眠时间
    kRbms_ScSGPC_HisSleepTiMin,                  // History Sleep Time (min) 历史休眠时间
    kRbms_ScSGPC_HisRunTiMin,                    // History Run Time (min) 历史运行时间
    kRbms_ScHIST_CycNbr,                         // Accumulated Cycle Number 累计循环次数
    kRbms_BSWSAllSOHCalIndicator,                // Rack All SOH Calibration Indicator Rack所有SOH标定指示
    kRbms_BSWSAllSOHCalValPct,                   // Rack All SOH Calibration Value Rack所有SOH标定值
    kRbms_ScBTCS_RackChrgCapmAh,                 // Accumulated Charge Capacity (mAh) 累积充电容量 (mAh)
    kRbms_ScBTCS_RackDsChrgCapmAh,               // Accumulated Discharge Capacity (mAh) 累积放电容量 (mAh)
    kRbms_SoxInput_Reserver1,                    // SOX算法调试的输入数据保留位1
    kRbms_SoxInput_Reserver2,                    // SOX算法调试的输入数据保留位2
    kRbms_BSWSRSOXHistInfoIndicator,             // Rack History Info Calibration Indicator Rack历史信息标定指示
    kRbms_BSWSAllSOCCalIndicator,                // Rack All SOC Calibration Indicator Rack所有SOC标定指示
    kRbms_BSWSAllSOCCalValPct,                   // Rack All SOC Calibration Value Rack所有SOC标定值
    kRbms_SbSGPC_BatIVldFlg,                     // Rack Current Validity 簇电流有效性
    kRbms_SbSGPC_GenlCellVVldFlg,                // Cell Voltage General Validity 电芯电压概要有效性
    kRbms_SbSGPC_GenlCellTVldFlg,                // Cell Temperature General Validity 电芯温度概要有效性
    kRbms_SbSGPC_MaxCellVMdulTVldFlg,            // Vmax Cell Temperature Validity 最大电压电芯温度有效性
    kRbms_SbSGPC_MinCellVMdulTVldFlg,            // Vmin Cell Temperature Validity 最小电压电芯温度有效性
    kRbms_SoxInput_Reserver3,                    // SOX算法调试的输入数据保留位3 (3 bit)
    kRbms_ScSGPC_MaxCellVMdulTDegC,              // Vmax Cell Temperature (℃) 最大电压电芯对应温度 (℃)
    kRbms_ScSGPC_MinCellVMdulTDegC,              // Vmin Cell Temperature (℃) 最小电压电芯对应温度 (℃)
    kRbms_SoxInput_End,                          // SOX算法调试的输入数据结束枚举

    /*SOX算法调试的输出数据，包含SOC、SOHC、容量等*/
    kRbms_SoxOutput_Start = kRbms_SoxInput_End, // SOX算法调试的输出数据开始枚举
    kRbms_SaSGPC_CellVmVxT_Start = kRbms_SoxOutput_Start, // Cell Voltage at Equidistant Positions (mV) 等间隔位置的电芯电压 (mV)
    kRbms_SaSGPC_CellVmVxT_1 = kRbms_SaSGPC_CellVmVxT_Start,  // Equidistant Position Cell Voltage (mV) 等间隔位置的电芯电压 (mV)
    kRbms_SaSGPC_CellVmVxT_10 = kRbms_SaSGPC_CellVmVxT_1 + 9, // Equidistant Position Cell Voltage (mV) 等间隔位置的电芯电压 (mV)
    kRbms_SaSGPC_CellVmVxT_End,
    kRbms_ScSOHA_RealSysSOHCPct = kRbms_SaSGPC_CellVmVxT_End, // Rack Real SOH (%) Rack真实SOH (%)
    kRbms_ScSOHA_DispSysSOHCPct,                              // Rack Display SOH (%) Rack显示SOH (%)
    kRbms_ScSOHA_DFCLCapResultAh,                             // DFCL Module Capacity (Ah) DFCL模块容量 (Ah)
    kRbms_ScSOHA_DFCLPointSOCPct,                             // DFCL Module SOH Feature Point SOC (%) DFCL模块SOH特征点SOC (%)
    kRbms_ScSOHA_DFCLPointStats,                              // DFCL Module SOH Feature Point Status DFCL模块SOH特征点标志位
    kRbms_ScSOHA_DFCLPointCapAh,                              // DFCL Module SOH Feature Point Capacity (Ah) DFCL模块SOH特征点容量 (Ah)
    kRbms_ScSOHA_DFCLPointTimeDay,                            // DFCL Module SOH Feature Point Time (day) DFCL模块SOH特征点时间 (day)
    kRbms_ScSOHA_MFCLCapResultAh,                             // MFCL Module Capacity (Ah) MFCL模块容量 (Ah)
    kRbms_ScSOHA_MFCLTargtValueVmax,                          // Vmax Cell MFCL Module Mid-Feature Point Target Value Vmax节电芯MFCL模块中特征点目标计算值
    kRbms_ScSOHA_MFCLTargtValueVmin,                          // Vmin Cell MFCL Module Mid-Feature Point Target Value Vmin节电芯MFCL模块中特征点目标计算值
    kRbms_SaSOCA_MaxMinSOCPct1,                               // Vmax Cell SOC (%) 最大电压电芯SOC (%)
    kRbms_SaSOCA_MaxMinSOCPct2,                               // Vmin Cell SOC (%) 最小电压电芯SOC (%)
    kRbms_SaSOCA_MaxMinSOCState1,                             // Vmax Cell SOC Status 最大电压电芯SOC状态
    kRbms_SaSOCA_MaxMinSOCState2,                             // Vmin Cell SOC Status 最小电压电芯SOC状态
    kRbms_SaSOCA_SimCellVmV1,                                 // Vmax Cell Simulation Voltage (mV) 最大电压电芯仿真电压 (mV)
    kRbms_SaSOCA_SimCellVmV2,                                 // Vmin Cell Simulation Voltage (mV) 最小电压电芯仿真电压 (mV)
    kRbms_ScSOHA_RealSysCapAh_dbug,                           // Rack Real Capacity (Ah) Rack真实容量 (Ah)
    kRbms_ScSOCA_RealSysSOCPct_dbug,                          // Rack Real SOC (%) Rack真实SOC (%)
    kRbms_ScSOCA_SmthSysSOCPct,                               // Rack Smooth SOC (%) Rack平滑SOC (%)
    kRbms_ScSOCA_DispSysSOCPct,                               // Rack Display SOC (%) Rack显示SOC (%)
    kRbms_ScSOHA_DFCLCalSuccFlg,                              // DFCL Module Capacity Calculation Success Flag DFCL模块容量计算成功标志位
    kRbms_ScSOHA_MFCLCalSuccFlg,                              // MFCL Module Capacity Calculation Success Flag MFCL模块容量计算成功标志位
    kRbms_ScSOHA_MFCLPoint1FlgVmax,                           // Vmax Cell MFCL Module Feature Point 1 Vmax节电芯MFCL模块特征点1
    kRbms_ScSOHA_MFCLPoint1FlgVmin,                           // Vmin Cell MFCL Module Feature Point 1 Vmin节电芯MFCL模块特征点1
    kRbms_ScSOHA_MFCLPoint2FlgVmax,                           // Vmax Cell MFCL Module Feature Point 2 Vmax节电芯MFCL模块中特征点2
    kRbms_ScSOHA_MFCLPoint2FlgVmin,                           // Vmin Cell MFCL Module Feature Point 2 Vmin节电芯MFCL模块特征点2
    kRbms_ScSOHA_MFCLPoint3FlgVmax,                           // Vmax Cell MFCL Module Feature Point 3 Vmax节电芯MFCL模块中特征点3
    kRbms_ScSOHA_MFCLPoint3FlgVmin,                           // Vmin Cell MFCL Module Feature Point 3 Vmin节电芯MFCL模块特征点3
    kRbms_ScSOCA_DispSOCState,                                // DispSOC Follow Strategy Status 显示SOC跟随策略状态
    kRbms_ScSOCA_TgtSysSOCPct,                                // Rack Target SOC (%) Rack目标SOC (%)
    kRbms_ScSOCA_AccuCapAh,                                   // Rack Accumulated Capacity (Ah) Rack累计容量 (Ah)
    kRbms_SoxOutput_End,                                      // SOX算法调试的输出数据结束枚举

    /*模型调试信息*/
    kRbms_ModelDebug_Start = kRbms_SoxOutput_End, // 模型调试信息开始枚举
    kRbms_SbCTSC_AllwPrechFlg,                    // Allow to Precharge Flag 允许预充标志位
    kRbms_SbCTSC_BSWSMainNegRlyDryContactFlg,     // Main Negative Contactor Status 主负继电器干接点状态
    kRbms_SbCTSC_BSWSMainPosRlyDryContactFlg,     // Main Positive Contactor Status 主正继电器干接点状态
    kRbms_SbCTSC_SafetyState3TriggerFlg,          // High-voltage indicator in safe state 3 安全状态3下高压标志
    kRbms_ModeDebug_Reserved1,                    // 预留占位
    kRbms_ScCTSC_HvPowupCmdNbr,                   // Hv Power-on Mode 上高压模式
    kRbms_ScCTSC_CtrlModeState,                   // Control Mode Status 控制模式状态
    kRbms_ScCTSC_MainPosRlyLsdDrvRawVoltV,        // Main positive relay low-side drive voltage (V) 主正低边驱动电压 (V)
    kRbms_ScCTSC_MainNegRlyLsdDrvRawVoltV,        // Main negative relay low-side drive voltage (V) 主负低边驱动电压 (V)
    kRbms_ScCTSC_GroundChannelRawVoltmV,          // Grounding channel voltage (mV) 接地通道电压 (mV)
    kRbms_ScCTSC_MainPosRlyHsdDrvRawCurmA,        // Main positive relay high-side drive current (mA) 主正高边驱动电流 (mA)
    kRbms_ScCTSC_PrechRlyHsdDrvRawCurmA,          // High-side drive current of the pre-charging relay (mA) 预充高边驱动电流 (mA)
    kRbms_ScCTSC_MainNegRlyHsdDrvRawCurmA,        // Main negative relay high-side drive current (mA) 主负高边驱动电流 (mA)
    kRbms_ScCTSC_PrechOverTiCnt,                  // Number of consecutive precharge timeout occurrences 连续预充超时次数
    kRbms_ScBTCS_BSWSBatIDiagInfoNbr,             // Current Sensor Hardware Flt 电流传感器硬件诊断信息
    kRbms_ScBTCS_BSWSMsgAvlNbr,                   // Current Sensor Msg Timeout Flt 电流传感器报文超时故障
    kRbms_SbLVPH_CurrSnsrPowSplyNotOutRngFlg,     // Current Sensor Power Supply Volt Out Range Flg 电流传感器供电电压超上下限标志
    kRbms_SbCTSC_RlyLoadBreakFlg,                 // Contactor Off-with-Load Flg 继电器带载断开标志位
    kRbms_ModeDebug_Reserved2,                    // 预留占位
    kRbms_ScCTSC_RlyLoadBreakCurrentA,            // Current when Contactor Off-with-Load (A) 继电器带载断开时的电池串电流 (A)
    kRbms_ScBTCS_BSWSMsgCtrResNbr,                // Current Sensor Msg Rolling Counter Flt 电流传感器报文RollingCounter故障
    kRbms_ScBTCS_BSWSMsgCRCResNbr,                // Current Sensor Msg CRC Flt 电流传感器报文CRC故障
    kRbms_ScBTCS_BSWSCurValVldNbr,                // Current Sensor Current Value Invalid 电流传感器电流值无效
    kRbms_ScBTCS_CANBatIZeroDriftNbr,             // Current Sensor Zero Drift 电流传感器零漂
    kRbms_BSWSAFESampErrMonitor,                  // BSW AFE Sampling Error Monitor 底层AFE采样错误监测
    kRbms_BSWEthPHYChipResetNbr,                  // BSW Ethernet PHY Chip Reset Counter 底层以太网PHY芯片复位次数
    kRbms_ModelDebug_End,                         // 模型调试信息结束枚举  1864

    /*预留数据1864~2000*/

    kRbms_Data_End = 2000,
} devRBMSPoint_e;


// PCS(kDev_Type_PCS)测点枚举
typedef enum
{
    kPCS_Data_Start = 0,
    kPCS_Online = kPCS_Data_Start, // 在线状态
    kPCS_RunningState,             // 运行状态（待机/充电/放电）
    kPCS_FaultStatus,              // 故障状态
    kPCS_AlarmStatus,              // 告警状态
    kPCS_ActivePower,              // 有功功率(kW)
    kPCS_ReactivePower,            // 无功功率(kVar)
    kPCS_ApparentPower,            // 视在功率(kVA)
    kPCS_PowerFactor,              // 功率因数
    kPCS_BatteryVoltage,           // 电池电压(V)
    kPCS_BatteryCurrent,           // 电池电流(A)
    kPCS_ACVoltagePhaseA,          // AC相电压A(V)
    kPCS_ACVoltagePhaseB,          // AC相电压B(V)
    kPCS_ACVoltagePhaseC,          // AC相电压C(V)
    kPCS_ACCurrentPhaseA,          // AC相电流A(A)
    kPCS_ACCurrentPhaseB,          // AC相电流B(A)
    kPCS_ACCurrentPhaseC,          // AC相电流C(A)
    kPCS_GridFrequency,            // 电网频率(Hz)
    kPCS_IGBT_TempPhaseA,          // IGBT温度A(℃)
    kPCS_IGBT_TempPhaseB,          // IGBT温度B(℃)
    kPCS_IGBT_TempPhaseC,          // IGBT温度C(℃)
    kPCS_InletTemp,                // 进风温度(℃)
    kPCS_ControlMode,              // 控制模式(本地/远程)
    kPCS_GridConnectStatus,        // 并网/离网状态
    kPCS_EfficiencyRating,         // 效率(%)
    kPCS_Reserved1,

    /*预留数据24~100*/
    kPCS_Data_End = 100
} devPCSPoint_e;

// 变压器(kDev_Type_Transfer)测点枚举
typedef enum
{
    kTran_Data_Start = 0,
    kTran_Online = kTran_Data_Start, // 在线状态
    kTran_OperatingState,            // 运行状态
    kTran_FaultStatus,               // 故障状态
    kTran_TemperatureA,              // A相温度(℃)
    kTran_TemperatureB,              // B相温度(℃)
    kTran_TemperatureC,              // C相温度(℃)
    kTran_FanStatus,                 // 风扇状态
    kTran_OilTemperature,            // 油温(℃)
    kTran_OilLevel,                  // 油位
    kTran_VoltagePhaseA,             // 一次侧相电压A(V)
    kTran_VoltagePhaseB,             // 一次侧相电压B(V)
    kTran_VoltagePhaseC,             // 一次侧相电压C(V)
    kTran_CurrentPhaseA,             // 一次侧相电流A(A)
    kTran_CurrentPhaseB,             // 一次侧相电流B(A)
    kTran_CurrentPhaseC,             // 一次侧相电流C(A)
    kTran_LoadRatio,                 // 负载率(%)
    kTran_TotalFault,                // 总故障
    kTran_Reserved1,
    kTran_Reserved2,

    /*预留数据24~100*/
    kTran_Data_End = 100
} devTransPoint_e;

// 液冷/空调(kDev_Type_Lqd)测点枚举
typedef enum
{
    kLqd_Data_Start = 0,
    kLqd_Online = kLqd_Data_Start, // 在线状态

    kLqd_WorkingMode,          // TMS 工作状态0：关机模式1：制冷模式2：制热模式3：自循环模式
    kLqd_K1RelayStatus,          // KMS K1继电器状态（压缩机急停用）0：断开状态1：闭合状态2~3：无效
    kLqd_K2RelayStatus,          // KMS K2继电器状态（压缩机急停用）0：断开状态1：闭合状态2~3：无效
    kLqd_HeheatFeedbackStatus,          // KMS 预热模式反馈0：退出预热模式1：进入预热模式2~3：无效
    //出水温度（机组到电池）分辨率： 1°C/bit 偏移量： -40°C 范围：－40℃~210℃(以实际出厂为准)；单位： °C 类型：测量 255：无效值
    kLqd_OutletTemp,          // 出水温度（机组到电池）
    kLqd_InletTemp,           // 回水温度（电池到机 组）
    kLqd_EnvTemp,             // 环境温度
    //进水口压力值分辨率：0.1bar/bit 偏移量：0 单位：bar 范围：0bar~25.0bar(以实际出厂为准)；255：无效值
    kLqd_InletPressure,       // 进水口压力值
    //出水口压力值分辨率：0.1bar/bit 偏移量：0 单位：bar 范围：0bar~25.0bar(以实际出厂为准)；255：无效值
    kLqd_OutletPressure,      // 出水口压力值
    //环境湿度（除湿一体机用）分辨率： 1%/bit 偏移量：0 单位：% 范围：0%~100%(以实际出厂为准)；255：无效值
    kLqd_EnvHumidity,         // 环境湿度（除湿一体机用）

    kLqd_FaultCode,          // ：无故障有关其他特定故障代码，请参见故障代码表
    kLqd_FaultLevel,          // TMS 故障等级0：无故障1：1级故障2：2级故障3：3级故障
    

    kLqd_ReservedStart,         // 预留起始枚举

    /* --- CANid:0x18FFC13B --- */
    kLqd_ACDC_Aoltage = kLqd_ReservedStart,          // ACDC_A电压
    kLqd_PumpPWM,              // 水泵PWM
    kLqd_HighPressure1,        // 1#压缩机高压压力
    kLqd_LowPressure1,         // 1#压缩机低压压力
    kLqd_FanPWM,               // 风机PWM 1%/bit 0~100% 255=无效
    kLqd_ExpvValve,            // 电子膨胀阀开度 2step/bit 0~250 255=无效

    /* --- CANid:0x18FFC13C --- */
    kLqd_CompressorVoltage1,    // 1#压缩机电压 3V/bit 255=无效
    kLqd_CompressorCurrent1,    // 1#压缩机电流 0.5A/bit 255=无效
    kLqd_CompressorRPM1,        // 1#压缩机转速 100rpm/bit 255=无效
    kLqd_PTC_TempSwitch,       // PTC温度开关 0=断开 1=闭合
    kLqd_PumpSwitch,           // 水泵开关 0=断开 1=闭合
    kLqd_DehumidifyState,      // 除湿状态 0=停止 1=开启
    kLqd_ACFaultBit,           // AC故障位(4.4~4.8)
    kLqd_ACFaultCode,          // AC故障码 00=无 01=电源 02=过压 03=欠压
    kLqd_TMSSetTempFeedback,   // TMS设定温度反馈 1℃/bit 偏移-40℃ 255=无效
    kLqd_RemoteMode,           // 远程模式 0=本地 1=远程
    kLqd_NaturalColdValve,     // 自然冷阀门 0=关闭 1=开启
    kLqd_LockState,            // 锁机状态 0=解锁 1=锁机

    /* --- CANid:0x18FFC13D --- */
    kLqd_DeviceVersion1,       // 机组版本号(5字节)
    kLqd_DeviceVersion2,       // 机组版本号(5字节)
    kLqd_DeviceVersion3,       // 机组版本号(5字节)
    kLqd_DeviceVersion4,       // 机组版本号(5字节)
    kLqd_DeviceVersion5,       // 机组版本号(5字节)

    kLqd_SoftwareVersion1,     // 软件版本号(3字节)
    kLqd_SoftwareVersion2,     // 软件版本号(3字节)
    kLqd_SoftwareVersion3,     // 软件版本号(3字节)


    /* --- CANid:0x18FFC13E --- */
    kLqd_CompressorRPM2,        // 2#压缩机转速 100rpm/bit 255=无效
    kLqd_HighPressure2,        // 2#高压压力 1KPa/bit 65535=无效
    kLqd_LowPressure2,         // 2#低压压力 1KPa/bit 65535=无效
    kLqd_CompressorVoltage2,    // 2#压缩机电压 3V/bit 255=无效
    kLqd_CompressorCurrent2,    // 2#压缩机电流 0.5A/bit 255=无效

    /* --- 预留 --- */

    kLqd_ReservedToEnd = 70,    // 预留枚举到80
    kLqd_ReservedEnd,           // 预留结束枚举
    
    //控制参数显示
    kLqd_BMSRequestMode,                //BMS请求模式
    kLqd_BMSStatus,                     //BMS 状态 
    kLqd_ThFault,                       //热热失控故障
    kLqd_ControlMode,                   //控制模式
    kLqd_Reserved,                      //预留
    
    kLqd_HighestTemp,                   // 电芯最高温度
    kLqd_LowTemp,                       // 电芯最低温度
    kLqd_AverageTemp,                   // 电芯平均温度
    kLqd_SetTemp,                       // 设定温度
    kLqd_Current,                       // 电流
    kLqd_HighPressureStatus,            // 高压状态
    kLqd_WeatherMode,                   // 除雪模式
    kLqd_CrtlReserved,                  //预留
    
    kLqd_Year,                          // RTC-年
    kLqd_Month,                         // RTC-月
    kLqd_Day,                           // RTC-日
    kLqd_Hour,                          // RTC-时
    kLqd_Minute,                        // RTC-分
    kLqd_Second,                        // RTC-秒
    kLqd_Reserved3,                     //预留

    kLqd_Data_End = 100
} devLqdPoint_e;

// 电表(kDev_Type_Meter)测点枚举
typedef enum
{
    kMeter_Data_Start = 0,
    kMeter_Online = kMeter_Data_Start, // 在线状态
    kMeter_Voltage_A,                  // A相电压(V)
    kMeter_Voltage_B,                  // B相电压(V)
    kMeter_Voltage_C,                  // C相电压(V)
    kMeter_Current_A,                  // A相电流(A)
    kMeter_Current_B,                  // B相电流(B)
    kMeter_Current_C,                  // C相电流(C)
    kMeter_ActivePower,                // 有功功率(W)
    kMeter_ReactivePower,              // 无功功率(Var)
    kMeter_ApparentPower,              // 视在功率(VA)
    kMeter_PowerFactor,                // 功率因数
    kMeter_Frequency,                  // 频率(Hz)
    kMeter_TotalActiveEnergy,          // 总有功电能(kWh)
    kMeter_TotalReactiveEnergy,        // 总无功电能(kVarh)
    kMeter_ActiveEnergyPositive,       // 正向有功电能(kWh)
    kMeter_ActiveEnergyReverse,        // 反向有功电能(kWh)
    kMeter_DemandPower,                // 需量功率(W)
    kMeter_THD_Voltage,                // 电压THD(%)
    kMeter_THD_Current,                // 电流THD(%)
    kMeter_Reserved1,

    /*预留数据20~100*/
    kMeter_Data_End = 100
} devMeterPoint_e;

// UPS(kDev_Type_UPS)测点枚举
typedef enum
{
    kUPS_Data_Start = 0,
    kUPS_Online = kUPS_Data_Start, // 在线状态

    kUPS_AlarmStart,              // 告警状态
    kUPS_MainsPowerAbnFault = kUPS_AlarmStart,      // 市电异常   -- 枚举号不能改，这个故障判断需要
    kUPS_AlarmEnd = 10,         // 预留10

    //从11开始是遥测，遥测可以随便配
    kUPS_RunningState,             // 运行状态(充电/放电/旁路)
    kUPS_BatterySOC,               // 电池SOC(%)
    kUPS_BatteryVoltage,           // 电池电压(V)
    kUPS_BatteryCurrent,           // 电池电流(A)
    kUPS_BatteryTemp,              // 电池温度(℃)
    kUPS_InputVoltage,             // 输入电压(V)
    kUPS_InputCurrent,             // 输入电流(A)
    kUPS_OutputVoltage,            // 输出电压(V)
    kUPS_OutputCurrent,            // 输出电流(A)
    kUPS_OutputPower,              // 输出功率(kW)
    kUPS_BypassVoltage,            // 旁路电压(V)
    kUPS_BypassCurrent,            // 旁路电流(A)
    kUPS_Frequency,                // 频率(Hz)
    kUPS_Efficiency,               // 效率(%)
    kUPS_InverterTemp,             // 逆变器温度(℃)
    kUPS_Reserved1,
    kUPS_Reserved2,

    /*预留数据20~100*/
    kUPS_Data_End = 100
} devUPSPoint_e;

// 除湿机(kDev_Type_dehumidifier)测点枚举
typedef enum
{
    kDehumidifier_Data_Start = 0,
    kDehumidifier_Online = kDehumidifier_Data_Start, // 在线状态
    kDehumidifier_CurrentTemp,                       // 当前温度
    kDehumidifier_CurrentHumidity,                   // 当前湿度
    kDehumidifier_TargetHumidityStart,               // 湿度启动阈值
    kDehumidifier_TargetHumidityStop,                // 湿度停止阈值
    kDehumidifier_HeaterTempStart,                   // 加热启动阈值
    kDehumidifier_HeaterStatus,                      // 加热状态
    kDehumidifier_WorkingStatus,                     // 除湿机工作状态
    kDehumidifier_WorkModeQuery,                     // 除湿机手自动切换查询
    //设置值
    kDehumidifier_TargetHumidityStartSet = 10,       // 湿度启动阈值设置
    kDehumidifier_TargetHumidityStopSet,             // 湿度停止阈值设置
    kDehumidifier_HeaterTempStartSet,                // 加热启动阈值设置
    kDehumidifier_WorkModeSet,                       // 除湿机手自动切换设置
    kDehumidifier_Data_End = 100
} devDehumidifierPoint_e;


// 电池舱除湿机(kDev_Type_dehumidifier)测点枚举
typedef enum
{
    kDehumidifierBatt_Data_Start = 0,
    kDehumidifierBatt_Online = kDehumidifierBatt_Data_Start, // 在线状态
    kDehumidifierBatt_RunStatus,                 // 除湿机运行状态
    kDehumidifierBatt_StatusWord,                // 状态字
    kDehumidifierBatt_DewpointTemp,              // 露点温度
    kDehumidifierBatt_EnvTemp,                   // 环境温度
    kDehumidifierBatt_EnvHumidity,               // 环境湿度
    kDehumidifierBatt_CoilTemp,                  // 盘管温度
    kDehumidifierBatt_PowerStatus,               // 开/关机状态
    kDehumidifierBatt_HumiditySetpoint,          // 湿度设置值
    kDehumidifierBatt_CtrlModeInd,               // 手/自动控制模式

    //设置值
    kDehumidifierBatt_PowerCtrl = 10,       // 开关机
    kDehumidifierBatt_TargetHumidity,       // 湿度设置
    kDehumidifierBatt_PoweronTime,          // 定时开机时间
    kDehumidifierBatt_PoweroffTime,         // 定时关机时间
    kDehumidifierBatt_AutoCtrlMode,         // 手/自动控制模式
    kDehumidifierBatt_Data_End = 100
} devDehumidifierBatt_BattPoint_e;

// 温度传感器(kDev_Type_TempSensor)测点枚举
typedef enum
{
    kTempSensor_Data_Start = 0,
    kTempSensor_Online = kTempSensor_Data_Start, // 在线状态
    kTempSensor_Temperature,                     // 温度(℃)
    kTempSensor_Min,                             // 最小温度(℃)
    kTempSensor_Max,                             // 最大温度(℃)
    kTempSensor_Average,                         // 平均温度(℃)
    kTempSensor_Status,                          // 传感器状态
    kTempSensor_Reserved1,
    kTempSensor_Reserved2,

    /*预留数据8~10*/
    kTempSensor_Data_End = 10
} devTempSensorPoint_e;

// 湿度传感器(kDev_Type_HumiditySensor)测点枚举
typedef enum
{
    kHumiditySensor_Data_Start = 0,
    kHumiditySensor_Online = kHumiditySensor_Data_Start, // 在线状态
    kHumiditySensor_Humidity,                            // 湿度(%)
    kHumiditySensor_Temperature,                         // 温度(℃)
    kHumiditySensor_DewPoint,                            // 露点温度(℃)
    kHumiditySensor_Min,                                 // 最小湿度(%)
    kHumiditySensor_Max,                                 // 最大湿度(%)
    kHumiditySensor_Average,                             // 平均湿度(%)
    kHumiditySensor_Status,                              // 传感器状态
    kHumiditySensor_Reserved1,

    /*预留数据9-10*/
    kHumiditySensor_Data_End = 10
} devHumiditySensorPoint_e;

// 水浸传感器(kDev_Type_WaterSensor)测点枚举
typedef enum
{
    kWaterSensor_Data_Start = 0,
    kWaterSensor_Online = kWaterSensor_Data_Start, // 在线状态
    kWaterSensor_WaterDetected,                    // 水浸检测状态(0=未检测/1=已检测)
    kWaterSensor_Sensitivity,                      // 灵敏度
    kWaterSensor_AlarmThreshold,                   // 报警阈值
    kWaterSensor_Reserved1,
    kWaterSensor_Reserved2,

    /*预留数据6~10*/
    kWaterSensor_Data_End = 10
} devWaterSensorPoint_e;

// 烟雾传感器(kDev_Type_SmokeSensor)测点枚举
typedef enum
{
    kSmokeSensor_Data_Start = 0,
    kSmokeSensor_Online = kSmokeSensor_Data_Start, // 在线状态
    kSmokeSensor_SmokeLevel,                       // 烟雾浓度
    kSmokeSensor_SmokeDetected,                    // 烟雾检测状态
    kSmokeSensor_AlarmThreshold,                   // 报警阈值
    kSmokeSensor_Sensitivity,                      // 灵敏度
    kSmokeSensor_BatteryVoltage,                   // 电池电压(V)
    kSmokeSensor_Reserved1,
    kSmokeSensor_Reserved2,

    /*预留数据6~10*/
    kSmokeSensor_Data_End = 10
} devSmokeSensorPoint_e;

// 其他传感器1(kDev_Type_OtherSensor1)测点枚举
typedef enum
{
    kOtherSensor1_Data_Start = 0,
    kOtherSensor1_Online = kOtherSensor1_Data_Start, // 在线状态
    kOtherSensor1_Value,                             // 测点值
    kOtherSensor1_Status,                            // 状态
    kOtherSensor1_Reserved1,
    kOtherSensor1_Reserved2,

    /*预留数据6~10*/
    kOtherSensor1_Data_End = 10
} devOtherSensor1Point_e;

// 其他传感器2(kDev_Type_OtherSensor2)测点枚举
typedef enum
{
    kOtherSensor2_Start = 0,
    kOtherSensor2_Online = kOtherSensor2_Start, // 在线状态
    kOtherSensor2_Value,                        // 测点值
    kOtherSensor2_Status,                       // 状态
    kOtherSensor2_Reserved1,
    kOtherSensor2_Reserved2,

    /*预留数据6~10*/
    kOtherSensor2_Data_End = 10
} devOtherSensor2Point_e;

// 其他传感器3(kDev_Type_OtherSensor3)测点枚举
typedef enum
{
    kOtherSensor3_Data_Start = 0,
    kOtherSensor3_Online = kOtherSensor3_Data_Start, // 在线状态
    kOtherSensor3_Value,                             // 测点值
    kOtherSensor3_Status,                            // 状态
    kOtherSensor3_Reserved1,
    kOtherSensor3_Reserved2,
    kOtherSensor3_Data_End = 10
} devOtherSensor3Point_e;

// 预留设备类型17(kDev_Type_Resv17)测点枚举
typedef enum
{
    kResv17_Data_Start = 0,
    kResv17_Online = kResv17_Data_Start, // 在线状态
    kResv17_Value,                       // 测点值
    kResv17_Status,                      // 状态
    kResv17_Reserved1,
    kResv17_Reserved2,
    kResv17_Data_End = 100
} devResv17Point_e;

// 消防设备(kDev_Type_FireDevice)测点枚举
typedef enum
{
    kFireDevice_Data_Start = 0,
    kFireDevice_Online = kFireDevice_Data_Start, // 在线状态
    kFireDevice_ActivateStatus,                  // 激活状态(0=待命/1=激活)
    kFireDevice_Pressure,                        // 压力(MPa)
    kFireDevice_Temperature,                     // 温度(℃)
    kFireDevice_BatteryVoltage,                  // 电池电压(V)
    kFireDevice_FireLevel,                       // 消防等级
    kFireDevice_MaintenanceRequired,             // 需要维护(0=否/1=是)
    kFireDevice_Reserved1,
    kFireDevice_Reserved2,

    /*预留数据9~100*/
    kFireDevice_Data_End = 100
} devFireDevicePoint_e;

// clang-format on
#endif /**/