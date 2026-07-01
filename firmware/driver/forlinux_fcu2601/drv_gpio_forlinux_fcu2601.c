/*****************************************************************************
 * @copyright       2024-2024,AO DI LTD.
 * @file            drv_gpio_zlg_em1000.c
 * @brief           飞凌fc2601的gpio驱动
 * @author          Gary
 * @date            2024/12/14
 * @remark          初修订
 *****************************************************************************/
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/stat.h>
#include "kit_log.h"
#include "drv_comm.h"
#include "drv_gpio_forlinux_fcu2601.h"


// GPIO信息结构体
typedef struct
{
    int id;          // GPIO ID
    const char* path; // sysfs路径
} gpio_info_t;

// GPIO查找表
const gpio_info_t gpioLookup[] =
{
    {kForlinux_FC2601_DI1, "/sys/class/gpio-input/DIN1/state"},
    {kForlinux_FC2601_DI2, "/sys/class/gpio-input/DIN2/state"},
    {kForlinux_FC2601_DI3, "/sys/class/gpio-input/DIN3/state"},
    {kForlinux_FC2601_DI4, "/sys/class/gpio-input/DIN4/state"},
    {kForlinux_FC2601_DI5, "/sys/class/gpio-input/DIN5/state"},
    {kForlinux_FC2601_DI6, "/sys/class/gpio-input/DIN6/state"},
    {kForlinux_FC2601_DI7, "/sys/class/gpio-input/DIN7/state"},
    {kForlinux_FC2601_DI8, "/sys/class/gpio-input/DIN8/state"},
    {kForlinux_FC2601_DO1, "/sys/class/leds/do1/brightness"},
    {kForlinux_FC2601_DO2, "/sys/class/leds/do2/brightness"},
    {kForlinux_FC2601_DO3, "/sys/class/leds/do3/brightness"},
    {kForlinux_FC2601_DO4, "/sys/class/leds/do4/brightness"},
    {kForlinux_FC2601_DO5, "/sys/class/leds/do5/brightness"},
    {kForlinux_FC2601_DO6, "/sys/class/leds/do6/brightness"},
    {kForlinux_FC2601_LED1, "/sys/class/leds/can0/brightness"},
    {kForlinux_FC2601_LED2, "/sys/class/leds/can1/brightness"},
    {kForlinux_FC2601_LED3, "/sys/class/leds/led1/brightness"},
    {kForlinux_FC2601_LED4, "/sys/class/leds/led2/brightness"},
    {kForlinux_FC2601_Key, "/sys/class/gpio-input/FWn/state"},
    {kForlinux_FC2601_Pow, "/sys/class/gpio-input/POW_DET/state"}
};

/*****************************************************************************
 * @brief           通过GPIO ID查找sysfs路径
 * @param[in]       gpioId：forlinux的fcu2601的gpio序号
 * @return          NULL：找不到路径，非NULL:找到的路径字符串
 *****************************************************************************/
static const char* findGpioPath(int gpioId)
{
    for (size_t i = 0; i < sizeof(gpioLookup) / sizeof(gpioLookup[0]); ++i)
    {
        if (gpioLookup[i].id == gpioId)
        {
            return gpioLookup[i].path;
        }
    }
    return NULL;
}

/*****************************************************************************
 * @brief           设置所有DI、DO和LED端口为低电平（0值）。
 * @return          0-成功，1-失败
 *****************************************************************************/
int setAllGpioLow()
{
    int ret;
    for (int i = kForlinux_FC2601_DO_Start; i < kForlinux_FC2601_DO_End; ++i)
    {
        ret = drvGpioFlFcu2601Write(i, 0);
        if (ret != 0)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "初始化GPIO %d 失败，错误码: %d\n", i, ret);
            return ret;
        }
    }
    for (int i = kForlinux_FC2601_LED_Start; i < kForlinux_FC2601_LED_End; ++i)
    {
        ret = drvGpioFlFcu2601Write(i, 0);
        if (ret != 0)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "初始化GPIO %d 失败，错误码: %d\n", i, ret);
            return ret;
        }
    }
    for (int i = kForlinux_FC2601_Reserve_Start; i < kForlinux_FC2601_Reserve_End; ++i)
    {
        ret = drvGpioFlFcu2601Write(i, 0);
        if (ret != 0)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "初始化GPIO %d 失败，错误码: %d\n", i, ret);
            return ret;
        }
    }

    return 0;
}

/*****************************************************************************
 * @brief           函数：验证路径是否存在且可访问
 * @param[in]       path：访问路径
 * @return          true：可访问，false：不可访问
 *****************************************************************************/
static bool validatePath(const char* path)
{
    struct stat buffer;
    return (stat(path, &buffer) == 0);
}

/*****************************************************************************
 * @brief       打开GPIO设备
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvGpioFlFcu2601Open(void)
{
    int ret = setAllGpioLow();
    return ret;
}

/*****************************************************************************
 * @brief       关闭初始化GPIO设备
 * @return      0-成功  1-失败
 *****************************************************************************/
int drvGpioFlFcu2601Close(void)
{
    return 0;
}

/*****************************************************************************
 * @brief       写入gpio设备值
 * @param[in]   gpioId: forlinux的fcu2601的gpio序号
 * @param[in]   gpioValue: gpio值
 * @return      0-成功  -1-打开文件失败  -2无效GPIO ID或路径错误  -3无效GPIO值  -4写入失败  -5写入数据长度不匹配  -6关闭文件失败
 *****************************************************************************/
int drvGpioFlFcu2601Write(int gpioId, int gpioValue)
{
    const char* path = findGpioPath(gpioId);
    int fd;
    int ret;
    char valueStr[2]; // 字符串缓冲区，足够存储"0"或"1"

    // 输入参数验证
    if (gpioValue != 0 && gpioValue != 1)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "错误: 无效的GPIO值，必须为0或1");
        return -3; // 无效GPIO值
    }

    // 路径验证
    if (path == NULL || !validatePath(path))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "错误: 无效的GPIO ID或路径不存在: %d", gpioId);
        return -2; // 无效GPIO ID或路径错误
    }

    snprintf(valueStr, sizeof(valueStr), "%d", gpioValue);

    fd = open(path, O_WRONLY);
    if (fd == -1)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "打开GPIO设备文件失败");
        return -1; // 打开文件失败
    }

    ret = write(fd, valueStr, strlen(valueStr));
    if (ret == -1)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "写入GPIO设备文件失败");
        return -4; // 写入失败
    }
    else if (ret != strlen(valueStr))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "错误: 写入数据长度不匹配");
        return -5; // 写入数据长度不匹配
    }

    if (close(fd) == -1)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "关闭GPIO设备文件失败");
        return -6; // 关闭文件失败
    }

    return 0; // 成功
}

/*****************************************************************************
 * @brief           读取GPIO值
 * @param[in]       gpioId: forlinux的fcu2601的gpio序号
 * @return          0-成功 -1打开文件失败  -2无效GPIO ID或路径错误  -3读取失败  -4读取数据长度不匹配  -5关闭文件失败
 *****************************************************************************/
int drvGpioFlFcu2601Read(int gpioId)
{
    const char* path = findGpioPath(gpioId);
    int fd;
    char fvalue;
    int ret;

    // 路径验证
    if (path == NULL || !validatePath(path))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "错误: 无效的GPIO ID或路径不存在: %d", gpioId);
        return -2; // 无效GPIO ID或路径错误
    }

    fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "打开GPIO设备文件失败");
        return -1; // 打开文件失败
    }

    ret = read(fd, &fvalue, 1);
    if (ret == -1)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "读取GPIO设备文件失败");
        return -3; // 读取失败
    }
    else if (ret != 1)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "错误: 读取数据长度不匹配");
        return -4; // 读取数据长度不匹配
    }

    if (close(fd) == -1)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "关闭GPIO设备文件失败");
        return -5; // 关闭文件失败
    }
    uint8_t value = fvalue - '0'; // 将字符'0'或'1'转换为整数0或1

    // if (gpioId >= kForlinux_FC2601_DI_Start && gpioId < kForlinux_FC2601_DI_End)
    // {
    //     return !value;
    // }

    return value;
}