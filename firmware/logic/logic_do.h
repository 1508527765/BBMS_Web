/*****************************************************************************
 * @copyright       1997-2050, Gary. POWER SUPPLY CO., LTD. 
 * @file            logic_dido.h
 * @brief           dido功能
 * @author          gary
 * @date            2026-04-20
 * @remark          
 *****************************************************************************/

#ifndef LOGIC_DO_H
#define LOGIC_DO_H

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "bsp_rtdb.h"
#include "kit_log.h"
#include <math.h>
#include "kit_math.h"
#include "kit_model.h"
#include "drv_gpio.h"
#include "app_bms_fault.h"

#define MAX_DO_CONFIG_NUM 12



// 获取do的回调函数，当前do序号作为参数传入，调用回调控制do接口输出
typedef int (*do_callback)(uint16_t do_index);


extern do_callback do_callbacks[kDo_Ctrl_End];
extern uint8_t do_status[kDo_Ctrl_End];


uint8_t get_do_output_status(do_ctrl_type_t do_ctrl_type);

void set_do_output_status(do_ctrl_type_t do_ctrl_type, uint8_t status);


// 逻辑do控制任务
void *logic_do_ctrl(void *arg);

// 创建逻辑do控制任务
void creatLogicDoTask(void);


#endif // LOGIC_DIDO_H




