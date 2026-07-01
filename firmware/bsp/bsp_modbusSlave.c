/*****************************************************************************
 * @file            bsp_modbusSlave.h
 * @brief           上层文件
 * @author          Gary
 * @date            2025/09/04
 * @remark          初修订
 *****************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include "bsp_modbusSlave.h"
#include "drv_tcp_server.h"  // 包含getInterfaceIP函数声明

// Modbus TCP 调试打印开关（默认关闭）
static bool g_modbus_tcp_debug_enable = false;

// Modbus TCP 网络接口名称（默认使用eth3）
static const char *g_ems_modbus_interface_name = "eth3";

/*****************************************************************************
 * @brief       打印Modbus TCP报文（十六进制格式）
 * @param[in]   prefix: 前缀字符串（如"接收"或"发送"）
 * @param[in]   buf: 报文缓冲区
 * @param[in]   len: 报文长度
 * @return      void
 *****************************************************************************/
static void printModbusTcpFrame(const char *prefix, const uint8_t *buf, size_t len)
{
    // 检查调试开关，如果未启用则直接返回
    if (!g_modbus_tcp_debug_enable)
    {
        return;
    }

    if (buf == NULL || len == 0)
    {
        return;
    }

    printf("[Modbus TCP %s] 长度=%zu 数据: ", prefix, len);
    for (size_t i = 0; i < len; i++)
    {
        printf("%02X ", buf[i]);
    }
    printf("\n");

    // 解析并打印关键字段
    if (len >= 8)
    {
        uint16_t transaction_id = (buf[0] << 8) | buf[1];
        uint16_t protocol_id = (buf[2] << 8) | buf[3];
        uint16_t length = (buf[4] << 8) | buf[5];
        uint8_t unit_id = buf[6];
        uint8_t function_code = buf[7];

        // 判断是读操作还是写操作
        const char *op_type = "未知";
        if (function_code == 0x01 || function_code == 0x02 || function_code == 0x03 || function_code == 0x04)
        {
            op_type = "读";
        }
        else if (function_code == 0x05 || function_code == 0x06 || function_code == 0x0F || function_code == 0x10)
        {
            op_type = "写";
        }
        else if ((function_code & 0x80) != 0)
        {
            // 错误响应
            op_type = "错误";
        }

        printf("  Transaction ID: 0x%04X, Protocol ID: 0x%04X, Length: %u, Unit ID: %u, Function Code: 0x%02X (%s操作)\n",
               transaction_id, protocol_id, length, unit_id, function_code, op_type);

        // 判断是请求还是响应（通过prefix判断，"接收"是请求，"发送"是响应）
        int is_request = (strcmp(prefix, "接收") == 0);
        const char *addr_label = is_request ? "请求地址" : "响应地址";

        // 根据功能码解析不同的字段
        if (len >= 12)
        {
            uint16_t addr = (buf[8] << 8) | buf[9];

            // 读操作请求：0x01, 0x02, 0x03, 0x04
            if (is_request && (function_code == 0x01 || function_code == 0x02 || function_code == 0x03 || function_code == 0x04))
            {   
                uint16_t req_reg_num = (buf[10] << 8) | buf[11];
                printf("  %s: %u, 请求寄存器数量: %u\n", addr_label, addr, req_reg_num);
            }
            // 读操作响应：功能码 + 字节数 + 数据
            else if (!is_request && (function_code == 0x01 || function_code == 0x02 || function_code == 0x03 || function_code == 0x04))
            {
                // 读操作响应格式不同，这里len>=12时可能是错误，应该用len>=9的情况
                // 但为了兼容，这里先检查字节数
                if (len >= 9)
                {
                    uint8_t byte_count = buf[8];
                    printf("  字节数: %u, 数据长度: %u\n", byte_count, byte_count);
                }
            }
            // 写单个线圈：0x05（请求和响应格式相同）
            else if (function_code == 0x05)
            {
                uint16_t coil_value = (buf[10] << 8) | buf[11];
                printf("  %s: %u, 线圈值: 0x%04X (%s)\n", addr_label, addr, coil_value, 
                       (coil_value == 0xFF00) ? "ON" : (coil_value == 0x0000) ? "OFF" : "无效");
            }
            // 写单个寄存器：0x06（请求和响应格式相同）
            else if (function_code == 0x06)
            {
                uint16_t reg_value = (buf[10] << 8) | buf[11];
                printf("  %s: %u, 寄存器值: %u (0x%04X)\n", addr_label, addr, reg_value, reg_value);
            }
            // 写多个线圈：0x0F
            else if (function_code == 0x0F)
            {
                uint16_t count = (buf[10] << 8) | buf[11];
                if (is_request)
                {
                    uint8_t byte_count = (len >= 13) ? buf[12] : 0;
                    printf("  %s: %u, 线圈数量: %u, 字节数: %u\n", addr_label, addr, count, byte_count);
                }
                else
                {
                    // 响应格式：地址 + 数量
                    printf("  %s: %u, 线圈数量: %u\n", addr_label, addr, count);
                }
            }
            // 写多个寄存器：0x10
            else if (function_code == 0x10)
            {
                uint16_t count = (buf[10] << 8) | buf[11];
                if (is_request)
                {
                    uint8_t byte_count = (len >= 13) ? buf[12] : 0;
                    printf("  %s: %u, 寄存器数量: %u, 字节数: %u\n", addr_label, addr, count, byte_count);
                    // 打印寄存器值
                    if (len >= 13 + byte_count && byte_count > 0)
                    {
                        printf("  寄存器值: ");
                        for (uint8_t i = 0; i < byte_count && (13 + i + 1) < len; i += 2)
                        {
                            uint16_t val = (buf[13 + i] << 8) | buf[13 + i + 1];
                            printf("%u(0x%04X) ", val, val);
                        }
                        printf("\n");
                    }
                }
                else
                {
                    // 响应格式：地址 + 数量
                    printf("  %s: %u, 寄存器数量: %u\n", addr_label, addr, count);
                }
            }
            // 错误响应（功能码最高位为1表示错误）
            else if ((function_code & 0x80) != 0)
            {
                uint8_t error_code = (len >= 9) ? buf[8] : 0;
                printf("  错误码: 0x%02X\n", error_code);
            }
        }
        // 处理读操作的响应（功能码 + 字节数 + 数据，len < 12的情况）
        else if (len >= 9 && !is_request && (function_code == 0x01 || function_code == 0x02 || function_code == 0x03 || function_code == 0x04))
        {
            uint8_t byte_count = buf[8];
            printf("  字节数: %u, 数据长度: %u\n", byte_count, byte_count);
        }
    }
}


/* 标准的modbus tcp从站回调使用 */
static modbusFunchandler mdTcpStd0x01Call = NULL, mdTcpStd0x02Call = NULL, mdTcpStd0x03Call = NULL, mdTcpStd0x04Call = NULL,mdTcpStd0x05Call = NULL,  mdTcpStd0x06Call = NULL, mdTcpStd0x10Call = NULL;


/*****************************************************************************
 * @brief       标准的modbus转发回调注册
 * @param[in]
 * @return      void
 *****************************************************************************/
void register_modbus_func_call(uint8_t func, modbusFunchandler Call)
{
    if (Call == NULL)
    {
        return;
    }
    switch (func)
    {
    case 0x01:
        mdTcpStd0x01Call = Call;
        break;
    case 0x02:
        mdTcpStd0x02Call = Call;
        break;
    case 0x03:
        mdTcpStd0x03Call = Call;
        break;
    case 0x04:
        mdTcpStd0x04Call = Call;
        break;
    case 0x05:
        mdTcpStd0x05Call = Call;
        break;
    case 0x06:
        mdTcpStd0x06Call = Call;
        break;
    case 0x10:
        mdTcpStd0x10Call = Call;
        break;
    default:
        break;
    }
    return;
}

static int mdTcpNoneFuncCall(int fd, modbus_tcp_frame_t *frame, tcp_server_lib_t *p_tcp)
{
    // Modbus TCP 错误响应格式：
    // 字节 0-1: Transaction ID (由调用者设置)
    // 字节 2-3: Protocol ID (固定为 0x0000)
    // 字节 4-5: Length (固定为 0x0003)
    // 字节 6: Unit ID (必须与请求的 Unit ID 一致)
    // 字节 7: Function Code (请求功能码 | 0x80)
    // 字节 8: Exception Code

    // 设置 Protocol ID (固定为 0x0000)
    p_tcp->send_buf[2] = 0x00;
    p_tcp->send_buf[3] = 0x00;
    // 设置 Length (固定为 0x0003)
    p_tcp->send_buf[4] = 0x00;
    p_tcp->send_buf[5] = 0x03;
    // 设置 Unit ID (必须与请求的 Unit ID 一致)
    p_tcp->send_buf[6] = frame->unit_id;
    // 设置错误功能码 (请求功能码 | 0x80)
    p_tcp->send_buf[7] = frame->function_code | 0x80;
    // 设置异常码
    p_tcp->send_buf[8] = E_ILLEGAL_FUNCTION;

    // 计算发送总长度
    p_tcp->send_len = 9;
    return 0;
}

static int mdTcpNoneAddrCall(int fd, modbus_tcp_frame_t *frame, tcp_server_lib_t *p_tcp)
{
    // Modbus TCP 错误响应格式：
    // 字节 0-1: Transaction ID (由调用者设置)
    // 字节 2-3: Protocol ID (固定为 0x0000)
    // 字节 4-5: Length (固定为 0x0003)
    // 字节 6: Unit ID (必须与请求的 Unit ID 一致)
    // 字节 7: Function Code (请求功能码 | 0x80)
    // 字节 8: Exception Code

    // 设置 Protocol ID (固定为 0x0000)
    p_tcp->send_buf[2] = 0x00;
    p_tcp->send_buf[3] = 0x00;
    // 设置 Length (固定为 0x0003)
    p_tcp->send_buf[4] = 0x00;
    p_tcp->send_buf[5] = 0x03;
    // 设置 Unit ID (必须与请求的 Unit ID 一致)
    p_tcp->send_buf[6] = frame->unit_id;
    // 设置错误功能码 (请求功能码 | 0x80)
    p_tcp->send_buf[7] = frame->function_code | 0x80;
    // 设置异常码
    p_tcp->send_buf[8] = E_ILLEGAL_DATA_ADDRESS;

    // 计算发送总长度
    p_tcp->send_len = 9;
    return 0;
}



// 解析Modbus TCP请求
int parseMdTcpDataframe(uint8_t *buffer, size_t len, modbus_tcp_frame_t *frame)
{
    if (len < 8)
    {
        fprintf(stderr, "Invalid Modbus TCP frame length\n");
        return -1;
    }
    // 解析Modbus TCP包头
    frame->transaction_id = (buffer[0] << 8) | buffer[1];
    frame->protocol_id = (buffer[2] << 8) | buffer[3];
    frame->length = (buffer[4] << 8) | buffer[5];
    frame->unit_id = buffer[6];
    frame->function_code = buffer[7];
    frame->reqAddr = (buffer[8] << 8) + buffer[9];
    frame->reqRegNum = (buffer[10] << 8) + buffer[11];
    // 数据部分从第8字节开始
    frame->data = buffer + 10;

    return 0;
}
void fillMdTcpConstPos(modbus_tcp_frame_t *frame, tcp_server_lib_t *p_tcp, uint8_t *data, uint8_t len)
{
    p_tcp->send_buf[0] = frame->transaction_id >> 8;
    p_tcp->send_buf[1] = frame->transaction_id;
    p_tcp->send_buf[2] = 00;
    p_tcp->send_buf[3] = 00;

    p_tcp->send_buf[4] = (len + 3) >> 8;
    p_tcp->send_buf[5] = len + 3;
    p_tcp->send_buf[6] = frame->unit_id;
    p_tcp->send_buf[7] = frame->function_code;
    p_tcp->send_buf[8] = len;
    // 计算发送总长度
    p_tcp->send_len = len + 6 + 3;
}




/*****************************************************************************
 * @brief       获取标准回调函数
 * @param[in]
 * @return      void
 *****************************************************************************/
modbusFunchandler getModbusTcpStdCall(uint8_t function_code)
{
    switch (function_code)
    {
    case 0x02:
        return mdTcpStd0x02Call;
    case 0x03:
        return mdTcpStd0x03Call;
    case 0x04:
        return mdTcpStd0x04Call;
    case 0x05:
        return mdTcpStd0x05Call;
    case 0x06:
        return mdTcpStd0x06Call;
    case 0x10:
        return mdTcpStd0x10Call;
    default:
        return mdTcpNoneFuncCall;
    }
}

/*****************************************************************************
 * @brief       解析接收及回调
 * @param[in]   p_tcp: tcp驱动结构体指针
 * @return      void
 *****************************************************************************/
int8_t stdModbusTcpSlavespondCall(void *item, int fd)
{
    tcp_server_lib_t *p_tcp = (tcp_server_lib_t *)item;
    modbus_tcp_frame_t repframe = {0};

    // 打印接收到的Modbus TCP报文
    //printModbusTcpFrame("接收", p_tcp->recv_buf, p_tcp->recv_len);

    parseMdTcpDataframe(p_tcp->recv_buf, p_tcp->recv_len, &repframe);
    modbusFunchandler handler = getModbusTcpStdCall(repframe.function_code);

    if(handler == NULL)
    {
        p_tcp->send_buf[0] = repframe.transaction_id >> 8;
        p_tcp->send_buf[1] = repframe.transaction_id;
        mdTcpNoneFuncCall(fd, &repframe, p_tcp);
    }
    else
    {
        int result = handler(fd, &repframe, p_tcp);
        if(result == 1)
        {
            // 进入错误地址范围
            p_tcp->send_buf[0] = repframe.transaction_id >> 8;
            p_tcp->send_buf[1] = repframe.transaction_id;
            mdTcpNoneAddrCall(fd, &repframe, p_tcp);
        }
        // 如果 result == 0，说明处理函数已经设置了响应数据，不需要额外处理
    }

    // 打印发送的Modbus TCP报文
    printModbusTcpFrame("发送", p_tcp->send_buf, p_tcp->send_len);

    if (drvTcpServerSend2(p_tcp, fd) == 0)
    {
#if 0
            printf("响应成功\n");// 响应成功
#endif
    }
    else
    {
#if 0
            printf("响应失败\n");// 响应成功
#endif
    }
    return 0;
}

/*****************************************************************************
 * @brief       创建标准的modbus从站任务
 * @param[in]   arg: tcp驱动结构体指针
 * @return      void
 *****************************************************************************/
void *modbusTcpStdSlaveThread(void *arg)
{
    if (arg == NULL)
    {
        return (void *)NULL;
    }

    tcp_server_lib_t *item = (tcp_server_lib_t *)arg;

    item->port = EMS_MODBUS_TCP_PORT;

    for (;;)
    {
        if (drvTcpServerOpen(item) != 0)
        {
            KITLOG(LOG_APP_EN, ERROR_EN, "EMS Modbus TCP %u 启动失败，5s 后重试", item->port);
            KITPTF(LOG_APP_EN, ERROR_EN, "EMS Modbus TCP %u 启动失败，5s 后重试", item->port);
            sleep(5);
            continue;
        }

        while (item->isListen)
        {
            if (drvTcpServerRecv(item) < 0)
            {
                KITLOG(LOG_APP_EN, ERROR_EN, "EMS Modbus TCP %u 接收异常，关闭后 5s 重试",
                       item->port);
                KITPTF(LOG_APP_EN, ERROR_EN, "EMS Modbus TCP %u 接收异常，关闭后 5s 重试",
                       item->port);
                drvTcpServerClose(item);
                sleep(5);
                break;
            }
        }
    }
}

#include "drv_tcp_server.h"  // 包含getInterfaceIP函数声明

/*****************************************************************************
 * @brief       创建标准的modbus从站任务
 * @param[in]   arg: tcp驱动结构体指针（可选，如果为NULL则使用默认eth2配置）
 * @return      void
 * @note        不需要绑定任何网口，没有意义，还会在成潜在不必要的问题，直接绑定到所有网口，不指定特定网口
 *****************************************************************************/
static tcp_server_lib_t p_tcp = {0};
void creatModbusTcpStdSlaveTask(void *arg)
{
    // 回调注册
    drvInitServerCall(&p_tcp, stdModbusTcpSlavespondCall);

    pthread_t tfd;
    if (pthread_create(&tfd, NULL, modbusTcpStdSlaveThread, (void *)&p_tcp) == 0)
    {
        KITPTF(LOG_APP_EN, INFO_EN, "modbus tcp服务器线程创建成功，绑定IP: %s",
               p_tcp.ip[0] != '\0' ? p_tcp.ip : "0.0.0.0(所有接口)");
        KITLOG(LOG_APP_EN, INFO_EN, "modbus tcp服务器线程创建成功，绑定IP: %s",
               p_tcp.ip[0] != '\0' ? p_tcp.ip : "0.0.0.0(所有接口)");
        return;
    }
}



 conn_status_e getNetCommStatus(void)
 {
    return p_tcp.status;
 }