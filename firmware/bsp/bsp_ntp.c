/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved. 
 * @file            bsp_ntp.c
 * @brief           ntp对时功能
 * @author          Gary
 * @remark          
 *****************************************************************************/
#include "bsp_ntp.h"


/*********************************************************************
 * @brief       ntp随时请求
 * @param[in]   ntpServer: ntp地址,
 * @param[in]   ntpPort : 对时端口号一般默认123
 * @param[in]   ntpPacket : ntp结构
 * @return      seconds ： linux秒数
 *********************************************************************/
int sendNTPRequest(const char *ntpServer, int ntpPort, NTPPacket *ntpPacket)
{
    int sockfd;
    struct sockaddr_in servaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("NTP timing socket creation failed");
        return -1;
    }
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(ntpPort);
    int errno = inet_pton(AF_INET, ntpServer, &servaddr.sin_addr);
    if (errno <= 0)
    {

        perror("NTP timing inet_pton error");
        fprintf(stderr, "Error code: %d\n", errno); // 输出错误号
        close(sockfd);
        return -1;
    }
    // Set timeout for receiving data
    struct timeval timeout = {0};
    timeout.tv_sec = 3; // 5 seconds timeout
    timeout.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        perror("NTP timing setsockopt error");
        close(sockfd);
        return -1;
    }

    if (sendto(sockfd, ntpPacket, NTP_PACKET_SIZE, 0, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("NTP timing sendto error");
        close(sockfd);
        return -1;
    }

    int8_t recv_len = recvfrom(sockfd, ntpPacket, NTP_PACKET_SIZE, 0, NULL, NULL);
    if (recv_len <= 0)
    {
        perror("NTP timing recvfrom error");
        close(sockfd);
        return -1;
    }

    close(sockfd);
    return 0;
}

/*********************************************************************
 * @brief       ntp时间戳转换为linux时间
 * @param[in]   ntpTimestamp: ntp的秒数
 * @return      seconds ： linux秒数
 *********************************************************************/
time_t convertNTPtoUNIX(uint32_t ntpTimestamp)
{
    const uint32_t ntpEpochDiff = 2208988800;
    uint32_t seconds = ntpTimestamp;
    seconds = seconds - ntpEpochDiff;
    return (time_t)seconds;
}

/*********************************************************************
 * @brief       ntp对时接口
 * @param[in]   addr: 对时ntp地址
 * @param[in]   port: 对时端口
 * @param[in]   linuxPass: linux密码
 * @return      0：success  other：fault
 *********************************************************************/
uint8_t sync_ntp_timestamp(char *addr, uint16_t port, char linuxPass[50])
{
    NTPPacket ntpRequest = {0};
    time_t unixTime = 0;
    memset(&ntpRequest, 0, sizeof(ntpRequest));

    ntpRequest.li_vn_mode = 0x1B;

    if (sendNTPRequest(addr, port, &ntpRequest) == 0)
    {
        uint32_t ntpTimestamp = ntohl(ntpRequest.txTm_s);
        
        // 检查 NTP 时间戳的有效性（NTP 时间戳应该大于 2208988800，即 1970-01-01 00:00:00 UTC）
        if (ntpTimestamp < 2208988800U)
        {
            KITPTF(LOG_KIT_EN, ERROR_EN, "对时失败: NTP时间戳无效 (0x%08X, %u)", ntpTimestamp, ntpTimestamp);
            KITLOG(LOG_KIT_EN, ERROR_EN, "对时失败: NTP时间戳无效 (0x%08X, %u)", ntpTimestamp, ntpTimestamp);
            return -1;
        }
        
        unixTime = (time_t)(ntpTimestamp - 2208988800U); // ntohl网络字节顺序大端转换成主机字节顺序 // NTP timestamp starts from 1900

        // 检查转换后的 Unix 时间戳是否在合理范围内（1970-01-01 到 2100-01-01）
        // 1970-01-01 00:00:00 UTC = 0
        // 2100-01-01 00:00:00 UTC = 4102444800
        if (unixTime < 0 || unixTime > 4102444800L)
        {
            KITPTF(LOG_KIT_EN, ERROR_EN, "对时失败: Unix时间戳超出有效范围 (%ld)", (long)unixTime);
            KITLOG(LOG_KIT_EN, ERROR_EN, "对时失败: Unix时间戳超出有效范围 (%ld)", (long)unixTime);
            return -1;
        }

        setSysTime((time_t)unixTime, linuxPass);

        // 使用 localtime_r 和 strftime 安全地格式化时间（线程安全，避免 ctime() 可能返回 NULL 的问题）
        char timeStr[64] = {0};
        struct tm timeInfo = {0};
        struct tm *result = localtime_r(&unixTime, &timeInfo);
        if (result != NULL)
        {
            if (strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeInfo) > 0)
            {
                KITPTF(LOG_KIT_EN, DEBUG_EN, "对时成功,NTP time: %s", timeStr);
                KITLOG(LOG_KIT_EN, DEBUG_EN, "对时成功,NTP time: %s", timeStr);
            }
            else
            {
                KITPTF(LOG_KIT_EN, WARN_EN, "对时成功,但时间格式化失败 (时间戳: %ld)", (long)unixTime);
                KITLOG(LOG_KIT_EN, WARN_EN, "对时成功,但时间格式化失败 (时间戳: %ld)", (long)unixTime);
            }
        }
        else
        {
            KITPTF(LOG_KIT_EN, WARN_EN, "对时成功,但时间转换失败 (时间戳: %ld)", (long)unixTime);
            KITLOG(LOG_KIT_EN, WARN_EN, "对时成功,但时间转换失败 (时间戳: %ld)", (long)unixTime);
        }

        return 0;
    }
    else
    {
        KITPTF(LOG_KIT_EN, DEBUG_EN, "对时成功");
        KITLOG(LOG_KIT_EN, DEBUG_EN, "对时失败");
        return -1;
    }
    return 0;
}