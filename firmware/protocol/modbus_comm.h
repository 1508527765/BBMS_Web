/*****************************************************************************
 * @file            modbus_comm.h
 * @brief           modbus点表层代码，可定制为不同点表
 * @author          Gary
 * @date            2025/04/19
 * @remark          初修订
 *****************************************************************************/
#ifndef MODBUS_COMM_H
#define MODBUS_COMM_H

#include "bsp_modbusSlave.h"

#define READ_REGISITER_BEGIN() \
switch(index) \
{


#define READ_REGISITER_END() \
        default: \
           data = 0x00; \
           break; \
}

#define READ_REGISITER_DATA(INDEX, VARIABLE) \
        case INDEX: \
           data = VARIABLE; \
           break;

#define WRITE_BT_INT16U(BUFFER, POS, VALUE)      \
    {                                             \
        BUFFER[POS++] = (uint8_t)((VALUE) >> 8); \
        BUFFER[POS++] = (uint8_t)(VALUE & 0xFF); \
    }

extern sem_t event_semaphore_pcs_dist; // 功率分配使用
extern sem_t event_semaphore_pcs_reac_dist; // 无功分配使用
extern sem_t event_semaphore_pcs_switch; // 开关机控制使用

/*****************************************************************************
 * @brief       注册默认 Modbus TCP 从站回调（通用模板，见 modbus_default_server.c）
 *****************************************************************************/
void protocol_modbus_tcp_default_init(void);

/*****************************************************************************
 * @brief       注册 EMS / 上位机 Modbus TCP 回调（V1.0.13，见 protocol_ems_modbus_server.c）
 *****************************************************************************/
void protocol_ems_modbus_tcp_init(void);

/*****************************************************************************
 * @brief       分部署架构下的ems离线主动监测
 * @return
 *****************************************************************************/
void checkEMSOnlineTimeCall(void);


uint8_t getEMSOnlineStateForeDistruted(uint16_t emsIdx);
#endif // MODBUS_COMM_H