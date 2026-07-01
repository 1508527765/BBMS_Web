/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, Gary. All rights reserved.
 * @file            bsp_can_sched.c
 * @brief           can任务调度器
 * @author          gary
 * @date            2026-05-20
 * @remark          
 *****************************************************************************/
#include "bsp_can_sched.h"


#define CAN_MAX_QUEUE_ITEM_SIZE 10
CanDevice can_devices[MAX_CAN_DEVICES];

//定义一个can接收队列
static kit_queue_t can_recv_queue[MAX_CAN_DEVICES];

static uint8_t can1_recv_queue_buf[CAN_MAX_QUEUE_ITEM_SIZE * sizeof(CanMsg)];
static uint8_t can2_recv_queue_buf[CAN_MAX_QUEUE_ITEM_SIZE * sizeof(CanMsg)];

//初始化can接收队列
void bsp_can_queue_init(void)
{
    if (kit_queue_init(&can_recv_queue[0], can1_recv_queue_buf, CAN_MAX_QUEUE_ITEM_SIZE, sizeof(CanMsg)) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Failed to init can1_recv_queue");
    }
    else
    {
        KITLOG(LOG_APP_EN, INFO_EN, "can1_recv_queue initialized successfully");
    }

    if (kit_queue_init(&can_recv_queue[1], can2_recv_queue_buf, CAN_MAX_QUEUE_ITEM_SIZE, sizeof(CanMsg)) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Failed to init can2_recv_queue");
    }
    else
    {
        KITLOG(LOG_APP_EN, INFO_EN, "can2_recv_queue initialized successfully");
    }
}


//创建can接收线程任务函数，接收的数据入接受队列
void *canClientThreadFunc(void *arg)
{
    uint8_t can_index = (uint8_t)(uintptr_t)arg; // 获取CAN设备索引
    kit_queue_t* recv_queue = &can_recv_queue[can_index];

    if(drvCanOpen(can_index) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Failed to open CAN%d", can_index);
        return NULL;
    }

    CanMsg msg;

    while (1)
    {
        drvCanNewRecv(can_index, &msg);
        // 将接收到的数据入队到接收队列
        if (kit_queue_enqueue(recv_queue, &msg) != 0)
        {
            KITLOG(LOG_APP_EN, ERROR_EN, "Failed to enqueue received CAN message for CAN%d", can_index);
        }

        usleep(10000); // 10ms
    }
}



//业务层函数，定时调用协议层的轮询函数
void protocol_can_poll(uint32_t base_time, can_dev_e can)
{

    kit_queue_t* recv_queue = &can_recv_queue[can];
	CanMsg msg;

    if(can < kCanDevEnd)
    {
        if(can_devices[can].recv != NULL)
        {
            //获取接收队列元素
            if(kit_queue_dequeue(recv_queue, &msg) == 0)
            {
                can_devices[can].recv(can, &msg);
            }
        }
            
        if(can_devices[can].send != NULL)
        {
            can_devices[can].send(base_time, can);
        }
        
        if(can_devices[can].poll != NULL)
        {
            can_devices[can].poll(base_time, can);
        }
    }
}



//创建can定时任务任务10ms执行一次，负责调用协议层的轮询函数protocol_can_poll
void creatCanTimerTask()
{
    while (1)
    {
        protocol_can_poll(0, kCanDev0); // 轮询CAN1
        protocol_can_poll(0, kCanDev1); // 轮询CAN2
        usleep(100000); // 10ms
    }   
}


//创建can任务入口函数
/*****************************************************************************
 * @brief       创建can任务入口函数,实现认为接收数据入队到接收队列，并定时调用协议层的轮询函数
 * @param[in]   None
 * @return      none
 *****************************************************************************/
void creatCanTaskEntry()
{
    bsp_can_queue_init(); // 初始化CAN接收队列

    // 创建CAN1接收线程
    pthread_t can1_thread;
    if (pthread_create(&can1_thread, NULL, canClientThreadFunc, (void*)(uintptr_t)kCanDev0) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Failed to create CAN1 thread");
    }
    else
    {
        KITLOG(LOG_APP_EN, INFO_EN, "CAN1 thread created successfully");
    }

    // 创建CAN2接收线程
    pthread_t can2_thread;
    if (pthread_create(&can2_thread, NULL, canClientThreadFunc, (void*)(uintptr_t)kCanDev1) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Failed to create CAN2 thread");
    }
    else
    {
        KITLOG(LOG_APP_EN, INFO_EN, "CAN2 thread created successfully");
    }


    // 创建定时器，周期性调用协议层的轮询函数
    pthread_t timer_thread;
    if (pthread_create(&timer_thread, NULL, (void *)creatCanTimerTask, NULL) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Failed to create CAN timer thread");
    }
    else
    {
        KITLOG(LOG_APP_EN, INFO_EN, "CAN timer thread created successfully");
    }

}