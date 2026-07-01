/*****************************************************************************
 * @copyright       1997-2050,AO DI LTD.
 * @file            drv_wifi.c
 * @brief           wifi功能
 * @author          Gary
 * @date            2024/12/13
 * @remark
 *****************************************************************************/
#include "drv_wifi.h"

/*****************************************************************************
 * @brief       打开wifi
 * @param[in]   item: wifi驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvOpenWifi(drv_wifi_t *item)
{
    if (item == NULL)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "打开wifi失败");
    }

    char cmd[128] = {0};    // 执行命令内容
    char buffer[256] = {0}; // 接收执行的返回值

    if (kit_popen_exec("modprobe moal", buffer, sizeof(buffer)) == 0)
    {
        sprintf(cmd, "nmcli dev wifi connect \"%s\" password \"%s\"", item->wifiName, item->wifiPassword);
        return kit_popen_exec(cmd, buffer, sizeof(buffer));
    }
    else
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "加载wifi模块出现问题");
        return 1;
    }
}

/*****************************************************************************
 * @brief       关闭wifi
 * @param[in]   item: wifi驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvCloseWifi(drv_wifi_t *item)
{
    return 0;
}
