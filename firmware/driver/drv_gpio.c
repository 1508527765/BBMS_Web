/*****************************************************************************
 * @copyright       2024-2024, conpany. POWER SUPPLY CO., LTD.
 * @file            drv_gpio.c
 * @brief           gpio设备驱动程序
 * @author          gary
 * @date            2025/08/04
 * @remark          初修订
 *****************************************************************************/
#include <stdio.h>
#include <stdint.h>
#include "drv_gpio.h"

/*****************************************************************************
 * @brief       打开gpio设备驱动
 * @return      0-成功  1失败
 *****************************************************************************/
int drvGpioOpen(void)
{
    int ret = 0;
    ret = drvGpioFlFcu2601Open();
    return ret;
}

/*****************************************************************************
 * @brief       关闭gpio设备驱动
 * @return      0-成功  1失败
 *****************************************************************************/
int drvGpioClose(void)
{
    int ret = 0;
    ret = drvGpioFlFcu2601Close();

    return ret;
}

/*****************************************************************************
 * @brief       写gpio设备驱动
 * @param[in]   gpio: gpio序列号
 * @param[in]   value: gpio值
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvGpioWrite(di_do_led_e gpio, int value)
{
    int ret = 0;
    ret = drvGpioFlFcu2601Write((int)gpio, value);

    return ret;
}

/*****************************************************************************
 * @brief       读取gpio设备驱动
 * @param[in]   gpio: gpio序列号
 * @return      0-低电平  1-高电平  2-失败
 *****************************************************************************/
int drvGpioRead(di_do_led_e gpioId)
{
    int ret = 0;

    ret = drvGpioFlFcu2601Read((int)gpioId);

    return ret;
}