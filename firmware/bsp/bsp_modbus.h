/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            bsp_modbus.h
 * @brief           xx功能
 * @author          Gary
 * @remark
 *****************************************************************************/
#ifndef BSP_MODBUS_H
#define BSP_MODBUS_H
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include "kit_log.h"
#include "kit_data.h"
#include "kit_math.h"
#include "drv_tcp_client.h"
#include "drv_uart.h"
#include "kit_math.h"
#include "bsp_rtdb.h"

// modbus基础功能码定义
#define FC_RD_COILS 0x01              /* 读线圈 */
#define FC_RD_DISCRETE_INPUTS 0x02    /* 读离散量输入 */
#define FC_RD_HOLDING_REGISTERS 0x03  /* 读保持寄存器 */
#define FC_RD_INPUT_REGISTERS 0x04    /* 读输入寄存器 */
#define FC_WR_SINGLE_COIL 0x05        /* 写单个线圈 */
#define FC_WR_SINGLE_REGISTER 0x06    /* 写单个寄存器 */
#define FC_RD_EXCEPTION_STATUS 0x07   /* 获取异常状态 */
#define FC_WR_MULTIPLE_COILS 0x0F     /* 写多个线圈 */
#define FC_WR_MULTIPLE_REGISTERS 0x10 /* 写多个寄存器 */
#define FC_REPORT_SLAVE_ID 0x11       /* 报告从机标识 */
#define FC_RD_PARAMETER_FILE 0x14     /* 读取参数文件 */
#define FC_WR_PARAMETER_FILE 0x15     /* 写参数文件 */

// 内部日志定义开关，不打印日志可注释该宏
//  #define DBEUG_MODBUS

typedef enum
{
    E_MODBUS_RTU,
    E_MODBUS_TCP
} modbus_type_e;

// 定义modbus状态码/错误码
typedef enum
{
    E_OK = 0x00,                   /* 合法 */
    E_ILLEGAL_FUNCTION = 0x01,     /* 不合法功能 */
    E_ILLEGAL_DATA_ADDRESS = 0x02, /* 不合法的数据地址 */
    E_ILLEGAL_DATA_VALUE = 0x03,   /* 不合法的数据值 */
    E_SLAVE_FAILURE = 0x04,        /* 从站设备故障 */
    E_ACKNOWLEDGE = 0x05,          /* 确认 */
    E_SLAVE_BUSY = 0x06,           /* 从站设备繁忙 */
    E_NEGATIVE_ACKNOWLEDGE = 0x07, /* 不确认 */
    E_MEMORY_PARITY = 0x08,        /* 存储奇偶校验出错 */
    E_NOT_DEFINED = 0x09,          /* 未定义错误 */
    E_GATEWAY_PATH = 0x0A,         /* 错误网关路径 */
    E_GATEWAY_TARGET = 0x0B,       /* 目标网关响应失败 */
} modbus_status_e;

// 用于匹配真实功能码的对应
typedef enum
{
    kModbus_Start = 0,  // 开始值 用于存储func为none的情况
    kModbus_F01,        // 读取线圈寄存器
    kModbus_F02,        // 读取离散输入寄存器
    kModbus_F03,        // 读取保持寄存器
    kModbus_F04,        // 读取输入寄存器
    kModbus_F05,        // 写入单个线圈寄存器
    kModbus_F06,        // 写单个保持寄存器
    kModbus_F0F,        // 写多个线圈寄存器
    kModbus_F10,        // 写多个寄存器
    kModbus_End         // 结束值
} modbus_func_select_e; //

extern uint8_t func_code[kModbus_End];

// 用于测点的功能码分类
typedef struct
{
    int pointNum;
    point_t *pointItem;
} mb_point_t;

// 用于功能码分组后的地址合并组帧
typedef struct
{
    int startAddr;
    int endAddr;
} mb_req_group_t;

// 记录某个功能码的地址块
typedef struct
{
    int groupNum;
    mb_req_group_t *groupItem;
} mb_group_ret_t;

#define MAX_DATA_LEN 247

// 队列元素
typedef struct
{
    uint8_t frameLen;  /* 发送报文长度 */
    uint8_t byte[256]; /* 发送报文 */
} list_item_t;

// 队列结构体
typedef struct
{
    uint16_t size;
    uint16_t listNum;
    uint16_t rear;
    uint16_t cur;
    list_item_t *listArray;
} list_t;

// modbus tcp head
typedef struct
{
    uint16_t transaction; /* Modbus事务标识符 */
    uint16_t protocol;    /* 协议标志符 */
    uint16_t bufLen;      /* 报文后部分数据长度 */
} modbus_head_t;

// 报文与报文数据相互转换
typedef struct
{
    uint16_t transaction; /* Modbus事务标识符 */
    uint16_t protocol;    /* 协议标志符 */
    uint16_t bufLen;      /* 报文后部分数据长度 */
    uint8_t devAddr;      /* 从设备地址 */
    uint8_t func;         /* 功能码 */
    uint16_t startAddr;   /* 起始地址 */
    uint16_t reqNum;      /* 请求数量 */
    uint8_t data[255];
} mbtcp_req_frame_t;

// 控制报文
typedef struct
{
    uint8_t devAddr;            /* 从设备地址 */
    uint8_t func;               /* 功能码 */
    uint16_t startAddr;         /* 起始地址 */
    uint16_t num_or_val;        /* 数量或数值 */
    uint8_t data_len;           /* (可选)数据的长度 */
    uint8_t data[MAX_DATA_LEN]; /* (可选)数据 */
} modbus_frame_t;

typedef struct
{
    uint16_t tranction;    // 事务序列号
    uint16_t protocolType; // 协议类型
    uint8_t devAddr;       /* 从设备地址 */
    uint16_t function;     // 功能吗
    uint16_t startAddress; // 功能吗
    uint16_t len;          // 字节数
    uint8_t *data;
    uint8_t errorFunction; //
} modbus_rev_frame_t;

// tcp状态机函数
typedef tcp_client_state_e (*tcpStateMacFun)(void *);

/*modbus客户端信息*/
typedef struct
{
    /*线程属性*/
    pthread_t tfd;        // 线程fd
    uint8_t exitTaskFlag; // 退出线程标记

    /*设备属性*/
    uint8_t isUsed;     // 设备是否启用
    uint8_t isOnline;   // 设备是否启用
    dev_type_e devType; // 设备类型
    uint16_t devId;     // 设备编号
    uint32_t last_time; // 上次操作时间

    /*接口属性*/
    modbus_type_e mdType; // 协议类型
    tcp_client_lib_t itemTcp;
    uart_lib_t itemUart;

    /*协议属性*/
    uint16_t mbFuncNum[kModbus_End];    // 统计每个功能码对应的测点数量
    mb_point_t mbFuncInfo[kModbus_End]; // 每个功能码对应的信息

    uint16_t mbParseAlarmNum[kModbus_End];     // 统计每个功能码需要解析bit位测点数量
    mb_point_t mbParseAlarm[kModbus_End];      // 用于存储需要解析bit位信息
    uint16_t *mbAlarmParent[kModbus_End];      // 用于存储需要解析bit位信息的父测点
    mb_group_ret_t mbFuncReqInfo[kModbus_End]; // 每个fc对应的组帧信息

    /*PDU属性*/
    uint16_t transaction;
    modbus_head_t mbHead;
    mbtcp_req_frame_t sendFrame;
    uint16_t mbReqIdx;
    uint16_t mbCmdIdx;
    list_t mbReqList;
    list_t mbCmdList;
} modbus_master_t;

// 为了适配一个串口接多种设备用
typedef struct
{
    uint16_t devNum;
    uart_lib_t itemUart;
    modbus_master_t *mbRtuMasterItem;
} modbus_rtu_master_t;

// 为了适配同一个ip和端口配多个不同id设备用
typedef struct
{
    uint16_t devNum;
    tcp_client_lib_t itemTcp;
    modbus_master_t *mbMasterItem;
} modbus_tcp_multid_master_t;

extern rtdb_type_e rtdbType;

// void initPointModbus(dev_info_t *infoItem, modbus_master_t *mbItem);

// 初始化modbus tcp 多id任务框架入口
void creatNetModbusMultTaskEntry(proto_dev_point_map_t *inputItem);

// 初始化modbus tcp 任务入口
void creatNetModbusTaskEntry(proto_dev_point_map_t *inputItem);

// 初始化modbus rtu 任务入口
void creatUartModbusTaskEntry(proto_dev_point_map_t *inputItem);
#endif // BSP_MODBUS_H