/*****************************************************************************
 * @copyright       1997-2050,AO DI LTD. 
 * @file            drv_4g.c
 * @brief           xx功能
 * @author          Gary
 * @date            2024/12/13
 * @remark          
 *****************************************************************************/
#include "drv_4g.h"

/*****************************************************************************
 * @brief       打开4G
 * @param[in]   item: wifi驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvOpen4G(drv_4g_t* item)
{
    char buffer[256]; // 接收执行返回值
    // 使用 kit_popen_exec 执行 quectel-CM & 命令
    if (kit_popen_exec((const char *)item->cmdContent, buffer, sizeof(buffer)) != 0) 
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "%s 命令执行失败", item->cmdContent);
        return 1; // 返回错误
    }

    return 0; // 成功
}


/*****************************************************************************
 * @brief       关闭4G
 * @param[in]   item: 4G驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvClose4G(drv_4g_t* item)
{
    return 0;
}