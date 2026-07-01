/*****************************************************************************
 * @copyright       1997-2050,AO DI LTD. 
 * @file            drv_4g.h
 * @brief           xx功能
 * @author          Gary
 * @date            2024/12/13
 * @remark          
 *****************************************************************************/
#ifndef DRV_4G_H
#define DRV_4G_H
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <drv_comm.h>
#include "kit_log.h"
#include "kit_core.h"

#define MAX_CMD_4G_LEN 40		// 4G开启执行命令最大长度

// 4G初始化命令内容
typedef struct
{
    uint8_t cmdContent[MAX_CMD_4G_LEN]; // 命令内容
    uint8_t enable;                     // 是否使能：0否，1是
} drv_4g_t;

/*****************************************************************************
 * @brief       打开4G
 * @param[in]   item: wifi驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvOpen4G(drv_4g_t* item);

/*****************************************************************************
 * @brief       关闭4G
 * @param[in]   item: 4G驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvClose4G(drv_4g_t* item);


#endif // DRV_4G_H