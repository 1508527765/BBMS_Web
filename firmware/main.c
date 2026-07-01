/*****************************************************************************
 * @copyright       2024-2024,AO DI LTD.
 * @file            main.c
 * @brief           主进程入口文件
 * @author          Gary
 * @date            2024/12/30
 * @remark          初修订
 *****************************************************************************/

#include "app_task_regedit.h"
#include "bsp_rtdb.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

// 打印版本信息（格式可自定义）
void print_version()
{
    // 现代方式（推荐）：使用数字格式符，与 CMake 中定义的整数类型匹配
    printf("Version: %s.%s.%s\n", MAJOR_VERSION, MINOR_VERSION, BUILD_VERSION);
    printf("Copyright (C) 2026 Odin Power Corp.\n"); // 可选：添加版权信息
}
/**
 * @brief 启用 core dump 功能，用于程序崩溃时的调试
 *
 * @note 仅在 Debug 模式下启用 coredump，Release 模式下直接返回
 * @note 根据 NDEBUG 宏判断：CMake 在 Release 模式下会自动定义 NDEBUG
 */
void enable_coredump()
{
#ifdef NDEBUG
    // Release 模式：NDEBUG 已定义，直接返回，不启用 coredump
    return;
#endif

    // Debug 模式：启用 coredump，无限大小，便于调试
    struct rlimit core_limit;
    core_limit.rlim_cur = RLIM_INFINITY;
    core_limit.rlim_max = RLIM_INFINITY;

    if (setrlimit(RLIMIT_CORE, &core_limit) != 0)
    {
        perror("setrlimit(RLIMIT_CORE) failed");
        // 非致命错误，继续执行
    }
}

static uint16_t heartCount = 0;
static uint16_t curheartCount = 0;

/*****************************************************************************
 * @brief           主函数入口
 * @param[in]       argc：命令行参数个数
 * @param[in]       argv：命令行参数字符数组
 * @return          返回状态码
 *****************************************************************************/
int main(int argc, char *argv[])
{
    // 解析命令行参数
    for (int i = 1; i < argc; i++)
    {
        // 检测 --version 或 -v 参数
        if (strcmp(argv[i], "--version") == 0 || strcmp(argv[i], "-v") == 0)
        {
            print_version();
            return 0; // 输出后直接退出，不执行主逻辑
        }
    }
    enable_coredump();

    int ret = 0;
    void *arg = NULL; // 初始化参数

    // 控制器初始化
    ret = initConSystem(arg);

    // 注册线程入口
    if (ret == 0)
    {
        regeditThreadEntry(arg);
    }

    //task_6000();
    while (1)
    {
        // 添加进程心跳
        setRtdbPointValue(rtdbType, kDev_Type_Con, 0, kCon_RunHeartCount, heartCount++);
        curheartCount = (uint32_t)getRtdbPointValue(rtdbType, kDev_Type_Con, 0, kCon_RunHeartCount);
        printf("获取当前系统心跳：%u\n", curheartCount);
        if(heartCount > 65535)
        {
            heartCount = 0;
        } 
        sleep(2); // 每6秒检查一次
    }
    
}