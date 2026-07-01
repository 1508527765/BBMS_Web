/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            app_cool_crtl.h
 * @brief           水冷机组控制头文件
 * @author          Gary
 * @date            2026-05-09
 * @remark
 *****************************************************************************/
#ifndef APP_COOL_CRTL_H
#define APP_COOL_CRTL_H

#include "kit_model.h"
#include "bsp_rtdb.h"
#include "bsp_parse.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdint.h>

/*****************************************
 * 热管理系统(TMS/KMS) 状态数据结构体
 * 适配协议：所有属性 + 分辨率/偏移量/无效值定义
 *****************************************/

 /* 数据解析宏（直接得到真实物理值） */
#define GET_TEMP(val)        ((int16_t)(val) - 40)        // 温度真实值 = 原始值 - 40
#define GET_CURRENT(val)     ((int32_t)(val)*0.1f - 3200) // 电流真实值 = 原始值*0.1 -3200

#define TMS_COMM_LOST_MS     10000U

/* 工作模式枚举 (增强代码可读性) */
typedef enum {
    TMS_MODE_SHUTDOWN  = 0,  // 关机模式
    TMS_MODE_COOLING   = 1,  // 制冷模式
    TMS_MODE_HEATING   = 2,  // 制热模式
    TMS_MODE_CIRCULATE = 3   // 自循环模式
} TMS_WorkingMode_e;

/* 继电器状态枚举 (0=断开,1=闭合,其他无效) */
typedef enum {
    RELAY_OFF = 0,           // 断开状态
    RELAY_ON  = 1,           // 闭合状态
    RELAY_INVALID = 2        // 无效值(>=2都无效)
} Relay_Status_e;

/* 预热模式反馈枚举 */
typedef enum {
    PREHEAT_EXIT  = 0,       // 退出预热模式
    PREHEAT_ENTER = 1,       // 进入预热模式
    PREHEAT_INVALID = 2      // 无效值(>=2都无效)
} PreHeat_Status_e;

/* 故障等级枚举 */
typedef enum {
    FAULT_NONE  = 0,         // 无故障
    FAULT_LEVEL1 = 1,        // 1级故障
    FAULT_LEVEL2 = 2,        // 2级故障
    FAULT_LEVEL3 = 3         // 3级故障
} Fault_Level_e;

/* 温度无效值定义 */
#define TEMP_INVALID_VAL  255
/* 压力无效值定义 */
#define PRESSURE_INVALID_VAL 255
/* 湿度无效值定义 */
#define HUMIDITY_INVALID_VAL 255

/* 核心状态结构体 */
typedef struct {
    uint8_t online;          // 0=离线,1=在线 (kLqd_Online)

    /* --- CANid:0x18FFC13A --- */
    TMS_WorkingMode_e working_mode;  // 工作模式
    Relay_Status_e k1_relay;        // K1继电器(压缩机急停)
    Relay_Status_e k2_relay;        // K2继电器(压缩机急停)
    PreHeat_Status_e preheat_feedback; // 预热模式反馈 
    int16_t outlet_temp;     // 出水温度(机组→电池) 温度类 (单位:℃, 255=无效, 原始值=-40~210℃)
    int16_t inlet_temp;      // 回水温度(电池→机组) 温度类 (单位:℃, 255=无效, 原始值=-40~210℃)
    int16_t env_temp;        // 环境温度 温度类 (单位:℃, 255=无效, 原始值=-40~210℃)
    float inlet_pressure;  // 进水口压力 //压力类 (单位:bar, 分辨率0.1bar, 255=无效) 
    float outlet_pressure; // 出水口压力 压力类 (单位:bar, 分辨率0.1bar, 255=无效) 
    uint8_t env_humidity;    // 除湿一体机用 环境湿度 (单位:%, 255=无效)
    uint16_t fault_code;     // 故障码(0=无故障)
    Fault_Level_e fault_level;// 故障等级

    /* --- CANid:0x18FFC13B --- */
    uint8_t acdc_a_voltage;          // ACDC_A电压 1V/bit 0~32V 255=无效
    uint8_t pump_pwm;                // 水泵PWM 1%/bit 0~100% 255=无效
    uint16_t high_pressure_1;        // 1#压缩机高压压力 1KPa/bit 65535=无效
    uint16_t low_pressure_1;         // 1#压缩机低压压力 1KPa/bit 65535=无效
    uint8_t fan_pwm;                 // 风机PWM 1%/bit 0~100% 255=无效
    uint8_t expv_valve;              // 电子膨胀阀开度 2step/bit 0~250 255=无效

    /* --- CANid:0x18FFC13C --- */
    uint8_t compressor_voltage_1;    // 1#压缩机电压 3V/bit 255=无效
    float compressor_current_1;    // 1#压缩机电流 0.5A/bit 255=无效
    uint8_t compressor_rpm_1;        // 1#压缩机转速 100rpm/bit 255=无效
    uint8_t ptc_temp_switch : 1;     // PTC温度开关 0=断开 1=闭合
    uint8_t pump_switch : 1;         // 水泵开关 0=断开 1=闭合
    uint8_t dehumidify_state : 1;    // 除湿状态 0=停止 1=开启
    uint8_t ac_fault_bit : 5;        // AC故障位(4.4~4.8)
    uint8_t ac_fault_code;           // AC故障码 00=无 01=电源 02=过压 03=欠压
    int8_t tms_set_temp_feedback;    // TMS设定温度反馈 1℃/bit 偏移-40℃ 255=无效
    uint8_t remote_mode : 1;         // 远程模式 0=本地 1=远程
    uint8_t natural_cold_valve : 1;  // 自然冷阀门 0=关闭 1=开启
    uint8_t lock_state : 1;          // 锁机状态 0=解锁 1=锁机

    /* --- CANid:0x18FFC13D --- */
    uint8_t device_version1;       // 机组版本号(5字节)
    uint8_t device_version2;       // 机组版本号(5字节)
    uint8_t device_version3;       // 机组版本号(5字节)
    uint8_t device_version4;       // 机组版本号(5字节
    uint8_t device_version5;       // 机组版本号(5字节)

    uint8_t software_version1;     // 软件版本号(3字节)
    uint8_t software_version2;     // 软件版本号(3字节)
    uint8_t software_version3;     // 软件版本号(3字节)

    /* --- CANid:0x18FFC13E --- */
    uint8_t compressor_rpm_2;        // 2#压缩机转速 100rpm/bit 255=无效
    uint16_t high_pressure_2;        // 2#高压压力 1KPa/bit 65535=无效
    uint16_t low_pressure_2;         // 2#低压压力 1KPa/bit 65535=无效
    uint8_t compressor_voltage_2;    // 2#压缩机电压 3V/bit 255=无效
    float compressor_current_2;    // 2#压缩机电流 0.5A/bit 255=无效


} TMS_Status_t;



/*****************************************
 * BMS -> TMS1 控制指令 (CAN ID: 0x18FF45F4)
 * 周期：1000ms
 * 严格按照协议位域、分辨率、偏移量定义
 *****************************************/

/* 1.2-1.1 BMS请求模式 */
typedef enum {
    BMS_REQ_SHUTDOWN  = 0,  // 关机模式
    BMS_REQ_COOLING   = 1,  // 制冷模式
    BMS_REQ_HEATING   = 2,  // 制热模式
    BMS_REQ_CIRCULATE = 3   // 自循环模式
} BMS_RequestMode_e;

/* 1.4-1.3 BMS 状态 */
typedef enum {
    BMS_STS_LOW_POWER   = 0,  // 低压上电
    BMS_STS_HIGH_POWER  = 1,  // 高压上电
    BMS_STS_FAULT       = 2,  // 严重故障状态
    BMS_STS_INVALID     = 3   // 不可信状态
} BMS_Status_e;

/* 1.6-1.5 热失控故障 */
typedef enum {
    TH_NORMAL         = 0,  // 正常
    TH_OVER_TEMP      = 1,  // 极限过温
    TH_AEROSOL_FAULT  = 2,  // 气溶胶故障
    TH_THERMAL_RUNAWAY= 3   // 热失控
} TH_Fault_e;

/* 1.7 控制模式 */
typedef enum {
    CTRL_MODE_AUTO    = 0,  // 自动模式
    CTRL_MODE_BMS     = 1   // BMS控制(根据请求模式执行)
} Ctrl_Mode_e;

/* 8.1-8.2 高压状态 */
typedef enum {
    HP_STATIC         = 0,  // 静置
    HP_DISCHARGE      = 1,  // 放电
    HP_CHARGE         = 2,  // 充电
    HP_INVALID        = 3   // 不可信状态
} HighPressure_Status_e;

/* 8.3-8.4 除雪模式 */
typedef enum {
    WEATHER_CLOSE     = 0,  // 除雪关闭
    WEATHER_LEVEL1    = 1,  // 持续开启
    WEATHER_LEVEL2    = 2,  // 60min一次，运行10min
    WEATHER_LEVEL3    = 3   // 180min一次，运行10min
} Weather_Mode_e;

/* ----------------------------
   核心：CAN 8字节 位域结构体
   严格对应协议字节/位
---------------------------- */
typedef struct {
    /* ===== 字节1 ===== */
    uint8_t bms_req_mode      : 2;  // [1.1~1.2] BMS请求模式
    uint8_t bms_status        : 2;  // [1.3~1.4] BMS状态
    uint8_t th_fault          : 2;  // [1.5~1.6] 热失控故障
    uint8_t ctrl_mode         : 1;  // [1.7] 控制模式
    uint8_t reserved1         : 1;  // [1.8] 预留

    /* ===== 字节2 ===== */
    uint8_t cell_highest_temp;      // 电芯最高温度 (-40℃偏移)

    /* ===== 字节3 ===== */
    uint8_t cell_lowest_temp;       // 电芯最低温度 (-40℃偏移)

    /* ===== 字节4 ===== */
    uint8_t cell_average_temp;      // 电芯平均温度 (-40℃偏移)

    /* ===== 字节5 ===== */
    uint8_t set_temp;               // 设定温度 (-40℃偏移)

    /* ===== 字节6~7 ===== */
    int16_t current;                // 电流(0.1A/bit, -3200A偏移)

    /* ===== 字节8 ===== */
    uint8_t hp_status         : 2;  // [8.1~8.2] 高压状态
    uint8_t weather_mode      : 2;  // [8.3~8.4] 除雪模式
    uint8_t reserved8         : 4;  // [8.5~8.8] 预留
} __attribute__((packed)) BMS_To_TMS_Cmd_t;


/*****************************************
 * BMS -> TMS Controller RTC 时间帧
 * CAN ID: 0x18FF46F4
 * 严格按协议字节位置定义
 *****************************************/

/* 时间无效值定义 */
#define RTC_YEAR_INVALID     65535U   // 年份无效值
#define RTC_MONTH_INVALID    255U     // 月份无效值
#define RTC_DAY_INVALID      255U     // 日期无效值
#define RTC_HOUR_INVALID     255U     // 小时无效值
#define RTC_MINUTE_INVALID   255U     // 分钟无效值
#define RTC_SECOND_INVALID   255U     // 秒无效值

/* ----------------------------
   8字节 严格对齐结构体
---------------------------- */
typedef struct {
    /* 字节 1~2 */  uint16_t year;    // RTC 年 (0~3000, 65535=无效)
    /* 字节 3 */   uint8_t  month;   // RTC 月 (0~12, 255=无效)
    /* 字节 4 */   uint8_t  day;     // RTC 日 (0~31, 255=无效)
    /* 字节 5 */   uint8_t  hour;    // RTC 时 (0~24, 255=无效)
    /* 字节 6 */   uint8_t  minute;  // RTC 分 (0~60, 255=无效)
    /* 字节 7 */   uint8_t  second;  // RTC 秒 (0~60, 255=无效)
    /* 字节 8 */   uint8_t  reserved;// 预留（补齐8字节）
} __attribute__((packed)) BMS_To_TMS_RTC_t;



//tms温度控制目标输出结构
typedef struct{
    BMS_RequestMode_e req_mode; // BMS请求模式
    Ctrl_Mode_e ctrl_mode; // 控制模式
    int16_t target_temp; // 目标温度 (单位:℃, 原始值)
    uint8_t weather_mode; // 除雪模式 0=除雪关闭 1=持续开启 2=60min一次，运行10min 3=180min一次，运行10min
} TMS_Target_t;




extern TMS_Target_t tms_target; // 全局TMS目标变量
extern TMS_Status_t tms_status; // 全局TMS状态变量
extern BMS_To_TMS_RTC_t tms_rtc; // 全局TMS RTC时间变量
extern BMS_To_TMS_Cmd_t tms_cmd; // 全局TMS控制指令变量


void creatTmsTempCtrlTaskEntry(uint16_t TmsNum);


#endif // APP_COOL_CRTL_H
