/*****************************************************************************
 * @copyright       2024-2024,AO DI LTD.
 * @file            drv_uart.h
 * @brief           串口底层驱动程序
 * @author          Gary
 * @date            2024/12/02
 * @remark          初修订
 *****************************************************************************/

#ifndef DRY_UART_H
#define DRY_UART_H

#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include "drv_comm.h"
// 波特率
typedef enum
{
    kUart_Baud_Start = 0,
    kUart_Baud_115200 = kUart_Baud_Start,
    kUart_Baud_57600,
    kUart_Baud_38400,
    kUart_Baud_19200,
    kUart_Baud_9600,
    kUart_Baud_4800,
    kUart_Baud_2400,
    kUart_Baud_1800,
    kUart_Baud_1200,
    kUart_Baud_600,
    kUart_Baud_300,
    kUart_Baud_200,
    kUart_Baud_150,
    kUart_Baud_End
} uart_baud_e;

// 校验位
typedef enum
{
    kUart_Parity_Start = 0,
    kUart_Parity_None = kUart_Parity_Start,
    kUart_Parity_Odd,
    kUart_Parity_Even,
    kUart_Parity_Mark,
    kUart_Parity_Space,
    kUart_Parity_End
} uart_parity_e;

// 停止位
typedef enum
{
    kUart_Stop_Start = 0,
    kUart_Stop_1 = kUart_Stop_Start,
    kUart_Stop_2,
    kUart_Stop_End
} uart_stop_e;

// 数据位
typedef enum
{
    kUart_Data_Start = 0,
    kUart_Data_5 = kUart_Data_Start,
    kUart_Data_6,
    kUart_Data_7,
    kUart_Data_8,
    kUart_Data_End
} uart_data_e;

// 流控制
typedef enum
{
    kUart_Flow_Start = 0,
    kUart_Flow_Disable = kUart_Flow_Start,
    kUart_Flow_Hardware_Enable,
    kUart_Flow_Software_Enable,
    kUart_Flow_End
} uart_flow_e;

// 串口驱动结构体
typedef struct
{
    int32_t fd;                          // 串口对应的标识符
    uint8_t uartId;                      // COM口对应序号
    uint8_t address[MAX_UART_ADDR_LEN];  // 通道驱动地址
    uint16_t uId;                        // 设备序号
    conn_status_e status;                // 连接状态 0：离线， 1：在线
    uart_data_e data;                    // 数据位
    uart_stop_e stop;                    // 停止位
    uart_baud_e baud;                    // 波特率
    uart_parity_e parity;                // 校验位
    uart_flow_e flow;                    // 流控制
    uint16_t timeout;                    // 超时时间(milisecond)
    uint16_t error_cnt;                  // 超时次数
    uint32_t send_len;                   // 发送的数据长度
    uint32_t recv_len;                   // 接收的数据长度
    uint8_t send_buf[MAX_UART_DATA_LEN]; // 发送数据buffer数组
    uint8_t recv_buf[MAX_UART_DATA_LEN]; // 接收数据buffer数组
} uart_lib_t;

/*****************************************************************************
 * @brief       打开串口驱动
 * @param[in]   p_uart: 串口驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int32_t drvUartOpen(uart_lib_t* p_uart);
/*****************************************************************************
 * @brief       关闭串口驱动
 * @param[in]   p_uart: 串口驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int32_t drvUartClose(uart_lib_t* p_uart);
/*****************************************************************************
 * @brief       串口驱动发送
 * @param[in]   p_uart: 串口驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int32_t drvUartSend(uart_lib_t* p_uart);
/*****************************************************************************
 * @brief       串口驱动接收
 * @param[in]   p_uart: 串口驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int32_t drvUartRecv(uart_lib_t* p_uart);

#endif