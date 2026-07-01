/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, Gary. All rights reserved.
 * @file            bsp_rbms_com.h
 * @brief           获取RBMS通信协议相关定义和函数
 * @author          gary
 * @date            2026-03-19
 * @remark
 *****************************************************************************/

#ifndef bsp_rbms_com_H
#define bsp_rbms_com_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "kit_log.h"
#include "kit_data.h"
#include "bsp_rtdb.h"
#include "drv_tcp_client.h"
#include "drv_tcp_server.h"
#include "kit_queue.h"





#define BMS_BROADCAST_DEV_TYPE_SIGN 0x00              // BMS广播指令ID
#define BMS_BROADCAST_SUB_DEV_TYPE_SIGN 0x00          // BMS广播指令子ID


#define RBMS_PROTOCOL_HEAD 0xA5
#define RBMS_PROTOCOL_VERSION 0x02

#define RBMS_MAX_DATA_LEN 1500
#define RBMS_MAX_PAYLOAD_LEN (RBMS_MAX_DATA_LEN - 8U) /* byte6~n 固定头 8 字节后的 data 区最大长度 */
// 接收队列缓存区个数
#define RBMS_RECV_QUEUE_ITEM_SIZE 100
#define RBMS_CTRL_QUEUE_TOTAL_SIZE 100
#define RBMS_CTRL_QUEUE_ITEM_SIZE 20

#define BBMS_INFO_QUEUE_ITEM_SIZE 100

#define RBMS_FRAME_PRE_SIZE 13 // 数据帧除去 data字段的固定前缀长度

/* 解析RBMS数据
 * RBMS通信协议定义
| Byte1 | Byte2-3          | Byte4-5 | Byte6 | Byte7  | Byte8 | Byte9   | Byte10        | Byte11  | Byte12   | Byte13 | Byte14-n |
| ----- | ---------------- | ------- | ----- | ------ | ----- | ------- | ------------- | ------- | -------- | ------ | -------- |
| Head  | Version&datalen  | CRC16   | src   | srcSub | dest  | destSub | transportType | frameId | cmdGroup | cmdId  | data     |
| 0xA5  | version=2        |         |       |        |       |         |               |         |          |        |          |
*/

/*
设备类型	主节点号	子节点号
Broadcast	0x00	0
HMI & BBMS_A	0x01	0
HMI	0x01	1
BBMS_A	0x01	2
BBMS_M	0x03	1
RBMS	0x04	N
Active_Balance	0x05	N
*/


typedef struct __attribute__((packed))
{
   uint8_t head;         // 协议头部，固定值 0xA5
   uint16_t version : 5; // 协议版本，当前协议版本为 V2
   uint16_t len : 11;    // data 总长度（byte6-n 的长度）最大值 1500
   uint16_t check;       // byte6-n CRC16-Modbus 的结果
} LinkMsg;

// 协议数据定义
typedef struct __attribute__((packed))
{
   uint8_t src;     // 源器件地址
   uint8_t srcSub;  // 源器件子地址
   uint8_t dest;    // 目的器件地址
   uint8_t destSub; // 目的器件子地址
} SrcDestMsgV2;


// 传输层报文类型定义
typedef enum
{
   kTransportType_0x01 = 0x01, // 不需要回应
   kTransportType_0x02 = 0x02, // 需要回应
   kTransportType_0x03 = 0x03, // 回应报文
} transport_type_e;


// 传输层报文定义
typedef struct __attribute__((packed))
{
   uint8_t transportType; // 报文类型 0x01:不需要回应; 0x02:需要回应; 0x03:回应报文
   uint8_t frameId;       // 帧序号，每个主动发出的帧时 +1
} TransportMsgV1;

// 表达层报文定义
typedef struct __attribute__((packed))
{
   uint8_t cmdGroup; // 命令组
   uint8_t cmdId;    // 命令号
   uint8_t data[];   // 有效负载
} ExpressionMsgV1;



// 接收回调函数指针
typedef void (*rbmsTcpMasterParseRecvFun)(dev_type_e devType, uint16_t devId, uint8_t *data, int len);
/*rbms tcp 客户端信息*/
typedef struct
{
   /*线程属性*/
   pthread_t tfd;        // 线程fd
   uint8_t exitTaskFlag; // 退出线程标记

   /*设备属性*/
   uint8_t isUsed;     // 设备是否启用
   uint8_t isOnline;   // 设备是否启用
   uint32_t last_time; // 上次操作时间

   /*设备信息*/
   dev_type_e devType; // 设备类型
   uint16_t devId;     // 设备编号

   /*协议属性*/
   tcp_client_lib_t itemTcp; // TCP协议相关配置

   /*接收回调*/
   rbmsTcpMasterParseRecvFun recvCall;

   /*发送队列*/
   kit_queue_t send_queue;
   pthread_mutex_t send_queue_mutex;

} rbms_tcp_master_t;

typedef struct
{
   int len;  // 发送数据帧长度
   char data[RBMS_MAX_DATA_LEN];  //此处是存储好的发送数据帧，用于发送至rbms服务器或者上送至上位机
} bms_queue_msg_t;

//已知data创建发送帧，依据RBMS通信协议定义
void createSendFrame(SrcDestMsgV2 *srcDestMsg, TransportMsgV1 *transportMsg, uint8_t cmdGroup, uint8_t cmdId, uint8_t *data, int len, uint8_t *frame);

bms_queue_msg_t createSendFrameQueueItem(SrcDestMsgV2 *networkMsg, TransportMsgV1 *transportMsg, uint8_t cmdGroup, uint8_t cmdId, uint8_t *data, int len);

//对外开放send_queue入队列函数
void send_queue_enqueue(bms_queue_msg_t *msg);

//对外开放 将bbms数据添加至bbms_info_queue队列，等待上位机线程处理
void bbms_info_enqueue(bms_queue_msg_t *msg);



// TCP客户端状态函数指针
typedef tcp_client_state_e (*rbmsTcpMasterTaskFun)(void *);

// 创建RBMS任务线程-tcp客户端
void creatNetRbmsTaskEntry(proto_dev_point_map_t *proto);

/**
 * @brief 根据目的地址获取对应的RBMS连接
 * @param dest    目的器件地址 (协议中的dest字段)
 * @param destSub 目的器件子地址 (协议中的destSub字段)
 * @return 返回对应的rbms_tcp_master_t指针，未找到返回NULL
 */
rbms_tcp_master_t* getRbmsByDest(uint8_t dest, uint8_t destSub);

/**
 * @brief 启动从全局send_queue分发到各RBMS的线程
 */
void startSendQueueDequeueThread(void);



/* =========================================== 与上位机通信相关 ================================= */


typedef void (*uplink_cycle_time_data)(uint16_t);


void registerUplinkCycleTimeCallback(uplink_cycle_time_data callback);
void registerUplinkFaultCycleTimeCallback(uplink_cycle_time_data callback);

/* 上位机线程上下文 */
typedef struct
{
   tcp_server_lib_t server;
   uplink_cycle_time_data uplink_cycle_callback;
   uplink_cycle_time_data uplink_fault_cycle_callback;
   pthread_t send_thread;  // 发送线程
   pthread_t recv_thread;  // 接收线程
} uplink_context_t;

/**
 * @brief 上位机消息类型
 */
typedef enum
{
   UPLINK_MSG_TYPE_DATA = 0,    // 周期性数据上报
   UPLINK_MSG_TYPE_CTRL_REPLY = 1 // 控制命令回复（高优先级）
} uplink_msg_type_e;

/**
 * @brief 上位机通信消息结构
 */
typedef struct
{
   //uplink_msg_type_e msg_type; // 消息类型
   int len;                   // 数据长度
   uint8_t data[RBMS_MAX_DATA_LEN];
} uplink_msg_t;

/**
 * @brief 向上位机发送控制回复（高优先级）
 */
void uplink_ctrl_reply_enqueue(uint8_t *data, int len);

void registerUplinkRecvCallback(respondCall recv_callback);
/**
 * @brief 向上位机发送控制回复或者读取回复的入队函数（高优先级）
 */
void uplink_bbms_ctrl_reply_enqueue(bms_queue_msg_t *msg);

/**
 * @brief 启动与上位机通信的任务
 */
void startUplinkTcpTask(const char *ip, uint16_t port);



#endif // bsp_rbms_com_H