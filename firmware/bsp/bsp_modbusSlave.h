/*****************************************************************************
 * @file            bsp_modbusSlave.h
 * @brief           上层文件
 * @author          Gary
 * @date            2025/04/19
 * @remark          初修订
 *****************************************************************************/
#ifndef BSP_MODBUS_SLAVE_H
#define BSP_MODBUS_SLAVE_H

#include <pthread.h>
#include "drv_tcp_server.h"
#include "bsp_modbus.h"

/* EMS / 上位机 Modbus TCP 从站监听端口（>1024，避免 root 权限） */
#define EMS_MODBUS_TCP_PORT 1502

// Modbus TCP帧头结构
typedef struct {
    uint16_t transaction_id;  // 2字节
    uint16_t protocol_id;     // 2字节
    uint16_t length;          // 2字节
    uint8_t unit_id;          // 1字节
    uint8_t function_code;    // 1字节
    uint8_t *data;            // 数据部分

    uint16_t reqAddr;         //请求地址
    uint16_t reqRegNum;       //请求的寄存器数量
} modbus_tcp_frame_t;

// 功能码处理回调类型
typedef int (*modbusFunchandler)(int fd, modbus_tcp_frame_t *request, tcp_server_lib_t* p_tcp);

/*****************************************************************************
 * @brief       并柜默认TCP回调
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @param[in]   call: 回调函数
 * @return      void
 *****************************************************************************/
int8_t mdTcpSlavespondCall(void* p_tcp, int fd);


void fillMdTcpConstPos(modbus_tcp_frame_t *frame, tcp_server_lib_t *p_tcp,uint8_t* data,uint8_t len);

/*****************************************************************************
 * @brief       创建任务
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      void
 *****************************************************************************/
void register_modbus_func_call(uint8_t func, modbusFunchandler Call);


/*****************************************************************************
 * @brief       创建标准的modbus从站任务
 * @param[in]   arg: tcp驱动结构体指针
 * @return      void
 *****************************************************************************/
void creatModbusTcpStdSlaveTask(void *arg);

/*****************************************************************************
 * @brief       获取网络通信状态
 * @return      通信状态
 *****************************************************************************/
 conn_status_e getNetCommStatus(void);
#endif