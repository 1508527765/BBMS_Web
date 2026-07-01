/*****************************************************************************
 * @copyright       1997-2050, Gary. POWER SUPPLY CO., LTD. 
 * @file            logic_dido.h
 * @brief           dido功能
 * @author          gary
 * @date            2026-04-20
 * @remark          
 *****************************************************************************/

#ifndef LOGIC_DI_H
#define LOGIC_DI_H

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "bsp_rtdb.h"
#include "kit_log.h"
#include <math.h>
#include "kit_math.h"
#include "kit_model.h"
#include "drv_gpio.h"


#define MAX_DI_CONFIG_NUM 12


// 获取di的回调函数，当前di序号作为参数传入，调用系统结构得到di的值
typedef void (*di_callback)(di_signal_value_t signal_value);

extern di_callback di_callbacks[kDi_Signal_Stop];

// 这个里面定义所有的di信号接入后的值
extern uint8_t diSignalValue[kDi_Signal_Stop];

// 获取di信号值
uint8_t get_di_signal_value(di_signal_type_t sign);

// 逻辑di do控制任务
void *logic_di_ctrl(void *arg);

// 创建逻辑di do控制任务
void creatLogicDiTask(void);


#endif // LOGIC_DIDO_H




