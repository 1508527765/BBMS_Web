/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, Gary. All rights reserved.
 * @file            logic_dido.c
 * @brief           逻辑di do控制任务
 * @author          gary
 * @date            2026-04-20
 * @remark          
 *****************************************************************************/

#include "logic_di.h"

// 这个里面定义所有的di信号接入后的值
uint8_t diSignalValue[kDi_Signal_Stop] = {0};

di_config_t di_config; // di配置数据


// kDi_Signal_Water_AlarmI
void water_alarm_i_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Water_AlarmI] = signal_value;
    }
}

// kDi_Signal_Fire_Fault
void fire_fault_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Fire_Fault] = signal_value;
    }
}

// kDi_Signal_Fire_AlarmI
void fire_alarm_i_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Fire_AlarmI] = signal_value;
    }
}

// kDi_Signal_Fire_AlarmII
void fire_alarm_ii_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Fire_AlarmII] = signal_value;
    }
}

// kDi_Signal_Gas_AlarmI
void gas_alarm_i_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Gas_AlarmI] = signal_value;
    }
}

// kDi_Signal_BMS_DCSPD_FaultI
void bms_dcspd_fault_i_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_BMS_DCSPD_FaultI] = signal_value;
    }
}

// kDi_Signal_Container_EStopI
void container_estop_i_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Container_EStopI] = signal_value;
    }
}

// kDi_Signal_QF1_Feedback_AlarmII
void qf1_feedback_alarm_ii_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_QF1_Feedback_AlarmII] = signal_value;
    }
}

// kDi_Signal_Door_AlarmIII
void door_alarm_iii_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Door_AlarmIII] = signal_value;
    }
}

// kDi_Signal_ACSPD1_AlarmI
void acspd1_alarm_i_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_ACSPD1_AlarmI] = signal_value;
    }
}

// kDi_Signal_Fan_FaultII
void fan_fault_ii_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Fan_FaultII] = signal_value;
    }
}

// kDi_Signal_Fuse_FaultI
void fuse_fault_i_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Fuse_FaultI] = signal_value;
    }
}

// kDi_Signal_Crtl_Switch_AlarmIII
void ctrl_switch_alarm_iii_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Crtl_Switch_AlarmIII] = signal_value;
    }
}

// kDi_Signal_Low_Pressure_AlarmI
void low_pressure_alarm_i_callback(di_signal_value_t signal_value)
{
    if(signal_value == kDiSignalValue_Trigger)
    {
        // 处理di信号触发的逻辑
        diSignalValue[kDi_Signal_Low_Pressure_AlarmI] = signal_value;
    }
}




//按照di信号类型枚举索引顺序，将回调函数指针存入di_callbacks数组
di_callback di_callbacks[kDi_Signal_Stop] = {
    NULL,                                   // kDi_Signal_None
    water_alarm_i_callback,                 //kDi_Signal_Water_AlarmI
    fire_fault_callback,                    //kDi_Signal_Fire_Fault
    fire_alarm_i_callback,                  //kDi_Signal_Fire_AlarmI
    fire_alarm_ii_callback,                 //kDi_Signal_Fire_AlarmII
    gas_alarm_i_callback,                   //kDi_Signal_Gas_AlarmI
    bms_dcspd_fault_i_callback,             //kDi_Signal_BMS_DCSPD_FaultI
    container_estop_i_callback,             //kDi_Signal_Container_EStopI
    qf1_feedback_alarm_ii_callback,         //kDi_Signal_QF1_Feedback_AlarmII
    door_alarm_iii_callback,                //kDi_Signal_Door_AlarmIII
    acspd1_alarm_i_callback,                //kDi_Signal_ACSPD1_AlarmI
    fan_fault_ii_callback,                  //kDi_Signal_Fan_FaultII
    fuse_fault_i_callback,                  //kDi_Signal_Fuse_FaultI
    ctrl_switch_alarm_iii_callback,         //kDi_Signal_Crtl_Switch_AlarmIII
    low_pressure_alarm_i_callback,          //kDi_Signal_Low_Pressure_AlarmI
};




uint8_t get_di_signal_value(di_signal_type_t signal_type)
{
    if(signal_type >= kDi_Signal_Stop)
    {
        return kDiSignalValue_None;
    }

    return diSignalValue[signal_type];
}



//初始化di do配置数据
void logic_di_init(void)
{
    kit_db_get_di_config(&di_config);
    
}


//主任务中调用，用于调度di do逻辑控制
void* logic_di_ctrl(void *arg)
{

    logic_di_init();

    while(1)
    {
        // 更新配置参数
        if (readWebSign(kSign_ShMem, kSign_DiLogicConfig) != 0)
        {
            writeWebSign(kSign_ShMem, kSign_DiLogicConfig, 0);
            kit_db_get_di_config(&di_config);
        }

        //根据di_config和do_config中的配置，调用di_callbacks获取di信号值，进行逻辑判断，调用do_callbacks控制do输出
        for(uint16_t i = 0; i < di_config.di_config_num; i++)
        {
            uint16_t di_idx = di_config.di_logic_config[i].di_idx;
            uint16_t logic_idx = di_config.di_logic_config[i].logic_idx;
            uint16_t bms_idx = di_config.di_logic_config[i].bms_idx;
            //先获取di信号值
            di_config.di_logic_config[i].actual_value = drvGpioRead(di_idx);

            if(di_config.di_logic_config[i].di_input_mode == kDI_Valid_Disable)
            {
                //如果di输入模式是禁用，跳过这个di的逻辑判断
                continue;
            }
            else if(di_config.di_logic_config[i].di_input_mode == kDI_Valid_Normally_Open)
            {
                //如果di输入模式是使能，则进行逻辑判断
                if(di_config.di_logic_config[i].actual_value == kDiSignalValue_Trigger)
                {
                    di_config.di_logic_config[i].signal_value = kDiSignalValue_Trigger;
                }
                else
                {
                    di_config.di_logic_config[i].signal_value = kDiSignalValue_None;
                }
            }
            else if(di_config.di_logic_config[i].di_input_mode == kDI_Valid_Normally_Closed)
            {
                //如果di输入模式是常闭，则进行逻辑判断
                if(di_config.di_logic_config[i].actual_value != kDiSignalValue_Trigger)
                {
                    //如果实际值不等于逻辑值，则调用回调函数，将di信号值存入diSignalValue数组
                    diSignalValue[di_idx] = di_config.di_logic_config[i].actual_value;
                }
                else
                {
                    di_config.di_logic_config[i].signal_value = kDiSignalValue_None;
                }
            }


            if(di_callbacks[logic_idx] != NULL)
            {
                di_callbacks[logic_idx](di_config.di_logic_config[i].signal_value);
            }

        }

        usleep(20*1000); // 20ms执行一次
    }
    
}





// 创建逻辑di do控制任务
void creatLogicDiTask(void)
{
    pthread_t logic_di_thread;
    if(pthread_create(&logic_di_thread, NULL, logic_di_ctrl, NULL) != 0)
    {
        KITLOG(LOG_DB_EN, ERROR_EN, "Failed to create logic_di_thread");
    }
    else
    {
        KITLOG(LOG_DB_EN, INFO_EN, "logic_di_thread created successfully");
    }


}