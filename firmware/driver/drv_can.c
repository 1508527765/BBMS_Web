/*****************************************************************************
 * @copyright       2024-2024,AO DI LTD.
 * @file            drv_can.h
 * @brief           can底层驱动程序
 * @author          Gary
 * @date            2024/01//16
 * @remark          优化
 *****************************************************************************/
#include "drv_can.h"

// 全局 CAN 设备数组
CanDevice can_devices[MAX_CAN_DEVICES] = {
    {.dev_num = 0,.index = 0,.socket_fd = -1,.ifname = "can0",.bitrate = 2,.send = NULL,.recv = NULL,.poll = NULL,.timeout = 0},
    {.dev_num = 0,.index = 1,.socket_fd = -1,.ifname = "can1",.bitrate = 2,.send = NULL,.recv = NULL,.poll= NULL,.timeout = 0} // 初始化 can1
};

static int can_bitrate[kCan_BitrateEnd] = {125000, 250000, 500000};

// 修正：atomic_bool 不是标准C类型，使用 stdatomic.h 并用 _Atomic 替代
#include <stdatomic.h>
static _Atomic bool recovery_flag = false; // C11标准写法
// 旧写法：static atomic_bool recovery_flag = ATOMIC_VAR_INIT(false);
/*****************************************************************************
 * @brief 将上传配置传输到给index对应can的波特率赋值
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @param baud 波特率
 * @return 成功返回0，失败返回-1
 *****************************************************************************/
int drvSetBaudBuf(int index, int baud)
{
    if (index < MAX_CAN_DEVICES)
    {
        can_devices[index].bitrate = baud;
        return 0;
    }
    return 1;
}

/*************************************************************************
 * @brief 设置CAN接口的波特率
 * @param ifname CAN接口名称（如 "can0"）
 * @param bitrate 波特率（如 500000 表示 500 kbps）
 * @return 成功返回0，失败返回-1
 **************************************************************************/
#include "kit_core.h"
int drvSetCanBitrate(const char *ifname, int bitrate)
{
    char command[128] = {0}; // 设置can及设置波特率
    char down[128] = {0};
    char buffer[128] = {0};

    sprintf(down, "echo \"forlinx\" | sudo ip link set %s down", ifname);
    // printf("初始化can：%s\n", down);
    if (kit_popen_exec(down, buffer, sizeof(buffer)) != 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Failed to set bitrate for %s\n", ifname);
        return -1;
    }
    sprintf(command,
            "echo \"forlinx\" | sudo ip link set %s up type can bitrate %d",
            ifname, bitrate);
    // printf("设置can波特率：%s\n", command);
    if (kit_popen_exec(command, buffer, sizeof(buffer)) != 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Failed to set bitrate for %s\n", ifname);
        return -1;
    }
    // printf("Bitrate set to %d for %s\n", bitrate, ifname);
    return 0;
}

/*************************************************************************
 * @brief 打开CAN设备
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @param bitrate 波特率（如 500000 表示 500 kbps）
 * @return 成功返回0，失败返回-1
 **************************************************************************/
#include <linux/can/raw.h> // 这个通常不包含 CAN_RAW_TX_QLEN
#ifndef CAN_RAW_TX_QLEN
#define CAN_RAW_TX_QLEN 5
#endif
#include <linux/can/netlink.h> // 包含SIOCGCANCTRLMODE的定义
#include <fcntl.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/if.h>
int drvCanOpen(int index)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Invalid CAN device index\n");
        return -1;
    }

    struct sockaddr_can addr;
    struct ifreq ifr;

    if (can_devices[index].bitrate >= kCan_BitrateEnd)
    {
        can_devices[index].bitrate = kCan_500k;
    }
    // 设置波特率
    if (drvSetCanBitrate2(can_devices[index].ifname,
                          can_bitrate[can_devices[index].bitrate]) != 0)
    {
        return -1;
    }

    // 创建Socket
    can_devices[index].socket_fd = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (can_devices[index].socket_fd < 0)
    {
        perror("Socket creation failed");
        return -1;
    }

    // 非阻塞设置
    int flags = fcntl(can_devices[index].socket_fd, F_GETFL, 0);
    if (flags < 0 || fcntl(can_devices[index].socket_fd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("Failed to set CAN socket to non-blocking mode");
        close(can_devices[index].socket_fd);
        return -1;
    }

    int txqlen = 100;
    setsockopt(can_devices[index].socket_fd, SOL_CAN_RAW, CAN_RAW_TX_QLEN, &txqlen, sizeof(txqlen));

    int enable = 0;
    setsockopt(can_devices[index].socket_fd, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS, &enable, sizeof(enable));

    // 设置网络设备队列长度
    char temp[300] = {0};                                                                                                       // 命令缓冲区
    char buffer[256];                                                                                                           // 获取执行返回结果
    snprintf(temp, sizeof(temp), "echo \"%s\" | sudo -S ip link set %s txqueuelen 1000", "forlinx", can_devices[index].ifname); // 使用 DHCP 获取 IP
    kit_popen_exec(temp, buffer, sizeof(buffer));

    // int loopback = 0;
    // setsockopt(can_devices[index].socket_fd, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback));

    // 获取接口索引
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, can_devices[index].ifname, IFNAMSIZ - 1);
    if (ioctl(can_devices[index].socket_fd, SIOCGIFINDEX, &ifr) < 0)
    {
        perror("ioctl SIOCGIFINDEX failed");
        close(can_devices[index].socket_fd);
        return -1;
    }

    // 连接CAN设备
    memset(&addr, 0, sizeof(struct sockaddr_can));
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    // addr.can_ifindex = if_nametoindex("can0"); // 使用can0接口

    if (bind(can_devices[index].socket_fd, (struct sockaddr *)&addr,
             sizeof(addr)) < 0)
    {
        perror("Binding failed");
        close(can_devices[index].socket_fd);
        can_devices[index].socket_fd = -1;
        return -1;
    }

    // 配置错误帧过滤器 - 关键修复！
    // 只有配置了错误帧过滤器，才能接收到CAN错误帧
    can_err_mask_t err_mask = CAN_ERR_TX_TIMEOUT | // 发送超时
                              CAN_ERR_LOSTARB |    // 仲裁丢失
                              CAN_ERR_CRTL |       // 控制器错误
                              CAN_ERR_PROT |       // 协议错误
                              CAN_ERR_TRX |        // 收发器错误
                              CAN_ERR_ACK |        // 应答错误
                              CAN_ERR_BUSOFF |     // 总线关闭
                              CAN_ERR_BUSERROR |   // 总线错误
                              CAN_ERR_RESTARTED;   // 重启完成

    if (setsockopt(can_devices[index].socket_fd, SOL_CAN_RAW, CAN_RAW_ERR_FILTER,
                   &err_mask, sizeof(err_mask)) < 0)
    {
        perror("setsockopt CAN_RAW_ERR_FILTER");
        close(can_devices[index].socket_fd);
        return -1;
    }

    KITLOG(LOG_DRIVER_EN, INFO_EN,
           "CAN device %s opened successfully at index %d\n",
           can_devices[index].ifname, index);
    printf("CAN device %s opened successfully at index %d\n",
           can_devices[index].ifname, index);
    return 0;
}

// 检查CAN接口状态
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 检查CAN接口状态
int drvCanCheckStatus(int index)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        return -1;
    }

    char cmd[128];
    char result[256];
    FILE *fp;
    int bus_off_detected = 0;
    int interface_down = 0;
    // 使用ip命令获取接口详细信息
    snprintf(cmd, sizeof(cmd), "echo \"forlinx\" | sudo -S ip -details link show %s", can_devices[index].ifname);

    fp = popen(cmd, "r");
    if (fp == NULL)
    {
        perror("popen failed");
        return -1;
    }

    while (fgets(result, sizeof(result), fp) != NULL)
    {
        // 检查BUS-OFF状态
        if (strstr(result, "BUS-OFF") != NULL)
        {
            bus_off_detected = 1;
            KITLOG(LOG_DRIVER_EN, ERROR_EN, "BUS-OFF state detected\n");
        }
        // 检查接口是否DOWN
        if (strstr(result, "state DOWN") != NULL)
        {
            interface_down = 1;
            KITLOG(LOG_DRIVER_EN, ERROR_EN, "Interface is DOWN\n");
        }
        // 检查错误计数器
        if (strstr(result, "error") != NULL)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Interface stats: %s", result);
        }
    }

    pclose(fp);

    if (interface_down)
    {
        return -1; // 接口关闭
    }

    if (bus_off_detected)
    {
        return -2; // BUS-OFF状态
    }

    return 0; // 状态正常
}

// CAN接口恢复函数
int drvCanRecover(int index)
{
    if (recovery_flag == true)
    {
        return 0;
    }
    recovery_flag = true;
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        return -1;
    }

    KITLOG(LOG_DRIVER_EN, WARN_EN, "Attempting to recover CAN interface %s\n",
           can_devices[index].ifname);

    // 1. 先关闭socket
    if (can_devices[index].socket_fd >= 0)
    {
        close(can_devices[index].socket_fd);
        can_devices[index].socket_fd = -1;
    }

    // 2. 重启网络接口
    char temp[300] = {0}; // 命令缓冲区
    char buffer[256];     // 获取执行返回结果
    snprintf(temp, sizeof(temp), "echo \"forlinx\" | sudo -S ip link set %s down && ip link set %s up", can_devices[index].ifname, can_devices[index].ifname);
    int ret = kit_popen_exec(temp, buffer, sizeof(buffer));

    if (ret != 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Failed to restart CAN interface\n");
        return -1;
    }

    // 3. 等待接口就绪
    usleep(500000); // 等待500ms

    // 4. 重新打开CAN设备
    return drvCanOpen(index);
}
#include <errno.h>  // 用于errno和错误码
#include <string.h> // 用于strerror
/*************************************************************************
 * @brief 发送CAN帧
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @param frame 指向CanFrame结构体的指针，包含CAN ID、数据内容和长度
 * @return 成功返回发送的字节数，失败返回-1
 **************************************************************************/
int drvCanSend(int index, const CanFrame *frame)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Invalid CAN device index\n");
        return -1;
    }

    if (can_devices[index].socket_fd < 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "CAN device at index %d not opened\n",
               index);
        return -1;
    }

    if (frame->len > 8)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Data length too long\n");
        return -1;
    }

    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        return -1;
    }
    static int recovery_attempts[MAX_CAN_DEVICES] = {0};
    static time_t last_recovery_time[MAX_CAN_DEVICES] = {0};

    struct can_frame raw_frame;
    raw_frame.can_id = frame->can_id | CAN_EFF_FLAG;
    raw_frame.can_dlc = frame->len;
    memcpy(raw_frame.data, frame->data, frame->len);

    // printf("Send CAN frame: ID=0x%X, len=%d\n", frame->can_id, frame->len);

    fd_set write_fds;
    struct timeval timeout;
    int ret;

    FD_ZERO(&write_fds);
    FD_SET(can_devices[index].socket_fd, &write_fds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000; // 50ms超时

    ret = select(can_devices[index].socket_fd + 1, NULL, &write_fds, NULL, &timeout);
    // printf("######################select ret = %d\n", ret);
    // if (ret > 0 && FD_ISSET(can_devices[index].socket_fd, &write_fds))
    if (ret > 0)
    {
        // 可写，正常发送
        int nbytes = write(can_devices[index].socket_fd, &raw_frame, sizeof(struct can_frame));
        if (nbytes >= 0)
        {
            recovery_attempts[index] = 0; // 重置恢复计数

            return nbytes;
        }
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "CAN send failed, errno = %d\n", errno);
        KITPTF(LOG_DRIVER_EN, ERROR_EN, "CAN send failed, errno = %d\n", errno);
        if (errno == ENOBUFS)
        {
            // 只是队列满，不是硬件故障
            KITLOG(LOG_DRIVER_EN, WARN_EN, "CAN queue full, will retry\n");
            KITPTF(LOG_DRIVER_EN, WARN_EN, "CAN queue full, will retry\n");
            return -1; // 让上层稍后重试
        }
    }
    // 到这里说明出现严重问题
    KITLOG(LOG_DRIVER_EN, ERROR_EN, "CAN send failed, checking interface status...\n");
    KITPTF(LOG_DRIVER_EN, ERROR_EN, "CAN send failed, checking interface status...\n");

    // 检查接口状态
    int status = drvCanCheckStatus(index);
    if (status == -2)
    {
        // BUS-OFF状态，需要恢复
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "CAN bus-off detected!\n");
        KITPTF(LOG_DRIVER_EN, ERROR_EN, "CAN bus-off detected!\n");
    }

    // 实施恢复策略
    time_t now = time(NULL);

    // 限制恢复频率：最多每30秒尝试一次恢复
    if (now - last_recovery_time[index] > 30)
    {
        recovery_attempts[index] = 0;
    }

    if (1)
    { // 最多尝试3次恢复
        recovery_attempts[index]++;
        last_recovery_time[index] = now;

        KITLOG(LOG_DRIVER_EN, WARN_EN, "Attempting recovery (%d/3)\n", recovery_attempts[index]);

        if (drvCanRecover(index) == 0)
        {
            KITLOG(LOG_DRIVER_EN, INFO_EN, "CAN interface recovered successfully\n");
            printf("######################CAN interface recovered successfully\n");
            recovery_attempts[index] = 0;
            recovery_flag = false;
            return -2; // 特殊返回码，表示已恢复，建议重发
        }
    }
    recovery_flag = false;
    // 恢复失败
    KITLOG(LOG_DRIVER_EN, ERROR_EN, "CAN interface recovery failed after %d attempts\n",
           recovery_attempts[index]);
    printf("######################CAN interface recovery failed after %d attempts\n",
           recovery_attempts[index]);

#if CAN_DEBUG
    KITLOG(LOG_DRIVER_EN,
           DEBUG_EN "Sent %d bytes on %s (index %d): CAN ID 0x%X, Data: ", nbytes,
           can_devices[index].ifname, index, raw_frame.can_id);
    for (int i = 0; i < raw_frame.can_dlc; i++)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN "%02X ", raw_frame.data[i]);
        KITPTF(LOG_DRIVER_EN, DEBUG_EN "%02X ", raw_frame.data[i]);
    }
    printf("\n");
#endif

    // 可以选择彻底关闭接口或进入安全模式或重启系统
    return -3; // 严重错误
}



/*************************************************************************
 * @brief 发送CAN消息
 * @param index CAN设备索引
 * @param frame CAN消息指针
 **********************************************************************/
int drvCanNewSend(int index, CanMsg *frame)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Invalid CAN device index\n");
        return -1;
    }

    if (can_devices[index].socket_fd < 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "CAN device at index %d not opened\n",
               index);
        return -1;
    }

    if (frame->len > 8)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "Data length too long\n");
        return -1;
    }

    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        return -1;
    }
    static int recovery_attempts[MAX_CAN_DEVICES] = {0};
    static time_t last_recovery_time[MAX_CAN_DEVICES] = {0};

    struct can_frame raw_frame;
    raw_frame.can_id = frame->id.value | CAN_EFF_FLAG;
    raw_frame.can_dlc = frame->len;
    memcpy(raw_frame.data, frame->data.byte, frame->len);

    fd_set write_fds;
    struct timeval timeout;
    int ret;

    FD_ZERO(&write_fds);
    FD_SET(can_devices[index].socket_fd, &write_fds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 50000; // 50ms超时

    ret = select(can_devices[index].socket_fd + 1, NULL, &write_fds, NULL, &timeout);
    // printf("######################select ret = %d\n", ret);
    // if (ret > 0 && FD_ISSET(can_devices[index].socket_fd, &write_fds))
    if (ret > 0)
    {
        // 可写，正常发送
        int nbytes = write(can_devices[index].socket_fd, &raw_frame, sizeof(struct can_frame));
        if (nbytes >= 0)
        {
            recovery_attempts[index] = 0; // 重置恢复计数

            return nbytes;
        }
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "CAN send failed, errno = %d\n", errno);
        KITPTF(LOG_DRIVER_EN, ERROR_EN, "CAN send failed, errno = %d\n", errno);
        if (errno == ENOBUFS)
        {
            // 只是队列满，不是硬件故障
            KITLOG(LOG_DRIVER_EN, WARN_EN, "CAN queue full, will retry\n");
            KITPTF(LOG_DRIVER_EN, WARN_EN, "CAN queue full, will retry\n");
            return -1; // 让上层稍后重试
        }
    }
    // 到这里说明出现严重问题
    KITLOG(LOG_DRIVER_EN, ERROR_EN, "CAN send failed, checking interface status...\n");
    KITPTF(LOG_DRIVER_EN, ERROR_EN, "CAN send failed, checking interface status...\n");

    // 检查接口状态
    int status = drvCanCheckStatus(index);
    if (status == -2)
    {
        // BUS-OFF状态，需要恢复
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "CAN bus-off detected!\n");
        KITPTF(LOG_DRIVER_EN, ERROR_EN, "CAN bus-off detected!\n");
    }

    // 实施恢复策略
    time_t now = time(NULL);

    // 限制恢复频率：最多每30秒尝试一次恢复
    if (now - last_recovery_time[index] > 30)
    {
        recovery_attempts[index] = 0;
    }

    if (1)
    { // 最多尝试3次恢复
        recovery_attempts[index]++;
        last_recovery_time[index] = now;

        KITLOG(LOG_DRIVER_EN, WARN_EN, "Attempting recovery (%d/3)\n", recovery_attempts[index]);

        if (drvCanRecover(index) == 0)
        {
            KITLOG(LOG_DRIVER_EN, INFO_EN, "CAN interface recovered successfully\n");
            printf("######################CAN interface recovered successfully\n");
            recovery_attempts[index] = 0;
            recovery_flag = false;
            return -2; // 特殊返回码，表示已恢复，建议重发
        }
    }
    recovery_flag = false;
    // 恢复失败
    KITLOG(LOG_DRIVER_EN, ERROR_EN, "CAN interface recovery failed after %d attempts\n",
           recovery_attempts[index]);
    printf("######################CAN interface recovery failed after %d attempts\n",
           recovery_attempts[index]);

#if CAN_DEBUG
    KITLOG(LOG_DRIVER_EN,
           DEBUG_EN "Sent %d bytes on %s (index %d): CAN ID 0x%X, Data: ", nbytes,
           can_devices[index].ifname, index, raw_frame.can_id);
    for (int i = 0; i < raw_frame.can_dlc; i++)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN "%02X ", raw_frame.data[i]);
        KITPTF(LOG_DRIVER_EN, DEBUG_EN "%02X ", raw_frame.data[i]);
    }
    printf("\n");
#endif

    // 可以选择彻底关闭接口或进入安全模式或重启系统
    return -3; // 严重错误
}






/*************************************************************************
 * @brief 接收CAN帧
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @param frame 指向CanFrame结构体的指针，用于存储接收到的CAN帧信息
 * @return 成功返回接收的字节数，失败返回-1
 **************************************************************************/
#include <errno.h>
int drvCanRecv(int index, CanFrame *frame)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        fprintf(stderr, "Invalid CAN device index\n");
        return -1;
    }

    if (can_devices[index].socket_fd < 0)
    {
        // fprintf(stderr, "######################CAN device at index %d not opened\n", index);
        return -1;
    }

    struct can_frame raw_frame;

// 接收CAN帧
#if 0
    int nbytes = read(can_devices[index].socket_fd, &raw_frame, sizeof(struct can_frame));
    if (nbytes < 0)
    {
        perror("Read failed");
        return -1;
    }
#endif

    int nbytes = read(can_devices[index].socket_fd, &raw_frame, sizeof(struct can_frame));
    if (nbytes == sizeof(struct can_frame))
    {
        // 检查是否为错误帧
        if (raw_frame.can_id & CAN_ERR_FLAG)
        {
            // 调用统一的错误处理函数
            // 该函数会详细分析错误类型并提供处理建议
            printf(&raw_frame);
        }
    }
    if (nbytes < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            // 非阻塞模式下，没有数据可读
            return 0; // 表示没有新数据，不算错误
        }
        else
        {
            perror("Read failed");
            return -1;
        }
    }

    // 填充CanFrame结构体
    frame->can_id = raw_frame.can_id;
    frame->len = raw_frame.can_dlc;
    memcpy(frame->data, raw_frame.data, raw_frame.can_dlc);

    // 记录接收的CAN报文

    // KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Received %d bytes on %s (index %d): CAN ID 0x%X, Data: ", nbytes,
    //        can_devices[index].ifname, index, raw_frame.can_dlc);

    return nbytes;
}

/*************************************************************************
 * @brief 接收CAN帧
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @param frame 指向CanFrame结构体的指针，用于存储接收到的CAN帧信息
 * @return 成功返回接收的字节数，失败返回-1
 **************************************************************************/
#include <sys/select.h>
#define RECV_TIMEOUT_SEC 1 // 设置超时时间为5秒

int drvCanRecv2(int index, CanFrame *frame)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        fprintf(stderr, "Invalid CAN device index\n");
        return -1;
    }

    if (can_devices[index].socket_fd < 0)
    {
        fprintf(stderr, "CAN device at index %d not opened\n", index);
        return -1;
    }

    struct can_frame raw_frame;
    fd_set readfds;
    struct timeval timeout;

    // 初始化文件描述符集合
    FD_ZERO(&readfds);
    FD_SET(can_devices[index].socket_fd, &readfds);

    // 设置超时时间
    timeout.tv_sec = RECV_TIMEOUT_SEC;
    timeout.tv_usec = 0;

    // 使用select进行超时等待
    int activity = select(can_devices[index].socket_fd + 1, &readfds, NULL, NULL, &timeout);
    if (activity < 0)
    {
        perror("Select failed");
        return -1;
    }
    else if (activity == 0)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Receive timeout on %s (index %d)\n", can_devices[index].ifname, index);
        return -1;
    }

    // 接收CAN帧
    int nbytes = read(can_devices[index].socket_fd, &raw_frame, sizeof(struct can_frame));
    if (nbytes < 0)
    {
        perror("Read failed");
        return -1;
    }

    // 填充CanFrame结构体
    frame->can_id = raw_frame.can_id;
    frame->len = raw_frame.can_dlc;
    memcpy(frame->data, raw_frame.data, raw_frame.can_dlc);

    KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Received %d bytes on %s (index %d): CAN ID 0x%X, Data: ", nbytes,
           can_devices[index].ifname, index, raw_frame.can_id);

    return nbytes;
}

/*************************************************************************
 * @brief 接收CAN帧，带超时机制
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 * @param frame 指向CanFrame结构体的指针，用于存储接收到的CAN帧信息
 * @return 成功返回接收的字节数，失败返回-1
 **************************************************************************/
int drvCanNewRecv(int index, CanMsg *frame)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        fprintf(stderr, "Invalid CAN device index\n");
        return -1;
    }

    if (can_devices[index].socket_fd < 0)
    {
        fprintf(stderr, "CAN device at index %d not opened\n", index);
        return -1;
    }

    struct can_frame raw_frame;
    fd_set readfds;
    struct timeval timeout;

    // 初始化文件描述符集合
    FD_ZERO(&readfds);
    FD_SET(can_devices[index].socket_fd, &readfds);

    // 设置超时时间
    timeout.tv_sec = RECV_TIMEOUT_SEC;
    timeout.tv_usec = 0;

    // 使用select进行超时等待
    int activity = select(can_devices[index].socket_fd + 1, &readfds, NULL, NULL, &timeout);
    if (activity < 0)
    {
        perror("Select failed");
        return -1;
    }
    else if (activity == 0)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Receive timeout on %s (index %d)\n", can_devices[index].ifname, index);
        return -1;
    }

    // 接收CAN帧
    int nbytes = read(can_devices[index].socket_fd, &raw_frame, sizeof(struct can_frame));
    if (nbytes < 0)
    {
        perror("Read failed");
        return -1;
    }

    // 填充CanFrame结构体
    frame->id.value = raw_frame.can_id;
    frame->len = raw_frame.can_dlc;
    memcpy(frame->data.byte, raw_frame.data, raw_frame.can_dlc);

    KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Received %d bytes on %s (index %d): CAN ID 0x%X, Data: ", nbytes,can_devices[index].ifname, index, raw_frame.can_id);
    for (int i = 0; i < raw_frame.can_dlc; i++)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "0x%02X ", frame->data.byte[i]);
    }
    KITLOG(LOG_DRIVER_EN, DEBUG_EN, "\n");

    return nbytes;
}


/*************************************************************************
 * @brief 关闭CAN设备
 * @param index CAN设备的索引（0 到 MAX_CAN_DEVICES-1）
 **************************************************************************/
void drvCanClose(int index)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        fprintf(stderr, "Invalid CAN device index\n");
        return;
    }

    if (can_devices[index].socket_fd >= 0)
    {
        close(can_devices[index].socket_fd);
        can_devices[index].socket_fd = -1;
        printf("CAN device at index %d closed\n", index);
    }
}




//注册CAN设备的轮询回调函数
void drvCanRegisterPollCallback(int index, can_poll_callback poll)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        fprintf(stderr, "Invalid CAN device index\n");
        return;
    }

    can_devices[index].poll = poll;
}

//注册CAN设备的发送回调函数
void drvCanRegisterSendCallback(int index, can_send_callback send)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        fprintf(stderr, "Invalid CAN device index\n");
        return;
    }

    can_devices[index].send = send;
}

//注册CAN设备的接收回调函数
void drvCanRegisterRecvCallback(int index, can_recv_callback recv)
{
    if (index < 0 || index >= MAX_CAN_DEVICES)
    {
        fprintf(stderr, "Invalid CAN device index\n");
        return;
    }

    can_devices[index].recv = recv;
}


/**
 * 执行 Linux 命令并获取输出
 *
 * @param command 要执行的命令
 * @param output  用于存储输出的缓冲区
 * @param output_size 缓冲区大小
 * @return 0 表示成功，非 0 表示失败
 */
int execute_command(const char *command, char *output, size_t output_size)
{
    FILE *fp;
    char buffer[1024];
    size_t total_len = 0;

    // 打开管道
    fp = popen(command, "r");
    if (fp == NULL)
    {
        perror("popen failed");
        return 1;
    }

    // 读取输出
    while (fgets(buffer, sizeof(buffer), fp) != NULL)
    {
        size_t len = strlen(buffer);
        if (total_len + len >= output_size)
        {
            fprintf(stderr, "Output buffer too small\n");
            pclose(fp);
            return 1;
        }
        strcpy(output + total_len, buffer);
        total_len += len;
    }

    // 关闭管道
    pclose(fp);

    return 0;
}

int drvSetCanBitrate2(const char *ifname, int bitrate)
{
    char command1[128];
    char current_device_status[128];
    snprintf(command1, sizeof(command1),
             "ip link show %s|grep -oP 'state \\K(DOWN|UP)(?= mode)'", ifname);
    if (execute_command(command1, current_device_status,
                        sizeof(current_device_status)) != 0)
    {
        fprintf(stderr, "Failed to execute command1\n");
        return 1;
    }

    printf("current_device_status: %s\n", current_device_status);

    char command2[128];
    char current_bitrate_str[128];
    snprintf(command2, sizeof(command2),
             "ip -details link show %s | grep -oP 'bitrate \\K\\d+(?= sample)'", ifname);
    if (execute_command(command2, current_bitrate_str,
                        sizeof(current_bitrate_str)) != 0)
    {
        fprintf(stderr, "Failed to execute command2\n");
        return -1;
    }
    printf("current_bitrate_str: %s\n", current_bitrate_str);
    int current_bitrate = atoi(current_bitrate_str);

    // 移除current_device_status末尾的换行符
    size_t len = strlen(current_device_status);
    if (len > 0 && current_device_status[len - 1] == '\n')
    {
        current_device_status[len - 1] = '\0';
    }

    // 移除current_bitrate_str末尾的换行符
    len = strlen(current_bitrate_str);
    if (len > 0 && current_bitrate_str[len - 1] == '\n')
    {
        current_bitrate_str[len - 1] = '\0';
    }

    if (current_bitrate == bitrate && strcmp(current_device_status, "UP") == 0)
    {
        printf("Bitrate already set to %d and %s is UP for %s\n", bitrate, ifname,
               ifname);
        return 0; // 退出
    }
    else if (current_bitrate == bitrate &&
             strcmp(current_device_status, "DOWN") == 0)
    {
        // 启动 can 口
        char command3[128];
        snprintf(command3, sizeof(command3), "ip link set %s up", ifname);
        if (execute_command(command3, NULL, 0) != 0)
        {
            fprintf(stderr, "Failed to start %s\n", ifname);
            return 1;
        }
        printf("Bitrate already set to %d, and the %s port just set up\n", bitrate,
               ifname);
        return 0;
    }
    else
    {
        // 设置 bitrate
        char command4[256];
        snprintf(command4, sizeof(command4),
                 "ip link set %s down && ip link set %s up type can bitrate %d",
                 ifname, ifname, bitrate);
        if (execute_command(command4, NULL, 0) != 0)
        {
            fprintf(stderr, "Failed to set bitrate to %d for %s\n", bitrate, ifname);
            return 1;
        }
        printf("Bitrate set to %d for %s and the port just restart\n", bitrate, ifname);
        return 0;
    }
}


/**
 * @brief 处理CAN错误帧
 *
 * 功能：
 * - 解析CAN错误帧的详细错误信息
 * - 根据错误类型提供具体的处理建议
 * - 记录详细的错误日志用于调试
 * - 对严重错误执行自动恢复操作
 *
 * 设计原理：
 * - 统一错误处理逻辑，便于维护和扩展
 * - 提供详细的错误分析，帮助快速定位问题
 * - 区分临时错误和严重错误，采用不同处理策略
 * - 记录错误计数器值，监控总线健康状态
 *
 * @param frame 包含错误信息的CAN帧指针
 */
void print_can_error(const struct can_frame *frame)
{
    // 更新错误时间戳
    KITLOG(LOG_DRIVER_EN, DEBUG_EN, "\n=== CAN ERROR DETECTED ===\n");
    KITPTF(LOG_DRIVER_EN, DEBUG_EN, "\n=== CAN ERROR DETECTED ===\n");
    KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Error Frame ID: 0x%X\n", frame->can_id);
    KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Error Frame ID: 0x%X\n", frame->can_id);
    KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Error Data: ");
    KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Error Data: ");
    for (int i = 0; i < 8; i++)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "0x%02X ", frame->data[i]);
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "0x%02X ", frame->data[i]);
    }
    KITLOG(LOG_DRIVER_EN, DEBUG_EN, "\n");
    KITPTF(LOG_DRIVER_EN, DEBUG_EN, "\n");

    // 处理发送超时错误
    if (frame->can_id & CAN_ERR_TX_TIMEOUT)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] TX Timeout - ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] TX Timeout - ");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "发送超时，可能原因：\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "发送超时，可能原因：\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线负载过高，无法获得发送机会\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线负载过高，无法获得发送机会\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 其他节点持续占用总线\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 其他节点持续占用总线\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 发送优先级设置过低\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 发送优先级设置过低\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件发送缓冲器故障\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件发送缓冲器故障\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "建议：检查总线负载，调整发送优先级\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "建议：检查总线负载，调整发送优先级\n");
    }

    // 处理仲裁丢失错误
    if (frame->can_id & CAN_ERR_LOSTARB)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Arbitration Lost - ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Arbitration Lost - ");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "仲裁丢失，可能原因：\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "仲裁丢失，可能原因：\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 多个节点同时发送相同ID的帧\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 多个节点同时发送相同ID的帧\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 发送时序不当\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 发送时序不当\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件仲裁逻辑故障\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件仲裁逻辑故障\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线信号质量问题\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线信号质量问题\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "建议：检查ID分配，优化发送时序\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "建议：检查ID分配，优化发送时序\n");
    }

    // 处理控制器错误
    if (frame->can_id & CAN_ERR_CRTL)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Controller Error - ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Controller Error - ");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "data[1]=0x%02X ", frame->data[1]);
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "data[1]=0x%02X ", frame->data[1]); // 调试信息

        // 调试：显示所有可能的标志位
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Flags: ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Flags: ");
        if (frame->data[1] & CAN_ERR_CRTL_TX_WARNING)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "TX_WARN ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "TX_WARN ");
        }
        if (frame->data[1] & CAN_ERR_CRTL_RX_WARNING)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "RX_WARN ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "RX_WARN ");
        }
        if (frame->data[1] & CAN_ERR_CRTL_TX_PASSIVE)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "TX_PASS ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "TX_PASS ");
        }
        if (frame->data[1] & CAN_ERR_CRTL_RX_PASSIVE)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "RX_PASS ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "RX_PASS ");
        }
        if (frame->data[1] & CAN_ERR_CRTL_TX_OVERFLOW)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "TX_OVER ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "TX_OVER ");
        }
        if (frame->data[1] & CAN_ERR_CRTL_RX_OVERFLOW)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "RX_OVER ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "RX_OVER ");
        }
        if (frame->data[1] & CAN_ERR_CRTL_ACTIVE)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "ACTIVE ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "ACTIVE ");
        }
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "\n");

        // 分析具体的控制器错误类型
        // 注意：错误状态是累积的，TX Passive状态会同时包含TX Warning标志

        // 检查TX错误计数器值来判断状态（因为驱动可能不设置TX_PASSIVE标志）
        if (frame->data[6] > 127) // TX错误计数器 > 127
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "TX Passive (TX error counter = %d > 127) - ", frame->data[6]);
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "TX Passive (TX error counter = %d > 127) - ", frame->data[6]);
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "发送错误计数器达到被动级别，只能发送被动错误帧\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "发送错误计数器达到被动级别，只能发送被动错误帧\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "严重警告：控制器进入被动状态，建议立即检查总线连接\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "严重警告：控制器进入被动状态，建议立即检查总线连接\n");

            // 主动重置CAN接口，避免进入Bus-Off状态
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "主动重置CAN接口以防止进入Bus-Off状态...\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "主动重置CAN接口以防止进入Bus-Off状态...\n");
            // reset_can();
        }
        else if (frame->data[1] & CAN_ERR_CRTL_TX_WARNING || frame->data[6] > 96)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "TX Warning (TX error counter = %d > 96) - ", frame->data[6]);
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "TX Warning (TX error counter = %d > 96) - ", frame->data[6]);
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "发送错误计数器达到警告级别\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "发送错误计数器达到警告级别\n");
        }

        if (frame->data[1] & CAN_ERR_CRTL_RX_PASSIVE)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "RX Passive (RX error counter > 127) - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "RX Passive (RX error counter > 127) - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "接收错误计数器达到被动级别，只能发送被动错误帧\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "接收错误计数器达到被动级别，只能发送被动错误帧\n");
        }
        else if (frame->data[1] & CAN_ERR_CRTL_RX_WARNING)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "RX Warning (RX error counter > 96) - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "RX Warning (RX error counter > 96) - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "接收错误计数器达到警告级别\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "接收错误计数器达到警告级别\n");
        }
        if (frame->data[1] & CAN_ERR_CRTL_TX_OVERFLOW)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "TX Overflow - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "TX Overflow - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "发送缓冲区溢出，可能原因：\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "发送缓冲区溢出，可能原因：\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 发送速度过快，超过硬件处理能力\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 发送速度过快，超过硬件处理能力\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 发送队列管理不当\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 发送队列管理不当\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件发送缓冲器容量不足\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件发送缓冲器容量不足\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "建议：降低发送频率，优化发送队列管理\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "建议：降低发送频率，优化发送队列管理\n");
        }
        if (frame->data[1] & CAN_ERR_CRTL_RX_OVERFLOW)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "RX Overflow - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "RX Overflow - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "接收缓冲区溢出，可能原因：\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "接收缓冲区溢出，可能原因：\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 应用程序处理速度过慢\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 应用程序处理速度过慢\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线负载过高\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线负载过高\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 系统资源不足\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 系统资源不足\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "建议：优化接收处理逻辑，增加缓冲区大小\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "建议：优化接收处理逻辑，增加缓冲区大小\n");
        }
        if (frame->data[1] & CAN_ERR_CRTL_ACTIVE)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Controller Active - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Controller Active - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "控制器恢复到活动状态，可能原因：\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "控制器恢复到活动状态，可能原因：\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 从错误被动状态恢复\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 从错误被动状态恢复\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 从错误警告状态恢复\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 从错误警告状态恢复\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线质量改善\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线质量改善\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "总线状态已恢复正常\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "总线状态已恢复正常\n");
        }

        // 打印错误计数器值
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Error counters - TX: %d, RX: %d\n", frame->data[6], frame->data[7]);
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Error counters - TX: %d, RX: %d\n", frame->data[6], frame->data[7]);

        // 根据计数器值给出建议
        if (frame->data[6] > 96 || frame->data[7] > 96)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "警告：错误计数器值较高，建议检查总线质量\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "警告：错误计数器值较高，建议检查总线质量\n");
        }
    }

    // 处理协议错误
    if (frame->can_id & CAN_ERR_PROT)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Protocol Error - ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Protocol Error - ");

        if (frame->data[2] & CAN_ERR_PROT_BIT)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Bit Error - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Bit Error - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "位错误，可能原因：\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "位错误，可能原因：\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线信号质量问题\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线信号质量问题\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 电磁干扰\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 电磁干扰\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 线路阻抗不匹配\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 线路阻抗不匹配\n");
        }
        if (frame->data[2] & CAN_ERR_PROT_FORM)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Form Error - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Form Error - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "格式错误，可能原因：\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "格式错误，可能原因：\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 帧格式不符合CAN规范\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 帧格式不符合CAN规范\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件发送逻辑故障\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件发送逻辑故障\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线信号失真\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线信号失真\n");
        }
        if (frame->data[2] & CAN_ERR_PROT_STUFF)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Stuff Error - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Stuff Error - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "填充错误，可能原因：\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "填充错误，可能原因：\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 位填充规则违反\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 位填充规则违反\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件编码器故障\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件编码器故障\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 时钟同步问题\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 时钟同步问题\n");
        }
        if (frame->data[2] & CAN_ERR_PROT_BIT0)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Bit0 Error - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Bit0 Error - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "位0错误，在仲裁或ACK阶段检测到\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "位0错误，在仲裁或ACK阶段检测到\n");
        }
        if (frame->data[2] & CAN_ERR_PROT_BIT1)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Bit1 Error - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Bit1 Error - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "位1错误，在CRC界定符中检测到\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "位1错误，在CRC界定符中检测到\n");
        }
        if (frame->data[2] & CAN_ERR_PROT_OVERLOAD)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Overload Error - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Overload Error - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "过载错误，可能原因：\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "过载错误，可能原因：\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 接收器处理能力不足\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 接收器处理能力不足\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线负载过高\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线负载过高\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 系统资源不足\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 系统资源不足\n");
        }
        if (frame->data[2] & CAN_ERR_PROT_ACTIVE)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Active Error - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Active Error - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "主动错误宣告，可能原因：\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "主动错误宣告，可能原因：\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 检测到错误并主动发送错误帧\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 检测到错误并主动发送错误帧\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 错误恢复过程中的正常行为\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 错误恢复过程中的正常行为\n");
        }
        if (frame->data[2] & CAN_ERR_PROT_TX)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "TX Error - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "TX Error - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "发送错误，可能原因：\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "发送错误，可能原因：\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 发送过程中检测到错误\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 发送过程中检测到错误\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 发送时序问题\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 发送时序问题\n");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件发送逻辑故障\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 硬件发送逻辑故障\n");
        }

        // 分析错误位置（data[3]字段）
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Error Location: ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Error Location: ");
        switch (frame->data[3])
        {
        case CAN_ERR_PROT_LOC_UNSPEC:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Unspecified");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Unspecified");
            break;
        case CAN_ERR_PROT_LOC_SOF:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Start of Frame");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Start of Frame");
            break;
        case CAN_ERR_PROT_LOC_ID28_21:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "ID bits 28-21 (SFF: 10-3)");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "ID bits 28-21 (SFF: 10-3)");
            break;
        case CAN_ERR_PROT_LOC_ID20_18:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "ID bits 20-18 (SFF: 2-0)");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "ID bits 20-18 (SFF: 2-0)");
            break;
        case CAN_ERR_PROT_LOC_SRTR:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Substitute RTR (SFF: RTR)");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Substitute RTR (SFF: RTR)");
            break;
        case CAN_ERR_PROT_LOC_IDE:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Identifier Extension");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Identifier Extension");
            break;
        case CAN_ERR_PROT_LOC_ID17_13:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "ID bits 17-13");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "ID bits 17-13");
            break;
        case CAN_ERR_PROT_LOC_ID12_05:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "ID bits 12-5");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "ID bits 12-5");
            break;
        case CAN_ERR_PROT_LOC_ID04_00:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "ID bits 4-0");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "ID bits 4-0");
            break;
        case CAN_ERR_PROT_LOC_RTR:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "RTR");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "RTR");
            break;
        case CAN_ERR_PROT_LOC_RES1:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Reserved bit 1");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Reserved bit 1");
            break;
        case CAN_ERR_PROT_LOC_RES0:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Reserved bit 0");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Reserved bit 0");
            break;
        case CAN_ERR_PROT_LOC_DLC:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Data Length Code");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Data Length Code");
            break;
        case CAN_ERR_PROT_LOC_DATA:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Data section");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Data section");
            break;
        case CAN_ERR_PROT_LOC_CRC_SEQ:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CRC sequence");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CRC sequence");
            break;
        case CAN_ERR_PROT_LOC_CRC_DEL:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CRC delimiter");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CRC delimiter");
            break;
        case CAN_ERR_PROT_LOC_ACK:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "ACK slot");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "ACK slot");
            break;
        case CAN_ERR_PROT_LOC_ACK_DEL:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "ACK delimiter");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "ACK delimiter");
            break;
        case CAN_ERR_PROT_LOC_EOF:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "End of Frame");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "End of Frame");
            break;
        case CAN_ERR_PROT_LOC_INTERM:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Intermission");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Intermission");
            break;
        default:
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "Unknown (0x%02X)", frame->data[3]);
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "Unknown (0x%02X)", frame->data[3]);
            break;
        }
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "\n");

        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "建议：检查硬件连接，减少总线干扰\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "建议：检查硬件连接，减少总线干扰\n");
    }

    // 处理收发器错误
    if (frame->can_id & CAN_ERR_TRX)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Transceiver Error - ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Transceiver Error - ");

        if (frame->data[2] & CAN_ERR_TRX_CANH_NO_WIRE)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CANH No Wire - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CANH No Wire - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CAN_H线路断开或未连接\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CAN_H线路断开或未连接\n");
        }
        if (frame->data[2] & CAN_ERR_TRX_CANH_SHORT_TO_BAT)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CANH Short to Battery - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CANH Short to Battery - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CAN_H线路短路到电源正极\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CAN_H线路短路到电源正极\n");
        }
        if (frame->data[2] & CAN_ERR_TRX_CANH_SHORT_TO_VCC)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CANH Short to VCC - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CANH Short to VCC - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CAN_H线路短路到VCC\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CAN_H线路短路到VCC\n");
        }
        if (frame->data[2] & CAN_ERR_TRX_CANH_SHORT_TO_GND)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CANH Short to GND - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CANH Short to GND - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CAN_H线路短路到地\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CAN_H线路短路到地\n");
        }
        if (frame->data[2] & CAN_ERR_TRX_CANL_NO_WIRE)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CANL No Wire - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CANL No Wire - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CAN_L线路断开或未连接\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CAN_L线路断开或未连接\n");
        }
        if (frame->data[2] & CAN_ERR_TRX_CANL_SHORT_TO_BAT)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CANL Short to Battery - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CANL Short to Battery - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CAN_L线路短路到电源正极\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CAN_L线路短路到电源正极\n");
        }
        if (frame->data[2] & CAN_ERR_TRX_CANL_SHORT_TO_VCC)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CANL Short to VCC - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CANL Short to VCC - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CAN_L线路短路到VCC\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CAN_L线路短路到VCC\n");
        }
        if (frame->data[2] & CAN_ERR_TRX_CANL_SHORT_TO_GND)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CANL Short to GND - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CANL Short to GND - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CAN_L线路短路到地\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CAN_L线路短路到地\n");
        }
        if (frame->data[2] & CAN_ERR_TRX_CANL_SHORT_TO_CANH)
        {
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CANL Short to CANH - ");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CANL Short to CANH - ");
            KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CAN_L和CAN_H线路短路\n");
            KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CAN_L和CAN_H线路短路\n");
        }

        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "严重硬件错误！建议立即检查线路连接\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "严重硬件错误！建议立即检查线路连接\n");
    }

    // 处理ACK错误
    if (frame->can_id & CAN_ERR_ACK)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] ACK Error - ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] ACK Error - ");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "应答错误，可能原因：\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "应答错误，可能原因：\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线上没有其他节点应答\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线上没有其他节点应答\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 所有其他节点都处于错误状态\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 所有其他节点都处于错误状态\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线线路断开或短路\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线线路断开或短路\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 终端电阻缺失或阻值不正确\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 终端电阻缺失或阻值不正确\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 其他节点电源故障\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 其他节点电源故障\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "建议：检查总线连接，确认其他节点状态\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "建议：检查总线连接，确认其他节点状态\n");
    }

    // 处理Bus-Off错误
    if (frame->can_id & CAN_ERR_BUSOFF)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Bus-Off Error - ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Bus-Off Error - ");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "总线关闭错误，可能原因：\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "总线关闭错误，可能原因：\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 发送错误计数器超过255\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 发送错误计数器超过255\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 严重的硬件故障\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 严重的硬件故障\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线短路或断路\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线短路或断路\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 收发器芯片损坏\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 收发器芯片损坏\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 电源电压异常\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 电源电压异常\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "严重错误！正在自动重置CAN接口...\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "严重错误！正在自动重置CAN接口...\n");
        // reset_can();
    }

    // 处理总线错误
    if (frame->can_id & CAN_ERR_BUSERROR)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Bus Error - ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[ERROR] Bus Error - ");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "总线错误，可能原因：\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "总线错误，可能原因：\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 总线信号质量问题\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 总线信号质量问题\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 电磁干扰严重\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 电磁干扰严重\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 线路阻抗不匹配\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 线路阻抗不匹配\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 终端电阻问题\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 终端电阻问题\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 多个节点同时发送冲突\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 多个节点同时发送冲突\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "建议：检查总线物理连接，减少干扰源\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "建议：检查总线物理连接，减少干扰源\n");
    }

    // 处理重启完成
    if (frame->can_id & CAN_ERR_RESTARTED)
    {
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "[INFO] CAN Controller Restarted - ");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "[INFO] CAN Controller Restarted - ");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "CAN控制器已重启完成，可能原因：\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "CAN控制器已重启完成，可能原因：\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 从Bus-Off状态自动恢复\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 从Bus-Off状态自动恢复\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 手动重置CAN接口\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 手动重置CAN接口\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "  - 系统重启后初始化\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "  - 系统重启后初始化\n");
        KITLOG(LOG_DRIVER_EN, DEBUG_EN, "总线状态已恢复正常\n");
        KITPTF(LOG_DRIVER_EN, DEBUG_EN, "总线状态已恢复正常\n");
    }

    KITLOG(LOG_DRIVER_EN, DEBUG_EN, "=== END CAN ERROR ===\n\n");
    KITPTF(LOG_DRIVER_EN, DEBUG_EN, "=== END CAN ERROR ===\n\n");
}
