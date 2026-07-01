/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, Gary. All rights reserved.
 * @file            app_bms_statistics.h
 * @brief           BMS数据统计
 * @author          gary
 * @date            2026-03-23
 * @remark
 *****************************************************************************/
#ifndef APP_BMS_STATISTICS_H
#define APP_BMS_STATISTICS_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include "kit_data.h"
#include "kit_log.h"
#include "bsp_rtdb.h"
#include "kit_model.h"
#include "app_bms_ctrl.h"
#include "app_bms_fault.h"
#include "bsp_rtdb.h"
#include "protocol_bms.h"
#include "logic_do.h"
#include "logic_di.h"


#define kRBMS_MAX_COUNT 20 // 最大RBMS数量

// 电池类型枚举
typedef enum
{
    kBAT_TYPE_Start = 0,
    kBAT_TYPE_LFP = kBAT_TYPE_Start, // 磷酸铁锂
    kBAT_TYPE_NCM = 1,               // 三元锂
    kBAT_TYPE_UNKNOWN = 2,           // 未知类型
    kBAT_TYPE_End                    // 电池类型枚举结束值
} battery_type_e;

typedef struct
{
    uint16_t rbms_count;                // RBMS 数量
    uint16_t rbms_soc[kRBMS_MAX_COUNT]; // RBMS 总SOC值（百分比×100）
    uint16_t soc_lower_limit;           // SOC 下界（百分比×100）
    uint16_t soc_upper_limit;           // SOC 上界（百分比×100）
    uint16_t soc_formula_numerator;     // 公式分子系数
    battery_type_e battery_type;        // 0=LFP, 1=NCM, 2=Unknown
    uint8_t temp_compensation_en;       // 温度补偿使能
    uint16_t bank_soc;                  // Bank SOC 值
} bms_soc_config_t;


//bbms 充放电状态
typedef enum
{
    kBms_ChargeStatus_Start = 0,
    kBms_ChargeStatus_NotConnected = kBms_ChargeStatus_Start, // 未连接
    kBms_ChargeStatus_FullDischarge,                          // 充电完成
    kBms_ChargeStatus_Discharge,                              // 放电中
    kBms_ChargeStatus_Charge,                                 // 充电中
    kBms_ChargeStatus_FullCharge,                             // 充电完成
    kBms_ChargeStatus_Standing,                               // 静置中
    kBms_ChargeStatus_End
} bms_charge_status_e;


//Bank工作状态
typedef enum
{
    kBms_BankStatus_Start = 0,
    kBms_BankStatus_Allocation = kBms_BankStatus_Start,        // Allocation 已分配 / 配置中
    kBms_BankStatus_NotConnected = 0x01,                       // Not Connected
    kBms_BankStatus_Reserved1 = 0x02,                          // Reserved
    kBms_BankStatus_Connected = 0x03,                          // Connected
    kBms_BankStatus_Reserved2 = 0x04,                          // Reserved
    kBms_BankStatus_SOCProtection = 0x05,                      // SOC Protection SOC 保护状态
    kBms_BankStatus_Fault = 0x63,                              // Fault  故障 / 异常
    kBms_BankStatus_End
} bms_bank_status_e;    


// Bank控制模式枚举
typedef enum
{
    kBankBmsMode_Debug = 1, // Debug
    kBankBmsMode_BMS = 2,   // BMS Control
    kBankBmsMode_EMS = 3    // EMS Control
} bank_bms_mode_e;

// 地址分配枚举
typedef enum
{
    kBankBmsAddrAlloc_Assigning = 0, // Assigniging
    kBankBmsAddrAlloc_AssignDone = 1, // AssignDone
} bank_bms_addr_alloc_e;

// Bank运行控制指令枚举
typedef enum
{
    kBankBmsOperateCmd_Default = 0,   // Default
    kBankBmsOperCmd_Connect = 1,     // Connect
    kBankBmsOperCmd_Disconnect = 2, // Disconnect
} bank_bms_ope_ctrl_cmd_e;

// 故障清除枚举
typedef enum
{
    kBankBmsFaultClear_Disable = 0, // Disable
    kBankBmsFaultClear_Enable = 1, // Enable
} bank_bms_fault_clear_e;


// Bank系统运行状态结构体
typedef struct
{
    uint8_t control_mode : 2;      // Bit0-Bit1: Bank Control Mode;Bank控制模式
    uint8_t address_allocation : 1; // Bit2: Address Allocation;地址分配
    uint8_t operate_control_cmd : 2; // Bit3-Bit4: Bank Operate Control CMD;Bank运行控制指令
    uint8_t fault_clear : 1;        // Bit5: Fault Clear;故障清除
    uint8_t reserved : 2;           // Bit6-Bit7: Reserved;预留
} bms_bank_oper_status_t;


typedef struct
{
    uint16_t rbms_count;                // RBMS 数量
    uint8_t rbms_en[kRBMS_MAX_COUNT];  //rbms使能数组
    uint16_t rbms_soh[kRBMS_MAX_COUNT]; // RBMS 总SOC值（百分比×100）
    float bank_soh;                  // Bank SOH 值
} bms_soh_config_t;

extern bbms_ctrl_t bbms_ctrl;

/*****************************************************************************
 * @brief       获取rbms数量
 *****************************************************************************/
uint16_t get_rbms_num(void);

bool get_rbms_online(uint16_t rbms_idx);

uint16_t bms_soc_statistics(bms_soc_config_t *config);
uint16_t bms_soh_statistics(bms_soh_config_t *config);

bms_charge_status_e get_bms_charge_status(void);

//sop统计
uint16_t bms_sop_statistics(void);

/*****************************************************************************
 * @brief       获取系统BMS中最高故障等级
 * @param[in]   bbms_idx: 为模拟堆预留，从1开始
 * @return      最高故障等级
 *****************************************************************************/
uint16_t bms_get_total_fault_level(uint16_t bbms_idx);
/** 堆级统计写 RTDB（EMS/上位机 Modbus 映射的数据源，建议 1s 周期调用） */
void bms_bank_statistics_update(uint16_t bbms_idx);

/** 堆离散 1~64：任一簇对应 RBMS 故障位为 1 则堆为 1（STEP 2 映射使用；参数同 kRbmsFault_*） */
uint8_t bms_bank_rbms_fault_or(uint16_t fault_id);

#endif // APP_BMS_STATISTICS_H