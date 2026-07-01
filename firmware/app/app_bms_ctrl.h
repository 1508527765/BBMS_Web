/*****************************************************************************
 * @copyright       1997-2050, Gary. POWER SUPPLY CO., LTD. 
 * @file            app_bms.h
 * @brief           bms功能接口
 * @author          gary
 * @date            2026-03-31
 * @remark          
 *****************************************************************************/

#ifndef APP_BMS_CTRL_H
#define APP_BMS_CTRL_H

#include "kit_model.h"
#include "bsp_rtdb.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include "kit_db.h"
#include "bsp_bms_com.h"
#include "protocol_bms.h"
#include "kit_log.h"
#include "app_bms_statistics.h"

typedef enum{
	kRunCtrl_Start = 0,
	kRunCtrl_local = kRunCtrl_Start, //本地自控模式
	kRunCtrl_Cmd,	//命令模式
	kRunCtrl_End = 3, //结束模式
}run_ctrl_mode_e;//上下电模式枚举


//bms状态机枚举 初始，自检，运行，故障
typedef enum 
{
    kRunStatus_Start = 0,
    kRunStatus_Init = kRunStatus_Start,
    kRunStatus_SelfCheck,
    kRunStatus_Run = 2,
    kRunStatus_Fault = 3,
    kRunStatus_End = 4,
}run_status_e;

typedef enum
{
    kStackState_Init = 0,
    kStackState_SelfCheck,      //自检状态
    kStackState_Standby,        //待机状态

    kStackState_Prepare,      //准备状态
    kStackState_MasterGridOn, //主簇并网状态
    kStackState_GridOn,       //从簇并网状态

    kStackState_Run,            //运行状态
    kStackState_PowerOff,       //下电状态
    kStackState_Fault,          //故障状态
}stack_state_e;


typedef struct
{
    uint8_t online;
    uint8_t ready;
    uint8_t fault;
    float voltage;
    uint8_t contactor_on;
    uint8_t selected;
}stack_cluster_t;


extern bms_sys_config_t  bbms_sys_cfg;

void generateRbmsPowerOnCmd(uint8_t id);   //指定簇控发送“并网/上电请求”
void generateRbmsPowerOffCmd(uint8_t id);  //指定簇控发送“断网/下电请求”
void bbms_send_safety_signal_to_rbms(uint16_t bbms_idx); // BBMS_SafetySignal 0x02/0x14 (原 0x02/0x0E)

uint8_t getBmsPowerOnReady(void);       //判断系统母线是否允许上电（外部条件）
uint32_t get_tick_ms(void);
uint8_t bbmsSelfCheckOk(void);       //判断自检是否通过

// 上下电控制逻辑
void app_bms_run_control(void);

// 上下电控制逻辑
void bmsStackRunControl(void);

/*************************************************************************
 * @brief        启动簇控管理线程
 * @param[in]    无       
 * @details:    创建一个线程来运行bmsStackRunControl，周期100ms
 * @return       无
 *************************************************************************/
void startBmsStackControlThread(void);
#endif // APP_BMS_CTRL_H





