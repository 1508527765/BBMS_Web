/*****************************************************************************
 * @copyright       1997-2050,AO DI LTD.
 * @file            drv_wifi.h
 * @brief           wifi功能
 * @author          Gary
 * @date            2024/12/13
 * @remark
 *****************************************************************************/
#ifndef DRV_WIFI_H
#define DRV_WIFI_H
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>
#include <drv_comm.h>
#include "kit_log.h"
#include "kit_core.h"

#define MAX_WIFI_NAME_LEN 40		// wifi名称最大长度
#define MAX_WIFI_PASS_LEN 40		// 密码最大长度

/*用于连接Wifi的结构体*/
typedef struct
{
    uint8_t wifiName[MAX_WIFI_NAME_LEN];     // wifi名称
    uint8_t wifiPassword[MAX_WIFI_PASS_LEN]; // wifi密码
    uint8_t enable;                          // 是否使能：0否，1是
} drv_wifi_t;

/*****************************************************************************
 * @brief       打开wifi
 * @param[in]   item: wifi驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvOpenWifi(drv_wifi_t *item);

/*****************************************************************************
 * @brief       关闭wifi
 * @param[in]   item: wifi驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvCloseWifi(drv_wifi_t *item);

#endif // DRV_WIFI_H