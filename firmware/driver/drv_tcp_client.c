/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2026, gary. All rights reserved.
 * @file            drv_tcp.c
 * @brief           tcp驱动文件
 * @author          Gary
 * @date            2026/02/01
 * @remark          初修订
 *****************************************************************************/
#include "drv_tcp_client.h"

/*****************************************************************************
 * @brief       判断IP地址类型
 * @param[in]   ip地址
 * @return      1: IPv4, 2: IPv6, 0: 无效地址
 *****************************************************************************/
static int is_ip(const char *ip)
{
    struct sockaddr_in sa;
    if (inet_pton(AF_INET, ip, &(sa.sin_addr)) != 0)
    {
        return 1;
    }

    struct sockaddr_in6 sa6;
    if (inet_pton(AF_INET6, ip, &(sa6.sin6_addr)) != 0)
    {
        return 2;
    }

    return 0;
}

/*****************************************************************************
 * @brief       打印发送内容
 * @param[in]   str：发送内容标题
 * @param[in]   data：发送内容
 * @param[in]   len：发送长度
 * @return      void
 *****************************************************************************/
static void tcpPrintLog(const char *str, uint8_t *data, uint16_t len)
{
    char sdata[512 * 2] = {0};
    int pos = 0;

    if ((uint8_t)getRtdbPointValue(rtdbType, kDev_Type_Con, 0, kCon_TcpLog_Enable) == 1)
    {
        if (len > 0)
        {
            for (int i = 0; i < len; i++)
            {
                pos += snprintf(sdata + pos, sizeof(sdata) - pos, "%02x ", data[i]);
                // 检查snprintf返回值，确保写入成功，并处理潜在错误
                if (pos < 0 || pos >= sizeof(sdata))
                {
                    KITPTF(LOG_DRIVER_EN, ERROR_EN, "tcpPrintLog: snprintf failed!");
                    return;
                    break; // 停止继续处理
                }
            }
            // 使用snprintf，避免缓冲区溢出
            pos += snprintf(sdata + pos, sizeof(sdata) - pos, "\n");
            // 检查snprintf返回值，确保写入成功，并处理潜在错误
            if (pos < 0 || pos >= sizeof(sdata))
            {
                KITPTF(LOG_DRIVER_EN, ERROR_EN, "tcpPrintLog: snprintf failed!");
            }
        }
        KITPTF(LOG_DRIVER_EN, INFO_EN, "%s:%s", str, sdata);
        KITLOG(LOG_DRIVER_EN, INFO_EN, "%s:%s", str, sdata);
    }
}

/*****************************************************************************
 * @brief       打开tcp驱动
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpOpen(tcp_client_lib_t *p_tcp)
{
    // 判断IP地址类型：1: IPv4, 2: IPv6, 0: 无效地址
    int ip_type = is_ip((const char *)p_tcp->ip);

    if (ip_type == 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "无效 ip: %s", p_tcp->ip);
        return 1;
    }
    p_tcp->socket_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in remote_in;
    remote_in.sin_family = AF_INET;
    remote_in.sin_port = htons(p_tcp->port);

    if (inet_pton(AF_INET, (const char *)p_tcp->ip, &remote_in.sin_addr) <= 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Invalid address or address not supported");
        return 1;
    }

    if (p_tcp->socket_fd == -1)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "socket 错误: %s(%d)\n", strerror(errno), errno);
        return 1;
    }

    int ret = connect(p_tcp->socket_fd, (struct sockaddr *)&remote_in, sizeof(struct sockaddr_in));
    if (ret == -1)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "连接错误: %s(%d)\n", strerror(errno), errno);
        KITPTF(LOG_DRIVER_EN, ERROR_EN, "连接错误: %s(%d)\n", strerror(errno), errno);
        close(p_tcp->socket_fd);
        return 1;
    }

    return 0; // 返回成功
}

/*****************************************************************************
 * @brief       关闭tcp驱动
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpClose(tcp_client_lib_t *p_tcp)
{
    if (p_tcp->socket_fd > 0)
    {
        close(p_tcp->socket_fd);
        p_tcp->socket_fd = 0;
        p_tcp->status = CONN_OFFLINE;
    }
    else
    {
        perror("Close fail!");
        return 1;
    }
    return 0;
}

/*****************************************************************************
 * @brief       tcp驱动发送
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpSend(tcp_client_lib_t *p_tcp)
{
    ssize_t ret = 0;
    fd_set writefds;
    struct timeval timeout;
    timeout.tv_sec = 3; // 3秒超时
    timeout.tv_usec = 0;

    FD_ZERO(&writefds);
    FD_SET(p_tcp->socket_fd, &writefds);

    int retn = select(p_tcp->socket_fd + 1, NULL, &writefds, NULL, &timeout);
    if (retn > 0 && FD_ISSET(p_tcp->socket_fd, &writefds))
    {
        goto sendpos;
    }
    else if (retn == 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Socket write timeout");
        return 1;
    }
    else
    {
        // 发生错误
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "IP(%s) 发送错误! ret: %ld, errno: %s(%d)\n",
               p_tcp->ip, ret, strerror(errno), errno);
        // close(p_tcp->socket_fd); // 关闭 socket   协议层面去关闭，驱动层面不做处理，否则灵敏度过高
        return 128;
    }

sendpos:
    while (ret < p_tcp->send_len)
    {
        ssize_t rc = send(p_tcp->socket_fd, p_tcp->send_buf + ret, p_tcp->send_len - ret, MSG_NOSIGNAL);
        if (rc > 0)
        {
            ret += rc;
        }
        else
        {
            // 发生错误
            if (rc < 0)
            {
                KITLOG(LOG_DRIVER_EN, ERROR_EN, "IP(%s) 发送错误! ret: %ld, errno: %s(%d)\n",
                    p_tcp->ip, ret, strerror(errno), errno);
                KITPTF(LOG_DRIVER_EN, ERROR_EN, "IP(%s) 发送错误! ret: %ld, errno: %s(%d)\n",
                    p_tcp->ip, ret, strerror(errno), errno);    
            }
            return 1;
        }
    }
    tcpPrintLog("发送数据:", p_tcp->send_buf, p_tcp->send_len);
    return 0;
}

/*****************************************************************************
 * @brief       tcp驱动接收
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpRecv(tcp_client_lib_t *p_tcp)
{

    if(p_tcp->timeout == 0)
    {
        p_tcp->timeout = 1000;
    }
    // 超时时间检测
    if (0 != p_tcp->timeout)
    {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(p_tcp->socket_fd, &set);
        // 最大阻塞30s
        if (p_tcp->timeout / 1000 > 30)
        {
            p_tcp->timeout = 30 * 1000;
        }
        struct timeval time_out = {1, 0}; // 秒和微秒,1s超时
        // 检测超时时间
        int ret = select(p_tcp->socket_fd + 1, &set, NULL, NULL, &time_out);

        if (0 == ret)
        {
            return -128;
        }
        else if (ret < 0)
        {
            return -1;
        }
    }

    p_tcp->recv_len = recv(p_tcp->socket_fd, p_tcp->recv_buf, MAX_TCP_REV_LEN, 0);

    if (p_tcp->recv_len < 0)
    {
        p_tcp->recv_len = 0;
        return -1;
    }

    else if (0 == p_tcp->recv_len)
    {
        return -128;
    }
    else
    {
        tcpPrintLog("接收数据:", p_tcp->recv_buf, p_tcp->recv_len);
    }

    p_tcp->last_time = get_epoch_time_ms();

    return 0;
}
