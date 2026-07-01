/*****************************************************************************

 * @file            protocol_ems_modbus_map.h

 * @brief           EMS / 上位机共用 Modbus TCP 点表映射（V1.0.13）

 *

 * 配套实现文件（共 2 个 .c，外加本头文件）：

 *   protocol_ems_modbus_map.c   — 点表 + 读 RTDB/系统时间（所有寄存器含义）

 *   protocol_ems_modbus_server.c — TCP 502 功能码回调（只组帧，不解析点表）

 *

 * --------------------------------------------------------------------------

 * V1.0.13 主点表：三种 Modbus 寄存器类型（勿与「线圈 Coil」混淆）

 * --------------------------------------------------------------------------

 *  类型              功能码      读/写    地址范围（示例）           map.c 区块

 *  ----------------  ----------  -------  -------------------------  --------

 *  离散输入          0x02        只读     堆 1~199，簇 200+200*n      [1]

 *  (Discrete Input)

 *

 *  输入寄存器        0x04        只读     堆 1~99，簇 100+3000*n      [2][2R]

 *  (Input Register)

 *

 *  保持寄存器        0x03 读     读       500~699（501/503 不做）     [4]

 *  (Holding Reg)     0x06/0x10写 写       524~529 RTC 等

 *

 *  外设段（另一份点表）未实现：60100+

 * --------------------------------------------------------------------------

 * 数据流：RBMS/堆统计/故障 → RTDB → map.c → server.c → EMS/上位机

 * 与 5002 私有 HMI、modbus_default_server.c 通用模板无关。

 *****************************************************************************/

#ifndef PROTOCOL_EMS_MODBUS_MAP_H

#define PROTOCOL_EMS_MODBUS_MAP_H



#include <stdint.h>



/* ===================== 离散输入 Discrete Input（0x02）===================== */

#define EMS_MODBUS_BANK_DISCRETE_START     1u

#define EMS_MODBUS_BANK_DISCRETE_END       199u

#define EMS_MODBUS_RACK_DISCRETE_BASE      200u

#define EMS_MODBUS_RACK_DISCRETE_STRIDE    200u



/* ===================== 输入寄存器 Input Register（0x04）==================== */

#define EMS_MODBUS_BANK_INPUT_REG_START    1u

#define EMS_MODBUS_BANK_INPUT_REG_END      99u

#define EMS_MODBUS_RACK_INPUT_BASE         100u

#define EMS_MODBUS_RACK_INPUT_STRIDE       3000u



/* ===================== 保持寄存器 Holding Register（0x03/06/10）============ */

#define EMS_MODBUS_HOLD_REG_START          500u

#define EMS_MODBUS_HOLD_REG_END            699u

#define EMS_MODBUS_HOLD_SKIP_FAULT_RESET   501u   /* 不实现 */

#define EMS_MODBUS_HOLD_SKIP_EMS_POWER     503u   /* 不实现 */

#define EMS_MODBUS_HOLD_RTC_YEAR_ADDR      524u

#define EMS_MODBUS_HOLD_RTC_SECOND_ADDR    529u



/* ===================== 外设段（STEP 5，另一 xlsx）========================= */

#define EMS_MODBUS_EXT_DEVICE_REG_START    60100u
#define EMS_MODBUS_EXT_DEVICE_REG_END      61599u



/* ---------- 0x02 离散输入（只读） ---------- */

int ems_modbus_read_discrete_inputs(uint16_t start_addr, uint16_t bit_count, uint8_t *out_bits);



/* ---------- 0x04 输入寄存器（只读） ---------- */

int ems_modbus_read_input_registers(uint16_t start_addr, uint16_t reg_count, uint16_t *out_regs);



/* ---------- 0x03 保持寄存器（读） ---------- */

int ems_modbus_read_hold_registers(uint16_t start_addr, uint16_t reg_count, uint16_t *out_regs);



/* ---------- 0x06 / 0x10 保持寄存器（写） ---------- */

int ems_modbus_write_single_hold_register(uint16_t addr, uint16_t value);

int ems_modbus_write_multiple_hold_registers(uint16_t start_addr, uint16_t reg_count,

                                           const uint16_t *values);



#endif /* PROTOCOL_EMS_MODBUS_MAP_H */

