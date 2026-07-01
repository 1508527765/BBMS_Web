/*****************************************************************************
 * @copyright       1997-2050,AO DI LTD. 
 * @file            kit_core.h
 * @brief           二次封装
 * @author          Gary
 * @date            2024/12/13
 * @remark          
 *****************************************************************************/
#ifndef KIT_CORE_H
#define KIT_CORE_H

#define RUNIN_MAC 0 // 实机运行
#define RUNIN_VM 1  // 虚拟机

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <semaphore.h>
#include "kit_log.h"

extern sem_t event_semaphore_pcs_dist; // 功率分配使用
extern sem_t event_semaphore_pcs_reac_dist; // 无功分配使用
extern sem_t event_semaphore_pcs_switch; // 开关机控制使用
// 定义一个结构体表示一个 CPU 时间
typedef struct cpu_time {
    uint64_t user;   // 用户态时间
    uint64_t nice;   // 低优先级用户态时间
    uint64_t system; // 内核态时间
    uint64_t idle;   // 空闲时间
    uint64_t iowait; // 等待 I/O 完成时间
    uint64_t irq;    // 硬中断时间
    uint64_t softirq;// 软中断时间
    uint64_t steal;  // 被其他虚拟机偷走的时间
} cpu_time_t;

// 定义一个结构体表示内存信息
typedef struct memory_info {
    uint64_t total;   // 总内存大小
    uint64_t free;    // 空闲内存大小
    uint64_t available; // 可用内存大小
    uint64_t buffers; // 缓冲区内存大小
    uint64_t cached;  // 缓存内存大小
} memory_info_t;


// 全局信号量功率分配使用
extern sem_t event_semaphore_pcs_dist;

/*****************************************************************************
 * @brief           通过管道执行shell命令 (使用 system)
 * @param[in]       cmd：shell命令
 * @param[in]       buffer：读取执行结果
 * @param[in]       buffer_size：读取执行结果长度
 * @return          1-执行错误  0-执行成功
 *****************************************************************************/
int kit_popen_exec(const char *cmd, char* buffer, size_t buffer_size);
/*****************************************************************************
 * @brief           获取SCU的SN号
 * @param[in]       serial：返回SN号
 * @return          1-执行错误  0-执行成功
 *****************************************************************************/
int kit_get_scu_sn(char serial[128]);

/*****************************************************************************
 * @brief           
 * @param[in]       
 * @return          1-执行错误  0-执行成功
 *****************************************************************************/
void getSysCpuTime(cpu_time_t *time);

/*****************************************************************************
 * @brief           
 * @param[in]       
 * @return          1-执行错误  0-执行成功
 *****************************************************************************/
void getProCpuTime(cpu_time_t *time, pid_t pid);

/*****************************************************************************
 * @brief           
 * @param[in]       
 * @return          1-执行错误  0-执行成功
 *****************************************************************************/
void getMemInfo(memory_info_t *info);

/*****************************************************************************
 * @brief           
 * @param[in]       
 * @return          1-执行错误  0-执行成功
 *****************************************************************************/
double calculateProcMemUsage(memory_info_t info, pid_t pid);

/*****************************************************************************
 * @brief           获取SCU的SN号
 * @param[in]       serial：返回SN号
 * @return          1-执行错误  0-执行成功
 *****************************************************************************/
int kit_get_ems_sn(char serial[50]);

/*********************************************************************
 * @brief       设置系统时间
 * @param[in]   sec: 1970年以来的秒数
 * @param[in]   linuxPass: linux密码
 * @return      0：成功；1：失败
 *********************************************************************/
int setSysTime(time_t sec, char linuxPass[50]);

/*********************************************************************
 * @brief       初始化信号量
 * @return      无
 *********************************************************************/
int initSysSemaphore(void);

void kit_creat_sys_folder(const char *base_path, const char *date_str);


int isDirectoryExists(const char *dir_path);
int createDirectoryIfNotExists(const char *dir_path);
int delete_old_dirs(const char *abs_basepath, int days);
int manageDirectoryFileCount(const char *dir_name, int max_files);

#endif // KIT_CORE_H