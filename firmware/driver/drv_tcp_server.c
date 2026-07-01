#include "drv_tcp_server.h" // 包含您提供的头文件
#include "kit_log.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>


// time_t ems_to_pcs_last_time[20] = {0};s
// uint8_t ems_distrute_online_state[20] = {0};


/*****************************************************************************
 * @brief       初始化接收回调 GQ 2024/12/28
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      void
 *****************************************************************************/
void drvInitServerCall(tcp_server_lib_t *p_tcp, respondCall call)
{
    if (p_tcp != NULL)
    {
        p_tcp->revCall = call;
    }
    else
    {
        p_tcp->revCall = NULL;
    }
}
/*****************************************************************************
 * @brief           根据网络接口名称获取IP地址
 * @param[in]       ifname: 网络接口名称（如"eth2"）
 * @param[out]      ip_str: 输出的IP地址字符串缓冲区
 * @param[in]       ip_str_len: IP地址字符串缓冲区长度
 * @return          0-成功  1-失败
 * @note            使用getifaddrs获取网络接口信息，兼容性更好
 *****************************************************************************/
int getInterfaceIP(const char *ifname, char *ip_str, size_t ip_str_len)
{
    if (ifname == NULL || ip_str == NULL || ip_str_len == 0)
    {
        return 1;
    }

    struct ifaddrs *ifaddrs_list = NULL;
    struct ifaddrs *ifa = NULL;
    int ret = 1;

    // 获取所有网络接口信息
    if (getifaddrs(&ifaddrs_list) != 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "getifaddrs failed: %s", strerror(errno));
        return 1;
    }

    // 遍历所有网络接口
    for (ifa = ifaddrs_list; ifa != NULL; ifa = ifa->ifa_next)
    {
        // 检查接口名称是否匹配
        if (ifa->ifa_name == NULL || strcmp(ifa->ifa_name, ifname) != 0)
        {
            continue;
        }

        // 检查是否为IPv4地址
        if (ifa->ifa_addr != NULL && ifa->ifa_addr->sa_family == AF_INET)
        {
            struct sockaddr_in *sin = (struct sockaddr_in *)ifa->ifa_addr;
            const char *ip = inet_ntoa(sin->sin_addr);

            if (ip != NULL)
            {
                // CERT C STR31-C: 使用strncpy并确保字符串以null结尾
                strncpy(ip_str, ip, ip_str_len - 1);
                ip_str[ip_str_len - 1] = '\0';
                ret = 0;
                break;
            }
        }
    }

    // 释放资源
    freeifaddrs(ifaddrs_list);
    return ret;
}

/*****************************************************************************
 * @brief           设置文件描述符为非阻塞模式
 * @param[in]       fd:文件描述符
 * @return          0-成功  1-失败
 *****************************************************************************/
static int setNonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "fcntl F_GETFL 失败");
        return 1;
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "fcntl F_SETFL 失败");
        return 1;
    }
    return 0;
}

/*****************************************************************************
 * @brief           打开tcp驱动
 * @param[in]       p_tcp:
 * @return          0-成功  1-失败
 *****************************************************************************/
int drvTcpServerOpen(tcp_server_lib_t *p_tcp)
{
    if (p_tcp == NULL)
    {
        return 1;
    }

    // 创建socket
    p_tcp->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (p_tcp->socket_fd < 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "socket创建失败");
        return 1;
    }

    // 设置SO_REUSEADDR以快速重用端口
    int opt = 1;
    if (setsockopt(p_tcp->socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "设置socket通信选项失败");
        close(p_tcp->socket_fd);
        return 1;
    }

    // 设置IP和端口
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(p_tcp->port);

    // 如果指定了IP地址，则绑定到该IP；否则绑定到所有接口
    if (p_tcp->ip[0] != '\0')
    {
        // 如果IP地址有效，则绑定到指定IP
        if (inet_pton(AF_INET, p_tcp->ip, &server_addr.sin_addr) <= 0)
        {
            KITLOG(LOG_DRIVER_EN, ERROR_EN, "IP地址无效: %s", p_tcp->ip);
            close(p_tcp->socket_fd);
            return 1;
        }
        KITLOG(LOG_DRIVER_EN, INFO_EN, "绑定到指定IP地址: %s:%d", p_tcp->ip, p_tcp->port);
    }
    else
    {
        // 如果未指定IP，则绑定到所有接口
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        KITLOG(LOG_DRIVER_EN, INFO_EN, "绑定到所有网络接口: 0.0.0.0:%d", p_tcp->port);
    }
    // 绑定socket到IP和端口
    if (bind(p_tcp->socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "绑定socket到IP和端口失败");
        close(p_tcp->socket_fd);
        return 1;
    }

    int buffer_size = 1024 * 64; // 设置为 64KB
    setsockopt(p_tcp->socket_fd, SOL_SOCKET, SO_RCVBUF, &buffer_size, sizeof(buffer_size));
    // 开始监听
    if (listen(p_tcp->socket_fd, MAX_CLIENTS) < 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "监听失败");
        close(p_tcp->socket_fd);
        return 1;
    }

    // 设置非阻塞模式
    if (setNonblocking(p_tcp->socket_fd) > 0)
    {
        close(p_tcp->socket_fd);
        return 1;
    }

    // 创建epoll实例
    p_tcp->epoll_fd = epoll_create1(0);
    if (p_tcp->epoll_fd == -1)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "epoll创建失败");
        close(p_tcp->socket_fd);
        return 1;
    }

    // 将监听socket添加到epoll实例中
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // 设置为边缘触发模式
    ev.data.fd = p_tcp->socket_fd;
    if (epoll_ctl(p_tcp->epoll_fd, EPOLL_CTL_ADD, p_tcp->socket_fd, &ev) == -1)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "epoll添加fd监听失败");
        close(p_tcp->socket_fd);
        close(p_tcp->epoll_fd);
        return 1;
    }

    p_tcp->isListen = 1;
    KITLOG(LOG_DRIVER_EN, INFO_EN, "服务端启动成功，正在监听 %s:%d\n", p_tcp->ip, p_tcp->port);

    return 0;
}

/*****************************************************************************
 * @brief       关闭tcp驱动
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpServerClose(tcp_server_lib_t *p_tcp)
{
    if (p_tcp == NULL || p_tcp->socket_fd < 0)
    {
        return 1;
    }

    // 关闭所有已连接的客户端
    for (int i = 0; i < p_tcp->nClient; i++)
    {
        if (p_tcp->clients[i].fd >= 0)
        {
            close(p_tcp->clients[i].fd);
        }
    }

    // 关闭服务器socket和epoll实例
    close(p_tcp->socket_fd);
    close(p_tcp->epoll_fd);
    p_tcp->isListen = 0;
    p_tcp->status = 0; // 设置为离线
    KITLOG(LOG_DRIVER_EN, INFO_EN, "服务端关闭");

    return 0;
}

/*****************************************************************************
 * @brief       tcp驱动发送
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
ssize_t drvTcpServerSend(tcp_server_lib_t *p_tcp)
{
    if (p_tcp == NULL || p_tcp->send_len == 0)
    {
        return -1;
    }

    ssize_t total_sent = 0;
    for (int i = 0; i < p_tcp->nClient; i++)
    {
        if (p_tcp->clients[i].fd >= 0)
        {
            ssize_t sent = send(p_tcp->clients[i].fd, p_tcp->send_buf, p_tcp->send_len, 0);
            if (sent < 0 || sent != p_tcp->send_len)
            {
                KITLOG(LOG_DRIVER_EN, ERROR_EN, "发送失败");
                return -1;
            }
            total_sent += sent;
        }
    }

    return total_sent;
}

/*****************************************************************************
 * @brief       tcp驱动发送
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvTcpServerSend2(tcp_server_lib_t *p_tcp, int fd)
{
    ssize_t ret = 0;
    fd_set writefds;
    struct timeval timeout;
    timeout.tv_sec = 3; // 3秒超时
    timeout.tv_usec = 0;

    FD_ZERO(&writefds);
    FD_SET(fd, &writefds);

    int retn = select(fd + 1, NULL, &writefds, NULL, &timeout);
    if (retn > 0 && FD_ISSET(fd, &writefds))
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
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "IP(%s) 发送错误! ret: %ld, errno: %s(%d)\n", p_tcp->ip,
               ret, strerror(errno), errno);
        // close(fd); // 关闭 socket   协议层面去关闭，驱动层面不做处理，否则灵敏度过高
        return 128;
    }

sendpos:
    while (ret < p_tcp->send_len)
    {
        ssize_t rc = send(fd, p_tcp->send_buf + ret, p_tcp->send_len - ret, MSG_NOSIGNAL);
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
    //tcpPrintLog("发送数据:", p_tcp->send_buf, p_tcp->send_len);
    return 0;
}

/*****************************************************************************
 * @brief       tcp驱动接收优化
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
ssize_t drvTcpServerRecv(tcp_server_lib_t *p_tcp)
{
    if (p_tcp == NULL || p_tcp->epoll_fd <= 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Invalid tcp or epoll_fd");
        return -1;
    }

    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(p_tcp->epoll_fd, events, MAX_EVENTS, 3000);
    if (nfds == -1)
    {
        if (errno == EINTR)
        {
            // 被信号打断，忽略继续
            return 0;
        }
        KITLOG(LOG_DRIVER_EN, ERROR_EN,"epoll_wait failed: errno=%d (%s), epoll_fd=%d, MAX_EVENTS=%d",errno, strerror(errno), p_tcp->epoll_fd, MAX_EVENTS);
        return -1;
    }

    ssize_t total_recv = 0;
    time_t current_time = time(NULL); // 获取当前时间
    for (int i = 0; i < nfds; i++)
    {
        int fd = events[i].data.fd;
        if (fd == p_tcp->socket_fd)
        {
            // 边缘触发模式（EPOLLET）：需要循环 accept 直到没有更多连接
            // CERT C CON33-C: 在边缘触发模式下，必须处理所有等待的连接
            while (p_tcp->nClient < MAX_CLIENTS)
            {
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int client_fd =
                    accept(p_tcp->socket_fd, (struct sockaddr *)&client_addr, &addr_len);
                if (client_fd == -1)
                {
                    // 如果没有更多连接，accept 会返回 -1 并设置 errno
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        // 所有连接都已处理完毕，正常退出循环
                        break;
                    }
                    // 其他错误需要记录日志
                    KITLOG(LOG_DRIVER_EN, ERROR_EN, "accept failed: errno=%d (%s)", errno,
                           strerror(errno));
                    KITPTF(LOG_DRIVER_EN, ERROR_EN, "accept failed: errno=%d (%s)", errno,
                           strerror(errno));
                    break;
                }

                // 设置新连接为非阻塞
                if (setNonblocking(client_fd) > 0)
                {
                    close(client_fd);
                    continue;
                }

                // 添加新客户端到epoll监听
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                if (epoll_ctl(p_tcp->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
                {
                KITLOG(LOG_DRIVER_EN, ERROR_EN,"epoll_ctl add client failed");
                KITPTF(LOG_DRIVER_EN, ERROR_EN,"epoll_ctl add client failed");
                    close(client_fd);
                    continue;
                }

                // 添加到客户端列表中
                p_tcp->clients[p_tcp->nClient].fd = client_fd;
                p_tcp->clients[p_tcp->nClient].client = client_addr;
                p_tcp->clients[p_tcp->nClient].last_active_time =
                    current_time; // 初始化最后活动时间
                p_tcp->nClient++;
                KITLOG(LOG_DRIVER_EN, DEBUG_EN,
                       "New client connected: fd=%d,[连接总数] 当前连接数: %d\n", client_fd,
                       p_tcp->nClient);
                KITPTF(LOG_DRIVER_EN, DEBUG_EN,
                       "New client connected: fd=%d,[连接总数] 当前连接数: %d\n", client_fd,
                       p_tcp->nClient);
            }

            // 如果达到最大客户端数，继续 accept 并关闭，避免连接堆积
            if (p_tcp->nClient >= MAX_CLIENTS)
            {
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int client_fd =
                    accept(p_tcp->socket_fd, (struct sockaddr *)&client_addr, &addr_len);
                if (client_fd >= 0)
                {
                KITLOG(LOG_DRIVER_EN, ERROR_EN,"Max clients reached");
                KITLOG(LOG_DRIVER_EN, ERROR_EN,"Max clients reached");
                    close(client_fd);
                }
            }
        }
        else
        {
            // 找到对应的客户端索引
            int client_index = -1;
            for (int j = 0; j < p_tcp->nClient; j++)
            {
                if (p_tcp->clients[j].fd == fd)
                {
                    client_index = j;
                    break;
                }
            }

            if (client_index == -1)
            {
                KITLOG(LOG_DRIVER_EN, ERROR_EN, "Unknown client fd=%d", fd);
                epoll_ctl(p_tcp->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
                continue;
            }

            // 接收客户端数据
            ssize_t recv_len = recv(fd, p_tcp->recv_buf, 1024, 0);
            if (recv_len < 0)
            {
                if (errno != EWOULDBLOCK && errno != EAGAIN)
                {
                    KITLOG(LOG_DRIVER_EN, ERROR_EN, "接收数据出错断开连接: fd=%d",fd);
                    KITPTF(LOG_DRIVER_EN, ERROR_EN, "接收数据出错断开连接: fd=%d",fd);
                    epoll_ctl(p_tcp->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                    close(fd);
                    // 从客户端列表中移除
                    for (int k = client_index; k < p_tcp->nClient - 1; k++)
                    {
                        p_tcp->clients[k] = p_tcp->clients[k + 1];
                    }
                    p_tcp->nClient--;
                    KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[连接总数] 当前连接数: %d", p_tcp->nClient);
                    KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[连接总数] 当前连接数: %d", p_tcp->nClient);
                }
            }
            else if (recv_len == 0)
            {
                KITLOG(LOG_DRIVER_EN, ERROR_EN, "接收数据长度为0，Client disconnected: fd=%d",fd);
                KITPTF(LOG_DRIVER_EN, ERROR_EN, "接收数据长度为0，Client disconnected: fd=%d",fd);
                epoll_ctl(p_tcp->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                close(fd);
                // 从客户端列表中移除
                for (int k = client_index; k < p_tcp->nClient - 1; k++)
                {
                    p_tcp->clients[k] = p_tcp->clients[k + 1];
                }
                p_tcp->nClient--;
                KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[连接总数] 当前连接数: %d", p_tcp->nClient);
                KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[连接总数] 当前连接数: %d", p_tcp->nClient);
            }
            else
            {
                p_tcp->status = 1; // 表示在线
                p_tcp->clients[client_index].last_active_time = current_time;
                p_tcp->recv_len = recv_len; // 更新接收数据的长度
                p_tcp->revCall(p_tcp, fd);
            }
        }
    }
    // 检查所有客户端的最后活动时间，关闭超时的连接
    for (int i = 0; i < p_tcp->nClient; i++)
    {
        if (current_time - p_tcp->clients[i].last_active_time > CLIENT_TIMEOUT)
        {
            KITLOG(LOG_DRIVER_EN, ERROR_EN, "Client timeout: fd=%d",p_tcp->clients[i].fd);
            KITPTF(LOG_DRIVER_EN, ERROR_EN, "Client timeout: fd=%d",p_tcp->clients[i].fd);    

            epoll_ctl(p_tcp->epoll_fd, EPOLL_CTL_DEL, p_tcp->clients[i].fd, NULL);
            close(p_tcp->clients[i].fd);

            // 从客户端列表中移除
            for (int j = i; j < p_tcp->nClient - 1; j++)
            {
                p_tcp->clients[j] = p_tcp->clients[j + 1];
            }
            p_tcp->nClient--;
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[连接总数] 当前连接数: %d", p_tcp->nClient);
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[连接总数] 当前连接数: %d", p_tcp->nClient);
            i--; // 调整索引
        }
    }
    if (p_tcp->nClient == 0)
    {
        p_tcp->status = CONN_OFFLINE;
    }

    return total_recv;
}
