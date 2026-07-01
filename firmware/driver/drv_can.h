/*****************************************************************************
 * @copyright       2024-2024,AO DI LTD.
 * @file            drv_can.h
 * @brief           can底层驱动程序
 * @author          Gary
 * @date            2024/01//16
 * @remark          优化
 *****************************************************************************/
#ifndef DRV_CAN_H
#define DRV_CAN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can/error.h>
#include <sys/epoll.h>
#include "kit_data.h"
#define CAN_DEBUG 0

#define MAX_CAN_DEVICES 2

// CAN 帧结构体
typedef struct
{
    uint32_t can_id; // CAN ID
    uint8_t data[8]; // 数据内容（最大 8 字节）
    uint8_t len;     // 数据长度
} CanFrame;
typedef enum
{
    kCanIde_Std = 0, 
    kCanIde_Ext,
    kCanIde_End,     
} CanIde;

//1远程帧 0数据帧
typedef enum
{
    kCanRtr_Data = 0,
    kCanRtr_Remote,
    kCanRtr_End,
}CanRtr;

typedef enum
{
    kCanFilter_Mask,
    kCanFilter_List,
    kCanFilter_End,
}CanFilter;

typedef union
{
    uint32_t value;
    struct
    {
        uint8_t b0;
        uint8_t b1;
        uint8_t b2;
        uint8_t b3;
    }b;
}CanId;

typedef union
{
    uint8_t  byte[8];
    uint32_t value[2];
}CanData;

typedef struct
{
    CanId   id;  
    uint8_t len;
    CanIde  ide; //标示符类型
    CanRtr  rtr; //远程数据类型
    CanData data;
}CanMsg;

typedef void (*can_send_callback)(uint32_t base_time,uint8_t can);
typedef void (*can_recv_callback)(uint8_t can, CanMsg *msg);
typedef void (*can_poll_callback)(uint32_t base_time,uint8_t can);
// typedef uint8_t (*can_ota_callback)(CanFrame *, int);

// CAN 设备上下文结构体
typedef struct
{
    uint8_t dev_num;                               // 接入的设备数量
    uint8_t index;                                 // 接入的设备索引
    int socket_fd;                                 // Socket 文件描述符
    uint32_t bitrate;                              // 波特率
    const char *ifname;                            // CAN 接口名称（如 "can0"）
    can_send_callback send;                        // 数据发送回调函数指针
    can_recv_callback recv;                        // 数据接收回调函数指针
    can_poll_callback poll;                        // 数据轮询回调函数指针
    uint16_t timeout;                              // 超时时间
} CanDevice;

typedef enum
{
    kCan_125k = 0,
    kCan_250k,
    kCan_500k,
    kCan_BitrateEnd
} can_bitrate_e;

// CAN 设备序号
typedef enum
{
    kCanDevSrart = 0,
    kCanDev0 = kCanDevSrart,
    kCanDev1,
    kCanDevEnd,
} can_dev_e;

extern CanDevice can_devices[MAX_CAN_DEVICES];

/*****************************************************************************
 * @brief 打开CAN设备
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @return 成功返回0，失败返回-1
 *****************************************************************************/
int drvCanOpen(int index);

/*****************************************************************************
 * @brief 发送CAN帧
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @param frame 指向CanFrame结构体的指针，包含CAN ID、数据内容和长度
 * @return 成功返回发送的字节数，失败返回-1
 *****************************************************************************/
int drvCanSend(int index, const CanFrame *frame);

/*************************************************************************
 * @brief 发送CAN消息
 * @param index CAN设备索引
 * @param frame CAN消息指针
 **********************************************************************/
int drvCanNewSend(int index, CanMsg *frame);

/*************************************************************************
 * @brief 接收CAN帧
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @param frame 指向CanFrame结构体的指针，用于存储接收到的CAN帧信息
 * @return 成功返回接收的字节数，失败返回-1
 **************************************************************************/
int drvCanRecv(int index, CanFrame *frame);
int drvCanRecv2(int index, CanFrame *frame); // 加入select超时等待

/*************************************************************************
 * @brief 接收CAN帧，带超时机制
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @param frame 指向CanFrame结构体的指针，用于存储接收到的CAN帧信息
 * @return 成功返回接收的字节数，失败返回-1
 **************************************************************************/
int drvCanNewRecv(int index, CanMsg *frame);

/*************************************************************************
 * @brief 设置CAN接口的波特率
 * @param ifname CAN接口名称（如 "can0"）
 * @param bitrate 波特率（如 500000 表示 500 kbps）
 * @return 成功返回0，失败返回-1
 **************************************************************************/
int drvSetCanBitrate(const char *ifname, int bitrate);

/*************************************************************************
 * @brief 设置CAN接口的波特率（增强版）
 * @param ifname CAN接口名称（如 "can0"）
 * @param bitrate 波特率（如 500000 表示 500 kbps）
 * @return 成功返回0，失败返回-1
 **************************************************************************/
int drvSetCanBitrate2(const char *ifname, int bitrate);

void print_can_error(const struct can_frame *frame); // 打印CAN错误帧

/*************************************************************************
 * @brief 设置CAN设备的波特率缓冲区
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @param baud 波特率
 * @return 成功返回0，失败返回1
 **************************************************************************/
int drvSetBaudBuf(int index, int baud);

/*************************************************************************
 * @brief 关闭CAN设备
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 **************************************************************************/
void drvCanClose(int index);

//注册CAN设备的轮询回调函数
void drvCanRegisterPollCallback(int index, can_poll_callback poll);

//注册CAN设备的发送回调函数
void drvCanRegisterSendCallback(int index, can_send_callback send);

//注册CAN设备的接收回调函数
void drvCanRegisterRecvCallback(int index, can_recv_callback recv);

/**
 * @brief 执行 Linux 命令并获取输出
 * @param command 要执行的命令
 * @param output 用于存储输出的缓冲区
 * @param output_size 缓冲区大小
 * @return 0 表示成功，非 0 表示失败
 */
int execute_command(const char *command, char *output, size_t output_size);

// CAN报文保存模块函数声明
int initCanMessageSave(void);
void cleanupCanMessageSave(void);
void recordCanMessage(const CanFrame *frame, int can_index, uint8_t direction);
void manualSaveMessages(void);

// 主入口函数（推荐使用）
int startCanMessageSave(void);
void stopCanMessageSave(void);

#endif // DRV_CAN_H