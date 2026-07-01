/*****************************************************************************
 * @copyright       2024-2024, . POWER SUPPLY CO., LTD.
 * @file            drv_tcp.h
 * @brief           驱动文件
 * @author          Gary
 * @date            2025/01/03
 * @remark          初修订
 *****************************************************************************/
#ifndef DRY_TCP_H_
#define DRY_TCP_H_

#include <stdint.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include "drv_comm.h"
#include <stdio.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "kit_log.h"
#include "kit_data.h"
#include "bsp_rtdb.h"
// tcp client驱动结构体
typedef struct
{
	int socket_fd;				   // socket fd
	uint8_t netId;				   // NET口对应序号
	uint16_t uId;				   // 设备序号
	uint8_t ip[MAX_TCP_IP_LEN];	   // ip地址
	conn_status_e status;		   // 连接状态 0：离线， 1：在线
	uint32_t port;				   // 端口号
	uint16_t timeout;			   // 超时时间(milisecond)
	uint64_t last_time;			   // 上次操作时间
	uint16_t count;				   // 超时次数
	uint32_t send_len;			   // 发送报文的长度
	uint32_t recv_len;			   // 接收报文的长度
	uint32_t transaction;		   //
	uint8_t send_buf[MAX_TCP_LEN]; // 发送报文缓存
	uint8_t recv_buf[MAX_TCP_LEN]; // 接收报文缓存
} tcp_client_lib_t;

/*****************************************************************************
 * @brief       打开tcp驱动
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpOpen(tcp_client_lib_t *p_tcp);
/*****************************************************************************
 * @brief       关闭tcp驱动
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpClose(tcp_client_lib_t *p_tcp);
/*****************************************************************************
 * @brief       tcp驱动发送
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpSend(tcp_client_lib_t *p_tcp);
/*****************************************************************************
 * @brief       tcp驱动接收
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpRecv(tcp_client_lib_t *p_tcp);

#endif