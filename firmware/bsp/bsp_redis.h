/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved. 
 * @file            bsp_redis.h
 * @brief           xx功能
 * @author          Gary
 * @date            2025/01-03
 * @remark          
 *****************************************************************************/

#ifndef BSP_REDIS_H
#define BSP_REDIS_H
#if 0
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdint.h>
//#include <hiredis/hiredis.h>
#include "bsp_rtdb.h"

// 定义 Redis 服务器的 IP 和端口
#define REDIS_SERVER_IP "127.0.0.1"
#define REDIS_SERVER_PORT 6379

int initRtdbRedis(void *arg,shm_creat_type_e type);
double getRedisPointValue(uint16_t devType, uint16_t devId, uint16_t pointId, point_e pointField);
void setRedisPointValue(uint16_t devType, uint16_t devId, uint16_t pointId, point_e pointField, double value);

#endif
#endif // BSP_REDIS_H
