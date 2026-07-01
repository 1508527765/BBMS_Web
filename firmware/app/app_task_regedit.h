/*****************************************************************************
 * @copyright       2024-202,AO DI LTD.
 * @file            xxx.c
 * @brief           xxxx
 * @author          xx
 * @date            2024/12/30
 * @remark          初修订
 *****************************************************************************/
#ifndef APP_TASK_REGEDIT_H
#define APP_TASK_REGEDIT_H

#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>

#include <cJSON.h>
#include "kit_data.h"
#include "kit_log.h"
#include "utextend.h"
#include "kit_db.h"
#include "bsp_ntp.h"
#include "bsp_rtdb.h"
#include "bsp_modbusSlave.h"
#include "modbus_comm.h"
#include "bsp_can_sched.h"
#include "app_data_storage.h"
#include "app_cool_crtl.h"
#include "app_timer.h"
#include "bsp_bms_com.h"
#include "protocol_bms.h"
#include "logic_do.h"
#include "logic_di.h"
#include "app_bms_ctrl.h"



#define THREAD_PRIORITY_MAX 90
#define INIT_ERR_DB 1
#define INIT_ERR_SCU 2
#define INIT_ERR_DEV 4
#define INIT_ERR_NORTHCFG 8

// typedef void* (*RegeditThreadMan)(void* arg);
// typedef struct
// {
// 	RegeditThreadMan getapi;
// } ThreadMan_T; // modify by wp at 2023-03-01 删除优先级设置

// uint8_t initSystem(void* arg);
// void regedit_thread_entry(void* map_t);
extern proto_dev_point_map_t protoTable[kProto_Master_End];

/*********************************************************************
 * @brief       SCU初始化
 * @param[in]   arg：相关配置
 * @return      0-成功  1-失败
 *********************************************************************/
uint8_t initConSystem(void *arg);
/*********************************************************************
 * @brief       创建任务的总入口
 * @param[in]   arg：相关配置
 * @return      void
 *********************************************************************/
void regeditThreadEntry(void *map_t); // 创建线程入口
/*********************************************************************
 * @brief       启动SCU的web后台
 * @return      void
 *********************************************************************/
// void runWebApi();

#endif // APP_TASK_REGEDIT_H
