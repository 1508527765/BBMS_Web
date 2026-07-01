/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            app_data_storage.c
 * @brief           数据存储模块源文件
 * @author          gary
 * @date            2025-07-10
 * @remark
 *****************************************************************************/

#ifndef app_data_storage_H
#define app_data_storage_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "bsp_rtdb.h"
#include "kit_model.h"
#include "kit_db.h"
#include "kit_log.h"
#include "utextend.h"
#include "bsp_ntp.h"
#include "kit_data.h"
#include "kit_math.h"

/*****************************************************************************
 * @brief           创建定时存储任务
 * @return          0-成功  1-失败
 *****************************************************************************/
void creatDataStorageTask();

/*****************************************************************************
 * @brief           清理磁盘空间，当磁盘使用率超过指定门限时删除最老的子文件夹
 * @param[in]       target_path: 目标文件夹路径
 * @param[in]       clean_count: 删除文件夹的个数
 * @param[in]       threshold_percent: 磁盘使用率门限（百分比）
 * @return          0-成功  1-失败
 *****************************************************************************/
int clean_disk_space(const char *target_path, int clean_count, double threshold_percent);

/*****************************************************************************
 * @brief           使用预定义路径清理磁盘空间
 * @return          0-成功  1-失败
 *****************************************************************************/
int clean_disk_space_default_paths();

void *database_monitor_thread(void *arg);

#endif // app_data_storage_H