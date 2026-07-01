/*****************************************************************************
 * @copyright       1997-2050,AO DI LTD.
 * @file            kit_core.h
 * @brief           二次封装
 * @author          Gary
 * @date            2024/01/15
 * @remark
 *****************************************************************************/
#ifndef PROTOCOL_CAN_COMM_H
#define PROTOCOL_CAN_COMM_H

#include <unistd.h>
#include "drv_can.h"
#include "bsp_rtdb.h"
#include "kit_log.h"
#include <math.h>
#include "kit_math.h"
#include "kit_model.h"
#include "drv_tcp_server.h"
#include "app_cool_crtl.h"
#include "app_bms_statistics.h"

extern TMS_Status_t tms_status; // 全局TMS状态变量
extern BMS_To_TMS_RTC_t tms_rtc; // 全局TMS RTC时间变量
extern BMS_To_TMS_Cmd_t tms_cmd; // 全局TMS控制指令变量
extern TMS_Target_t tms_target; // 全局TMS目标变量

void protocol_can_tms_init(uint32_t base_time, can_dev_e can);


#endif // PROTOCOL_CAN_COMM_H