/*****************************************************************************
 * @copyright       2000-2050, Copyright (c) 2026 Gary. All Rights Reserved.
 * @file            protocol_can_tms.c
 * @brief           海辰 TMS通信协议实现
 * @author          Gary
 * @date            2026-05-11
 * @remark          实现TMS CAN通信协议，包括接收、解析、发送等操作、
 *                  我们这里仅接入一台TMS设备，如果特殊需求接入多台，在做变更处理
 *****************************************************************************/
#include "protocol_can_comm.h"

/*
typedef void (*can_send_callback)(CanMsg *, uint8_t);
typedef void (*can_recv_callback)(uint8_t, CanMsg *);
typedef int (*can_poll_callback)(uint32_t base_time,uint8_t can);
*/

static uint32_t tms_heart = 0; // TMS心跳计数器



/**********************************************************************
 * @brief 解析TMS CAN消息并存储到TMS信息结构体中
 * @param can CAN设备
 * @param msg CAN消息指针
 **********************************************************************/
void protocol_can_tms_recv(uint8_t can, CanMsg *msg)    
{
    // 解析CAN消息并存储到TMS信息结构体中
    if(msg == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "protocol_can_tms_recv: msg is NULL");
        return;
    }

	tms_heart++;
	TMS_Status_t* item = &tms_status;
	uint8_t *data = msg->data.byte;
	switch(msg->id.value)
	{
		case 0x18FFC13A:
		{
			item->working_mode = data[0] & 0x03;
            item->k1_relay = (data[0] >> 2) & 0x03;
            item->k2_relay = (data[0] >> 4) & 0x03;
            item->preheat_feedback = (data[0] >> 6) & 0x03;
            item->outlet_temp = (int16_t)data[1] - 40;
            item->inlet_temp = (int16_t)data[2] - 40;
            item->env_temp = (int16_t)data[3] - 40;
            item->inlet_pressure = data[4] * 0.1f;
            item->outlet_pressure = data[5] * 0.1f;
            item->env_humidity = data[6];
            item->fault_code = (uint16_t)data[7] & 0x3F;
            item->fault_level = (Fault_Level_e)(data[7] >> 6);
			break;
		}
        case 0x18FFC13B:
        {
            item->acdc_a_voltage = data[0];
            item->pump_pwm = data[1];
            item->high_pressure_1 = (uint16_t)data[2] | ((uint16_t)data[3] << 8);
            item->low_pressure_1 = (uint16_t)data[4] | ((uint16_t)data[5] << 8);
            item->fan_pwm = data[6];
            item->expv_valve = data[7];
            break;
        }
        case 0x18FFC13C:
        {
            item->compressor_voltage_1 = data[0] * 3;
            item->compressor_current_1 = data[1] * 0.5f;
            item->compressor_rpm_1 = data[2] * 100;
            item->ptc_temp_switch = data[3] & 0x01;
            item->pump_switch = (data[3] >> 1) & 0x01;
            item->dehumidify_state = (data[3] >> 2) & 0x01;
            item->ac_fault_bit = (data[3] >> 3) & 0x1F;
            item->ac_fault_code = data[4] | ((data[5] << 8));
            item->tms_set_temp_feedback = (int8_t)data[6] - 40;
            item->remote_mode = data[7] & 0x01;
            item->natural_cold_valve = (data[7] >> 1) & 0x01;
            item->lock_state = (data[7] >> 2) & 0x01;
            break;
        }
        case 0x18FFC13D:
        {
            item->device_version1 = data[0];
            item->device_version2 = data[1];
            item->device_version3 = data[2];
            item->device_version4 = data[3];
            item->device_version5 = data[4];
            item->software_version1 = data[5];
            item->software_version2 = data[6];
            item->software_version3 = data[7];
            break;
        }
        case 0x18FFC13E:
        {
            item->compressor_rpm_2 = data[0] * 100;
            item->high_pressure_2 = (uint16_t)data[1] | ((uint16_t)data[2] << 8);
            item->low_pressure_2 = (uint16_t)data[3] | ((uint16_t)data[4] << 8);
            item->compressor_voltage_2 = data[5] * 3;
            item->compressor_current_2 = data[6] * 0.5f;
            break;
        }

		default:
			break;
	}

}



void protocol_can_tms_temp_control(uint32_t base_time, can_dev_e can)
{
    tms_cmd.bms_req_mode = tms_target.req_mode;
    tms_cmd.bms_status = 0;   //文档说-不做处理
    tms_cmd.th_fault = 0;     //文档说-不做处理
    tms_cmd.ctrl_mode = tms_target.ctrl_mode;    
    tms_cmd.reserved1 = 0;   //预留位填0

    tms_cmd.cell_highest_temp = (uint8_t)(getRtdbPointValue(rtdbType, kDev_Type_BBMS, 0, kBbms_MaxCellT) + 40);
    tms_cmd.cell_lowest_temp = (uint8_t)(getRtdbPointValue(rtdbType, kDev_Type_BBMS, 0, kBbms_MinCellT) + 40);
    tms_cmd.cell_average_temp = (uint8_t)(getRtdbPointValue(rtdbType, kDev_Type_BBMS, 0, kBbms_AvgCellT) + 40);
    tms_cmd.set_temp = (uint8_t)(tms_target.target_temp + 40);

    tms_cmd.current = (int16_t)(getRtdbPointValue(rtdbType, kDev_Type_BBMS, 0, kBbms_TotalDCCur) * 10 + 3200);


    bms_charge_status_e hp_status = (bms_charge_status_e)getRtdbPointValue(rtdbType, kDev_Type_BBMS, 0, kBbms_ChSt);
    uint8_t tmp = 0;
    if(hp_status == kBms_ChargeStatus_Charge)
    {
        tmp = 2; // 充电中和充电完成都算充电中
    }
    else if(hp_status == kBms_ChargeStatus_Discharge)
    {
        tmp = 1; // 放电中和放电完成都算放电中
    }
    else if(hp_status == kBms_ChargeStatus_Standing || hp_status == kBms_ChargeStatus_NotConnected)
    {
        tmp = 0; // 静置中和未连接都算未连接
    }

    tms_cmd.hp_status = tmp;      //高压状态 


    tms_cmd.weather_mode = tms_target.weather_mode;   //除雪模式
    tms_cmd.reserved8 = 0;

    CanMsg msg = {0};
    
    //在此填充一些消息
    msg.id.value = 0x18FF45F4U;
    msg.len = 8;

    memcpy(msg.data.byte, &tms_cmd, sizeof(BMS_To_TMS_Cmd_t));
    // 最后发送发送CAN消息到TMS
    drvCanNewSend(can, &msg);
}

///**********************************************************************
// * @brief 发送can消息到TMS，控制时间相关的参数
// * @param can CAN设备
// **********************************************************************/
void protocol_can_tms_time_control(uint32_t base_time, can_dev_e can)
{

    rtc_time_t rtc_time = kit_get_rtc_time();

    tms_rtc.year = rtc_time.year;
    tms_rtc.month = rtc_time.month;
    tms_rtc.day = rtc_time.day;
    tms_rtc.hour = rtc_time.hour;
    tms_rtc.minute = rtc_time.minute;
    tms_rtc.second = rtc_time.second;
    tms_rtc.reserved = 0;

    CanMsg msg = {0};
    
    //在此填充一些消息
    msg.id.value = 0x18FF46F4U;
    msg.len = 8;

    memcpy(msg.data.byte, &tms_rtc, sizeof(BMS_To_TMS_RTC_t));
    // 最后发送发送CAN消息到TMS
    drvCanNewSend(can, &msg);
}


/**********************************************************************
 * @brief 发送can消息到TMS
 * @param can CAN设备
 * @param msg CAN消息指针
 **********************************************************************/
void protocol_can_tms_send(uint32_t base_time,uint8_t can)
{

    static uint32_t send_time_daly = 0;
    send_time_daly += base_time;
    if(send_time_daly >= 1000)
    {
        send_time_daly = 0;
        protocol_can_tms_temp_control(base_time, can);
        protocol_can_tms_time_control(base_time, can);       
    }
}


/**********************************************************************
 * @brief 同步TMS状态到RTDB,供web端展示使用
 * @param devId TMS设备ID
 **********************************************************************/    
void app_tms_sync_rtdb(uint16_t devId)
{
    const TMS_Status_t *s = &tms_status;
    const BMS_To_TMS_Cmd_t *cmd = &tms_cmd;
    const BMS_To_TMS_RTC_t *rtc = &tms_rtc;
    const dev_type_e dt = kDev_Type_Lqd;

    /* TMS -> BMS  0x18FFC13A */
    setRtdbPointValue(rtdbType, dt, devId, kLqd_WorkingMode, (double)s->working_mode);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_K1RelayStatus, (double)s->k1_relay);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_K2RelayStatus, (double)s->k2_relay);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_HeheatFeedbackStatus, (double)s->preheat_feedback);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_OutletTemp, (double)s->outlet_temp);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_InletTemp, (double)s->inlet_temp);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_EnvTemp, (double)s->env_temp);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_InletPressure, (double)s->inlet_pressure);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_OutletPressure, (double)s->outlet_pressure);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_EnvHumidity, (double)s->env_humidity);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_FaultCode, (double)s->fault_code);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_FaultLevel, (double)s->fault_level);

    /* TMS -> BMS  0x18FFC13B */
    setRtdbPointValue(rtdbType, dt, devId, kLqd_ACDC_Aoltage, (double)s->acdc_a_voltage);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_PumpPWM, (double)s->pump_pwm);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_HighPressure1, (double)s->high_pressure_1);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_LowPressure1, (double)s->low_pressure_1);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_FanPWM, (double)s->fan_pwm);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_ExpvValve, (double)s->expv_valve);

    /* TMS -> BMS  0x18FFC13C */
    setRtdbPointValue(rtdbType, dt, devId, kLqd_CompressorVoltage1, (double)s->compressor_voltage_1);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_CompressorCurrent1, (double)s->compressor_current_1);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_CompressorRPM1, (double)s->compressor_rpm_1);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_PTC_TempSwitch, (double)s->ptc_temp_switch);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_PumpSwitch, (double)s->pump_switch);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_DehumidifyState, (double)s->dehumidify_state);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_ACFaultBit, (double)s->ac_fault_bit);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_ACFaultCode, (double)s->ac_fault_code);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_TMSSetTempFeedback, (double)s->tms_set_temp_feedback);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_RemoteMode, (double)s->remote_mode);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_NaturalColdValve, (double)s->natural_cold_valve);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_LockState, (double)s->lock_state);

    /* TMS -> BMS  0x18FFC13D */
    setRtdbPointValue(rtdbType, dt, devId, kLqd_DeviceVersion1, (double)s->device_version1);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_DeviceVersion2, (double)s->device_version2);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_DeviceVersion3, (double)s->device_version3);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_DeviceVersion4, (double)s->device_version4);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_DeviceVersion5, (double)s->device_version5);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_SoftwareVersion1, (double)s->software_version1);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_SoftwareVersion2, (double)s->software_version2);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_SoftwareVersion3, (double)s->software_version3);

    /* TMS -> BMS  0x18FFC13E */
    setRtdbPointValue(rtdbType, dt, devId, kLqd_CompressorRPM2, (double)s->compressor_rpm_2);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_HighPressure2, (double)s->high_pressure_2);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_LowPressure2, (double)s->low_pressure_2);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_CompressorVoltage2, (double)s->compressor_voltage_2);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_CompressorCurrent2, (double)s->compressor_current_2);

    /* BMS -> TMS 控制帧镜像（0x18FF45F4，写入RTDB供EMS/Web读取） */
    setRtdbPointValue(rtdbType, dt, devId, kLqd_BMSRequestMode, (double)cmd->bms_req_mode);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_BMSStatus, (double)cmd->bms_status);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_ThFault, (double)cmd->th_fault);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_ControlMode, (double)cmd->ctrl_mode);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_Reserved, 0.0);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_HighestTemp, (double)((int16_t)cmd->cell_highest_temp - 40));
    setRtdbPointValue(rtdbType, dt, devId, kLqd_LowTemp, (double)((int16_t)cmd->cell_lowest_temp - 40));
    setRtdbPointValue(rtdbType, dt, devId, kLqd_AverageTemp, (double)((int16_t)cmd->cell_average_temp - 40));
    setRtdbPointValue(rtdbType, dt, devId, kLqd_SetTemp, (double)((int16_t)cmd->set_temp - 40));
    setRtdbPointValue(rtdbType, dt, devId, kLqd_Current, (double)GET_CURRENT(cmd->current));
    setRtdbPointValue(rtdbType, dt, devId, kLqd_HighPressureStatus, (double)cmd->hp_status);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_WeatherMode, (double)cmd->weather_mode);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_CrtlReserved, 0.0);

    /* BMS -> TMS RTC（0x18FF46F4） */
    setRtdbPointValue(rtdbType, dt, devId, kLqd_Year, (double)rtc->year);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_Month, (double)rtc->month);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_Day, (double)rtc->day);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_Hour, (double)rtc->hour);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_Minute, (double)rtc->minute);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_Second, (double)rtc->second);
    setRtdbPointValue(rtdbType, dt, devId, kLqd_Reserved3, 0.0);
}


uint8_t tms_check_online(uint32_t base_time)
{
    static uint32_t online_time_delay = 0;
    static uint32_t tms_heart_old = 0;
    online_time_delay += base_time;
    uint8_t tms_online = 0; // TMS是否在线

    if(online_time_delay >= 5000)
    {
        online_time_delay = 0;
        if(tms_heart_old != tms_heart)
        {
            tms_heart_old = tms_heart;
            tms_online = 1;
        }
        else
        {
            tms_online = 0;
        }
    }

    return tms_online;
}


/**********************************************************************
 * @brief 发送can消息到TMS，有些特殊逻辑可以再这里做
 * @param can CAN设备
 * @param msg CAN消息指针
 **********************************************************************/
void protocol_can_tms_poll(uint32_t base_time,uint8_t can)
{
    app_tms_sync_rtdb(0);
    uint8_t tms_online = tms_check_online(base_time);
    setRtdbPointValue(rtdbType, kDev_Type_Lqd, 0, kLqd_Online, (double)tms_online);
}


void protocol_can_tms_init(uint32_t base_time, can_dev_e can)
{
    // 注册接收回调函数
    drvCanRegisterRecvCallback(can, protocol_can_tms_recv);
    
    // 注册发送回调函数
    drvCanRegisterSendCallback(can, protocol_can_tms_send);
    
    // 注册轮询回调函数
    drvCanRegisterPollCallback(can, protocol_can_tms_poll);
    
    // 其他初始化操作，如设置初始状态等
    tms_heart = 0;
}