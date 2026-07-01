/*****************************************************************************
 * @copyright       1997-2050,AO DI LTD.
 * @file            kit_core.h
 * @brief           二次封装
 * @author          Gary
 * @date            2024/01/15
 * @remark
 *****************************************************************************/
#ifndef BSP_CAN_SCHED_H
#define BSP_CAN_SCHED_H

#define MAX_EVENTS 1024
#include "protocol_can_comm.h"
#include "kit_log.h"
#include "drv_can.h"
#include "kit_queue.h"


extern CanDevice can_devices[MAX_CAN_DEVICES];
extern uint32_t tmp_ul_can_id;

typedef struct
{
    uint8_t canId;                 // 属于的can的序号
    uint8_t devnum;                // 当前下面的设备数量
    uint16_t timeout;              // 超时时间 单位s
    uint16_t arrayId[MAX_DSP_NUM]; // 单个can设备最大8个
} dsp_candev_t;



//创建can任务
void creatCanTaskEntry();


#endif // BSP_CAN_SCHED_H