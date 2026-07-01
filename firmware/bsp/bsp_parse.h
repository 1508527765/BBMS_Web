
/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            bsp_parse.h
 * @brief           解析所有配置文件的入口
 * @author          Gary
 * @remark
 *****************************************************************************/
#ifndef APP_PARSE_H
#define APP_PARSE_H
#include <stdio.h>
#include <sched.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include "kit_model.h"
#include "kit_data.h"
#include "app_comm.h"
#include "utextend.h"
#include "kit_db.h"
#include "kit_log.h"
#include <cjson/cJSON.h>
#include "kit_core.h"

extern proto_dev_point_map_t protoTable[kProto_Master_End];

// proto_dev_point_map_t *find_node_by_protocol_and_port(int protocolType, int portId);

/*********************************************************************
 * @brief       根据设备类型和设备ID获取设备的点位数组
 * @param[in]   devType 设备类型
 * @param[in]   devId 设备ID
 * @return      返回匹配设备的点位数组，未找到返回 NULL
 *********************************************************************/
point_t *get_pointArr(dev_type_e devType, uint16_t devId);

/*********************************************************************
 * @brief       读取系统配置的设备数量
 * @param[in]   item: 接入EMS的所有设备类型数量
 * @return      ret
 *********************************************************************/
int initDevInfo();

/*********************************************************************
 * @brief       读取系统配置的北向协议
 * @param[in]   none
 * @return      ret
 *********************************************************************/
int initNorthInfo();

/*********************************************************************
 * @brief       读取高级设置的信息
 * @return      0-成功  1-失败
 *********************************************************************/
int initAdvancedSettingInfo();

/*********************************************************************
 * @brief       根据设备类型和设备ID获取设备的点位数组
 * @param[in]   devType 设备类型
 * @param[in]   devId 设备ID
 * @return      返回匹配设备的点位数组，未找到返回 NULL
 *********************************************************************/
dev_info_t *get_devPointMapArr(dev_type_e devType, uint16_t devArrayId);
#endif // APP_PARSE_H