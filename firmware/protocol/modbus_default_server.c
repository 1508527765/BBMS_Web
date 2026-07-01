/*****************************************************************************
 * @file            modbus_default_server.c
 * @brief           Modbus TCP 从站默认回调模板（通用，与具体点表无关）
 * @author          Gary
 * @date            2025/04/19
 * @remark
 *   本文件仅提供可扩展的占位实现，使用 modbus_comm.h 中的 READ_REGISITER 宏。
 *   EMS / 上位机 V1.0.13 点表请见 protocol_ems_modbus_server.c + protocol_ems_modbus_map.c。
 *****************************************************************************/

#include "kit_time.h"
#include "modbus_comm.h"
#include <math.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/*****************************************************************************
 * @brief       读保持寄存器 0x03（默认模板，按地址扩展）
 *****************************************************************************/
static int protocol_modbus_tcp_fun03_read_hold_regisiter(int fd, modbus_tcp_frame_t *frame,
                                                         tcp_server_lib_t *p_tcp)
{
    uint16_t index = 0, data = 0, addr = 0;
    uint8_t *buffer = &p_tcp->send_buf[9];

    (void)fd;

    if (frame->reqAddr >= 0 && frame->reqAddr + frame->reqRegNum < 25)
    {
        for (index = frame->reqAddr; index < frame->reqAddr + frame->reqRegNum; index++)
        {
            READ_REGISITER_BEGIN();
            READ_REGISITER_DATA(0, (uint16_t)0);
            READ_REGISITER_DATA(1, (uint16_t)0);
            READ_REGISITER_DATA(2, (uint16_t)0);
            READ_REGISITER_DATA(3, (uint16_t)0);
            /* 其他寄存器地址在此追加 */
            READ_REGISITER_END();
            WRITE_BT_INT16U(buffer, addr, data);
        }
        fillMdTcpConstPos(frame, p_tcp, buffer, addr);
        return 0;
    }

    return 1;
}

/*****************************************************************************
 * @brief       读输入寄存器 0x04（默认模板，按地址扩展）
 *****************************************************************************/
static int protocol_modbus_tcp_fun04_read_input_regisiter(int fd, modbus_tcp_frame_t *frame,
                                                          tcp_server_lib_t *p_tcp)
{
    uint16_t index = 0, data = 0, addr = 0;
    uint8_t *buffer = &p_tcp->send_buf[9];

    (void)fd;

    if (frame->reqAddr >= 0 && frame->reqAddr + frame->reqRegNum < 25)
    {
        for (index = frame->reqAddr; index < frame->reqAddr + frame->reqRegNum; index++)
        {
            READ_REGISITER_BEGIN();
            READ_REGISITER_DATA(0, (uint16_t)0);
            READ_REGISITER_DATA(1, (uint16_t)0);
            READ_REGISITER_DATA(2, (uint16_t)0);
            READ_REGISITER_DATA(3, (uint16_t)0);
            /* 其他寄存器地址在此追加 */
            READ_REGISITER_END();
            WRITE_BT_INT16U(buffer, addr, data);
        }
        fillMdTcpConstPos(frame, p_tcp, buffer, addr);
        return 0;
    }

    return 1;
}

/*****************************************************************************
 * @brief       写单个保持寄存器 0x06（默认模板）
 *****************************************************************************/
static int protocol_modbus_tcp_fun06_set_regisiter(int fd, modbus_tcp_frame_t *frame,
                                                   tcp_server_lib_t *p_tcp)
{
    int32_t data;

    (void)fd;

    data = (frame->data[0] << 8) + frame->data[1];
    if ((frame->reqAddr >= 1) && (frame->reqAddr <= 3))
    {
        switch (frame->reqAddr)
        {
        case 1:
            /* 调用 setRtdbPointValue 将 data 写入实时库 */
            break;
        default:
            break;
        }
        memcpy(p_tcp->send_buf, p_tcp->recv_buf, p_tcp->recv_len);
        p_tcp->send_len = frame->length + 6;
        return 0;
    }

    return 1;
}

/*****************************************************************************
 * @brief       写多个保持寄存器 0x10（默认模板）
 *****************************************************************************/
static int protocol_modbus_tcp_fun0x10_set_hold_regisiter(int fd, modbus_tcp_frame_t *frame,
                                                         tcp_server_lib_t *p_tcp)
{
    if ((frame->reqAddr >= 0) && (frame->reqAddr + frame->reqRegNum <= 10 + 1))
    {
        if (frame->length < 3 + frame->reqRegNum * 2)
        {
            return 1;
        }

        uint8_t byte_count = frame->data[2];
        uint8_t *reg_values = frame->data + 3;

        if (byte_count != frame->reqRegNum * 2)
        {
            return 1;
        }

        for (uint16_t i = 0; i < frame->reqRegNum; i++)
        {
            uint16_t current_addr = frame->reqAddr + i;
            uint16_t data = (uint16_t)((reg_values[i * 2] << 8) | reg_values[i * 2 + 1]);

            switch (current_addr)
            {
            case 0:
                /* 调用 setRtdbPointValue 将 data 写入实时库 */
                break;
            default:
                break;
            }
        }

        p_tcp->send_buf[0] = frame->transaction_id >> 8;
        p_tcp->send_buf[1] = frame->transaction_id;
        p_tcp->send_buf[2] = 0x00;
        p_tcp->send_buf[3] = 0x00;
        p_tcp->send_buf[4] = 0x00;
        p_tcp->send_buf[5] = 0x06;
        p_tcp->send_buf[6] = frame->unit_id;
        p_tcp->send_buf[7] = frame->function_code;
        p_tcp->send_buf[8] = frame->reqAddr >> 8;
        p_tcp->send_buf[9] = frame->reqAddr & 0xFF;
        p_tcp->send_buf[10] = frame->reqRegNum >> 8;
        p_tcp->send_buf[11] = frame->reqRegNum & 0xFF;
        p_tcp->send_len = 12;
        return 0;
    }
    return 1;
}

/*****************************************************************************
 * @brief       注册默认 Modbus TCP 从站回调（非 EMS 点表场景可选用）
 *****************************************************************************/
void protocol_modbus_tcp_default_init(void)
{
    register_modbus_func_call(0x03, protocol_modbus_tcp_fun03_read_hold_regisiter);
    register_modbus_func_call(0x04, protocol_modbus_tcp_fun04_read_input_regisiter);
    register_modbus_func_call(0x06, protocol_modbus_tcp_fun06_set_regisiter);
    register_modbus_func_call(0x10, protocol_modbus_tcp_fun0x10_set_hold_regisiter);
}
