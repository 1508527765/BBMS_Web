/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved. 
 * @file            bsp_ntp.h
 * @brief           ntp对时功能
 * @author          Gary
 * @remark          
 *****************************************************************************/
#ifndef BSP_NTP_H
#define BSP_NTP_H

// 确保 localtime_r() 可用（POSIX.1-2001 及更高版本）
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "kit_log.h"
#include "kit_core.h"

// #define NTP_SERVER "203.107.6.88"
// #define NTP_PORT 123
#define NTP_PACKET_SIZE 48

// NTP报文结构体 
typedef struct {
    uint8_t li_vn_mode;      // Leap indicator, Version number, Mode
    uint8_t stratum;         // Stratum level of the local clock
    uint8_t poll;            // Poll interval
    uint8_t precision;       // Precision of the local clock
    uint32_t rootDelay;      // Root delay between local machine and server
    uint32_t rootDispersion; // Root dispersion (maximum error)
    uint32_t refId;          // Reference ID
    uint32_t refTm_s;        // Reference time-stamp seconds
    uint32_t refTm_f;        // Reference time-stamp fraction of a second
    uint32_t origTm_s;       // Originate time-stamp seconds
    uint32_t origTm_f;       // Originate time-stamp fraction of a second
    uint32_t rxTm_s;         // Received time-stamp seconds
    uint32_t rxTm_f;         // Received time-stamp fraction of a second
    uint32_t txTm_s;         // Transmit time-stamp seconds
    uint32_t txTm_f;         // Transmit time-stamp fraction of a second
} NTPPacket;


/*********************************************************************
 * @brief       ntp对时接口
 * @param[in]   addr: 对时ntp地址
 * @param[in]   port: 对时端口
 * @param[in]   linuxPass: linux密码
 * @return      0：success  other：fault
 *********************************************************************/
uint8_t sync_ntp_timestamp(char *addr, uint16_t port, char linuxPass[50]);

#endif // BSP_NTP_H