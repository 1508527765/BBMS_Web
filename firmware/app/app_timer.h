/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            app_timer.h
 * @brief           定时器相关头文件
 * @author          Gary
 * @date            2025-10-12
 * @remark          
 *****************************************************************************/
#ifndef APP_TIMER_H
#define APP_TIMER_H

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "kit_time.h"
#include "kit_log.h"
#include "kit_db.h"
// #include "app_comm.h"
#include "bsp_rtdb.h"
#include <dirent.h>
#include <sys/stat.h>



void creatTimerTask(void);

/**
 * @brief 创建故障统计任务线程
 * @remark 该线程每秒统计一次故障信息，并更新到RTDB
 *         统计内容：
 *         - 轻微故障数量（Minor故障数）
 *         - 严重故障数量（Major故障数）
 *         - 当前故障状态（故障等级：0=无故障，1=轻微故障，2=严重故障）
 */
void creatFaultStatisticsTask(void);

#endif // APP_TIMER_H