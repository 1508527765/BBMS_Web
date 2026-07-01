/*****************************************************************************
 * @copyright       1997-2050, Gary. POWER SUPPLY CO., LTD. 
 * @file            app_bms_fault.h
 * @brief           故障检测功能
 * @author          gary
 * @date            2026-04-22
 * @remark          
 *****************************************************************************/

#ifndef APP_BMS_FAULT_H
#define APP_BMS_FAULT_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "kit_data.h"
#include "kit_log.h"
#include "app_bms_statistics.h"
#include "kit_model.h"
#include "bsp_rtdb.h"
#include "logic_di.h"
#include "bsp_rtdb.h"
#include "logic_di.h"
#include "app_bms_ctrl.h"
//bbms m核故障类型枚举
typedef enum{
    kBmsMFault_Start = 0,

    //bms类故障
    kBmsMFault_AllRackCommLost = kBmsMFault_Start,          // 所有RBMS通讯丢失
    kBmsMFault_OneRackCommLost,          // 至少一个RBMS通讯丢失

    //系统类故障
    kBmsMFault_AllCluDeltaTempOver,      // 所有簇温差过大故障
    kBmsMFault_CluDeltaCurrOverLv1,      // 电池簇间环流越限1级
    kBmsMFault_CluDeltaCurrOverLv2,      // 电池簇间环流越限2级
    kBmsMFault_CluDeltaCurrOverLv3,      // 电池簇间环流越限3级
    //bms故障
    kBmsMFault_PCSCommLost,              // PCS通信丢失故障

    //系统类故障
    kBmsMFault_CluVoltDiffExces,         // 簇间压差过大故障
    kBmsMFault_IDLearnFail,              // IDLearm编码失败

    //外设类故障
    kBmsMFault_WaterCoolModeMismatch,    // 水冷机组工作模式不匹配故障
    kBmsMFault_WaterCoolLvl1Fault,       // 水冷机一级故障
    kBmsMFault_WaterCoolLvl2Fault,       // 水冷机二级故障
    kBmsMFault_TMSCommLost,              // TMS通讯丢失

    //bms故障
    kBmsMFault_24VVoltOutRange,          // 24V电压超范围故障
    kBmsMFault_12VVoltOutRange,          // 12V电压超范围故障
    //系统类故障
    kBmsMFault_OverPower,                // 过功率故障
    kBmsMFault_RackSOCDiffExces,         // Rack间SOC差距过大故障

    //外设类故障
    kBmsMFault_DoorAccessFault,          // 门禁故障
    kBmsMFault_QF1CircuitBrkFault,       // QF1断路器故障
    kBmsMFault_SPD2SurgeFault,           // SPD2浪涌故障
    kBmsMFault_WaterLeakAlarm,           // 水浸告警故障
    kBmsMFault_CombGasAlarm,             // 可燃气体告警故障
    kBmsMFault_ExproFanAlarm,            // 防爆风机告警故障
    kBmsMFault_EmergStopFault,           // 急停故障
    kBmsMFault_FireExtremeAlarm,         // 消防严重告警
    kBmsMFault_FireModerateAlarm,        // 消防中度告警
    kBmsMFault_FireFault,                // 消防故障
    kBmsMFault_UPSCommLost,              // UPS通讯节点丢失故障
    kBmsMFault_EnergyMeterCommLost,      // 电表通讯节点丢失故障
    kBmsMFault_PCSEmergStopFault,        // PCS急停故障
    kBmsMFault_ContainerEmergStop,       // 集装箱急停故障
    kBmsMFault_DistrCabEmergStop,        // 配电柜急停故障
    kBmsMFault_EMSCommLost,              // EMS通讯节点丢失故障
    kBmsMFault_BatCompDehumidCommLost,   // 电池舱除湿机通讯节点丢失故障
    kBmsMFault_MainsPowerAbnLvl1,        // 市电异常一级故障
    kBmsMFault_FireAlarmActAbn,          // 消防火警启动异常
    kBmsMFault_CtrlCabDCSurgeFault,      // 中控柜直流SPD浪涌故障
    kBmsMFault_BackupPowerSPD1Fault,     // 备电SPD1故障
    kBmsMFault_CustExtEmergStop,         // 客户外部急停
    kBmsMFault_WaterCoolLvl3Fault,       // 水冷机三级故障

    //bms类故障
    kBmsMFault_ACoreCommFault,           // A核通讯故障
    kBmsMFault_OneRackCanCommLost,       // 至少一个RBMS的CAN通讯丢失
    kBmsMFault_AllRackCanCommLost,       // 所有RBMS的CAN通讯丢失

    //外设类故障
    kBmsMFault_ConvFuseFault,            // 汇流熔断器故障
    kBmsMFault_CtrlCabACCommLost,        // 中控柜空调/除湿机通讯节点丢失故障
    kBmsMFault_CtrlCabTravelSwFault,     // 中控柜行程开关故障
    kBmsMFault_FireFightCommLost,        // 消防通讯节点丢失故障
    kBmsMFault_MainsPowerAbnLvl3,        // 市电异常三级故障
    //系统类故障
    kBmsMFault_HVBoxConvReverse,         // 高压盒与汇流柜反接故障

    kBmsMFault_End,

}bms_m_fault_type_e;


//bbms A核故障类型枚举
typedef enum{
    kBmsAFault_Start = 0,
    //按提供故障顺序，极简缩写，注释仅留中文
    kBmsAFault_SSDFault = kBmsAFault_Start,    // SSD故障
    kBmsAFault_SDFault,                 // SD故障
    kBmsAFault_MCoreCommFault,          // M核通讯故障
    kBmsAFault_EMSCommLost,             // EMS通讯节点丢失故障
    kBmsAFault_OneRackLanCommLost,      // 至少一个RBMS的LAN通讯丢失
    kBmsAFault_AllRackLanCommLost,      // 所有RBMS的LAN通讯丢失
    kBmsAFault_OneRackCommLost,         // 至少一个RBMS通讯丢失
    kBmsAFault_AllRackCommLost,         // 所有RBMS通讯丢失
    kBmsAFault_End,
}bms_a_fault_type_e;


// RBMS故障类型枚举，
typedef enum {
    kRbmsFault_Start = 0,           
     kRbmsFault_AfeHardware = kRbmsFault_Start,                        // bit0 - AFE硬件故障
    kRbmsFault_DaisyChnCommLost,                    // bit1 - 菊花链通信丢失故障
    kRbmsFault_AfeSmpCrc,                           // bit2 - AFE采样CRC故障
    kRbmsFault_CellVOffLine,                        // bit3 - 电芯电压采样线掉线
    kRbmsFault_DaisyChnSglOff,                      // bit4 - 菊花链单点断线
    kRbmsFault_DaisyChnMplOff,                      // bit5 - 菊花链多点断线
    kRbmsFault_BalLoopCirc,                         // bit6 - 均衡回路故障
    kRbmsFault_CellTStg,                            // bit7 - 电芯温度采样短地(STG)
    kRbmsFault_CellTStbOrOc,                        // bit8 - 电芯温度采样短电源(STB or Oc)
    kRbmsFault_CellTMux,                            // bit9 - 电芯温度采样MUX故障
    kRbmsFault_CellVOutRange,                       // bit10 - 电芯电压采样超范围
    kRbmsFault_HvSnsr,                              // bit11 - 高压采样传感器故障
    kRbmsFault_BatVOutRange,                        // bit12 - Battery电压超范围故障
    kRbmsFault_LinkPosVOutRange,                    // bit13 - LinkPos电压超范围故障
    kRbmsFault_CurSnsrCommLost,                     // bit14 - 电流传感器通讯丢失故障
    kRbmsFault_CurSnsrHw,                           // bit15 - 电流传感器硬件故障
    kRbmsFault_CurSnsrZeroDrift,                    // bit16 - 电流传感器零漂故障
    kRbmsFault_CurSnsrCrc,                          // bit17 - 电流传感器CRC故障
    kRbmsFault_Spd1Surge,                           // bit18 - 主控SPD1浪涌反馈
    kRbmsFault_Reserved19,                          // bit19 - 保留(原MCU故障复位原因-已删除)
    kRbmsFault_Reserved20,                          // bit20 - 保留(原MCU内部数据及存储地址错误-已删除)
    kRbmsFault_CellVHighL1,                         // bit21 - 单体电压高一级
    kRbmsFault_CellVHighL2,                         // bit22 - 单体电压高二级
    kRbmsFault_CellVHighL3,                         // bit23 - 单体电压高三级
    kRbmsFault_CellVLowL1,                          // bit24 - 单体电压低一级
    kRbmsFault_CellVLowL2,                          // bit25 - 单体电压低二级
    kRbmsFault_CellVLowL3,                          // bit26 - 单体电压低三级
    kRbmsFault_CellDeltaVL1,                        // bit27 - 单体压差大一级
    kRbmsFault_CellDeltaVL2,                        // bit28 - 单体压差大二级
    kRbmsFault_CellDeltaVL3,                        // bit29 - 单体压差大三级
    kRbmsFault_CellExtremeOvV,                      // bit30 - 单体极限过压
    kRbmsFault_CellExtremeUvV,                      // bit31 - 单体极限欠压
    kRbmsFault_CellVSumAfeMis,                      // bit32 - 单体电压和与AFE电压不匹配
    kRbmsFault_HvLoopBkn,                           // bit33 - 高压回路断路故障
    kRbmsFault_CellVWithBatVMis,                    // bit34 - 电芯电压与Battery电压不匹配
    kRbmsFault_MsdOpen,                             // bit35 - MSD开路(互锁)故障
    kRbmsFault_BatOvVL1,                            // bit36 - 总电压过高一级
    kRbmsFault_BatOvVL2,                            // bit37 - 总电压过高二级
    kRbmsFault_BatOvVL3,                            // bit38 - 总电压过高三级
    kRbmsFault_BatUvVL1,                            // bit39 - 总电压过低一级
    kRbmsFault_BatUvVL2,                            // bit40 - 总电压过低二级
    kRbmsFault_BatUvVL3,                            // bit41 - 总电压过低三级
    kRbmsFault_DChgCurL1,                           // bit42 - 放电电流大一级
    kRbmsFault_DChgCurL2,                           // bit43 - 放电电流大二级
    kRbmsFault_DChgCurL3,                           // bit44 - 放电电流大三级
    kRbmsFault_RChgCurL1,                           // bit45 - 充电电流大一级
    kRbmsFault_RChgCurL2,                           // bit46 - 充电电流大二级
    kRbmsFault_RChgCurL3,                           // bit47 - 充电电流大三级
    kRbmsFault_CellExtremeOt,                       // bit48 - 电芯极限过温
    kRbmsFault_CellDeltaTL1,                        // bit49 - 电芯温度差异大一级
    kRbmsFault_CellDeltaTL2,                        // bit50 - 电芯温度差异大二级
    kRbmsFault_CellDeltaTL3,                        // bit51 - 电芯温度差异大三级
    kRbmsFault_CellTRiseFast,                       // bit52 - 电芯温升过快
    kRbmsFault_CellThermRunaway,                    // bit53 - 电芯热失控故障
    kRbmsFault_SlvBalTempHL1,                       // bit54 - 从板均衡温度高一级
    kRbmsFault_SlvBalTempHL2,                       // bit55 - 从板均衡温度高二级
    kRbmsFault_AfeChipOt,                           // bit56 - AFE芯片过温报警
    kRbmsFault_HvBoxOtL1,                           // bit57 - 高压盒过温一级
    kRbmsFault_HvBoxOtL2,                           // bit58 - 高压盒过温二级
    kRbmsFault_HvBoxOtL3,                           // bit59 - 高压盒过温三级
    kRbmsFault_RlyPwrOvV,                           // bit60 - 继电器供电电压过高故障
    kRbmsFault_RlyPwrUvV,                           // bit61 - 继电器供电电压过低故障
    kRbmsFault_CurSnsrPwrOvV,                       // bit62 - 电流传感器供电电压过高故障
    kRbmsFault_CurSnsrPwrUvV,                       // bit63 - 电流传感器供电电压过低故障
    kRbmsFault_RbmsPwrOvV,                          // bit64 - RBMS供电电压过高故障
    kRbmsFault_RbmsPwrUvV,                          // bit65 - RBMS供电电压过低故障
    kRbmsFault_HvSnsrPwrUvV,                        // bit66 - 高压采样芯片供电电压过低故障
    kRbmsFault_HvIsLowL1,                           // bit67 - 高压绝缘低一级
    kRbmsFault_HvIsLowL2,                           // bit68 - 高压绝缘低二级
    kRbmsFault_HvIsLowL3,                           // bit69 - 高压绝缘低三级
    kRbmsFault_IsInvalid,                           // bit70 - 绝缘无效故障
    kRbmsFault_CellSelfDisL1,                       // bit71 - 电芯自放电差异过大一级
    kRbmsFault_CellSelfDisL2,                       // bit72 - 电芯自放电差异过大二级
    kRbmsFault_CellSelfDisL3,                       // bit73 - 电芯自放电差异过大三级
    kRbmsFault_RackOverChg,                         // bit74 - Rack过充
    kRbmsFault_PrechTimeout,                        // bit75 - 预充超时故障
    kRbmsFault_PrechOc,                             // bit76 - 预充过流故障
    kRbmsFault_Reserved77,                          // bit77 - 保留(原主控EPO开-已删除)
    kRbmsFault_IslSwitchOpen,                       // bit78 - 隔离开关开
    kRbmsFault_BankCommLost,                        // bit79 - Bank通信丢失故障
    kRbmsFault_BankCrcErr,                          // bit80 - Bank通信CRC异常故障
    kRbmsFault_IsPosVFault,                         // bit81 - 绝缘正对地电压故障
    kRbmsFault_IsNegVFault,                         // bit82 - 绝缘负对地电压故障
    kRbmsFault_CellPoleTempDifL1,                   // bit83 - 电芯单体温度与电芯极柱温度偏差过大一级故障
    kRbmsFault_SocLowL1,                            // bit84 - SOC低一级
    kRbmsFault_SocLowL2,                            // bit85 - SOC低二级
    kRbmsFault_SocDiffLarge,                        // bit86 - SOC差异过大
    kRbmsFault_SohLowL1,                            // bit87 - SOH低一级
    kRbmsFault_SohLowL2,                            // bit88 - SOH低二级
    kRbmsFault_SohLowL3,                            // bit89 - SOH低三级
    kRbmsFault_CombineSwUvV,                        // bit90 - 融合开关欠压故障
    kRbmsFault_SocLowL3,                            // bit91 - SOC低三级
    kRbmsFault_SocHighL1,                           // bit92 - SOC高一级
    kRbmsFault_SocHighL2,                           // bit93 - SOC高二级
    kRbmsFault_SocHighL3,                           // bit94 - SOC高三级
    kRbmsFault_PoleOtL1,                            // bit95 - 极柱温度过温一级告警
    kRbmsFault_PoleOtL2,                            // bit96 - 极柱温度过温二级告警
    kRbmsFault_PoleOtL3,                            // bit97 - 极柱温度过温三级告警
    kRbmsFault_ChgOtL1,                             // bit98 - 电芯充电温度过高一级报警
    kRbmsFault_ChgOtL2,                             // bit99 - 电芯充电温度过高二级报警
    kRbmsFault_ChgOtL3,                             // bit100 - 电芯充电温度过高三级报警
    kRbmsFault_DsgOtL1,                             // bit101 - 电芯放电温度过高一级报警
    kRbmsFault_DsgOtL2,                             // bit102 - 电芯放电温度过高二级报警
    kRbmsFault_DsgOtL3,                             // bit103 - 电芯放电温度过高三级报警
    kRbmsFault_ChgUtL1,                             // bit104 - 电芯充电温度过低一级报警
    kRbmsFault_ChgUtL2,                             // bit105 - 电芯充电温度过低二级报警
    kRbmsFault_ChgUtL3,                             // bit106 - 电芯充电温度过低三级报警
    kRbmsFault_DsgUtL1,                             // bit107 - 电芯放电温度过低一级报警
    kRbmsFault_DsgUtL2,                             // bit108 - 电芯放电温度过低二级报警
    kRbmsFault_DsgUtL3,                             // bit109 - 电芯放电温度过低三级报警
    kRbmsFault_ModVHighL1,                          // bit110 - 模组电压高一级
    kRbmsFault_ModVHighL2,                          // bit111 - 模组电压高二级
    kRbmsFault_ModVHighL3,                          // bit112 - 模组电压高三级
    kRbmsFault_ModVLowL1,                           // bit113 - 模组电压低一级
    kRbmsFault_ModVLowL2,                           // bit114 - 模组电压低二级
    kRbmsFault_ModVLowL3,                           // bit115 - 模组电压低三级
    kRbmsFault_MainNegRlyStkCls,                    // bit116 - 主负继电器粘连故障
    kRbmsFault_MainNegRlyStkOpn,                    // bit117 - 主负继电器开路故障
    kRbmsFault_MainPosRlyStkCls,                    // bit118 - 主正继电器粘连故障
    kRbmsFault_MainPosRlyStkOpn,                    // bit119 - 主正继电器开路故障
    kRbmsFault_Reserved120,                         // bit120 - 保留(原主正继电器辅助触点电路故障-已删除)
    kRbmsFault_Reserved121,                         // bit121 - 保留(原主负继电器辅助触点电路故障-已删除)
    kRbmsFault_DoorAccess,                          // bit122 - 门禁故障
    kRbmsFault_AfeVOutRange,                        // bit123 - AFE电压采样超范围
    kRbmsFault_ChgExtremeOc,                        // bit124 - 充电极限过流报警
    kRbmsFault_DsgExtremeOc,                        // bit125 - 放电极限过流报警
    kRbmsFault_RlyPwrOutRange,                      // bit126 - 继电器供电电压超范围故障
    kRbmsFault_CurSnsrPwrOutRange,                  // bit127 - 电流传感器供电电压超范围故障
    kRbmsFault_RbmsPwrOutRange,                     // bit128 - RBMS供电电压超范围故障
    kRbmsFault_CellExtremeUt,                       // bit129 - 电芯极限欠温
    kRbmsFault_CellTRedunChk,                       // bit130 - 电芯温度冗余检测故障
    kRbmsFault_EmergShdnCanTo,                      // bit131 - 紧急关断信号CAN通信超时故障
    kRbmsFault_EmergShdnCanCrc,                     // bit132 - 紧急关断信号CAN通信CRC故障
    kRbmsFault_EmergShdnCanCnt,                     // bit133 - 紧急关断信号CAN通信Counter故障
    kRbmsFault_McuInit,                             // bit134 - MCU初始化故障
    kRbmsFault_McuGpio,                             // bit135 - MCU芯片GPIO模块故障
    kRbmsFault_McuSpi,                              // bit136 - MCU芯片SPI模块故障
    kRbmsFault_McuCan,                              // bit137 - MCU芯片CAN模块故障
    kRbmsFault_McuAdc,                              // bit138 - MCU芯片ADC模块故障
    kRbmsFault_MainPosRlyDrvCir,                    // bit139 - 主正继电器驱动电路故障
    kRbmsFault_MainNegRlyDrvCir,                    // bit140 - 主负继电器驱动电路故障
    kRbmsFault_RlyDrvSmpSel,                        // bit141 - 继电器驱动回采选通故障
    kRbmsFault_FireL1,                              // bit142 - 消防一级故障
    kRbmsFault_RtcInvalid,                          // bit143 - RTC时间无效故障
    kRbmsFault_HvBoxFuse,                           // bit144 - 高压箱熔断器故障
    kRbmsFault_PackConnOtL1,                        // bit145 - Pack正负极连接件温度过温一级告警
    kRbmsFault_PackConnOtL2,                        // bit146 - Pack正负极连接件温度过温二级告警
    kRbmsFault_PackConnOtL3,                        // bit147 - Pack正负极连接件温度过温三级告警
    kRbmsFault_SlvBalTStg,                          // bit148 - 从板均衡温度采样短地(STG)
    kRbmsFault_SlvBalTStbOrOc,                      // bit149 - 从板均衡温度采样短电源(STB or Oc)
    kRbmsFault_ModDeltaVL1,                         // bit150 - 模组压差大一级
    kRbmsFault_ModDeltaVL2,                         // bit151 - 模组压差大二级
    kRbmsFault_ModDeltaVL3,                         // bit152 - 模组压差大三级
    kRbmsFault_DaisyChnCnt,                         // bit153 - 菊花链通信Counter故障
    kRbmsFault_CellVRedunChk,                       // bit154 - 电芯电压冗余检测故障
    kRbmsFault_NtcPwr,                              // bit155 - NTC供电故障
    kRbmsFault_MainPosBusBkn,                       // bit156 - 主正连接母线断路故障
    kRbmsFault_CoolFan,                             // bit157 - 散热风扇故障
    kRbmsFault_Smoke,                               // bit158 - 烟感故障
    kRbmsFault_TempDetect,                          // bit159 - 温感故障
    kRbmsFault_EStop,                               // bit160 - 急停故障
    kRbmsFault_WaterImm,                            // bit161 - 水浸告警故障
    kRbmsFault_Qf1Brk,                              // bit162 - QF1断路器故障
    kRbmsFault_Aerosol1,                            // bit163 - 气溶胶故障1
    kRbmsFault_Aerosol2,                            // bit164 - 气溶胶故障2
    kRbmsFault_DehumidifierLost,                    // bit165 - 除湿机通讯节点丢失故障
    kRbmsFault_IoModuleLost,                        // bit166 - IO模块通讯节点丢失故障
    kRbmsFault_PcsLost,                             // bit167 - PCS通信丢失故障
    kRbmsFault_BusShort,                            // bit168 - 总线短路故障
    kRbmsFault_CurSnsrOverRange,                    // bit169 - 电流传感器超量程故障
    kRbmsFault_HvSnsrPwrOvV,                        // bit170 - 高压采样芯片供电电压过高故障
    kRbmsFault_PrechRlyStkCls,                      // bit171 - 预充继电器粘连故障
    kRbmsFault_End
} rbms_fault_type;

// RBMS故障结构体，按位存储，共25字节(200位)
typedef struct __attribute__((packed)) {
    uint8_t faultBits[25];  // 200个故障位，每位对应一个故障
} RBMS_Fault_t;

// M 核BBMS的实际故障位定义
typedef struct __attribute__((packed)) {
    uint8_t byte[25];  // 25字节 = 200bit，直接访问整个字节数组（对应原byte0~byte24）
} BBMS_M_Fault_t;


// A 核BBMS的实际故障位定义，占用一个字节
typedef struct __attribute__((packed)) {
    uint8_t byte;
} BBMS_A_Fault_t;


/*****************************************************************************
 * 故障处理措施枚举
 *****************************************************************************/
typedef enum {
    kFaultHandle_None,          // 无处理
    kFaultHandle_AlarmOnly,     // 仅告警
    kFaultHandle_Derating,      // 降流处理
    kFaultHandle_Shutdown,      // 关机处理
    kFaultHandle_End
} fault_handle_e;



/*****************************************************************************
 * 数据源类型枚举
 *****************************************************************************/
typedef enum {
    kDataSource_Rtdb = 0,       // 从RTDB获取
    kDataSource_DI,             // 从DI信号获取
    kDataSource_Callback,       // 从回调函数获取
    kDataSource_End
} data_source_type_e;


/*****************************************************************************
 * 比较条件枚举
 *****************************************************************************/
typedef enum {
    kCompare_GreaterThan = 0,   // 大于
    kCompare_LessThan,          // 小于
    kCompare_Equal,             // 等于
    kCompare_NotEqual,          // 不等于
    kCompare_GreaterEqual,      // 大于等于
    kCompare_LessEqual,         // 小于等于
    kCompare_End
} compare_condition_e;


/*****************************************************************************
 * 故障数据源配置结构
 *****************************************************************************/
typedef struct {
    data_source_type_e type;        // 数据源类型
    uint16_t dev_type;              // 设备类型(RTDB用)
    uint16_t dev_id;                // 设备ID(RTDB用)
    uint16_t point_id;              // 点号(RTDB用)
    uint16_t di_idx;                // DI信号索引(DI用)
    double (*get_value_cb)(void);   // 回调函数(回调用)
} fault_data_source_t;

//故障使能
typedef enum{
    kFaultEnable_Enabled = 0,      // 使能
    kFaultEnable_Disabled,        // 禁用
} fault_enable_e;


//故障等级，一级，二级，三级
typedef enum{
    kFaultLevel_None = 0,      // 无故障
    kFaultLevel_1,      // 一级
    kFaultLevel_2,        // 二级
    kFaultLevel_3,        // 三级
    kFaultLevel_No_Fault = 0x07,      // 无故障
} fault_level_e;




/*****************************************************************************
 * 故障配置结构（const，静态配置）
 *****************************************************************************/
typedef struct {
    uint16_t fault_id;              // 故障ID
    fault_enable_e enable;          // 故障使能设置
    fault_level_e level;            // 故障等级
    fault_handle_e handle_type;     // 处理措施
    double trigger_threshold;       // 故障触发阈值
    double recover_threshold;       // 故障恢复阈值
    uint32_t trigger_time_ms;       // 故障触发持续时间(ms)
    uint32_t recover_time_ms;       // 故障恢复持续时间(ms)
    compare_condition_e compare_cond;   // 比较条件
    uint8_t derating_ratio;              //降流比例（0-100）
} fault_config_t;


/*****************************************************************************
 * 故障运行时状态结构
 *****************************************************************************/
typedef struct {
    bool is_fault;                      // 是否处于故障状态
    uint32_t fault_start_time;          // 故障开始时间
    uint32_t recover_start_time;        // 恢复开始时间
    bool trigger_timer_active;          // 触发计时器是否激活
    bool recover_timer_active;          // 恢复计时器是否激活
} fault_state_t;

/*****************************************************************************
 * @brief       获取rbms是否关机
 * @param[in]   cluster_id: RBMS簇ID，从1开始
 * @retval      电源状态: true-开机，false-关机
 *****************************************************************************/
uint8_t bms_get_rbms_is_off_status(uint16_t cluster_id);

/*****************************************************************************
 * @brief       初始化故障检测功能
 *****************************************************************************/
void bms_fault_init(void);


/*****************************************************************************
 * @brief       故障检测主循环（周期调用）
 *****************************************************************************/
void bms_fault_detect_loop(void);


/*****************************************************************************
 * @brief       获取M核故障状态
 * @param[in]   fault_type: 故障类型
 * @return      true-故障中  false-正常
 *****************************************************************************/
bool bms_get_m_fault_status(bms_m_fault_type_e fault_type);


/*****************************************************************************
 * @brief       获取A核故障状态
 * @param[in]   fault_type: 故障类型
 * @return      true-故障中  false-正常
 *****************************************************************************/
bool bms_get_a_fault_status(bms_a_fault_type_e fault_type);


/*****************************************************************************
 * @brief       获取M核故障数组状态
 * @param[in]   fault_type: 故障类型
 * @return      true-故障中  false-正常
 *****************************************************************************/
uint8_t  bms_get_m_bank_idx_byte_value(uint16_t fault_array_id);


/*****************************************************************************
 * @brief       获取 A 核故障数组字节（BBMS_A_Fault 25 字节中的某一字节）
 * @param[in]   fault_array_id: 字节索引 0~24，仅 byte0 含有效故障位
 * @return      故障字节值
 *****************************************************************************/
uint8_t bms_get_a_bank_idx_byte_value(uint16_t fault_array_id);

/*****************************************************************************
 * @brief       获取Bank自身a和m核故障最高故障等级
 * @param[in]   bbms_idx: 为模拟堆预留，从1开始
 * @return      最高故障等级
 *****************************************************************************/
fault_level_e bms_get_bank_max_flt_level(uint16_t bbms_idx);


/*****************************************************************************
 * @brief       获取M核故障处理措施
 * @param[in]   fault_type: 故障类型
 * @return      处理措施
 *****************************************************************************/
fault_handle_e bms_get_m_fault_handle(bms_m_fault_type_e fault_type);


/*****************************************************************************
 * @brief       获取A核故障处理措施
 * @param[in]   fault_type: 故障类型
 * @return      处理措施
 *****************************************************************************/
fault_handle_e bms_get_a_fault_handle(bms_a_fault_type_e fault_type);


/*****************************************************************************
 * @brief       获取rbms故障
 * @param[in]   fault_id: 故障类型
 *****************************************************************************/
uint8_t get_rbms_fault_bit(uint8_t cluster_id, rbms_fault_type fault_id);


/*****************************************************************************
 * @brief       获取rbms是否存在下电故障
 * @param[in]   rbms_id: rbms序号，从0开始
 *****************************************************************************/
uint8_t get_rbms_power_off_fault(uint8_t rbms_id);



/*****************************************************************************
 * @brief       更新rbms故障
 * @param[in]   fault_id: 故障类型
 * @param[in]   bit_value: 要设置的值 (0-清除故障, 1-设置故障)
 * @retval      0-成功, 1-无效故障ID, 2-无效bit值
 *****************************************************************************/
uint8_t update_rbms_fault_bit(uint8_t cluster_id, rbms_fault_type fault_id, uint8_t bit_value);

#endif // APP_BMS_FAULT_H