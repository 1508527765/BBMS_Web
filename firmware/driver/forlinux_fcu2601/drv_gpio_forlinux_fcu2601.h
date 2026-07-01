/*****************************************************************************
 * @copyright       2024-2024,AO DI LTD.
 * @file            drv_gpio_forlinux_fcu2601.h
 * @brief           飞凌fc2601的gpio驱动
 * @author          Gary
 * @date            2024/12/09
 * @remark
 *****************************************************************************/
#ifndef DRV_GPIO_FORLINUX_FCU2601_H
#define DRV_GPIO_FORLINUX_FCU2601_H

#include <stdint.h>
#include <drv_comm.h>

// forlinux的fcu2601的gpio序号
typedef enum
{
	kForlinux_FC2601_DI_Start = 1,						   // DI开始值
	kForlinux_FC2601_DI1 = kForlinux_FC2601_DI_Start,	   // 数字输入引脚1
	kForlinux_FC2601_DI2,								   // 数字输入引脚2
	kForlinux_FC2601_DI3,								   // 数字输入引脚3
	kForlinux_FC2601_DI4,								   // 数字输入引脚4
	kForlinux_FC2601_DI5,								   // 数字输入引脚5
	kForlinux_FC2601_DI6,								   // 数字输入引脚6
	kForlinux_FC2601_DI7,								   // 数字输入引脚7
	kForlinux_FC2601_DI8,								   // 数字输入引脚8
	kForlinux_FC2601_DI_End,							   // DI结束值
	kForlinux_FC2601_DO_Start = 41,						   // DO开始值
	kForlinux_FC2601_DO1 = kForlinux_FC2601_DO_Start,	   // 数字输出引脚1
	kForlinux_FC2601_DO2,								   // 数字输出引脚2
	kForlinux_FC2601_DO3,								   // 数字输出引脚3
	kForlinux_FC2601_DO4,								   // 数字输出引脚4
	kForlinux_FC2601_DO5,								   // 数字输出引脚5
	kForlinux_FC2601_DO6,								   // 数字输出引脚6
	kForlinux_FC2601_DO_End,							   // DO结束值
	kForlinux_FC2601_LED_Start = 81,					   // LED开始值
	kForlinux_FC2601_LED1 = kForlinux_FC2601_LED_Start,	   // 用户自定义LED灯1
	kForlinux_FC2601_LED2,								   // 用户自定义LED灯2
	kForlinux_FC2601_LED3,								   // 用户自定义LED灯3
	kForlinux_FC2601_LED4,								   // 用户自定义LED灯4
	kForlinux_FC2601_LED_End,							   // LED结束值
	kForlinux_FC2601_Reserve_Start = 101,				   // 保留开始值
	kForlinux_FC2601_Key = kForlinux_FC2601_Reserve_Start, // 键盘引脚
	kForlinux_FC2601_Pow,								   // 系统掉电检测
	kForlinux_FC2601_Reserve_End						   // 保留结束值
} gpio_forlinux_fcu2601_pin_e;

/*****************************************************************************
 * @brief       打开GPIO设备
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvGpioFlFcu2601Open(void);
/*****************************************************************************
 * @brief       关闭GPIO设备
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvGpioFlFcu2601Close(void);
/*****************************************************************************
 * @brief       写入gpio设备值
 * @param[in]   gpioId: forlinux的fcu2601的gpio序号
 * @param[in]   gpioValue: gpio值
 * @return      0-成功  -1-打开文件失败  -2无效GPIO ID或路径错误  -3无效GPIO值  -4写入失败  -5写入数据长度不匹配  -6关闭文件失败
 *****************************************************************************/
int drvGpioFlFcu2601Write(int gpioId, int gpioValue);
/*****************************************************************************
 * @brief           读取GPIO值
 * @param[in]       gpioId: forlinux的fcu2601的gpio序号
 * @return          0-成功 -1打开文件失败  -2无效GPIO ID或路径错误  -3读取失败  -4读取数据长度不匹配  -5关闭文件失败
 *****************************************************************************/
int drvGpioFlFcu2601Read(int gpioId);

#endif /* DRV_GPIO_FORLINUX_FCU2601_H */
