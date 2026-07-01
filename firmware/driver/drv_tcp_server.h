/*****************************************************************************
 * @copyright       2024-2024,POWER SUPPLY CO., LTD.
 * @file            drv_tcp_server.h
 * @brief           驱动文件
 * @author          nikon
 * @date            2024/10/19
 * @remark          修订 GQ 2025-01-03
 *****************************************************************************/
#ifndef DRV_TCP_SERVER_H_
#define DRV_TCP_SERVER_H_

#include <stdint.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <time.h>
#include "drv_comm.h"
#include "kit_log.h"

#define MAX_SERVER_IP_LEN 32				// 服务器ip地址最大长度
#define MAX_FRAME_LEN 2048   // 假设MAX_FRAME_LEN定义为2048
#define MAX_EVENTS 1024      // epoll等待的最大事件数
#define MAX_CLIENTS 5     // 最多客户端数

#define CLIENT_TIMEOUT 30 // 客户端超时时间（30秒）自动关闭

// extern time_t ems_to_pcs_last_time[20];
// extern uint8_t ems_distrute_online_state[20];

typedef int8_t (*respondCall)(void *item, int fd);
// 连接服务端的客户端结构体
typedef struct
{
    int fd;
    struct sockaddr_in client;
    time_t last_active_time;     // 最后活动时间
} tcp_client_t;

// tcp server驱动结构体
typedef struct
{
    int epoll_fd;                  // epoll fd
    int socket_fd;                 // socket fd
    uint8_t netId;                 // NET口对应序号
    char ip[MAX_SERVER_IP_LEN];    // ip地址
    conn_status_e status;          // 连接状态 0：离线， 1：在线
    uint32_t port;                 // 端口号
    uint8_t isListen;              // 是否已监听: 0否, 1是
    tcp_client_t clients[MAX_CLIENTS];         // 连接服务端的客户端指针
    int nClient;                   // 连接服务端的客户端数量
    uint16_t timeout;              // 超时时间(milisecond)
    uint16_t count;                // 超时次数
    uint32_t send_len;             // 发送报文的长度
    uint32_t recv_len;             // 接收报文的长度
    uint8_t send_buf[MAX_FRAME_LEN]; // 发送报文缓存
    uint8_t recv_buf[MAX_FRAME_LEN]; // 接收报文缓存
    respondCall revCall;           // 增加接收及解析回调 GQ 2024/12/28
} tcp_server_lib_t;

/*****************************************************************************
 * @brief       打开tcp驱动
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpServerOpen(tcp_server_lib_t* p_tcp);
/*****************************************************************************
 * @brief       关闭tcp驱动
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpServerClose(tcp_server_lib_t* p_tcp);
/*****************************************************************************
 * @brief       tcp驱动发送
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
ssize_t drvTcpServerSend(tcp_server_lib_t* p_tcp);
/*****************************************************************************
 * @brief       tcp驱动接收
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
ssize_t drvTcpServerRecv(tcp_server_lib_t* p_tcp);


/*****************************************************************************
 * @brief       初始化接收回调 GQ 2024/12/28
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      void
 *****************************************************************************/
void drvInitServerCall(tcp_server_lib_t* p_tcp,respondCall call);

int drvTcpServerSend2(tcp_server_lib_t *p_tcp,int fd);

/*****************************************************************************
 * @brief           根据网络接口名称获取IP地址
 * @param[in]       ifname: 网络接口名称（如"eth2"）
 * @param[out]      ip_str: 输出的IP地址字符串缓冲区
 * @param[in]       ip_str_len: IP地址字符串缓冲区长度
 * @return          0-成功  1-失败
 * @note            使用getifaddrs获取网络接口信息，兼容性更好
 *****************************************************************************/
int getInterfaceIP(const char *ifname, char *ip_str, size_t ip_str_len);
#endif