/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            app_timer.c
 * @brief           定时器相关头文件
 * @author          Gary
 * @date            2025-10-12
 * @remark
 *****************************************************************************/

#include "app_timer.h"
#include "modbus_comm.h"
#include "app_comm.h"
#include "app_bms_statistics.h"

#define SCU_VERSION_DB_ID 1

// 升级SCU版本号到数据库,0成功，1失败
int app_update_scu_version_record(void)
{
    // 扩大安全缓冲区
    char scu_version[128] = {0};
    snprintf(scu_version, sizeof(scu_version),"bbms_firmware_v%d.%d.%d", MAJOR_VERSION,MINOR_VERSION,BUILD_VERSION);
    printf("scu_version: %s\n", scu_version);
    return kit_update_pcs_soft_version(SCU_VERSION_DB_ID, NULL, NULL, NULL, scu_version);
}

int app_scu_soft_update(void)
{
    char cmd[256] = {0};
    char buffer[4096] = {0};
    const char *fireware_path = "/opt/firmware";
    char deb_file[256] = {0};
    DIR *dir;
    struct dirent *entry;
    int deb_count = 0;

    // 扫描目录，查找DEB文件
    dir = opendir(fireware_path);
    if (dir == NULL)
    {
        printf("Error: Cannot open directory %s\n", fireware_path);
        return -1;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        // 查找.deb文件
        if (strstr(entry->d_name, ".deb") != NULL)
        {
            deb_count++;
            snprintf(deb_file, sizeof(deb_file), "%s/%s", fireware_path, entry->d_name);

            // 如果找到多个DEB文件，报错
            if (deb_count > 1)
            {
                printf("Error: Multiple DEB files found in %s, please keep only one!\n", fireware_path);
                closedir(dir);
                return -1;
            }
        }
    }
    closedir(dir);

    // 检查是否找到DEB文件
    if (deb_count == 0)
    {
        printf("Error: No DEB file found in %s\n", fireware_path);
        return -1;
    }

    printf("Found DEB file: %s\n", deb_file);

    // 执行安装命令
    // snprintf(cmd, sizeof(cmd), "echo \"forlinx\" | sudo -S dpkg -i %s", deb_file);
    snprintf(cmd, sizeof(cmd), "echo \"%s\" | sudo -S  dpkg -i %s", "forlinx", deb_file);
    KITLOG(LOG_APP_EN, INFO_EN, "Executing SCU firmware upgrade command: %s", cmd);
    int ret = kit_popen_exec(cmd, buffer, sizeof(buffer));

    // 安装成功后删除DEB文件
    if (ret == 0)
    {
        // snprintf(cmd, sizeof(cmd), "echo forlinx | sudo -S rm -f %s", deb_file);
        // kit_popen_exec(cmd, buffer, sizeof(buffer));

        // snprintf(cmd, sizeof(cmd), "echo forlinx | sudo reboot");
        // kit_popen_exec(cmd, buffer, sizeof(buffer));
    }
    else
    {
        printf("Installation failed, keeping DEB file for debugging\n");
    }

    return ret;
}

/*****************************************************************************
 * @brief           升级工作线程
 * @param[in]       arg：线程执行参数
 *****************************************************************************/
static void *upgrade_worker(void *arg)
{
    KITLOG(LOG_APP_EN, INFO_EN, "开始执行固件升级...");

    // 执行升级
    int result = app_scu_soft_update();

    if (result == 0)
    {
        // 此时进程已经被杀掉了，不会执行下面的语句
        KITLOG(LOG_APP_EN, INFO_EN, "固件升级成功，准备重启系统");
    }
    else
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "固件升级失败，错误码: %d", result);
        // 升级失败时不重启，保持系统运行
    }

    return NULL;
}

/*****************************************************************************
 * @brief           每1秒钟执行一次的任务
 * @param[in]       arg：线程执行参数
 *****************************************************************************/
static void *timer1s_task(void *arg)
{

    uint16_t update_scu_fireware_flag = 0;    // SCU固件升级标志
    if (app_update_scu_version_record() == 0) // 更新SCU版本号到数据库
    {
        KITLOG(LOG_APP_EN, INFO_EN, "更新SCU版本号到数据库成功");
    }
    else
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "更新SCU版本号到数据库失败");
    }
    while (1)
    {
        // 1、scu固件升级
        update_scu_fireware_flag = readWebSign(kSign_ShMem, kSign_SysUpdate);
        if (update_scu_fireware_flag == 1)
        {
            writeWebSign(kSign_ShMem, kSign_SysUpdate, 0); // 清除升级标志，防止重复升级
            setRtdbPointValue(rtdbType, kDev_Type_Con, 0, kCon_UpgradeConPercent, 100);
            // 创建独立的升级线程,防止阻塞主循环
            pthread_t upgrade_thread;
            if (pthread_create(&upgrade_thread, NULL, upgrade_worker, NULL) != 0)
            {
                KITLOG(LOG_APP_EN, ERROR_EN, "创建升级线程失败");
            }
            else
            {
                pthread_detach(upgrade_thread); // 分离线程，自动回收资源
            }
        }

        // 堆级统计 → RTDB（EMS / 上位机 Modbus 502 共用映射的数据源）
        bms_bank_statistics_update(0);

        // checkEMSOnlineTimeCall(); // 分布式下离线监测执行，使用定时任务，减少线程建立

        sleep(1); // 每秒执行一次
    }
}

/*****************************************************************************
 * @brief           故障统计任务线程（每秒执行一次）
 * @param[in]       arg：线程执行参数
 *****************************************************************************/
static void *fault_statistics_task(void *arg)
{
    KITLOG(LOG_APP_EN, INFO_EN, "故障统计任务线程启动");

    while (1)
    {
        // 获取故障统计信息（包含故障数量、总故障数和故障级别）
        // if (bsp_fault_get_statistics(&fault_stat) == 0)
        if (1)
        {
            ;
        }
        else
        {
            KITLOG(LOG_APP_EN, ERROR_EN, "获取故障统计信息失败");
        }

        sleep(1); // 每秒执行一次
    }

    return NULL;
}

/*********************************************************************
 * @brief       创建定时器任务
 *********************************************************************/
void creatTimerTask(void)
{
    pthread_t timer_thread;
    if (pthread_create(&timer_thread, NULL, timer1s_task, NULL) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "创建定时器任务线程失败");
    }
}

/*********************************************************************
 * @brief       创建故障统计任务
 *********************************************************************/
void creatFaultStatisticsTask(void)
{
    pthread_t fault_stat_thread;
    if (pthread_create(&fault_stat_thread, NULL, fault_statistics_task, NULL) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "创建故障统计任务线程失败");
    }
    else
    {
        KITLOG(LOG_APP_EN, INFO_EN, "故障统计任务线程创建成功");
    }
}
