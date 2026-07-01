/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2025/01-04 11:32:56
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2025/01-05 14:10:48
 * @FilePath: /_EMS_C_V1.0.0/bsp/bsp_shm.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 */
/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            bsp_shm.h
 * @brief           共享内存接口封装
 * @author          Gary
 * @date            2025/01-03
 * @remark
 *****************************************************************************/
#ifndef BSP_SHM_H
#define BSP_SHM_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
#include "kit_data.h"
#include "bsp_parse.h"
#include "kit_log.h"
#include "kit_model.h"
#define IPC_WEB_SHEM_KEY 3000                    // WEB端设置实时通知C端
#define IPC_WEB_DSP_MSG_KEY IPC_WEB_SHEM_KEY + 1 // WEB端设置实时通知C端
#define CON_KERNEL_KEY 2000                      // RTDB实时库的测点属性缓存

#define ASSERT_SHM_ENABLE 0

typedef enum
{
    kEE_SHM_READ, // 0
    kEE_SHM_CREAT // 1
} shm_creat_type_e;

typedef enum
{
    SHM_OK,
    SHM_ERROR
} shm_state_e;

// 设备的基本测点结构
typedef struct
{
    point_t *sPoint;               // 考虑指针字节对齐，放前面
    uint32_t id;                   // 设备编号
    uint32_t num;                  // 测点总数
    char name[MAX_NAME_BUF_LEN];   // 设备中文名称
    char enname[MAX_NAME_BUF_LEN]; // 设备英文名称
} devPoint_t;

// 某一个设备类型所有设备
typedef struct
{
    devPoint_t *sDevPoint; // MAX_SDEV_MAP_NUM 此处的测点索引 = 测点ID - 1，二者关联相等
    uint32_t id;           // 设备类型
    uint32_t num;          // 该设备类型下的设备总数
} devPointType_t;

// 所有的设备类型
typedef struct
{
    uint32_t num;                // 设备类型总数
    devPointType_t *sDCodePoint; // MAX_DEVCODE_MAP_NUM 此处的测点索引 = 测点ID - 1，二者关联相等
} DEVPOINT_RTDB_T;

// #--------------------------------------------------------------------------------------------
// 全局变量web配置变化标志
typedef struct
{
    uint16_t reserver[40]; // 占位用，webSign_t占用40*2=80个字节,固定不变
} webSign_t;

// 全局变量Web配置变化索引
typedef enum
{
    kSign_Start = 0,
    kSign_SysReboot = kSign_Start, // 系统reboot标记
    kSign_SysSyncTime,             // 系统对时
    kSign_SysSetIp,                // 系统IP配置
    kSign_SysUpdate,               // 本机系统软件升级
    kSign_BBMSCtrlUpdate,          // BBMS控制配置更新
    kSign_BBMSAlarmUpdate,         // BBMS告警配置更新
    kSign_DiLogicConfig,         // DI策略配置参数
    kSign_DoLogicConfig,         // DO策略配置参数
    kSign_ExportCurDayLog,         // 导出当前数据信号
    kSign_Rev40 = kSign_Start + 39,
    kSign_End
} cfg_web_sign_e;

#define MAX_CONFIG_ITEMS 256
#define MAX_DSP_NUM 8 // 单边设备的最大台数
typedef struct
{
    uint8_t ready_flag;                         // Web 写入完成后设为 1，C 处理完成后设为 0,后者c遇到错误也会设为0
    uint8_t dsp_select_cpu;                     // 选择的 CPU，0 表示 CPU0，1 表示 CPU1
    uint8_t cur_dsp_id;                         // 当前操作的 DSP ID，从 0 开始
    uint16_t item_count;                        // 配置项个数
    uint16_t config_ids[MAX_CONFIG_ITEMS];      // Web 写入的配置项 ID 列表
    uint8_t config_result[MAX_CONFIG_ITEMS];    // C处理结果：0=失败，1=成功（可选）
    UpgradeError upgrade_ret[2 * MAX_DSP_NUM];  // 16台升级返回结果
    UpgradeError autoaddr_ret[2 * MAX_DSP_NUM]; // 自动编址返回结果
} cfg_dsp_msg_mem_t;

/*****************************************************************************
 * @brief       初始化用于web与c写配置变化的通知组件
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   arg: 所有测点属性的指针
 * @return      0-成功  1失败
 *****************************************************************************/
uint16_t readWebSignShm(cfg_web_sign_e signType);

/*****************************************************************************
 * @brief       读取web与c写配置变化的通知组件
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   signType: 通知的地址
 * @return      0-成功  1失败
 *****************************************************************************/
void writeWebSignShm(cfg_web_sign_e signType, uint16_t val);

/*****************************************************************************
 * @brief       写入web与c写配置变化的通知组件
 * @param[in]   type: 实时库的类型
 * @param[in]   signType: 通知的地址
 * @param[in]   val: 对应的值
 * @return      0-成功  1失败
 *****************************************************************************/
int initWebSignShm(shm_creat_type_e type);

/*****************************************************************************
 * @brief       初始化rtdb
 * @param[in]   type: 实时库的类型
 * @param[in]   arg: 所有测点属性的指针
 * @return      0-成功  1失败
 *****************************************************************************/
int initRtdbShm(void *arg, shm_creat_type_e type);

/*****************************************************************************
 * @brief       获取RTDB中的值
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   devType: 设备类型
 * @param[in]   devId: 设备序号 从0开始
 * @param[in]   pointId: 该设备中点号，从0开始
 * @return      获取值
 *****************************************************************************/
double getShmPointValue(uint16_t devType, uint16_t devId, uint16_t pointId);

/*****************************************************************************
 * @brief       获取RTDB中的设备值
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   devType: 设备类型
 * @param[in]   devId: 设备序号 从0开始
 *****************************************************************************/
point_t *getShmDevValue(uint16_t devType, uint16_t devId);

/*****************************************************************************
 * @brief       向RTDB中写入值
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   devType: 设备类型
 * @param[in]   devId: 设备序号 从0开始
 * @param[in]   pointId: 该设备中点号，从0开始
 * @param[in]   value: 向RTDB中写入的值
 *****************************************************************************/
void setShmPointValue(uint16_t devType, uint16_t devId, uint16_t pointId, double value);

#endif // BSP_SHM_H