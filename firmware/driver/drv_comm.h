/*****************************************************************************
 * @copyright       2024-2024,AO DI LTD. 
 * @file            drv_common.h
 * @brief           驱动公共声明文件
 * @author          Gary
 * @date            2024/12/07
 * @remark          初修订
 *****************************************************************************/

#ifndef DRV_COMMON_H
#define DRV_COMMON_H

#define MAX_TCP_IP_LEN 32         // IP地址最大长度
#define MAX_TCP_LEN 65535     // 报文最大长度
#define MAX_TCP_REV_LEN 1024  // 报文单包最大长度
#define MAX_COM_NUM 255       // 串口数量最大值
#define MAX_UART_ADDR_LEN 255 // 串口地址最大长度
#define MAX_UART_DATA_LEN 256 // 串口数据传输最大字节数
#define TCP_OPEN_SUCESS 0     // TCP打开成功
#define TCP_OPEN_FAIL 1       // TCP打开失败
#define MAX_NET_NUM 10        // 最大网口数量
#define ZLG_EM1000_NET_NUM 4  // ZLG网口数量
#define FORLINX_NET_NUM 4     // 飞凌网口数量

// 连接状态值
typedef enum conn_status 
{
    CONN_OFFLINE,
    CONN_ONLINE
} conn_status_e;

#endif