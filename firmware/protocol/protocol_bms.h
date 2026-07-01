/*****************************************************************************
 * @copyright       1997-2050, Gary. POWER SUPPLY CO., LTD.
 * @file            protocol_rbms.h
 * @brief           RBMS通信协议点表配置
 * @author          gary
 * @date            2026-03-27
 * @remark
 *****************************************************************************/

#ifndef PROTOCOL_BMS_H
#define PROTOCOL_BMS_H

#include "bsp_bms_com.h"
#include "bsp_rtdb.h"
#include "drv_can.h"
#include "drv_tcp_server.h"
#include "kit_data.h"
#include "kit_log.h"
#include "kit_math.h"
#include "kit_model.h"
#include <math.h>
#include <unistd.h>

//上位机和BBMS_A共同节点和子节点号
#define HMI_BBMSA_DEV_TYPE_SIGN 0x01
#define HMI_BBMSA_SUB_TYPE_SIGN 0x00

//上位机节点和子节点号
#define HMI_DEV_TYPE_SIGN 0x01
#define HMI_SUB_TYPE_SIGN 0x01

//BMS_A节点和子节点号
#define BBMSA_DEV_TYPE_SIGN 0x01
#define BBMSA_SUB_TYPE_SIGN 0x02

//BBMS_M节点和子节点号
#define BBMSM_DEV_TYPE_SIGN 0x03
#define BBMSM_SUB_TYPE_SIGN 0x01 //定死的为1，BBMS_M节点子地址为1

//RBMS节点和子节点号
#define RBMS_DEV_TYPE_SIGN 0x04
#define RBMS_SUB_TYPE_SIGN 0x00 // RBMS节点子地址范围是？

//Active_Balance节点和子节点号
#define ACTIVE_BALANCE_DEV_TYPE_SIGN 0x05
#define ACTIVE_BALANCE_SUB_TYPE_SIGN 0x00

// 功能安全相关信号, BBMS->RBMS (4 Bytes, Matrix V1.0.50 BBMS_SafetySignal)
typedef struct __attribute__((packed))
{
    uint8_t container_epo_flg : 8; // BBMS_ContainerEPOFlg 集装箱急停标志
    uint8_t rolling_counter : 8;   // BBMS_RollingCounter 滚动计数
    uint8_t checksum : 8;          // BBMS_Checksum 校验和
    uint8_t bbms_no : 4;           // BBMSNo 堆编号
    uint8_t byte4_reserved : 4;    // 预留占位 (byte4 bit4-7)
} bbms_safe_signal_ctrl_t;

extern bbms_safe_signal_ctrl_t bbms_safe_signal_ctrl;

// EMS控制指令
typedef struct __attribute__((packed))
{
    uint16_t ems_ctrl_power_cmd : 16; //0x0: 不操作   0x1: 上高压   0x2: 下高压
} bbms_ems_ctrl_t;

extern bbms_ctrl_t bbms_ctrl;

/** 组包 BBMS_CtlWord 前刷新运行时字段（心跳、堆编号等） */
void bbms_ctrl_prepare_for_send(uint16_t bbms_idx);

/** 组包 BBMS_SafetySignal 前刷新运行时字段（堆编号等） */
void bbms_safe_signal_prepare_for_send(uint16_t bbms_idx);

// 数据解析所需属性
/*
 * 点号、数据索引位、数据长度、数据类型、系数、偏移、最大值、最小值
 */
typedef struct
{
    devRBMSPoint_e pointId;
    uint16_t dataIdx;
    uint8_t dataBitLen;
    uint16_t dataStartBit;

    uint8_t dataType;
    float coeff;
    float offset;
    double maxVal;
    float minVal;
    uint16_t repeatCnt; // 重复点个数，针对数组类型
} bmsPointAttr_t;

// HMI控制指令结构体, 严格7字节, 无填充对齐
typedef struct __attribute__((packed))
{
    // Byte 1
    uint8_t HMI_AlmRst : 2;    // 故障复位
    uint8_t HMI_SetEnalso : 2; // 绝缘检测使能
    uint8_t Reserved_4_5 : 2;  // 预留
    uint8_t HMI_SetEna : 2;    // 电池串使能

    // Byte 2
    uint8_t HMI_EPORst : 2;           // EPO复位
    uint8_t HMI_EnalsoManualCtrl : 2; // 绝缘手动控制
    uint8_t HMI_SetCon : 4;           // 电池串连接控制

    // Byte 3
    uint8_t HMI_IDLearnManualCtrl; // ID手动编码

    // Byte 4
    uint8_t HMI_AutoHvUpConf; // 自动上高压设置

    // Byte 5
    uint8_t HMI_IDLearnDoCtrl; // 自动编码DO控制

    // Byte 6
    uint8_t HMI_PmtRkFltNum : 6;    // 最小允许Rack数量
    uint8_t HMI_PCSComDiagConf : 2; // PCS通讯诊断

    // Byte 7
    uint8_t HMI_LocRemCtrl : 3; // 控制模式
    uint8_t Reserved_3_7 : 5;   // 预留

} bbms_hmi_crtl_t;

// HMI TMS控制指令结构体 HMI_TMSCtrlWord
typedef struct __attribute__((packed))
{
    // Byte 1 (偏移0, Start Bit=0)：HMI_TMSManCtrlMode - 强控TMS工作模式
    uint8_t HMI_TMSManCtrlMode;

    // Byte 2 (偏移1, Start Bit=8)：HMI_TMSManCtrlTempDegC - 强控TMS工作温度(℃)
    uint8_t HMI_TMSManCtrlTempDegC;

    // Byte 3 (偏移2, Start Bit=16)：HMI_TMSManCtrlEnaFlg - 强控TMS使能信号(flg)
    uint8_t HMI_TMSManCtrlEnaFlg;

    // Byte 4 (偏移3, Start Bit=24)：TMSNo - TMS编号(Nbr)
    uint8_t TMSNo;

} bbms_hmi_tms_ctrl_t;

//HMI三级板DO控制指令（灯）共计1个字节
typedef struct __attribute__((packed))
{
    uint8_t HMI_DOManCtrlMode : 3; // 强控DO工作模式  bit0: 强控绿灯 bit1: 强控红灯 bit2: 强控黄灯
    uint8_t HMI_DOManCtrlReserved : 5; // 预留
} bbms_hmi_do_light_ctrl_t;

#include <stdint.h>

// HMI 解析获取BBMS热管理数据结构体 TMS_SumInfo
// 总长度：严格12字节，cmdGroup 0x02 CmdId 0x02
typedef struct __attribute__((packed))
{
    uint8_t BBMS_WtrTmpCmd; // Byte 1 (偏移0, Start Bit=0)：BBMS_WtrTmpCmd - 设定温度(℃)
    uint8_t BBMS_cellTmpMax; // Byte 2 (偏移1, Start Bit=8)：BBMS_cellTmpMax - 电芯最高温度(℃)
    uint8_t BBMS_cellTmpMin; // Byte 3 (偏移2, Start Bit=16)：BBMS_cellTmpMin - 电芯最低温度(℃)
    uint8_t BBMS_cellTmpAvg; // Byte 4 (偏移3, Start Bit=24)：BBMS_cellTmpAvg - 电芯平均温度(℃)
    uint8_t TMS_OutWtrTmp;  // Byte 5 (偏移4, Start Bit=32)：TMS_OutWtrTmp - 出水温度(℃)
    uint8_t TMS_BackWtrTmp; // Byte 6 (偏移5, Start Bit=40)：TMS_BackWtrTmp - 进水温度(℃)
    uint8_t TMS_EnvTmp;     // Byte 7 (偏移6, Start Bit=48)：TMS_EnvTmp - 环境温度(℃)
    uint8_t TMS_InWtrP;     // Byte 8 (偏移7, Start Bit=56)：TMS_InWtrP - 进水水压(Bar)
    uint8_t TMS_OutWtrP;    // Byte 9 (偏移8, Start Bit=64)：TMS_OutWtrP - 出水水压(Bar)
    // Byte 10 (偏移9, Start Bit=72)
    uint8_t TMS_BBMSFltLevel : 2; // TMS故障等级
    uint8_t TMS_BBMSFltCode : 6;  // TMS故障码
    // Byte 11 (偏移10, Start Bit=80)
    uint8_t BMS_TMSReqMode : 2; // BMS请求模式
    uint8_t TMS_BMSMode : 2;    // TMS工作模式
    uint8_t TMSNo : 4;          // TMS编号
    // Byte 12 (偏移11, Start Bit=88)：TMSCoolState - TMS制冷状态
    uint8_t TMSCoolState;
} bbms_tms_sum_info_t;

// TMS阈值参数结构体 (严格8字节)
typedef struct __attribute__((packed))
{
    // Byte 1 (Offset 0, Bit 0)
    uint8_t CcTHMC_EntHeatMinTemLpLmt; // 制热启动最小电芯温阈值
    // Byte 2 (Offset 1, Bit 8)
    uint8_t CcTHMC_EntHeatAvgTemLpLmt; // 制热启动平均电芯温阈值
    // Byte 3 (Offset 2, Bit 16)
    uint8_t CcTHMC_ExtHeatMinTemLowLmt; // 制热停止最小电芯温阈值
    // Byte 4 (Offset 3, Bit 24)
    uint8_t CcTHMC_ExtHeatAvgTemLowLmt; // 制热停止平均电芯温阈值
    // Byte 5 (Offset 4, Bit 32)
    uint8_t CcTHMC_EntCoolMaxTemLowLimtA; // 制冷A启动最大电芯温阈值
    // Byte 6 (Offset 5, Bit 40)
    uint8_t CcTHMC_EntCoolAvgTemLowLimtA; // 制冷A启动平均电芯温阈值
    // Byte 7 (Offset 6, Bit 48)
    uint8_t CcTHMC_ExtCoolMaxTemUpLimtA; // 制冷A停止最大电芯温阈值
    // Byte 8 (Offset 7, Bit 56)
    uint8_t CcTHMC_ExtCoolAvgTemUpLimtA; // 制冷A停止平均电芯温阈值
} bbms_para_thr_tms_t;

// HMI三级板DO控制命令结构体 HMI_BBMSDOCtrl
// 总长度：严格2字节，__attribute__((packed)) 禁止编译器自动填充/对齐
typedef struct __attribute__((packed))
{
    // Byte 1 (偏移0, Start Bit=0~7)
    uint8_t HMI_BBMSDO0CtrlFlg : 1; // DO0控制
    uint8_t HMI_BBMSDO1CtrlFlg : 1; // DO1控制
    uint8_t HMI_BBMSDO2CtrlFlg : 1; // DO2控制
    uint8_t HMI_BBMSDO3CtrlFlg : 1; // DO3控制
    uint8_t HMI_BBMSDO4CtrlFlg : 1; // DO4控制
    uint8_t HMI_BBMSDO5CtrlFlg : 1; // DO5控制
    uint8_t HMI_BBMSDO6CtrlFlg : 1; // DO6控制
    uint8_t HMI_BBMSDO7CtrlFlg : 1; // DO7控制

    // Byte 2 (偏移1, Start Bit=8~15)
    uint8_t HMI_BBMSDO8CtrlFlg : 1;  // DO8控制
    uint8_t HMI_BBMSDO9CtrlFlg : 1;  // DO9控制
    uint8_t HMI_BBMSDO10CtrlFlg : 1; // DO10控制
    uint8_t HMI_BBMSDO11CtrlFlg : 1; // DO11控制
    uint8_t Reserved_12_15 : 4;      // 预留位，补齐2字节
} bbms_hmi_bbms_do_ctrl_t;

/**
 * @brief 解析HMI三级板DO控制命令
 * @param item TCP服务器上下文
 * @param fd 文件描述符
 * @return 0 成功 -1 失败
 */
int8_t bbmsParseHmiData(tcp_server_lib_t *item, int fd);

/**
 * @brief 设置rbms开关机接口 
 * @param devId 设备ID
 * @param powerCtrlCmd 开关机命令 0: NO REQ 1: NORMAL CONNECT 2: MANUAL CONNECT 3: DISCONNECT
 */
void set_rbms_power_ctrl(uint16_t devId, uint8_t powerCtrlCmd);

// 解析RBMS接收数据
void rbmsParseRecvDataFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len);

// 解析HMI接收数据回调
int8_t bbms_uplink_recv_callback(void *arg, int fd);

// 生成BBMS上行信息
void generate_bbms_uplink_info(uint16_t devId);

// 生成BBMS故障上行信息（1000ms）
void generate_bbms_fault_uplink_info(uint16_t devId);

#endif // PROTOCOL_BMS_H
