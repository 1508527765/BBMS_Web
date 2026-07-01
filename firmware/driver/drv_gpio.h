/*****************************************************************************
 * @copyright       2024-2024,AO DI LTD.
 * @file            drv_gpio.h
 * @brief           gpio设备驱动程序
 * @author          Gary
 * @date            2024/12/14
 * @remark          初修订
 *****************************************************************************/
#ifndef DRY_GPIO_H_
#define DRY_GPIO_H_

#include <stdint.h>
#include "drv_comm.h"
#include "forlinux_fcu2601/drv_gpio_forlinux_fcu2601.h"

#define FORLINUX_FCU2601 1    // FCU2601 A55

// DI口、DO口、LED
typedef enum
{
    DI_Start = 1, // 起始值
    DI1 = DI_Start,
    DI2,
    DI3,
    DI4,
    DI5,
    DI6,
    DI7,
    DI8,
    DI9,
    DI10,
    DI11,
    DI12,
    DI13,
    DI14,
    DI15,
    DI16,
    DI17,
    DI18,
    DI19,
    DI20,
    DI21,
    DI22,
    DI23,
    DI24,
    DI25,
    DI26,
    DI27,
    DI28,
    DI29,
    DI30,
    DI31,
    DI32,
    DI33,
    DI34,
    DI35,
    DI36,
    DI37,
    DI38,
    DI39,
    DI40,
    DI_End,
    DO_Start = 41,
    DO1 = DO_Start,
    DO2,
    DO3,
    DO4,
    DO5,
    DO6,
    DO7,
    DO8,
    DO9,
    DO10,
    DO11,
    DO12,
    DO13,
    DO14,
    DO15,
    DO16,
    DO17,
    DO18,
    DO19,
    DO20,
    DO21,
    DO22,
    DO23,
    DO24,
    DO25,
    DO26,
    DO27,
    DO28,
    DO29,
    DO30,
    DO31,
    DO32,
    DO33,
    DO34,
    DO35,
    DO36,
    DO37,
    DO38,
    DO39,
    DO40,
    DO_End,
    LED_Start = DO_End,
    LED1 = LED_Start,
    LED2,
    LED3,
    LED4,
    LED5,
    LED6,
    LED7,
    LED8,
    LED9,
    LED10,
    LED_End,
} di_do_led_e;

/*****************************************************************************
 * @brief       打开gpio设备驱动
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvGpioOpen(void);

/*****************************************************************************
 * @brief       关闭gpio设备驱动
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvGpioClose(void);

/*****************************************************************************
 * @brief       写gpio设备驱动
 * @param[in]   gpio: gpio序列号
 * @param[in]   value: gpio值
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvGpioWrite(di_do_led_e gpio, int value);

/*****************************************************************************
 * @brief       读取gpio设备驱动
 * @param[in]   gpio: gpio序列号
 * @return      0-低电平  1-高电平  2-失败
 *****************************************************************************/
int drvGpioRead(di_do_led_e gpioId);

#endif