/*****************************************************************************
 * @file            protocol_ems_modbus_map.c
 * @brief           EMS / 上位机共用 Modbus TCP 点表映射（V1.0.13）
 *
 * 点表：BMS2.0_BMS_EMS_IP_TCP_Register_Table_V1.0.13.xlsx
 * 服务：TCP 502（creatModbusTcpStdSlaveTask），与 modbus_default_server.c 通用模板分离
 * 使用者：EMS、计划中的上位机（同一套地址，本文件为唯一数据源映射）
 *
 * 数据流：RBMS 协议 / 堆统计 / 故障检测 → RTDB → 本文件 → Modbus 应答
 * 不负责：TCP 收发、功能码注册（见 protocol_ems_modbus_server.c）
 *
 * 文件结构（按 Modbus 寄存器类型划分，详见 protocol_ems_modbus_map.h 顶部表）：
 *   [1]  离散输入 Discrete Input     — 0x02，堆 1~199 / 簇 200*n
 *   [2]  输入寄存器 Input Register    — 0x04，堆 1~99
 *   [2R] 输入寄存器 Input Register    — 0x04，簇 100+3000*n
 *   [4]  保持寄存器 Holding Register — 0x03 读 / 0x06·0x10 写，500~699
 *   [3]  对外 API（server.c 按功能码调用）
 *****************************************************************************/

#include "protocol_ems_modbus_map.h"
#include "app_bms_fault.h"
#include "app_bms_statistics.h"
#include "bsp_rtdb.h"
#include "kit_model.h"
#include "kit_time.h"
#include <string.h>

/* ========================================================================== */
/* [1] 离散输入 Discrete Input — 功能码 0x02（只读，按位 0/1）                    */
/*     堆：Modbus 地址 1~199                                                    */
/*     簇：Modbus 地址 200+200*i ~ 399+200*i（i 为簇号 0 基，见 s_rack_disc）       */
/* ========================================================================== */

/* [1.1] 离散点条目类型（对应 Excel Mapping Signal 的求值方式） */

typedef enum {
    EMS_DISC_ZERO = 0,
    EMS_DISC_RBMS_OR,
    EMS_DISC_RBMS_OR2,
    EMS_DISC_RBMS_BIT,
    EMS_DISC_M_FAULT,
    EMS_DISC_MAXFLT_LVL23, /* 堆离散 56：kBbms_BMSMaxFltLevel 为 2 或 3 */
    EMS_DISC_MAXFLT_LVL1,  /* 堆离散 57：kBbms_BMSMaxFltLevel 为 1 */
} ems_disc_kind_e;

typedef struct {
    uint8_t kind;
    uint16_t param1;
    uint16_t param2;
} ems_disc_entry_t;


/* [1.2] 堆离散点表 s_bank_disc[200]：下标 = Modbus地址 - 1（地址 1 对应 [0]） */
/*       由 V1.0.13 Excel 自动生成，勿手改条目 */
static const ems_disc_entry_t s_bank_disc[200] = {
    [  0] = { EMS_DISC_ZERO, 0, 0 },
    [  1] = { EMS_DISC_RBMS_OR, kRbmsFault_BatUvVL3, 0 },
    [  2] = { EMS_DISC_RBMS_OR, kRbmsFault_BatUvVL2, 0 },
    [  3] = { EMS_DISC_RBMS_OR, kRbmsFault_BatUvVL1, 0 },
    [  4] = { EMS_DISC_RBMS_OR, kRbmsFault_BatOvVL3, 0 },
    [  5] = { EMS_DISC_RBMS_OR, kRbmsFault_BatOvVL2, 0 },
    [  6] = { EMS_DISC_RBMS_OR, kRbmsFault_BatOvVL1, 0 },
    [  7] = { EMS_DISC_RBMS_OR, kRbmsFault_RChgCurL3, 0 },
    [  8] = { EMS_DISC_RBMS_OR, kRbmsFault_RChgCurL2, 0 },
    [  9] = { EMS_DISC_RBMS_OR, kRbmsFault_RChgCurL1, 0 },
    [ 10] = { EMS_DISC_RBMS_OR, kRbmsFault_DChgCurL3, 0 },
    [ 11] = { EMS_DISC_RBMS_OR, kRbmsFault_DChgCurL2, 0 },
    [ 12] = { EMS_DISC_RBMS_OR, kRbmsFault_DChgCurL1, 0 },
    [ 13] = { EMS_DISC_RBMS_OR, kRbmsFault_HvIsLowL3, 0 },
    [ 14] = { EMS_DISC_RBMS_OR, kRbmsFault_HvIsLowL2, 0 },
    [ 15] = { EMS_DISC_RBMS_OR, kRbmsFault_HvIsLowL1, 0 },
    [ 16] = { EMS_DISC_RBMS_OR, kRbmsFault_CellVHighL3, 0 },
    [ 17] = { EMS_DISC_RBMS_OR, kRbmsFault_CellVHighL2, 0 },
    [ 18] = { EMS_DISC_RBMS_OR, kRbmsFault_CellVHighL1, 0 },
    [ 19] = { EMS_DISC_RBMS_OR, kRbmsFault_CellVLowL3, 0 },
    [ 20] = { EMS_DISC_RBMS_OR, kRbmsFault_CellVLowL2, 0 },
    [ 21] = { EMS_DISC_RBMS_OR, kRbmsFault_CellVLowL1, 0 },
    [ 22] = { EMS_DISC_RBMS_OR, kRbmsFault_CellDeltaVL3, 0 },
    [ 23] = { EMS_DISC_RBMS_OR, kRbmsFault_CellDeltaVL2, 0 },
    [ 24] = { EMS_DISC_RBMS_OR, kRbmsFault_CellDeltaVL1, 0 },
    [ 25] = { EMS_DISC_RBMS_OR, kRbmsFault_ChgUtL3, 0 },
    [ 26] = { EMS_DISC_RBMS_OR, kRbmsFault_ChgUtL2, 0 },
    [ 27] = { EMS_DISC_RBMS_OR, kRbmsFault_ChgUtL1, 0 },
    [ 28] = { EMS_DISC_RBMS_OR, kRbmsFault_ChgOtL3, 0 },
    [ 29] = { EMS_DISC_RBMS_OR, kRbmsFault_ChgOtL2, 0 },
    [ 30] = { EMS_DISC_RBMS_OR, kRbmsFault_ChgOtL1, 0 },
    [ 31] = { EMS_DISC_RBMS_OR, kRbmsFault_DsgUtL3, 0 },
    [ 32] = { EMS_DISC_RBMS_OR, kRbmsFault_DsgUtL2, 0 },
    [ 33] = { EMS_DISC_RBMS_OR, kRbmsFault_DsgUtL1, 0 },
    [ 34] = { EMS_DISC_RBMS_OR, kRbmsFault_DsgOtL3, 0 },
    [ 35] = { EMS_DISC_RBMS_OR, kRbmsFault_DsgOtL2, 0 },
    [ 36] = { EMS_DISC_RBMS_OR, kRbmsFault_DsgOtL1, 0 },
    [ 37] = { EMS_DISC_RBMS_OR, kRbmsFault_CellDeltaTL3, 0 },
    [ 38] = { EMS_DISC_RBMS_OR, kRbmsFault_CellDeltaTL2, 0 },
    [ 39] = { EMS_DISC_RBMS_OR, kRbmsFault_CellDeltaTL1, 0 },
    [ 40] = { EMS_DISC_RBMS_OR, kRbmsFault_SocLowL3, 0 },
    [ 41] = { EMS_DISC_RBMS_OR, kRbmsFault_SocLowL2, 0 },
    [ 42] = { EMS_DISC_RBMS_OR, kRbmsFault_SocLowL1, 0 },
    [ 43] = { EMS_DISC_RBMS_OR, kRbmsFault_SocHighL3, 0 },
    [ 44] = { EMS_DISC_RBMS_OR, kRbmsFault_SocHighL2, 0 },
    [ 45] = { EMS_DISC_RBMS_OR, kRbmsFault_SocHighL1, 0 },
    [ 46] = { EMS_DISC_RBMS_OR, kRbmsFault_SohLowL3, 0 },
    [ 47] = { EMS_DISC_RBMS_OR, kRbmsFault_SohLowL2, 0 },
    [ 48] = { EMS_DISC_RBMS_OR, kRbmsFault_SohLowL1, 0 },
    [ 49] = { EMS_DISC_M_FAULT, kBmsMFault_OneRackCommLost, 0 },
    [ 50] = { EMS_DISC_RBMS_OR, kRbmsFault_DaisyChnMplOff, 0 },
    [ 51] = { EMS_DISC_M_FAULT, kBmsMFault_CluVoltDiffExces, 0 },
    [ 52] = { EMS_DISC_RBMS_OR, kRbmsFault_MainPosRlyStkOpn, 0 },
    [ 53] = { EMS_DISC_RBMS_OR, kRbmsFault_MainNegRlyStkOpn, 0 },
    [ 54] = { EMS_DISC_RBMS_OR, kRbmsFault_MainPosRlyStkCls, 0 },
    [ 55] = { EMS_DISC_RBMS_OR, kRbmsFault_MainNegRlyStkCls, 0 },
    /* Modbus 56：kind=EMS_DISC_MAXFLT_LVL23，param 不用；见 ems_eval_disc_entry()
     * Excel: (BBMS_BMSMaxFltLevel==2)||(BBMS_BMSMaxFltLevel==3)，堆轻/中度告警汇总 */
    [ 56] = { EMS_DISC_MAXFLT_LVL23, 0, 0 },
    /* Modbus 57：kind=EMS_DISC_MAXFLT_LVL1，param 不用；见 ems_eval_disc_entry()
     * Excel: BBMS_BMSMaxFltLevel==1，堆重度告警汇总 */
    [ 57] = { EMS_DISC_MAXFLT_LVL1, 0, 0 },
    [ 58] = { EMS_DISC_ZERO, 0, 0 },
    [ 59] = { EMS_DISC_ZERO, 0, 0 },
    [ 60] = { EMS_DISC_ZERO, 0, 0 },
    [ 61] = { EMS_DISC_ZERO, 0, 0 },
    [ 62] = { EMS_DISC_ZERO, 0, 0 },
    [ 63] = { EMS_DISC_ZERO, 0, 0 },
    [ 64] = { EMS_DISC_ZERO, 0, 0 },
    [ 65] = { EMS_DISC_ZERO, 0, 0 },
    [ 66] = { EMS_DISC_ZERO, 0, 0 },
    [ 67] = { EMS_DISC_ZERO, 0, 0 },
    [ 68] = { EMS_DISC_ZERO, 0, 0 },
    [ 69] = { EMS_DISC_ZERO, 0, 0 },
    [ 70] = { EMS_DISC_RBMS_OR, kRbmsFault_PoleOtL3, 0 },
    [ 71] = { EMS_DISC_RBMS_OR, kRbmsFault_PoleOtL2, 0 },
    [ 72] = { EMS_DISC_RBMS_OR, kRbmsFault_PoleOtL1, 0 },
    [ 73] = { EMS_DISC_RBMS_OR, kRbmsFault_ModVHighL3, 0 },
    [ 74] = { EMS_DISC_RBMS_OR, kRbmsFault_ModVHighL2, 0 },
    [ 75] = { EMS_DISC_RBMS_OR, kRbmsFault_ModVHighL1, 0 },
    [ 76] = { EMS_DISC_RBMS_OR, kRbmsFault_ModVLowL3, 0 },
    [ 77] = { EMS_DISC_RBMS_OR, kRbmsFault_ModVLowL2, 0 },
    [ 78] = { EMS_DISC_RBMS_OR, kRbmsFault_ModVLowL1, 0 },
    [ 79] = { EMS_DISC_RBMS_OR2, kRbmsFault_AfeHardware, kRbmsFault_CellVOffLine },
    [ 80] = { EMS_DISC_RBMS_OR2, kRbmsFault_AfeHardware, kRbmsFault_CellTMux },
    [ 81] = { EMS_DISC_M_FAULT, kBmsMFault_EMSCommLost, 0 },
    [ 82] = { EMS_DISC_M_FAULT, kBmsMFault_PCSCommLost, 0 },
    [ 83] = { EMS_DISC_M_FAULT, kBmsMFault_TMSCommLost, 0 },
    [ 84] = { EMS_DISC_M_FAULT, kBmsMFault_UPSCommLost, 0 },
    [ 85] = { EMS_DISC_M_FAULT, kBmsMFault_EnergyMeterCommLost, 0 },
    [ 86] = { EMS_DISC_M_FAULT, kBmsMFault_BackupPowerSPD1Fault, 0 },
    [ 87] = { EMS_DISC_M_FAULT, kBmsMFault_CombGasAlarm, 0 },
    [ 88] = { EMS_DISC_M_FAULT, kBmsMFault_ContainerEmergStop, 0 },
    [ 89] = { EMS_DISC_M_FAULT, kBmsMFault_DistrCabEmergStop, 0 },
    [ 90] = { EMS_DISC_M_FAULT, kBmsMFault_DoorAccessFault, 0 },
    [ 91] = { EMS_DISC_M_FAULT, kBmsMFault_EmergStopFault, 0 },
    [ 92] = { EMS_DISC_M_FAULT, kBmsMFault_ExproFanAlarm, 0 },
    [ 93] = { EMS_DISC_M_FAULT, kBmsMFault_CustExtEmergStop, 0 },
    [ 94] = { EMS_DISC_M_FAULT, kBmsMFault_FireAlarmActAbn, 0 },
    [ 95] = { EMS_DISC_M_FAULT, kBmsMFault_FireModerateAlarm, 0 },
    [ 96] = { EMS_DISC_M_FAULT, kBmsMFault_FireExtremeAlarm, 0 },
    [ 97] = { EMS_DISC_M_FAULT, kBmsMFault_FireFault, 0 },
    [ 98] = { EMS_DISC_M_FAULT, kBmsMFault_IDLearnFail, 0 },
    [ 99] = { EMS_DISC_M_FAULT, kBmsMFault_PCSEmergStopFault, 0 },
    [100] = { EMS_DISC_M_FAULT, kBmsMFault_QF1CircuitBrkFault, 0 },
    [101] = { EMS_DISC_M_FAULT, kBmsMFault_CtrlCabDCSurgeFault, 0 },
    [102] = { EMS_DISC_M_FAULT, kBmsMFault_SPD2SurgeFault, 0 },
    [103] = { EMS_DISC_M_FAULT, kBmsMFault_WaterLeakAlarm, 0 },
    [104] = { EMS_DISC_M_FAULT, kBmsMFault_CluDeltaCurrOverLv3, 0 },
    [105] = { EMS_DISC_M_FAULT, kBmsMFault_CluDeltaCurrOverLv2, 0 },
    [106] = { EMS_DISC_M_FAULT, kBmsMFault_CluDeltaCurrOverLv1, 0 },
    [107] = { EMS_DISC_M_FAULT, kBmsMFault_AllCluDeltaTempOver, 0 },
    [108] = { EMS_DISC_M_FAULT, kBmsMFault_OverPower, 0 },
    [109] = { EMS_DISC_M_FAULT, kBmsMFault_RackSOCDiffExces, 0 },
    [110] = { EMS_DISC_M_FAULT, kBmsMFault_ACoreCommFault, 0 },
    [111] = { EMS_DISC_M_FAULT, kBmsMFault_AllRackCanCommLost, 0 },
    [112] = { EMS_DISC_M_FAULT, kBmsMFault_AllRackCommLost, 0 },
    [113] = { EMS_DISC_M_FAULT, kBmsMFault_BatCompDehumidCommLost, 0 },
    [114] = { EMS_DISC_M_FAULT, kBmsMFault_OneRackCanCommLost, 0 },
    [115] = { EMS_DISC_M_FAULT, kBmsMFault_MainsPowerAbnLvl1, 0 },
    [116] = { EMS_DISC_M_FAULT, kBmsMFault_12VVoltOutRange, 0 },
    [117] = { EMS_DISC_M_FAULT, kBmsMFault_24VVoltOutRange, 0 },
    [118] = { EMS_DISC_M_FAULT, kBmsMFault_WaterCoolLvl3Fault, 0 },
    [119] = { EMS_DISC_M_FAULT, kBmsMFault_WaterCoolLvl2Fault, 0 },
    [120] = { EMS_DISC_M_FAULT, kBmsMFault_WaterCoolLvl1Fault, 0 },
    [121] = { EMS_DISC_M_FAULT, kBmsMFault_WaterCoolModeMismatch, 0 },
    [122] = { EMS_DISC_M_FAULT, kBmsMFault_MainsPowerAbnLvl3, 0 },
    [123] = { EMS_DISC_ZERO, 0, 0 },
    [124] = { EMS_DISC_ZERO, 0, 0 },
    [125] = { EMS_DISC_ZERO, 0, 0 },
    [126] = { EMS_DISC_ZERO, 0, 0 },
    [127] = { EMS_DISC_ZERO, 0, 0 },
    [128] = { EMS_DISC_ZERO, 0, 0 },
    [129] = { EMS_DISC_ZERO, 0, 0 },
    [130] = { EMS_DISC_ZERO, 0, 0 },
    [131] = { EMS_DISC_ZERO, 0, 0 },
    [132] = { EMS_DISC_ZERO, 0, 0 },
    [133] = { EMS_DISC_ZERO, 0, 0 },
    [134] = { EMS_DISC_ZERO, 0, 0 },
    [135] = { EMS_DISC_ZERO, 0, 0 },
    [136] = { EMS_DISC_ZERO, 0, 0 },
    [137] = { EMS_DISC_ZERO, 0, 0 },
    [138] = { EMS_DISC_ZERO, 0, 0 },
    [139] = { EMS_DISC_ZERO, 0, 0 },
    [140] = { EMS_DISC_ZERO, 0, 0 },
    [141] = { EMS_DISC_ZERO, 0, 0 },
    [142] = { EMS_DISC_ZERO, 0, 0 },
    [143] = { EMS_DISC_ZERO, 0, 0 },
    [144] = { EMS_DISC_ZERO, 0, 0 },
    [145] = { EMS_DISC_ZERO, 0, 0 },
    [146] = { EMS_DISC_ZERO, 0, 0 },
    [147] = { EMS_DISC_ZERO, 0, 0 },
    [148] = { EMS_DISC_ZERO, 0, 0 },
    [149] = { EMS_DISC_ZERO, 0, 0 },
    [150] = { EMS_DISC_ZERO, 0, 0 },
    [151] = { EMS_DISC_ZERO, 0, 0 },
    [152] = { EMS_DISC_ZERO, 0, 0 },
    [153] = { EMS_DISC_ZERO, 0, 0 },
    [154] = { EMS_DISC_ZERO, 0, 0 },
    [155] = { EMS_DISC_ZERO, 0, 0 },
    [156] = { EMS_DISC_ZERO, 0, 0 },
    [157] = { EMS_DISC_ZERO, 0, 0 },
    [158] = { EMS_DISC_ZERO, 0, 0 },
    [159] = { EMS_DISC_ZERO, 0, 0 },
    [160] = { EMS_DISC_ZERO, 0, 0 },
    [161] = { EMS_DISC_ZERO, 0, 0 },
    [162] = { EMS_DISC_ZERO, 0, 0 },
    [163] = { EMS_DISC_ZERO, 0, 0 },
    [164] = { EMS_DISC_ZERO, 0, 0 },
    [165] = { EMS_DISC_ZERO, 0, 0 },
    [166] = { EMS_DISC_ZERO, 0, 0 },
    [167] = { EMS_DISC_ZERO, 0, 0 },
    [168] = { EMS_DISC_ZERO, 0, 0 },
    [169] = { EMS_DISC_ZERO, 0, 0 },
    [170] = { EMS_DISC_ZERO, 0, 0 },
    [171] = { EMS_DISC_ZERO, 0, 0 },
    [172] = { EMS_DISC_ZERO, 0, 0 },
    [173] = { EMS_DISC_ZERO, 0, 0 },
    [174] = { EMS_DISC_ZERO, 0, 0 },
    [175] = { EMS_DISC_ZERO, 0, 0 },
    [176] = { EMS_DISC_ZERO, 0, 0 },
    [177] = { EMS_DISC_ZERO, 0, 0 },
    [178] = { EMS_DISC_ZERO, 0, 0 },
    [179] = { EMS_DISC_ZERO, 0, 0 },
    [180] = { EMS_DISC_ZERO, 0, 0 },
    [181] = { EMS_DISC_ZERO, 0, 0 },
    [182] = { EMS_DISC_ZERO, 0, 0 },
    [183] = { EMS_DISC_ZERO, 0, 0 },
    [184] = { EMS_DISC_ZERO, 0, 0 },
    [185] = { EMS_DISC_ZERO, 0, 0 },
    [186] = { EMS_DISC_ZERO, 0, 0 },
    [187] = { EMS_DISC_ZERO, 0, 0 },
    [188] = { EMS_DISC_ZERO, 0, 0 },
    [189] = { EMS_DISC_ZERO, 0, 0 },
    [190] = { EMS_DISC_ZERO, 0, 0 },
    [191] = { EMS_DISC_ZERO, 0, 0 },
    [192] = { EMS_DISC_ZERO, 0, 0 },
    [193] = { EMS_DISC_ZERO, 0, 0 },
    [194] = { EMS_DISC_ZERO, 0, 0 },
    [195] = { EMS_DISC_ZERO, 0, 0 },
    [196] = { EMS_DISC_ZERO, 0, 0 },
    [197] = { EMS_DISC_ZERO, 0, 0 },
    [198] = { EMS_DISC_ZERO, 0, 0 },
    [199] = { EMS_DISC_ZERO, 0, 0 },
};

/* [1.3] 簇内离散模板 s_rack_disc[200]：簇 i 基址 200+200*i，簇内偏移 0~199（与 V1.0.13 一致） */
/*       由 scripts/gen_ems_rack_disc.py 从 xlsx 生成；EMS_DISC_ZERO=点表未定义或预留 */
static const ems_disc_entry_t s_rack_disc[200] = {
    [   0] = { EMS_DISC_RBMS_BIT, kRbmsFault_BankCommLost, 0 },
    [   1] = { EMS_DISC_RBMS_BIT, kRbmsFault_BatUvVL3, 0 },
    [   2] = { EMS_DISC_RBMS_BIT, kRbmsFault_BatUvVL2, 0 },
    [   3] = { EMS_DISC_RBMS_BIT, kRbmsFault_BatUvVL1, 0 },
    [   4] = { EMS_DISC_RBMS_BIT, kRbmsFault_BatOvVL3, 0 },
    [   5] = { EMS_DISC_RBMS_BIT, kRbmsFault_BatOvVL2, 0 },
    [   6] = { EMS_DISC_RBMS_BIT, kRbmsFault_BatOvVL1, 0 },
    [   7] = { EMS_DISC_RBMS_BIT, kRbmsFault_RChgCurL3, 0 },
    [   8] = { EMS_DISC_RBMS_BIT, kRbmsFault_RChgCurL2, 0 },
    [   9] = { EMS_DISC_RBMS_BIT, kRbmsFault_RChgCurL1, 0 },
    [  10] = { EMS_DISC_RBMS_BIT, kRbmsFault_DChgCurL3, 0 },
    [  11] = { EMS_DISC_RBMS_BIT, kRbmsFault_DChgCurL2, 0 },
    [  12] = { EMS_DISC_RBMS_BIT, kRbmsFault_DChgCurL1, 0 },
    [  13] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVLowL3, 0 },
    [  14] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVLowL2, 0 },
    [  15] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVLowL1, 0 },
    [  16] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVHighL3, 0 },
    [  17] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVHighL2, 0 },
    [  18] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVHighL1, 0 },
    [  19] = { EMS_DISC_RBMS_BIT, kRbmsFault_ChgUtL3, 0 },
    [  20] = { EMS_DISC_RBMS_BIT, kRbmsFault_ChgUtL2, 0 },
    [  21] = { EMS_DISC_RBMS_BIT, kRbmsFault_ChgUtL1, 0 },
    [  22] = { EMS_DISC_RBMS_BIT, kRbmsFault_ChgOtL3, 0 },
    [  23] = { EMS_DISC_RBMS_BIT, kRbmsFault_ChgOtL2, 0 },
    [  24] = { EMS_DISC_RBMS_BIT, kRbmsFault_ChgOtL1, 0 },
    [  25] = { EMS_DISC_RBMS_BIT, kRbmsFault_DsgUtL3, 0 },
    [  26] = { EMS_DISC_RBMS_BIT, kRbmsFault_DsgUtL2, 0 },
    [  27] = { EMS_DISC_RBMS_BIT, kRbmsFault_DsgUtL1, 0 },
    [  28] = { EMS_DISC_RBMS_BIT, kRbmsFault_DsgOtL3, 0 },
    [  29] = { EMS_DISC_RBMS_BIT, kRbmsFault_DsgOtL2, 0 },
    [  30] = { EMS_DISC_RBMS_BIT, kRbmsFault_DsgOtL1, 0 },
    [  31] = { EMS_DISC_RBMS_BIT, kRbmsFault_SocLowL3, 0 },
    [  32] = { EMS_DISC_RBMS_BIT, kRbmsFault_SocLowL2, 0 },
    [  33] = { EMS_DISC_RBMS_BIT, kRbmsFault_SocLowL1, 0 },
    [  34] = { EMS_DISC_RBMS_BIT, kRbmsFault_SocHighL3, 0 },
    [  35] = { EMS_DISC_RBMS_BIT, kRbmsFault_SocHighL2, 0 },
    [  36] = { EMS_DISC_RBMS_BIT, kRbmsFault_SocHighL1, 0 },
    [  37] = { EMS_DISC_RBMS_BIT, kRbmsFault_SohLowL3, 0 },
    [  38] = { EMS_DISC_RBMS_BIT, kRbmsFault_SohLowL2, 0 },
    [  39] = { EMS_DISC_RBMS_BIT, kRbmsFault_SohLowL1, 0 },
    [  40] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellDeltaVL3, 0 },
    [  41] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellDeltaVL2, 0 },
    [  42] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellDeltaVL1, 0 },
    [  43] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellDeltaTL3, 0 },
    [  44] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellDeltaTL2, 0 },
    [  45] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellDeltaTL1, 0 },
    [  46] = { EMS_DISC_RBMS_BIT, kRbmsFault_DsgExtremeOc, 0 },
    [  47] = { EMS_DISC_RBMS_BIT, kRbmsFault_ChgExtremeOc, 0 },
    [  48] = { EMS_DISC_RBMS_BIT, kRbmsFault_CurSnsrZeroDrift, 0 },
    [  49] = { EMS_DISC_RBMS_BIT, kRbmsFault_BalLoopCirc, 0 },
    [  50] = { EMS_DISC_RBMS_BIT, kRbmsFault_AfeChipOt, 0 },
    [  51] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVRedunChk, 0 },
    [  52] = { EMS_DISC_RBMS_BIT, kRbmsFault_DaisyChnCnt, 0 },
    [  53] = { EMS_DISC_RBMS_BIT, kRbmsFault_DaisyChnMplOff, 0 },
    [  54] = { EMS_DISC_RBMS_BIT, kRbmsFault_DaisyChnSglOff, 0 },
    [  55] = { EMS_DISC_RBMS_BIT, kRbmsFault_NtcPwr, 0 },
    [  56] = { EMS_DISC_RBMS_BIT, kRbmsFault_AfeHardware, 0 },
    [  57] = { EMS_DISC_RBMS_BIT, kRbmsFault_DaisyChnCommLost, 0 },
    [  58] = { EMS_DISC_RBMS_BIT, kRbmsFault_AfeSmpCrc, 0 },
    [  59] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVOffLine, 0 },
    [  60] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellTStg, 0 },
    [  61] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellTStbOrOc, 0 },
    [  62] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellTMux, 0 },
    [  63] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVOutRange, 0 },
    [  64] = { EMS_DISC_RBMS_BIT, kRbmsFault_HvSnsr, 0 },
    [  65] = { EMS_DISC_RBMS_BIT, kRbmsFault_BatVOutRange, 0 },
    [  66] = { EMS_DISC_RBMS_BIT, kRbmsFault_LinkPosVOutRange, 0 },
    [  67] = { EMS_DISC_RBMS_BIT, kRbmsFault_CurSnsrCommLost, 0 },
    [  68] = { EMS_DISC_RBMS_BIT, kRbmsFault_CurSnsrHw, 0 },
    [  69] = { EMS_DISC_RBMS_BIT, kRbmsFault_CurSnsrCrc, 0 },
    [  70] = { EMS_DISC_ZERO, 0, 0 },
    [  71] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellExtremeOvV, 0 },
    [  72] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellExtremeUvV, 0 },
    [  73] = { EMS_DISC_RBMS_BIT, kRbmsFault_HvLoopBkn, 0 },
    [  74] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellExtremeOt, 0 },
    [  75] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellTRiseFast, 0 },
    [  76] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellThermRunaway, 0 },
    [  77] = { EMS_DISC_RBMS_BIT, kRbmsFault_SlvBalTempHL1, 0 },
    [  78] = { EMS_DISC_RBMS_BIT, kRbmsFault_HvIsLowL1, 0 },
    [  79] = { EMS_DISC_RBMS_BIT, kRbmsFault_IsInvalid, 0 },
    [  80] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellSelfDisL1, 0 },
    [  81] = { EMS_DISC_RBMS_BIT, kRbmsFault_RackOverChg, 0 },
    [  82] = { EMS_DISC_RBMS_BIT, kRbmsFault_MainNegRlyStkCls, 0 },
    [  83] = { EMS_DISC_RBMS_BIT, kRbmsFault_MainNegRlyStkOpn, 0 },
    [  84] = { EMS_DISC_RBMS_BIT, kRbmsFault_MainPosRlyStkCls, 0 },
    [  85] = { EMS_DISC_RBMS_BIT, kRbmsFault_MainPosRlyStkOpn, 0 },
    [  86] = { EMS_DISC_RBMS_BIT, kRbmsFault_PoleOtL3, 0 },
    [  87] = { EMS_DISC_RBMS_BIT, kRbmsFault_PoleOtL2, 0 },
    [  88] = { EMS_DISC_RBMS_BIT, kRbmsFault_PoleOtL1, 0 },
    [  89] = { EMS_DISC_RBMS_BIT, kRbmsFault_ModVHighL3, 0 },
    [  90] = { EMS_DISC_RBMS_BIT, kRbmsFault_ModVHighL2, 0 },
    [  91] = { EMS_DISC_RBMS_BIT, kRbmsFault_ModVHighL1, 0 },
    [  92] = { EMS_DISC_RBMS_BIT, kRbmsFault_ModVLowL3, 0 },
    [  93] = { EMS_DISC_RBMS_BIT, kRbmsFault_ModVLowL2, 0 },
    [  94] = { EMS_DISC_RBMS_BIT, kRbmsFault_ModVLowL1, 0 },
    [  95] = { EMS_DISC_RBMS_BIT, kRbmsFault_PrechTimeout, 0 },
    [  96] = { EMS_DISC_RBMS_BIT, kRbmsFault_PrechOc, 0 },
    [  97] = { EMS_DISC_RBMS_BIT, kRbmsFault_FireL1, 0 },
    [  98] = { EMS_DISC_RBMS_BIT, kRbmsFault_HvBoxFuse, 0 },
    [  99] = { EMS_DISC_RBMS_BIT, kRbmsFault_MsdOpen, 0 },
    [ 100] = { EMS_DISC_RBMS_BIT, kRbmsFault_DoorAccess, 0 },
    [ 101] = { EMS_DISC_RBMS_BIT, kRbmsFault_IslSwitchOpen, 0 },
    [ 102] = { EMS_DISC_RBMS_BIT, kRbmsFault_Spd1Surge, 0 },
    [ 103] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellSelfDisL3, 0 },
    [ 104] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellSelfDisL2, 0 },
    [ 105] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellPoleTempDifL1, 0 },
    [ 106] = { EMS_DISC_RBMS_BIT, kRbmsFault_HvBoxOtL3, 0 },
    [ 107] = { EMS_DISC_RBMS_BIT, kRbmsFault_HvBoxOtL2, 0 },
    [ 108] = { EMS_DISC_RBMS_BIT, kRbmsFault_HvBoxOtL1, 0 },
    [ 109] = { EMS_DISC_RBMS_BIT, kRbmsFault_PackConnOtL3, 0 },
    [ 110] = { EMS_DISC_RBMS_BIT, kRbmsFault_PackConnOtL2, 0 },
    [ 111] = { EMS_DISC_RBMS_BIT, kRbmsFault_PackConnOtL1, 0 },
    [ 112] = { EMS_DISC_RBMS_BIT, kRbmsFault_SlvBalTempHL2, 0 },
    [ 113] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellExtremeUt, 0 },
    [ 114] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellTRedunChk, 0 },
    [ 115] = { EMS_DISC_RBMS_BIT, kRbmsFault_SlvBalTStbOrOc, 0 },
    [ 116] = { EMS_DISC_RBMS_BIT, kRbmsFault_SlvBalTStg, 0 },
    [ 117] = { EMS_DISC_RBMS_BIT, kRbmsFault_ModDeltaVL3, 0 },
    [ 118] = { EMS_DISC_RBMS_BIT, kRbmsFault_ModDeltaVL2, 0 },
    [ 119] = { EMS_DISC_RBMS_BIT, kRbmsFault_ModDeltaVL1, 0 },
    [ 120] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVSumAfeMis, 0 },
    [ 121] = { EMS_DISC_RBMS_BIT, kRbmsFault_AfeVOutRange, 0 },
    [ 122] = { EMS_DISC_RBMS_BIT, kRbmsFault_MainNegRlyDrvCir, 0 },
    [ 123] = { EMS_DISC_RBMS_BIT, kRbmsFault_MainPosRlyDrvCir, 0 },
    [ 124] = { EMS_DISC_RBMS_BIT, kRbmsFault_RlyDrvSmpSel, 0 },
    [ 125] = { EMS_DISC_RBMS_BIT, kRbmsFault_BankCrcErr, 0 },
    [ 126] = { EMS_DISC_RBMS_BIT, kRbmsFault_EmergShdnCanCnt, 0 },
    [ 127] = { EMS_DISC_RBMS_BIT, kRbmsFault_EmergShdnCanCrc, 0 },
    [ 128] = { EMS_DISC_RBMS_BIT, kRbmsFault_EmergShdnCanTo, 0 },
    [ 129] = { EMS_DISC_RBMS_BIT, kRbmsFault_CellVWithBatVMis, 0 },
    [ 130] = { EMS_DISC_RBMS_BIT, kRbmsFault_IsNegVFault, 0 },
    [ 131] = { EMS_DISC_RBMS_BIT, kRbmsFault_IsPosVFault, 0 },
    [ 132] = { EMS_DISC_RBMS_BIT, kRbmsFault_HvIsLowL3, 0 },
    [ 133] = { EMS_DISC_RBMS_BIT, kRbmsFault_HvIsLowL2, 0 },
    [ 134] = { EMS_DISC_RBMS_BIT, kRbmsFault_RbmsPwrOvV, 0 },
    [ 135] = { EMS_DISC_RBMS_BIT, kRbmsFault_RbmsPwrUvV, 0 },
    [ 136] = { EMS_DISC_RBMS_BIT, kRbmsFault_CurSnsrPwrOvV, 0 },
    [ 137] = { EMS_DISC_RBMS_BIT, kRbmsFault_CurSnsrPwrUvV, 0 },
    [ 138] = { EMS_DISC_RBMS_BIT, kRbmsFault_HvSnsrPwrUvV, 0 },
    [ 139] = { EMS_DISC_RBMS_BIT, kRbmsFault_RlyPwrOvV, 0 },
    [ 140] = { EMS_DISC_RBMS_BIT, kRbmsFault_RlyPwrUvV, 0 },
    [ 141] = { EMS_DISC_RBMS_BIT, kRbmsFault_RbmsPwrOutRange, 0 },
    [ 142] = { EMS_DISC_RBMS_BIT, kRbmsFault_CurSnsrPwrOutRange, 0 },
    [ 143] = { EMS_DISC_RBMS_BIT, kRbmsFault_RlyPwrOutRange, 0 },
    [ 144] = { EMS_DISC_RBMS_BIT, kRbmsFault_McuAdc, 0 },
    [ 145] = { EMS_DISC_RBMS_BIT, kRbmsFault_McuCan, 0 },
    [ 146] = { EMS_DISC_RBMS_BIT, kRbmsFault_McuGpio, 0 },
    [ 147] = { EMS_DISC_RBMS_BIT, kRbmsFault_McuSpi, 0 },
    [ 148] = { EMS_DISC_RBMS_BIT, kRbmsFault_McuInit, 0 },
    [ 149] = { EMS_DISC_RBMS_BIT, kRbmsFault_RtcInvalid, 0 },
    [ 150] = { EMS_DISC_RBMS_BIT, kRbmsFault_SocDiffLarge, 0 },
    [ 151] = { EMS_DISC_ZERO, 0, 0 },
    [ 152] = { EMS_DISC_ZERO, 0, 0 },
    [ 153] = { EMS_DISC_ZERO, 0, 0 },
    [ 154] = { EMS_DISC_ZERO, 0, 0 },
    [ 155] = { EMS_DISC_ZERO, 0, 0 },
    [ 156] = { EMS_DISC_ZERO, 0, 0 },
    [ 157] = { EMS_DISC_ZERO, 0, 0 },
    [ 158] = { EMS_DISC_ZERO, 0, 0 },
    [ 159] = { EMS_DISC_ZERO, 0, 0 },
    [ 160] = { EMS_DISC_ZERO, 0, 0 },
    [ 161] = { EMS_DISC_ZERO, 0, 0 },
    [ 162] = { EMS_DISC_ZERO, 0, 0 },
    [ 163] = { EMS_DISC_ZERO, 0, 0 },
    [ 164] = { EMS_DISC_ZERO, 0, 0 },
    [ 165] = { EMS_DISC_ZERO, 0, 0 },
    [ 166] = { EMS_DISC_ZERO, 0, 0 },
    [ 167] = { EMS_DISC_ZERO, 0, 0 },
    [ 168] = { EMS_DISC_ZERO, 0, 0 },
    [ 169] = { EMS_DISC_ZERO, 0, 0 },
    [ 170] = { EMS_DISC_ZERO, 0, 0 },
    [ 171] = { EMS_DISC_ZERO, 0, 0 },
    [ 172] = { EMS_DISC_ZERO, 0, 0 },
    [ 173] = { EMS_DISC_ZERO, 0, 0 },
    [ 174] = { EMS_DISC_ZERO, 0, 0 },
    [ 175] = { EMS_DISC_ZERO, 0, 0 },
    [ 176] = { EMS_DISC_ZERO, 0, 0 },
    [ 177] = { EMS_DISC_ZERO, 0, 0 },
    [ 178] = { EMS_DISC_ZERO, 0, 0 },
    [ 179] = { EMS_DISC_ZERO, 0, 0 },
    [ 180] = { EMS_DISC_ZERO, 0, 0 },
    [ 181] = { EMS_DISC_ZERO, 0, 0 },
    [ 182] = { EMS_DISC_ZERO, 0, 0 },
    [ 183] = { EMS_DISC_ZERO, 0, 0 },
    [ 184] = { EMS_DISC_ZERO, 0, 0 },
    [ 185] = { EMS_DISC_ZERO, 0, 0 },
    [ 186] = { EMS_DISC_ZERO, 0, 0 },
    [ 187] = { EMS_DISC_ZERO, 0, 0 },
    [ 188] = { EMS_DISC_ZERO, 0, 0 },
    [ 189] = { EMS_DISC_ZERO, 0, 0 },
    [ 190] = { EMS_DISC_ZERO, 0, 0 },
    [ 191] = { EMS_DISC_ZERO, 0, 0 },
    [ 192] = { EMS_DISC_ZERO, 0, 0 },
    [ 193] = { EMS_DISC_ZERO, 0, 0 },
    [ 194] = { EMS_DISC_ZERO, 0, 0 },
    [ 195] = { EMS_DISC_ZERO, 0, 0 },
    [ 196] = { EMS_DISC_ZERO, 0, 0 },
    [ 197] = { EMS_DISC_ZERO, 0, 0 },
    [ 198] = { EMS_DISC_ZERO, 0, 0 },
    [ 199] = { EMS_DISC_ZERO, 0, 0 },
};


/* [1.4] 读取当前配置簇数量（RTDB kCon_RBMSNum） */
static uint16_t ems_get_rbms_num_local(void)
{
    return (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_Con, 0, kCon_RBMSNum);
}

/* [1.5] 按点表 kind 求单个离散位：堆侧多簇 OR / 单簇 bit / M 核故障 / 最大故障等级 */
static uint8_t ems_eval_disc_entry(const ems_disc_entry_t *entry, int16_t rack_idx)
{
    if (entry == NULL || entry->kind == EMS_DISC_ZERO)
    {
        return 0;
    }

    switch (entry->kind)
    {
    case EMS_DISC_RBMS_OR:
        if (rack_idx >= 0)
        {
            return (get_rbms_fault_bit((uint8_t)rack_idx, (rbms_fault_type)entry->param1) != 0) ? 1u : 0u;
        }
        return bms_bank_rbms_fault_or(entry->param1);

    case EMS_DISC_RBMS_OR2:
        if (rack_idx >= 0)
        {
            if (get_rbms_fault_bit((uint8_t)rack_idx, (rbms_fault_type)entry->param1) != 0)
            {
                return 1;
            }
            return (get_rbms_fault_bit((uint8_t)rack_idx, (rbms_fault_type)entry->param2) != 0) ? 1u : 0u;
        }
        if (bms_bank_rbms_fault_or(entry->param1) != 0)
        {
            return 1;
        }
        return bms_bank_rbms_fault_or(entry->param2);

    case EMS_DISC_RBMS_BIT:
        if (rack_idx < 0)
        {
            return 0;
        }
        return (get_rbms_fault_bit((uint8_t)rack_idx, (rbms_fault_type)entry->param1) != 0) ? 1u : 0u;

    case EMS_DISC_M_FAULT:
        return bms_get_m_fault_status((bms_m_fault_type_e)entry->param1) ? 1u : 0u;

    /* s_bank_disc[56]：堆离散 kind 分支，读 kBbms_BMSMaxFltLevel（各簇等级取 max） */
    case EMS_DISC_MAXFLT_LVL23:
    {
        uint8_t lvl = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_BMSMaxFltLevel);
        return (lvl == 2u || lvl == 3u) ? 1u : 0u;
    }

    /* s_bank_disc[57]：堆离散 kind 分支，同上 RTDB 点，等级==1 为告警 */
    case EMS_DISC_MAXFLT_LVL1:
    {
        uint8_t lvl = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_BMSMaxFltLevel);
        return (lvl == 1u) ? 1u : 0u;
    }

    default:
        return 0;
    }
}

/* [1.6] 离散地址路由：1~199 查堆表；≥200 换算簇号与簇内偏移后查簇表 */
static uint8_t ems_map_read_discrete_bit(uint16_t modbus_addr)
{
    if (modbus_addr >= EMS_MODBUS_BANK_DISCRETE_START && modbus_addr <= EMS_MODBUS_BANK_DISCRETE_END)
    {
        return ems_eval_disc_entry(&s_bank_disc[modbus_addr], -1);
    }

    if (modbus_addr >= EMS_MODBUS_RACK_DISCRETE_BASE)
    {
        uint16_t rel = (uint16_t)(modbus_addr - EMS_MODBUS_RACK_DISCRETE_BASE);
        uint16_t rack_idx = (uint16_t)(rel / EMS_MODBUS_RACK_DISCRETE_STRIDE);
        uint16_t local_off = (uint16_t)(rel % EMS_MODBUS_RACK_DISCRETE_STRIDE);

        if (rack_idx >= ems_get_rbms_num_local())
        {
            return 0;
        }
        return ems_eval_disc_entry(&s_rack_disc[local_off], (int16_t)rack_idx);
    }

    return 0;
}

/* ========================================================================== */
/* [2] 输入寄存器 Input Register — 功能码 0x04（只读，16 位字）                   */
/*     堆(Bank)：Modbus 地址 1~99 → kBbms_*（app_bms_statistics 写 RTDB）        */
/* ========================================================================== */

/* [2.1] 堆输入映射表：Modbus 地址 → RTDB 点号 + 分辨率/偏移（与 Excel 一致） */

typedef struct
{
    uint16_t modbus_addr;
    uint16_t point_id; /* devBBMSPoint_e；0xFFFF = 预留，固定回 0 */
    float resolution;
    float offset;
} ems_bank_input_map_t;

static const ems_bank_input_map_t s_bank_input_map[] = {
    {1, kBbms_DCBusV, 0.5f, 0.0f},
    {2, kBbms_TotalDCCur, 0.125f, -4000.0f},
    {3, kBbms_SOC, 0.5f, 0.0f},
    {4, kBbms_SOH, 1.0f, 0.0f},
    {5, kBbms_MaxCellV, 1.0f, 0.0f},
    {6, kBbms_MaxVCellRackNr, 1.0f, 0.0f},
    {7, kBbms_MaxVCellNr, 1.0f, 0.0f},
    {8, kBbms_MinCellV, 1.0f, 0.0f},
    {9, kBbms_MinVCellRackNr, 1.0f, 0.0f},
    {10, kBbms_MinVCellNr, 1.0f, 0.0f},
    {11, kBbms_MaxCellT, 0.1f, -40.0f},
    {12, kBbms_MaxTCellRackNr, 1.0f, 0.0f},
    {13, kBbms_MaxTCellNr, 1.0f, 0.0f},
    {14, kBbms_MinCellT, 0.1f, -40.0f},
    {15, kBbms_MinTCellRackNr, 1.0f, 0.0f},
    {16, kBbms_MinTCellNr, 1.0f, 0.0f},
    {17, kBbms_AccuChrgE, 1.0f, 0.0f},
    {19, kBbms_AccuDisChrgE, 1.0f, 0.0f},
    {21, kBbms_LstChEkWh, 1.0f, 0.0f},
    {22, kBbms_LstDchEkWh, 1.0f, 0.0f},
    {23, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {24, kBbms_MaxChrgE, 0.125f, 0.0f},
    {25, kBbms_MaxDisChrgE, 0.125f, 0.0f},
    {26, kBbms_MaxContDisChrgPower, 0.125f, 0.0f},
    {27, kBbms_MaxContChrgPower, 0.125f, 0.0f},
    {28, kBbms_MaxContDisChrgCur, 0.125f, 0.0f},
    {29, kBbms_MaxContChrgCur, 0.125f, 0.0f},
    {30, kBbms_AvgCellT, 0.1f, -40.0f},
    {31, kBbms_BatSt, 1.0f, 0.0f},
    {32, kBbms_ChSt, 1.0f, 0.0f},
    {33, kBbms_RackEnaNbr, 1.0f, 0.0f},
    {34, kBbms_RackOnlineNbr, 1.0f, 0.0f},
    {35, kBbms_ConFail, 1.0f, 0.0f},
    {36, kBbms_StrCtrlHb, 1.0f, 0.0f},
    {37, kBbms_RackFltLvl1Nbr, 1.0f, 0.0f},
    {38, 0xFFFFu, 1.0f, 0.0f}, /* 系统下高压指令，保持寄存器侧处理，输入段预留 */
    {39, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {40, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {41, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {42, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {43, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {44, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {45, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {46, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {47, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {48, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {49, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {50, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {51, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {52, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {53, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {54, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {55, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {56, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {57, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {58, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {59, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {60, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {61, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {62, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {63, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {64, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {65, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {66, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {67, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {68, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {69, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {70, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {71, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {72, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {73, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {74, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {75, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {76, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {77, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {78, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {79, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {80, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {81, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {82, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {83, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {84, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {85, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {86, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {87, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {88, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {89, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {90, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {91, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {92, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {93, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {94, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {95, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {96, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {97, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {98, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
    {99, 0xFFFFu, 1.0f, 0.0f}, /* 预留 */
};

/* [2.2] 物理量 → Modbus uint16：raw = (physical - offset) / resolution */
static uint16_t ems_encode_u16(double physical, float resolution, float offset)
{
    double raw;

    if (resolution <= 0.0f)
    {
        resolution = 1.0f;
    }
    raw = (physical - (double)offset) / (double)resolution;
    if (raw < 0.0)
    {
        raw = 0.0;
    }
    if (raw > 65535.0)
    {
        raw = 65535.0;
    }
    return (uint16_t)(raw + 0.5);
}

/* [2.3] 在堆输入表中按 Modbus 地址查找条目 */
static int ems_find_bank_input_map(uint16_t addr, const ems_bank_input_map_t **out)
{
    for (size_t i = 0; i < sizeof(s_bank_input_map) / sizeof(s_bank_input_map[0]); i++)
    {
        if (s_bank_input_map[i].modbus_addr == addr)
        {
            *out = &s_bank_input_map[i];
            return 0;
        }
    }
    return -1;
}

/* [2.4] 读单个堆输入寄存器：查表 → RTDB → 编码为 uint16 */
static int ems_read_bank_input_reg(uint16_t addr, uint16_t *out)
{
    const ems_bank_input_map_t *item = NULL;
    double physical = 0.0;

    if (out == NULL)
    {
        return -1;
    }

    if (ems_find_bank_input_map(addr, &item) != 0)
    {
        /* 表内未定义的 1~99 地址：按点表预留处理，回 0 */
        *out = 0;
        return 0;
    }

    if (item->point_id == 0xFFFFu)
    {
        *out = 0;
        return 0;
    }

    physical = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, item->point_id);
    *out = ems_encode_u16(physical, item->resolution, item->offset);
    return 0;
}

/* ========================================================================== */
/* [2R] 输入寄存器 Input Register — 功能码 0x04（只读，16 位字）                  */
/*     簇(Rack)：Modbus 基址 100+3000*n，簇内 local 0~2999 → kRbms_*             */
/*     概要/调试标量见 s_rack_input_map；电芯电压/温度等为数组区（见下方宏）       */
/* ========================================================================== */

#define EMS_RACK_LOCAL_CELLV_START         101u
#define EMS_RACK_LOCAL_CELLV_COUNT         700u
#define EMS_RACK_LOCAL_CELLT_START         801u
#define EMS_RACK_LOCAL_CELLT_COUNT         700u
#define EMS_RACK_LOCAL_POLET_START         1501u
#define EMS_RACK_LOCAL_POLET_COUNT         160u
#define EMS_RACK_LOCAL_PACKT_START         1661u
#define EMS_RACK_LOCAL_PACKT_COUNT         40u
#define EMS_RACK_LOCAL_RESERVED1_START     45u    /* 对应 Excel: i*3000+145 ~ +200 */
#define EMS_RACK_LOCAL_RESERVED1_END       100u
#define EMS_RACK_LOCAL_RESERVED_START      1701u
#define EMS_RACK_LOCAL_RESERVED_END        2100u
#define EMS_RACK_LOCAL_RTDB_CELLV_MAX      416u
#define EMS_RACK_LOCAL_RTDB_CELLT_MAX      416u
#define EMS_RACK_LOCAL_RTDB_POLET_MAX      128u
#define EMS_RACK_LOCAL_RTDB_PACKT_MAX      16u

typedef struct
{
    uint16_t local_off;
    uint16_t point_id;
    float resolution;
    float offset;
} ems_rack_input_map_t;

/* local_off = modbus_addr - (EMS_MODBUS_RACK_INPUT_BASE + EMS_MODBUS_RACK_INPUT_STRIDE * rack_idx)
 * 例如 Excel 地址 i*3000+201+j，在代码里对应 local_off=101+j（i 为簇号0基）。 */
static const ems_rack_input_map_t s_rack_input_map[] = {
    {0, kRbms_ChaSt, 1.0f, 0.0f},
    {1, kRbms_BMSMaxFltLevel, 1.0f, 0.0f},
    {2, kRbms_SoFPulseChrgPwr, 0.125f, 0.0f},
    {3, kRbms_SoFPulseDisChrgPwr, 0.125f, 0.0f},
    {4, kRbms_SoFContChrgCurr, 0.125f, 0.0f},
    {5, kRbms_SoFContDisChrgCurr, 0.125f, 0.0f},
    {6, kRbms_DCBusV, 0.5f, 0.0f},
    {7, kRbms_A, 0.125f, -2000.0f},
    {8, kRbms_SoC, 0.5f, 0.0f},
    {9, kRbms_SoH, 1.0f, 0.0f},
    {10, kRbms_IsoR, 1.0f, 0.0f},
    {11, kRbms_CellVAvg, 1.0f, 0.0f},
    {12, kRbms_ModTmpAvg, 0.1f, -40.0f},
    {13, kRbms_CellVMax, 1.0f, 0.0f},
    {14, kRbms_CellVMaxPstn, 1.0f, 0.0f},
    {15, kRbms_CellVMin, 1.0f, 0.0f},
    {16, kRbms_CellVMinPstn, 1.0f, 0.0f},
    {17, kRbms_ModTmpMax, 0.1f, -40.0f},
    {18, kRbms_ModTmpMaxPstn, 1.0f, 0.0f},
    {19, kRbms_ModTmpMin, 0.1f, -40.0f},
    {20, kRbms_ModTmpMinPstn, 1.0f, 0.0f},
    {21, kRbms_SysMaxSOC, 0.01f, 0.0f},
    {22, kRbms_CellVMaxPstn, 1.0f, 0.0f},
    {23, kRbms_SysMinSOC, 0.01f, 0.0f},
    {24, kRbms_CellVMinPstn, 1.0f, 0.0f},
    {25, kRbms_MaxPoleTDegC, 0.1f, -40.0f},
    {26, kRbms_MaxPoleTIdxNbr, 1.0f, 0.0f},
    {27, kRbms_MinPoleTDegC, 0.1f, -40.0f},
    {28, kRbms_MinPoleTIdxNbr, 1.0f, 0.0f},
    {29, kRbms_MaxPoleTMdulIdxNbr, 1.0f, 0.0f},
    {30, kRbms_MinPoleTMdulIdxNbr, 1.0f, 0.0f},
    {31, kRbms_AvgPoleTDegC, 0.1f, -40.0f},
    {32, kRbms_AccuChrgkWh, 1.0f, 0.0f},
    {34, kRbms_AccuDisChrgkWh, 1.0f, 0.0f},
    {36, kRbms_LstChEkWh, 0.1f, 0.0f},
    {37, kRbms_LstDchEkWh, 0.1f, 0.0f},
    {38, kRbms_SoEC, 0.125f, 0.0f},
    {39, kRbms_SoED, 0.125f, 0.0f},
    {40, kRbms_PCSChVoltLim, 0.1f, 0.0f},
    {41, kRbms_PCSDchVoltLim, 0.1f, 0.0f},
    {42, kRbms_PCSSOP, 0.1f, 0.0f},
    {43, kRbms_PCSBMSSt, 1.0f, 0.0f},
    {44, kRbms_St, 1.0f, 0.0f},
    {2101, kRbms_ConSt, 1.0f, 0.0f},
    {2102, kRbms_ConFail, 1.0f, 0.0f},
    {2103, kRbms_CtlBoxT0DegC, 0.1f, -40.0f},
    {2104, kRbms_CtlBoxT1DegC, 0.1f, -40.0f},
    {2105, kRbms_CtlBoxT2DegC, 0.1f, -40.0f},
    {2106, kRbms_CtlBoxT3DegC, 0.1f, -40.0f},
    {2107, kRbms_RlyCtrlSeqStState, 1.0f, 0.0f},
    {2108, kRbms_IslnInhibitNbr, 1.0f, 0.0f},
    {2109, kRbms_IslnFinalBatV, 1.0f, 0.0f},
    {2110, kRbms_IslnFinalPosV, 1.0f, 0.0f},
    {2111, kRbms_IslnFinalNegV, 1.0f, 0.0f},
    {2112, kRbms_IslnMeastStState, 1.0f, 0.0f},
    {2113, kRbms_GenlVldFlg, 1.0f, 0.0f},
    /* SOX debug（Modbus 2214~2253）：Excel Mapping Signal 为空，按 RBMS_SOXdebugData1/2 与 rbms 协议表对齐 */
    {2114, kRbms_ScCSPC_FullChEnaNbr, 1.0f, 0.0f},
    {2115, kRbms_ScCSPC_FullDischEnaNbr, 1.0f, 0.0f},
    {2116, kRbms_ScHIST_HistAccuChCapAh, 0.0625f, 0.0f},
    {2118, kRbms_ScHIST_HistAccuDischCapAh, 0.0625f, 0.0f},
    {2120, kRbms_ScSGPC_LstSleepDurTiMin, 1.0f, 0.0f},
    {2122, kRbms_ScSGPC_HisSleepTiMin, 1.0f, 0.0f},
    {2124, kRbms_ScSGPC_HisRunTiMin, 1.0f, 0.0f},
    {2126, kRbms_ScHIST_CycNbr, 1.0f, 0.0f},
    {2127, kRbms_BSWSAllSOHCalIndicator, 1.0f, 0.0f},
    {2128, kRbms_BSWSAllSOHCalValPct, 1.0f, 0.0f},
    {2129, kRbms_ScBTCS_RackChrgCapmAh, 0.001f, 0.0f},
    {2131, kRbms_ScBTCS_RackDsChrgCapmAh, 0.001f, 0.0f},
    {2133, kRbms_BSWSRSOXHistInfoIndicator, 1.0f, 0.0f},
    {2134, kRbms_BSWSAllSOCCalIndicator, 1.0f, 0.0f},
    {2135, kRbms_BSWSAllSOCCalValPct, 1.0f, 0.0f},
    {2136, kRbms_ScSGPC_MaxCellVMdulTDegC, 0.1f, -40.0f},
    {2137, kRbms_ScSGPC_MinCellVMdulTDegC, 0.1f, -40.0f},
    /* 2138：Vmax/Vmin 电芯温度有效性，见 ems_read_rack_input_sox_packed() */
    {2139, kRbms_ScSOHA_RealSysSOHCPct, 0.01f, 0.0f},
    {2140, kRbms_ScSOHA_DFCLCapResultAh, 0.1f, 0.0f},
    {2141, kRbms_ScSOHA_DFCLPointSOCPct, 0.01f, 0.0f},
    {2142, kRbms_ScSOHA_DFCLPointStats, 1.0f, 0.0f},
    {2143, kRbms_ScSOHA_DFCLPointCapAh, 0.1f, -1500.0f},
    {2144, kRbms_ScSOHA_DFCLPointTimeDay, 1.0f, 0.0f},
    {2145, kRbms_ScSOHA_MFCLCapResultAh, 0.1f, 0.0f},
    {2146, kRbms_ScSOHA_MFCLTargtValueVmax, 1.0f, 0.0f},
    {2147, kRbms_ScSOHA_MFCLTargtValueVmin, 1.0f, 0.0f},
    {2148, kRbms_SaSOCA_MaxMinSOCState1, 1.0f, 0.0f},
    {2149, kRbms_SaSOCA_MaxMinSOCState2, 1.0f, 0.0f},
    {2150, kRbms_ScSOHA_RealSysCapAh_dbug, 0.1f, 0.0f},
    {2151, kRbms_ScSOCA_RealSysSOCPct_dbug, 0.01f, 0.0f},
    {2152, kRbms_ScSOCA_DispSOCState, 1.0f, 0.0f},
    /* 2153：DFCL/MFCL 计算成功标志打包，见 ems_read_rack_input_sox_packed() */
};

static int ems_decode_rack_input_addr(uint16_t modbus_addr, uint16_t *rack_idx, uint16_t *local_off)
{
    uint32_t rel;

    if (modbus_addr < EMS_MODBUS_RACK_INPUT_BASE)
    {
        return -1;
    }
    rel = (uint32_t)modbus_addr - (uint32_t)EMS_MODBUS_RACK_INPUT_BASE;
    if (rel >= (uint32_t)EMS_MODBUS_RACK_INPUT_STRIDE * (uint32_t)ems_get_rbms_num_local())
    {
        return -1;
    }
    *rack_idx = (uint16_t)(rel / EMS_MODBUS_RACK_INPUT_STRIDE);
    *local_off = (uint16_t)(rel % EMS_MODBUS_RACK_INPUT_STRIDE);
    return 0;
}

static int ems_find_rack_input_map(uint16_t local_off, const ems_rack_input_map_t **out)
{
    for (size_t i = 0; i < sizeof(s_rack_input_map) / sizeof(s_rack_input_map[0]); i++)
    {
        if (s_rack_input_map[i].local_off == local_off)
        {
            *out = &s_rack_input_map[i];
            return 0;
        }
    }
    return -1;
}

 /* 输入寄存器中的83~86行，单独计算 */
static int ems_read_rack_input_array(uint16_t rack_idx, uint16_t local_off, uint16_t *out)
{
    uint16_t j;
    uint16_t point_id;
    double physical;

    if (local_off >= EMS_RACK_LOCAL_CELLV_START &&
        local_off < (uint16_t)(EMS_RACK_LOCAL_CELLV_START + EMS_RACK_LOCAL_CELLV_COUNT))
    {
        j = (uint16_t)(local_off - EMS_RACK_LOCAL_CELLV_START);
        if (j >= EMS_RACK_LOCAL_RTDB_CELLV_MAX)
        {
            *out = 0;
            return 0;
        }
        point_id = (uint16_t)(kRbms_CellVolt_Start + j);
        physical = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_idx, point_id);
        *out = ems_encode_u16(physical, 1.0f, 0.0f);
        return 0;
    }

    if (local_off >= EMS_RACK_LOCAL_CELLT_START &&
        local_off < (uint16_t)(EMS_RACK_LOCAL_CELLT_START + EMS_RACK_LOCAL_CELLT_COUNT))
    {
        j = (uint16_t)(local_off - EMS_RACK_LOCAL_CELLT_START);
        if (j >= EMS_RACK_LOCAL_RTDB_CELLT_MAX)
        {
            *out = 0;
            return 0;
        }
        point_id = (uint16_t)(kRbms_TempInfo_Cell_Start + j);
        physical = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_idx, point_id);
        *out = ems_encode_u16(physical, 0.1f, -40.0f);
        return 0;
    }

    if (local_off >= EMS_RACK_LOCAL_POLET_START &&
        local_off < (uint16_t)(EMS_RACK_LOCAL_POLET_START + EMS_RACK_LOCAL_POLET_COUNT))
    {
        j = (uint16_t)(local_off - EMS_RACK_LOCAL_POLET_START);
        if (j >= EMS_RACK_LOCAL_RTDB_POLET_MAX)
        {
            *out = 0;
            return 0;
        }
        point_id = (uint16_t)(kRbms_TempInfo_PoleTemp_Start + j);
        physical = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_idx, point_id);
        *out = ems_encode_u16(physical, 0.1f, -40.0f);
        return 0;
    }

    if (local_off >= EMS_RACK_LOCAL_PACKT_START &&
        local_off < (uint16_t)(EMS_RACK_LOCAL_PACKT_START + EMS_RACK_LOCAL_PACKT_COUNT))
    {
        j = (uint16_t)(local_off - EMS_RACK_LOCAL_PACKT_START);
        if (j >= EMS_RACK_LOCAL_RTDB_PACKT_MAX)
        {
            *out = 0;
            return 0;
        }
        point_id = (uint16_t)(kRbms_TempInfo_PackPosNegTemp_Start + j);
        physical = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_idx, point_id);
        *out = ems_encode_u16(physical, 0.1f, -40.0f);
        return 0;
    }

    return -1;
}

/* [2R.3b] SOX debug 单寄存器对应多个 RTDB 位（与 Excel 2138/2153 一致） */
static int ems_read_rack_input_sox_packed(uint16_t rack_idx, uint16_t local_off, uint16_t *out)
{
    uint16_t packed = 0;

    if (out == NULL)
    {
        return -1;
    }

    if (local_off == 2138u)
    {
        if (getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_idx, kRbms_SbSGPC_MaxCellVMdulTVldFlg) != 0)
        {
            packed |= 1u;
        }
        if (getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_idx, kRbms_SbSGPC_MinCellVMdulTVldFlg) != 0)
        {
            packed |= 2u;
        }
        *out = packed;
        return 0;
    }

    if (local_off == 2153u)
    {
        if (getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_idx, kRbms_ScSOHA_DFCLCalSuccFlg) != 0)
        {
            packed |= 1u;
        }
        if (getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_idx, kRbms_ScSOHA_MFCLCalSuccFlg) != 0)
        {
            packed |= 2u;
        }
        *out = packed;
        return 0;
    }

    return -1;
}

static int ems_read_rack_input_reg(uint16_t rack_idx, uint16_t local_off, uint16_t *out)
{
    const ems_rack_input_map_t *item = NULL;
    double physical = 0.0;

    if (out == NULL)
    {
        return -1;
    }

    if (rack_idx >= ems_get_rbms_num_local())
    {
        return -1;
    }

    if (ems_read_rack_input_array(rack_idx, local_off, out) == 0)
    {
        return 0;
    }

    if (ems_read_rack_input_sox_packed(rack_idx, local_off, out) == 0)
    {
        return 0;
    }

    if ((local_off >= EMS_RACK_LOCAL_RESERVED1_START && local_off <= EMS_RACK_LOCAL_RESERVED1_END) ||
        (local_off >= EMS_RACK_LOCAL_RESERVED_START && local_off <= EMS_RACK_LOCAL_RESERVED_END))
    {
        *out = 0;
        return 0;
    }

    if (ems_find_rack_input_map(local_off, &item) != 0)
    {
        *out = 0;
        return 0;
    }

    physical = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_idx, item->point_id);
    *out = ems_encode_u16(physical, item->resolution, item->offset);
    return 0;
}

/* ========================================================================== */
/* [5] 外设输入寄存器 Input Register — 功能码 0x04（只读，16 位字）                 */
/*     外设段：60100~61599（美版点表）；未定义地址回 0                               */
/* ========================================================================== */

typedef enum
{
    EMS_EXT_KIND_ZERO = 0,
    EMS_EXT_KIND_RTDB,
    EMS_EXT_KIND_M_FAULT,
    EMS_EXT_KIND_LINE_AB,      /* 线电压 AB：由相电压推算 */
    EMS_EXT_KIND_LINE_CB,      /* 线电压 CB：由相电压推算 */
    EMS_EXT_KIND_LINE_AC,      /* 线电压 AC：由相电压推算 */
    EMS_EXT_KIND_NONZERO_BOOL, /* 读取 RTDB 后按 >0 编码成 0/1 */
} ems_ext_kind_e;

typedef struct
{
    uint16_t modbus_addr;
    uint8_t kind;
    uint16_t dev_type;
    uint16_t point_id;
    float resolution;
    float offset;
} ems_ext_input_map_t;

#define EMS_EXT_LINE_V_FACTOR 1.7320508f /* sqrt(3) */

static const ems_ext_input_map_t s_ext_input_map[] = {
    /* 电表 */
    {60100u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_Current_A, 1.0f, 0.0f},
    {60101u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_Current_B, 1.0f, 0.0f},
    {60102u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_Current_C, 1.0f, 0.0f},
    {60103u, EMS_EXT_KIND_LINE_AB, kDev_Type_Meter, 0u, 1.0f, 0.0f},
    {60104u, EMS_EXT_KIND_LINE_CB, kDev_Type_Meter, 0u, 1.0f, 0.0f},
    {60105u, EMS_EXT_KIND_LINE_AC, kDev_Type_Meter, 0u, 1.0f, 0.0f},
    {60106u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_Voltage_A, 1.0f, 0.0f},
    {60107u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_Voltage_B, 1.0f, 0.0f},
    {60108u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_Voltage_C, 1.0f, 0.0f},
    {60109u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_Frequency, 1.0f, 0.0f},
    {60110u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_ActivePower, 1.0f, 0.0f},
    {60111u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_ReactivePower, 1.0f, 0.0f},
    {60112u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_ApparentPower, 1.0f, 0.0f},
    {60113u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_PowerFactor, 1.0f, 0.0f},
    {60114u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_ActiveEnergyPositive, 1.0f, 0.0f},
    {60116u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_TotalReactiveEnergy, 1.0f, 0.0f},

    /* UPS */
    {60121u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_BatteryTemp, 1.0f, 0.0f},
    {60123u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_BatteryVoltage, 1.0f, 0.0f},
    {60125u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_OutputPower, 1.0f, 0.0f},
    {60127u, EMS_EXT_KIND_RTDB, kDev_Type_Meter, kMeter_ApparentPower, 1.0f, 0.0f}, /* 弱等效 */
    {60129u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_AlarmStart, 1.0f, 0.0f},         /* 告警区 */
    {60130u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_BatterySOC, 1.0f, 0.0f},         /* 备电时间弱等效 */
    {60132u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_RunningState, 1.0f, 0.0f},
    {60133u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_RunningState, 1.0f, 0.0f},
    {60134u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_BatterySOC, 1.0f, 0.0f},
    {60136u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_InputVoltage, 1.0f, 0.0f},
    {60138u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_Frequency, 1.0f, 0.0f},
    {60140u, EMS_EXT_KIND_RTDB, kDev_Type_UPS, kUPS_MainsPowerAbnFault, 1.0f, 0.0f},

    /* 液冷 / TMS */
    {60146u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_WorkingMode, 1.0f, 0.0f},
    {60147u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_OutletTemp, 1.0f, 0.0f},
    {60148u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_InletTemp, 1.0f, 0.0f},
    {60149u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_EnvTemp, 1.0f, 0.0f},
    {60150u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_InletPressure, 1.0f, 0.0f},
    {60151u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_OutletPressure, 1.0f, 0.0f},
    {60152u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_FaultCode, 1.0f, 0.0f},
    {60153u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_FaultLevel, 1.0f, 0.0f},
    {60154u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_PumpPWM, 1.0f, 0.0f},
    {60155u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_HighPressure1, 1.0f, 0.0f},
    {60156u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_LowPressure1, 1.0f, 0.0f},
    {60157u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_FanPWM, 1.0f, 0.0f},
    {60158u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_ExpvValve, 1.0f, 0.0f},
    {60159u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_CompressorVoltage1, 1.0f, 0.0f},
    {60160u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_CompressorCurrent1, 1.0f, 0.0f},
    {60161u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_CompressorRPM1, 1.0f, 0.0f},
    {60162u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_PTC_TempSwitch, 1.0f, 0.0f},
    {60163u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_DehumidifyState, 1.0f, 0.0f},
    {60164u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_TMSSetTempFeedback, 1.0f, 0.0f},
    {60165u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_RemoteMode, 1.0f, 0.0f},
    {60166u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_DeviceVersion1, 1.0f, 0.0f},
    {60167u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_DeviceVersion2, 1.0f, 0.0f},
    {60168u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_DeviceVersion3, 1.0f, 0.0f},
    {60169u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_DeviceVersion4, 1.0f, 0.0f},
    {60170u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_DeviceVersion5, 1.0f, 0.0f},
    {60171u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_SoftwareVersion1, 1.0f, 0.0f},
    {60172u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_SoftwareVersion2, 1.0f, 0.0f},
    {60173u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_SoftwareVersion3, 1.0f, 0.0f},
    {60174u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_CompressorRPM2, 1.0f, 0.0f},
    {60175u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_HighPressure2, 1.0f, 0.0f},
    {60176u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_LowPressure2, 1.0f, 0.0f},
    {60177u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_CompressorVoltage2, 1.0f, 0.0f},
    {60178u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_CompressorCurrent2, 1.0f, 0.0f},
    {60179u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_BMSRequestMode, 1.0f, 0.0f},
    {60180u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_ControlMode, 1.0f, 0.0f},
    {60181u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_SetTemp, 1.0f, 0.0f},

    /* 除湿/空调段（按等效口径） */
    {60187u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_Online, 1.0f, 0.0f},
    {60188u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_WorkingStatus, 1.0f, 0.0f},
    {60189u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_EnvHumidity, 1.0f, 0.0f},
    {60190u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_EnvTemp, 1.0f, 0.0f},
    {60191u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_TargetHumidityStart, 1.0f, 0.0f},
    {60197u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_CurrentTemp, 1.0f, 0.0f},
    {60198u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_CurrentHumidity, 1.0f, 0.0f},
    {60199u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_TargetHumidityStart, 1.0f, 0.0f},
    {60200u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_TargetHumidityStop, 1.0f, 0.0f},
    {60201u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_HeaterStatus, 1.0f, 0.0f},
    {60202u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_WorkingStatus, 1.0f, 0.0f},

    /* 消防/可燃气体（M 核故障） */
    {60208u, EMS_EXT_KIND_M_FAULT, 0u, kBmsMFault_FireFault, 1.0f, 0.0f},
    {60209u, EMS_EXT_KIND_M_FAULT, 0u, kBmsMFault_FireExtremeAlarm, 1.0f, 0.0f},
    {60210u, EMS_EXT_KIND_M_FAULT, 0u, kBmsMFault_FireModerateAlarm, 1.0f, 0.0f},
    {60211u, EMS_EXT_KIND_M_FAULT, 0u, kBmsMFault_CombGasAlarm, 1.0f, 0.0f},

    /* 精密空调段等效 */
    {60217u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_SoftwareVersion1, 1.0f, 0.0f},
    {60218u, EMS_EXT_KIND_NONZERO_BOOL, kDev_Type_Lqd, kLqd_CompressorRPM1, 1.0f, 0.0f},
    {60219u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_HeaterStatus, 1.0f, 0.0f},
    {60220u, EMS_EXT_KIND_RTDB, kDev_Type_TempSensor, kTempSensor_Temperature, 1.0f, 0.0f},
    {60221u, EMS_EXT_KIND_RTDB, kDev_Type_HumiditySensor, kHumiditySensor_Humidity, 1.0f, 0.0f},
    {60222u, EMS_EXT_KIND_NONZERO_BOOL, kDev_Type_Lqd, kLqd_ACFaultCode, 1.0f, 0.0f},
    {60223u, EMS_EXT_KIND_RTDB, kDev_Type_HumiditySensor, kHumiditySensor_Status, 1.0f, 0.0f},
    {60224u, EMS_EXT_KIND_NONZERO_BOOL, kDev_Type_Lqd, kLqd_HighPressureStatus, 1.0f, 0.0f},
    {60225u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_SetTemp, 1.0f, 0.0f},
    {60226u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_HeaterTempStart, 1.0f, 0.0f},
    {60227u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_TargetHumidityStart, 1.0f, 0.0f},
    {60228u, EMS_EXT_KIND_RTDB, kDev_Type_Lqd, kLqd_WorkingMode, 1.0f, 0.0f},
    {60229u, EMS_EXT_KIND_RTDB, kDev_Type_dehumidifier, kDehumidifier_WorkModeSet, 1.0f, 0.0f},
};

static int ems_find_ext_input_map(uint16_t addr, const ems_ext_input_map_t **out)
{
    size_t i;

    if (out == NULL)
    {
        return -1;
    }

    for (i = 0; i < sizeof(s_ext_input_map) / sizeof(s_ext_input_map[0]); i++)
    {
        if (s_ext_input_map[i].modbus_addr == addr)
        {
            *out = &s_ext_input_map[i];
            return 0;
        }
    }
    return -1;
}

static uint16_t ems_get_dev_count_by_type(uint16_t dev_type)
{
    uint16_t con_point;

    switch (dev_type)
    {
    case kDev_Type_Meter:
        con_point = kCon_MeterNum;
        break;
    case kDev_Type_UPS:
        con_point = kCon_UPSNum;
        break;
    case kDev_Type_Lqd:
        con_point = kCon_LqdNum;
        break;
    case kDev_Type_dehumidifier:
        con_point = kCon_dehumidifierNum;
        break;
    case kDev_Type_TempSensor:
        con_point = kCon_TempSensorNum;
        break;
    case kDev_Type_HumiditySensor:
        con_point = kCon_HumiditySensorNum;
        break;
    default:
        return 0u;
    }

    return (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_Con, 0, con_point);
}

static int ems_pick_dev_id(uint16_t dev_type, uint16_t *dev_id)
{
    uint16_t cnt;

    if (dev_id == NULL)
    {
        return -1;
    }

    cnt = ems_get_dev_count_by_type(dev_type);
    if (cnt == 0u)
    {
        return -1;
    }

    /* 外设 EMS 点表固定读取同类型设备 0 号实例（由现场配置保证 EMS 目标设备为 dev_id=0）。 */
    *dev_id = 0u;
    return 0;
}

static int ems_read_ext_input_reg(uint16_t addr, uint16_t *out)
{
    const ems_ext_input_map_t *item = NULL;
    uint16_t dev_id = 0u;
    double va;
    double vb;
    double vc;
    double raw;

    if (out == NULL)
    {
        return -1;
    }

    if (ems_find_ext_input_map(addr, &item) != 0)
    {
        *out = 0u;
        return 0;
    }

    switch (item->kind)
    {
    case EMS_EXT_KIND_ZERO:
        *out = 0u;
        return 0;

    case EMS_EXT_KIND_M_FAULT:
        *out = bms_get_m_fault_status((bms_m_fault_type_e)item->point_id) ? 1u : 0u;
        return 0;

    case EMS_EXT_KIND_LINE_AB:
    case EMS_EXT_KIND_LINE_CB:
    case EMS_EXT_KIND_LINE_AC:
        if (ems_pick_dev_id(kDev_Type_Meter, &dev_id) != 0)
        {
            *out = 0u;
            return 0;
        }
        va = getRtdbPointValue(Rtdb_ShMem, kDev_Type_Meter, dev_id, kMeter_Voltage_A);
        vb = getRtdbPointValue(Rtdb_ShMem, kDev_Type_Meter, dev_id, kMeter_Voltage_B);
        vc = getRtdbPointValue(Rtdb_ShMem, kDev_Type_Meter, dev_id, kMeter_Voltage_C);
        if (item->kind == EMS_EXT_KIND_LINE_AB)
        {
            raw = va * (double)EMS_EXT_LINE_V_FACTOR;
        }
        else if (item->kind == EMS_EXT_KIND_LINE_CB)
        {
            raw = vb * (double)EMS_EXT_LINE_V_FACTOR;
        }
        else
        {
            raw = vc * (double)EMS_EXT_LINE_V_FACTOR;
        }
        *out = ems_encode_u16(raw, item->resolution, item->offset);
        return 0;

    case EMS_EXT_KIND_NONZERO_BOOL:
        if (ems_pick_dev_id(item->dev_type, &dev_id) != 0)
        {
            *out = 0u;
            return 0;
        }
        *out = (getRtdbPointValue(Rtdb_ShMem, item->dev_type, dev_id, item->point_id) > 0.0) ? 1u : 0u;
        return 0;

    case EMS_EXT_KIND_RTDB:
    default:
        if (ems_pick_dev_id(item->dev_type, &dev_id) != 0)
        {
            *out = 0u;
            return 0;
        }
        raw = getRtdbPointValue(Rtdb_ShMem, item->dev_type, dev_id, item->point_id);
        *out = ems_encode_u16(raw, item->resolution, item->offset);
        return 0;
    }
}

/* [2.5] 地址范围判断（供对外 API 校验） */

static int ems_addr_in_bank_input_range(uint16_t addr)
{
    return (addr >= EMS_MODBUS_BANK_INPUT_REG_START && addr <= EMS_MODBUS_BANK_INPUT_REG_END);
}

static int ems_addr_in_rack_input_range(uint16_t addr)
{
    uint16_t rack_idx;
    uint16_t local_off;

    if (ems_decode_rack_input_addr(addr, &rack_idx, &local_off) != 0)
    {
        return 0;
    }
    return 1;
}

static int ems_addr_in_ext_input_range(uint16_t addr)
{
    return (addr >= EMS_MODBUS_EXT_DEVICE_REG_START && addr <= EMS_MODBUS_EXT_DEVICE_REG_END);
}

static int ems_rack_input_same_segment(uint16_t start_addr, uint16_t end_addr)
{
    uint16_t rack_start;
    uint16_t rack_end;
    uint16_t off_dummy;

    if (ems_decode_rack_input_addr(start_addr, &rack_start, &off_dummy) != 0)
    {
        return 0;
    }
    if (ems_decode_rack_input_addr(end_addr, &rack_end, &off_dummy) != 0)
    {
        return 0;
    }
    return (rack_start == rack_end);
}

static int ems_addr_in_hold_range(uint16_t addr)
{
    return (addr >= EMS_MODBUS_HOLD_REG_START && addr <= EMS_MODBUS_HOLD_REG_END);
}

static int ems_hold_addr_is_skipped(uint16_t addr)
{
    return (addr == EMS_MODBUS_HOLD_SKIP_FAULT_RESET || addr == EMS_MODBUS_HOLD_SKIP_EMS_POWER);
}

/* ========================================================================== */
/* [4] 保持寄存器 Holding Register — 0x03 读 / 0x06·0x10 写                     */
/*     地址 500~699；501/503 不实现；524~529 系统 RTC（kit_get/set_rtc_time）    */
/* ========================================================================== */

static int ems_hold_range_is_valid(uint16_t start_addr, uint16_t end_addr)
{
    uint16_t addr;

    if (!ems_addr_in_hold_range(start_addr) || !ems_addr_in_hold_range(end_addr))
    {
        return 0;
    }

    for (addr = start_addr; addr <= end_addr; addr++)
    {
        if (ems_hold_addr_is_skipped(addr))
        {
            return 0;
        }
    }
    return 1;
}

static int ems_hold_apply_rtc_field(uint16_t addr, uint16_t reg_value, rtc_time_t *rtc)
{
    if (rtc == NULL)
    {
        return -1;
    }

    switch (addr)
    {
    case EMS_MODBUS_HOLD_RTC_YEAR_ADDR:
        if (reg_value > 100u)
        {
            return -1;
        }
        rtc->year = (uint16_t)(reg_value + 2000u);
        break;

    case 525u:
        if (reg_value < 1u || reg_value > 12u)
        {
            return -1;
        }
        rtc->month = (uint8_t)reg_value;
        break;

    case 526u:
        if (reg_value < 1u || reg_value > 31u)
        {
            return -1;
        }
        rtc->day = (uint8_t)reg_value;
        break;

    case 527u:
        if (reg_value > 23u)
        {
            return -1;
        }
        rtc->hour = (uint8_t)reg_value;
        break;

    case 528u:
        if (reg_value > 59u)
        {
            return -1;
        }
        rtc->minute = (uint8_t)reg_value;
        break;

    case EMS_MODBUS_HOLD_RTC_SECOND_ADDR:
        if (reg_value > 59u)
        {
            return -1;
        }
        rtc->second = (uint8_t)reg_value;
        break;

    default:
        return -1;
    }

    return 0;
}

static int ems_read_hold_reg(uint16_t addr, uint16_t *out)
{
    rtc_time_t rtc;

    if (out == NULL || ems_hold_addr_is_skipped(addr) || !ems_addr_in_hold_range(addr))
    {
        return -1;
    }

    if (addr >= EMS_MODBUS_HOLD_RTC_YEAR_ADDR && addr <= EMS_MODBUS_HOLD_RTC_SECOND_ADDR)
    {
        rtc = kit_get_rtc_time();
        switch (addr)
        {
        case EMS_MODBUS_HOLD_RTC_YEAR_ADDR:
            *out = (uint16_t)(rtc.year - 2000u);
            break;
        case 525u:
            *out = (uint16_t)rtc.month;
            break;
        case 526u:
            *out = (uint16_t)rtc.day;
            break;
        case 527u:
            *out = (uint16_t)rtc.hour;
            break;
        case 528u:
            *out = (uint16_t)rtc.minute;
            break;
        case EMS_MODBUS_HOLD_RTC_SECOND_ADDR:
            *out = (uint16_t)rtc.second;
            break;
        default:
            return -1;
        }
        return 0;
    }

    /* 500~523、530~699：点表预留，读 0 */
    *out = 0;
    return 0;
}

static int ems_write_hold_reg(uint16_t addr, uint16_t value)
{
    rtc_time_t rtc;

    if (ems_hold_addr_is_skipped(addr) || !ems_addr_in_hold_range(addr))
    {
        return -1;
    }

    if (addr >= EMS_MODBUS_HOLD_RTC_YEAR_ADDR && addr <= EMS_MODBUS_HOLD_RTC_SECOND_ADDR)
    {
        rtc = kit_get_rtc_time();
        if (ems_hold_apply_rtc_field(addr, value, &rtc) != 0)
        {
            return -1;
        }
        if (kit_set_rtc_time(&rtc) != 0)
        {
            return -1;
        }
        return 0;
    }

    /* 预留寄存器：接受写入但不持久化（与点表 Reserved 一致） */
    return 0;
}

/* ========================================================================== */
/* [3] 对外 API — server.c 按功能码转调（见下方分类型注释）                         */
/* ========================================================================== */

/* ---------- 保持寄存器 Holding · 功能码 0x03 读 ---------- */

int ems_modbus_read_hold_registers(uint16_t start_addr, uint16_t reg_count, uint16_t *out_regs)
{
    uint32_t end_addr;
    uint16_t i;

    if (out_regs == NULL || reg_count == 0)
    {
        return 1;
    }

    end_addr = (uint32_t)start_addr + (uint32_t)reg_count - 1u;
    if (end_addr > 0xFFFFu)
    {
        return 1;
    }

    if (!ems_hold_range_is_valid(start_addr, (uint16_t)end_addr))
    {
        return 1;
    }

    for (i = 0; i < reg_count; i++)
    {
        if (ems_read_hold_reg((uint16_t)(start_addr + i), &out_regs[i]) != 0)
        {
            return 1;
        }
    }
    return 0;
}

/* ---------- 输入寄存器 Input · 功能码 0x04 读 ---------- */

int ems_modbus_read_input_registers(uint16_t start_addr, uint16_t reg_count, uint16_t *out_regs)
{
    uint32_t end_addr;
    uint16_t i;
    uint16_t rack_idx;
    uint16_t local_off;

    if (out_regs == NULL || reg_count == 0)
    {
        return 1;
    }

    end_addr = (uint32_t)start_addr + (uint32_t)reg_count - 1u;
    if (end_addr > 0xFFFFu)
    {
        return 1;
    }

    if (ems_addr_in_hold_range(start_addr) || ems_addr_in_hold_range((uint16_t)end_addr))
    {
        return 1;
    }
    if (ems_addr_in_bank_input_range(start_addr) && ems_addr_in_bank_input_range((uint16_t)end_addr))
    {
        for (i = 0; i < reg_count; i++)
        {
            if (ems_read_bank_input_reg((uint16_t)(start_addr + i), &out_regs[i]) != 0)
            {
                return 1;
            }
        }
        return 0;
    }

    if (ems_addr_in_rack_input_range(start_addr) && ems_addr_in_rack_input_range((uint16_t)end_addr) &&
        ems_rack_input_same_segment(start_addr, (uint16_t)end_addr))
    {
        for (i = 0; i < reg_count; i++)
        {
            uint16_t addr = (uint16_t)(start_addr + i);

            if (ems_decode_rack_input_addr(addr, &rack_idx, &local_off) != 0)
            {
                return 1;
            }
            if (ems_read_rack_input_reg(rack_idx, local_off, &out_regs[i]) != 0)
            {
                return 1;
            }
        }
        return 0;
    }

    if (ems_addr_in_ext_input_range(start_addr) && ems_addr_in_ext_input_range((uint16_t)end_addr))
    {
        for (i = 0; i < reg_count; i++)
        {
            if (ems_read_ext_input_reg((uint16_t)(start_addr + i), &out_regs[i]) != 0)
            {
                return 1;
            }
        }
        return 0;
    }

    return 1;
}


/* -------------------------------------------------------------------------- */
/* [1.7] 将连续离散地址按 Modbus 规则打包为字节（bit0 对应起始地址） */
/* -------------------------------------------------------------------------- */

static void ems_pack_discrete_bits(uint16_t start_addr, uint16_t bit_count, uint8_t *out_bits)
{
    uint16_t i;

    memset(out_bits, 0, (bit_count + 7u) / 8u);
    for (i = 0; i < bit_count; i++)
    {
        if (ems_map_read_discrete_bit((uint16_t)(start_addr + i)) != 0)
        {
            out_bits[i / 8u] |= (uint8_t)(1u << (i % 8u));
        }
    }
}


/* ---------- 离散输入 Discrete · 功能码 0x02 读 ---------- */

int ems_modbus_read_discrete_inputs(uint16_t start_addr, uint16_t bit_count, uint8_t *out_bits)
{
    uint32_t end_addr;
    uint16_t i;

    if (out_bits == NULL || bit_count == 0)
    {
        return 1;
    }

    end_addr = (uint32_t)start_addr + (uint32_t)bit_count - 1u;
    if (end_addr > 0xFFFFu)
    {
        return 1;
    }

    /* 合法区：堆离散 1~199，簇离散自 200 起（每簇 200 点，见寄存器分布表） */
    for (i = 0; i < bit_count; i++)
    {
        uint16_t addr = (uint16_t)(start_addr + i);

        if (addr == 0u)
        {
            return 1;
        }
        if (addr >= EMS_MODBUS_BANK_DISCRETE_START && addr <= EMS_MODBUS_BANK_DISCRETE_END)
        {
            continue;
        }
        if (addr >= 200u)
        {
            continue;
        }
        return 1;
    }

    ems_pack_discrete_bits(start_addr, bit_count, out_bits);
    return 0;
}


/* ---------- 保持寄存器 Holding · 功能码 0x06 / 0x10 写 ---------- */

int ems_modbus_write_single_hold_register(uint16_t addr, uint16_t value)
{
    if (ems_write_hold_reg(addr, value) != 0)
    {
        return 1;
    }
    return 0;
}

int ems_modbus_write_multiple_hold_registers(uint16_t start_addr, uint16_t reg_count,
                                             const uint16_t *values)
{
    uint32_t end_addr;
    uint16_t i;

    if (values == NULL || reg_count == 0)
    {
        return 1;
    }

    end_addr = (uint32_t)start_addr + (uint32_t)reg_count - 1u;
    if (end_addr > 0xFFFFu)
    {
        return 1;
    }

    if (!ems_hold_range_is_valid(start_addr, (uint16_t)end_addr))
    {
        return 1;
    }

    for (i = 0; i < reg_count; i++)
    {
        if (ems_write_hold_reg((uint16_t)(start_addr + i), values[i]) != 0)
        {
            return 1;
        }
    }
    return 0;
}
