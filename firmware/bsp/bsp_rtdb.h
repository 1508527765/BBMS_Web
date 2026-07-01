/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            bsp_rtdb.h
 * @brief           rtdb组件
 * @author          Gary
 * @date            2024/08/30
 * @remark          初修订
 *****************************************************************************/
#ifndef BSP_RTDB_H
#define BSP_RTDB_H

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
#include "kit_data.h"
#include "bsp_shm.h"
#include "bsp_redis.h"
#define RTDB_CREATE_MAX 5 // RTDB创建失败超时

typedef enum
{
    RTDB_OK,
    RTDB_ERROR
} rtdb_state_e;

typedef enum
{
    Rtdb_ShMem,
    Rtdb_Redis,
    Rtdb_End
} rtdb_type_e;

extern rtdb_type_e rtdbType;

/*****************************************************************************
 * @brief       初始化rtdb
 * @param[in]   type: 实时库的类型
 * @param[in]   arg: 所有测点属性的指针
 * @return      0-成功  1失败     
 *****************************************************************************/
uint8_t initRtdb(rtdb_type_e type,void* arg,shm_creat_type_e shmType);

/*****************************************************************************
 * @brief       获取RTDB中的值
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   devType: 设备类型
 * @param[in]   devId: 设备序号 从0开始
 * @param[in]   pointId: 该设备中点号，从0开始
 * @return      获取值
 *****************************************************************************/
double getRtdbPointValue(rtdb_type_e type, uint16_t devType, uint16_t devId, uint16_t pointId);

/*****************************************************************************
 * @brief       向RTDB中写入值
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   devType: 设备类型
 * @param[in]   devId: 设备序号 从0开始
 * @param[in]   pointId: 该设备中点号，从0开始
 * @param[in]   value: 向RTDB中写入的值
 *****************************************************************************/
void setRtdbPointValue(rtdb_type_e type, uint16_t devType, uint16_t devId, uint16_t pointId, double value);

/*****************************************************************************
 * @brief       初始化用于web与c写配置变化的通知组件
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   arg: 所有测点属性的指针
 * @return      0-成功  1失败     
 *****************************************************************************/
uint8_t initWebSign(sign_share_type_e rtype,shm_creat_type_e rwtype);

/*****************************************************************************
 * @brief       读取web与c写配置变化的通知组件
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   signType: 通知的地址
 * @return      0-成功  1失败     
 *****************************************************************************/
uint16_t readWebSign(sign_share_type_e rtype, cfg_web_sign_e signType);

/*****************************************************************************
 * @brief       写入web与c写配置变化的通知组件
 * @param[in]   type: 实时库的类型
 * @param[in]   signType: 通知的地址
 * @param[in]   val: 对应的值
 * @return      0-成功  1失败     
 *****************************************************************************/
void writeWebSign(sign_share_type_e rtype, cfg_web_sign_e signType, uint16_t val);


//---------------------------------------------------------------------DSP新增配置函数--------------------------------------------------------------------------------------------------
/*********************************************************************
 * @brief       初始化设置DSP消息共享内存
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @return      ture-成功  0-fail
 *********************************************************************/
int initWebDSPMsg(rtdb_type_e type,shm_creat_type_e type1);

/*********************************************************************
 * @brief       web将当前修改配置项的id写入到共享内存中
 * @param[in]   cfgIndex: 配置项索引，从0开始依次加1
 * @param[in]   value: 配置项的值，为数据库的id
 * @return      配置项个数
 *********************************************************************/
uint8_t writeWebDspCfgArrayId(rtdb_type_e type,uint8_t cfgIndex,uint16_t value);

/*********************************************************************
 * @brief       web将当前修改配置项的个数写入到共享内存中
 * @param[in]   cfgCount: 配置项个数
 * @return      0-成功 1-失败
 *********************************************************************/
uint8_t writeWebDspCfgIdCount(rtdb_type_e type,uint16_t cfgCount);

/*********************************************************************
 * @brief       获取升级结果
 * @param[in]   dspIndex: DSP索引
 * @return      获取值
 *********************************************************************/
UpgradeError readWebUpgradeRet(rtdb_type_e type,uint8_t dspIndex);

/*********************************************************************
 * @brief       获取自动编址结果
 * @param[in]   dspIndex: DSP索引
 * @return      获取值
 *********************************************************************/
UpgradeError readWebAutoAddrRet(rtdb_type_e type,uint8_t dspIndex);

/*********************************************************************
 * @brief       获取配置返回结果
 * @param[in]   cfgId: 配置id
 * @return      获取值
 *********************************************************************/
int16_t readWebDspCfgRet(rtdb_type_e type,uint8_t cfgId);

/*********************************************************************
 * @brief       写入升级的Web配置的cpu选项
 * @details     该函数用于设置共享内存中的CPU选择值。
 * @param[in]   cpu_value: CPU选择值，0表示CPU0，1表示CPU1
 * @note        如果传入的值不在0或1之间，则不会修改共享内存中的值
 *********************************************************************/
void writeWebCfgUpgradeCpu(rtdb_type_e type,uint16_t cpu_value);

/*********************************************************************
 * @brief       获取升级的Web配置的cpu选项
 * @return      cpu选项值
 *********************************************************************/
int16_t readWebCfgUpgradeCpu(rtdb_type_e type);

/*********************************************************************
 * @brief       获取当前操作的DSP ID
 * @return      当前操作的DSP ID
 * @details     该函数用于获取当前正在操作的DSP ID。
 *              如果g_DspSign为NULL，则返回0。
 * *********************************************************************/
uint16_t readWebCfgCurDspId(rtdb_type_e type);

/*********************************************************************
 * @brief       写入当前操作的DSP ID
 * @param[in]   dspId: 当前操作的DSP ID，从0开始
 * @details     该函数用于设置当前正在操作的DSP ID。
 *              如果g_DspSign为NULL或dspId超过最大DSP数量，则不进行任何操作。
 *********************************************************************/
void writeWebCfgCurDspId(rtdb_type_e type,uint8_t dspId);

#endif // BSP_RTDB_H