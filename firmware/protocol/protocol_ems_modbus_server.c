/*****************************************************************************
 * @file            protocol_ems_modbus_server.c
 * @brief           EMS / 上位机 Modbus TCP 从站 — 功能码回调（V1.0.13，端口 502）
 *
 * 职责：仅处理 Modbus 报文格式（PDU 解析、字节序、应答帧长度），不解析点表含义。
 * 点表与数据：全部在 protocol_ems_modbus_map.c。
 *
 * 与 modbus_default_server.c 的关系：
 *   - default_server = 通用占位模板（protocol_modbus_tcp_default_init）
 *   - 本文件 = EMS/上位机 实际使用的 502 回调（protocol_ems_modbus_tcp_init）
 *
 * 启动：app_task_regedit.c → creatModbusTcpEmsServer()
 *        → protocol_ems_modbus_tcp_init() + creatModbusTcpStdSlaveTask()
 *
 * 本文件与 map.c 的功能码对应关系：
 *   0x02     → 离散输入 Discrete Input（只读）
 *   0x04     → 输入寄存器 Input Register（只读）
 *   0x03     → 保持寄存器 Holding Register（读）
 *   0x06/0x10→ 保持寄存器 Holding Register（写）
 *****************************************************************************/

#include "modbus_comm.h"
#include "protocol_ems_modbus_map.h"
#include <string.h>

/* 单次读寄存器/离散数量上限（Modbus 规范常用上限） */
#define EMS_MODBUS_MAX_READ_REGS     125u
#define EMS_MODBUS_MAX_DISCRETE_BITS 2000u
#define EMS_MODBUS_MAX_DISCRETE_BYTES \
    ((EMS_MODBUS_MAX_DISCRETE_BITS + 7u) / 8u)

/* ========================================================================== */
/* 【离散输入 Discrete Input】功能码 0x02 — 只读                                  */
/* ========================================================================== */

/*****************************************************************************
 * @brief  读离散输入寄存器
 * @note   请求：起始地址 + 数量；应答：字节数 + 位图（调用 map 层打包）
 * @return 0 成功，1 非法地址（框架回 Modbus 异常）
 *****************************************************************************/
static int ems_modbus_tcp_fun02_read_discrete_input(int fd, modbus_tcp_frame_t *frame,
                                                    tcp_server_lib_t *p_tcp)
{
    uint8_t bit_bytes[EMS_MODBUS_MAX_DISCRETE_BYTES];
    uint16_t bit_count;
    uint8_t byte_count;

    (void)fd;

    bit_count = frame->reqRegNum;
    if (bit_count == 0 || bit_count > EMS_MODBUS_MAX_DISCRETE_BITS)
    {
        return 1;
    }

    byte_count = (uint8_t)((bit_count + 7u) / 8u);
    if (ems_modbus_read_discrete_inputs(frame->reqAddr, bit_count, bit_bytes) != 0)
    {
        return 1;
    }

    /* 组装 Modbus TCP 应答（功能码 0x02：第 9 字节起为数据） */
    p_tcp->send_buf[0] = (uint8_t)(frame->transaction_id >> 8);
    p_tcp->send_buf[1] = (uint8_t)(frame->transaction_id);
    p_tcp->send_buf[2] = 0x00;
    p_tcp->send_buf[3] = 0x00;
    p_tcp->send_buf[4] = 0x00;
    p_tcp->send_buf[5] = (uint8_t)(byte_count + 3u);
    p_tcp->send_buf[6] = frame->unit_id;
    p_tcp->send_buf[7] = frame->function_code;
    p_tcp->send_buf[8] = byte_count;
    memcpy(&p_tcp->send_buf[9], bit_bytes, byte_count);
    p_tcp->send_len = (size_t)(9 + byte_count);
    return 0;
}

/* ========================================================================== */
/* 【保持寄存器 Holding Register】功能码 0x03 — 读（500~699，501/503 除外）       */
/* ========================================================================== */

static int ems_modbus_tcp_fun03_read_hold_regisiter(int fd, modbus_tcp_frame_t *frame,
                                                    tcp_server_lib_t *p_tcp)
{
    uint16_t regs[EMS_MODBUS_MAX_READ_REGS];
    uint16_t i;
    uint8_t *buffer = &p_tcp->send_buf[9];
    uint8_t pos = 0;

    (void)fd;

    if (frame->reqRegNum == 0 || frame->reqRegNum > EMS_MODBUS_MAX_READ_REGS)
    {
        return 1;
    }

    if (ems_modbus_read_hold_registers(frame->reqAddr, frame->reqRegNum, regs) != 0)
    {
        return 1;
    }

    for (i = 0; i < frame->reqRegNum; i++)
    {
        WRITE_BT_INT16U(buffer, pos, regs[i]);
    }

    fillMdTcpConstPos(frame, p_tcp, buffer, pos);
    return 0;
}

/* ========================================================================== */
/* 【输入寄存器 Input Register】功能码 0x04 — 只读                                */
/* ========================================================================== */

/*****************************************************************************
 * @brief  读输入寄存器
 * @note   堆 1~99、簇 100+3000*n 均由 map 层实现（STEP 1/3）
 *****************************************************************************/
static int ems_modbus_tcp_fun04_read_input_regisiter(int fd, modbus_tcp_frame_t *frame,
                                                     tcp_server_lib_t *p_tcp)
{
    uint16_t regs[EMS_MODBUS_MAX_READ_REGS];
    uint16_t i;
    uint8_t *buffer = &p_tcp->send_buf[9];
    uint8_t pos = 0;

    (void)fd;

    if (frame->reqRegNum == 0 || frame->reqRegNum > EMS_MODBUS_MAX_READ_REGS)
    {
        return 1;
    }

    if (ems_modbus_read_input_registers(frame->reqAddr, frame->reqRegNum, regs) != 0)
    {
        return 1;
    }

    for (i = 0; i < frame->reqRegNum; i++)
    {
        WRITE_BT_INT16U(buffer, pos, regs[i]);
    }

    fillMdTcpConstPos(frame, p_tcp, buffer, pos);
    return 0;
}

/* ========================================================================== */
/* 【保持寄存器 Holding Register】功能码 0x06 / 0x10 — 写                         */
/* ========================================================================== */

static int ems_modbus_tcp_fun06_set_regisiter(int fd, modbus_tcp_frame_t *frame,
                                              tcp_server_lib_t *p_tcp)
{
    int32_t data;

    (void)fd;

    data = (int32_t)((frame->data[0] << 8) + frame->data[1]);
    if (ems_modbus_write_single_hold_register(frame->reqAddr, (uint16_t)data) != 0)
    {
        return 1;
    }

    memcpy(p_tcp->send_buf, p_tcp->recv_buf, p_tcp->recv_len);
    p_tcp->send_len = frame->length + 6;
    return 0;
}

static int ems_modbus_tcp_fun0x10_set_hold_regisiter(int fd, modbus_tcp_frame_t *frame,
                                                     tcp_server_lib_t *p_tcp)
{
    uint8_t byte_count;
    const uint8_t *reg_values;
    uint16_t i;
    uint16_t values[EMS_MODBUS_MAX_READ_REGS];

    (void)fd;

    if (frame->reqRegNum == 0 || frame->reqRegNum > EMS_MODBUS_MAX_READ_REGS)
    {
        return 1;
    }

    if (frame->length < 3 + frame->reqRegNum * 2)
    {
        return 1;
    }

    byte_count = frame->data[2];
    reg_values = frame->data + 3;
    if (byte_count != frame->reqRegNum * 2)
    {
        return 1;
    }

    for (i = 0; i < frame->reqRegNum; i++)
    {
        values[i] = (uint16_t)((reg_values[i * 2] << 8) | reg_values[i * 2 + 1]);
    }

    if (ems_modbus_write_multiple_hold_registers(frame->reqAddr, frame->reqRegNum, values) != 0)
    {
        return 1;
    }

    p_tcp->send_buf[0] = (uint8_t)(frame->transaction_id >> 8);
    p_tcp->send_buf[1] = (uint8_t)(frame->transaction_id);
    p_tcp->send_buf[2] = 0x00;
    p_tcp->send_buf[3] = 0x00;
    p_tcp->send_buf[4] = 0x00;
    p_tcp->send_buf[5] = 0x06;
    p_tcp->send_buf[6] = frame->unit_id;
    p_tcp->send_buf[7] = frame->function_code;
    p_tcp->send_buf[8] = (uint8_t)(frame->reqAddr >> 8);
    p_tcp->send_buf[9] = (uint8_t)(frame->reqAddr & 0xFF);
    p_tcp->send_buf[10] = (uint8_t)(frame->reqRegNum >> 8);
    p_tcp->send_buf[11] = (uint8_t)(frame->reqRegNum & 0xFF);
    p_tcp->send_len = 12;
    return 0;
}

/* ========================================================================== */
/* 注册到 bsp_modbusSlave（502 从站收到报文时分发到此）                           */
/* ========================================================================== */

/*****************************************************************************
 * @brief  注册 EMS / 上位机 Modbus TCP 功能码回调
 * @note   由 creatModbusTcpEmsServer() 调用，在 creatModbusTcpStdSlaveTask 之前执行
 *****************************************************************************/
void protocol_ems_modbus_tcp_init(void)
{
    register_modbus_func_call(0x02, ems_modbus_tcp_fun02_read_discrete_input);   /* 离散输入 */
    register_modbus_func_call(0x03, ems_modbus_tcp_fun03_read_hold_regisiter);   /* 保持寄存器·读 */
    register_modbus_func_call(0x04, ems_modbus_tcp_fun04_read_input_regisiter);  /* 输入寄存器 */
    register_modbus_func_call(0x06, ems_modbus_tcp_fun06_set_regisiter);        /* 保持寄存器·写 */
    register_modbus_func_call(0x10, ems_modbus_tcp_fun0x10_set_hold_regisiter); /* 保持寄存器·写 */
}
