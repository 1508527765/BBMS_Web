/*****************************************************************************
 * @copyright       2024-2024,AO DI LTD.
 * @file            drv_uart.c
 * @brief           串口底层驱动程序
 * @author          Gary
 * @date            2024/12/02
 * @remark          初修订
 *****************************************************************************/
#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include "kit_log.h"
#include "drv_uart.h"

/*****************************************************************************
 * @brief       打开串口驱动
 * @param[in]   p_uart: 串口驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int32_t drvUartOpen(uart_lib_t* p_uart)
{
    // 定义串口时序控制
    struct termios options;
    p_uart->fd = open((const char *)p_uart->address,O_RDWR | O_NOCTTY | O_NDELAY | O_EXCL);
    if (p_uart->fd == -1) {
        //p_uart->lastError = SERIAL_PORT_ERROR_OPEN_FAILED;
        return 1;
    }

    // 获取当前串口配置
    if (tcgetattr(p_uart->fd, &options) != 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "tcgetattr failed");
        close(p_uart->fd);
        return 1;
    }

    // 设置等待时间和最小接受字符
    options.c_cc[VTIME] = p_uart->timeout / 100; // 设置等待时间（单位：十分之一秒）
    options.c_cc[VMIN] = 1;                      // 最小接受字符
    //options.c_cflag |= (CREAD | CLOCAL);
    options.c_cflag &= ~CRTSCTS; // 不进行流控制

    //options.c_cflag &= CRTSCTS; // 进行流控制

    // 设置波特率
    switch (p_uart->baud)
    {
        case kUart_Baud_115200:
            cfsetospeed(&options, B115200);
            cfsetispeed(&options, B115200);
            break;
        case kUart_Baud_57600:
            cfsetospeed(&options, B57600);
            cfsetispeed(&options, B57600);
            break;
        case kUart_Baud_38400:
            cfsetospeed(&options, B38400);
            cfsetispeed(&options, B38400);
            break;
        case kUart_Baud_19200:
            cfsetospeed(&options, B19200);
            cfsetispeed(&options, B19200);
            break;
        case kUart_Baud_9600:
            cfsetospeed(&options, B9600);
            cfsetispeed(&options, B9600);
            break;
        case kUart_Baud_4800:
            cfsetospeed(&options, B4800);
            cfsetispeed(&options, B4800);
            break;
        case kUart_Baud_2400:
            cfsetospeed(&options, B2400);
            cfsetispeed(&options, B2400);
            break;
        case kUart_Baud_1800:
            cfsetospeed(&options, B1800);
            cfsetispeed(&options, B1800);
            break;
        case kUart_Baud_1200:
            cfsetospeed(&options, B1200);
            cfsetispeed(&options, B1200);
            break;
        case kUart_Baud_600:
            cfsetospeed(&options, B600);
            cfsetispeed(&options, B600);
            break;
        case kUart_Baud_300:
            cfsetospeed(&options, B300);
            cfsetispeed(&options, B300);
            break;
        case kUart_Baud_200:
            cfsetospeed(&options, B200);
            cfsetispeed(&options, B200);
            break;
        case kUart_Baud_150:
            cfsetospeed(&options, B150);
            cfsetispeed(&options, B150);
            break;
        default:
            //sprintf(log, "未知波特率：%d", p_uart->baud);
            return 1;
    }


    options.c_cflag |= (CREAD | CLOCAL);
    /* Set data bits (5/6/7/8) */
    options.c_cflag &= ~CSIZE;
    switch (p_uart->data)
    {
        case kUart_Data_5:
            options.c_cflag |= CS5;
            break;
        case kUart_Data_6:
            options.c_cflag |= CS6;
            break;
        case kUart_Data_7:
            options.c_cflag |= CS7;
            break;
        case kUart_Data_8:
            options.c_cflag |= CS8;
            break;
        default:
            //sprintf(log, "未知数据位：%d", p_uart->data);
            return 1;
    }


    // 设置停止位
    switch (p_uart->stop)
    {
        case kUart_Stop_1:
            options.c_cflag &= ~CSTOPB; // 一位停止位
            break;
        case kUart_Stop_2:
            options.c_cflag |= CSTOPB; // 两位停止位
            break;
        default:
            //sprintf(log, "未知停止位：%d", p_uart->stop);
            return 1;
    }

    // 禁用输出处理
    options.c_oflag &= ~OPOST;
    // 禁用规范模式
    options.c_lflag &= ~(ICANON | ECHO | ECHONL | ISIG | ECHOE);
    // 启用接收器并忽略调制解调器状态行
    options.c_cflag |= CREAD | CLOCAL;
    // 禁用换行符转换
    options.c_oflag &= ~ONLCR;
    //禁用软件流特殊控制
    options.c_iflag &= ~(IXON | IXOFF | IXANY | ICRNL);
    // 禁用特殊输入处理
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);

    // 清空串口的输入输出缓冲区
    tcflush(p_uart->fd, TCIOFLUSH);

    // 激活配置，将配置的参数应用到串口
    if (tcsetattr(p_uart->fd, TCSANOW, &options) < 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "tcsetattr failed");
        close(p_uart->fd);
        p_uart->fd = -1;
        //p_uart->lastError = SERIAL_PORT_ERROR_INVALID_ARGUMENT;

        return 1;
    }

    return 0;
}
/*****************************************************************************
 * @brief       关闭串口驱动
 * @param[in]   p_uart: 串口驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int32_t drvUartClose(uart_lib_t* p_uart)
{
    if (p_uart->fd > 0)
    {
        close(p_uart->fd);
        p_uart->fd = 0;
        p_uart->status = CONN_OFFLINE;
    }
    else
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "关闭失败！");
        return 1;
    }
    return 0;
}

// #define UART_DEBUG  0
void uartPrintLog(const char *str, uint8_t *data, uint16_t len, uint8_t order)
{
    uint8_t sdata[256 * 2 * 2] = {0};
    int pos = 0;
    #ifdef UART_DEBUG
        if (len > 0)
        {
            for (int i = 0; i < len; i++)
            {
                pos += sprintf((char*)(sdata + pos), "%02x ", data[i]);
            }
            sprintf((char*)(sdata + pos), "\n");
        }
        KITPTF(LOG_DRIVER_EN, INFO_EN, "串口：%d,%s:%s", order, str, sdata);
        KITLOG(LOG_DRIVER_EN, INFO_EN, "串口：%d,%s:%s", order, str, sdata);
    #endif // DEBUG
}

/*****************************************************************************
 * @brief       串口驱动发送
 * @param[in]   p_uart: 串口驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int32_t drvUartSend(uart_lib_t* p_uart)
{
// 连接状态
    int ret = 0;
#if 0
    if (p_uart->status == CONN_OFFLINE)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "串口(%s) 离线\n", p_uart->address);
        return 1;
    }
    
    fd_set set;
    FD_ZERO(&set);
    FD_SET(p_uart->fd, &set);

    struct timeval time_out = {1, 0};
    //int ret = select(p_uart->fd + 1, NULL,&set, NULL, &time_out); 

    if (0 == ret)
    {
        printf("串口select 超时 !!! fd:%d\r\n",p_uart->fd);
        //return -128;
    }
    else if (ret < 0)
    {
        printf("串口select 失败 为负值 !!! fd:%d\r\n",p_uart->fd);
        //return -1;
    }
#endif

    ssize_t rett = 0;
    ///tcflush(p_uart->fd, TCIOFLUSH);
    while (rett < p_uart->send_len)
    {
        ssize_t rc = write(p_uart->fd, p_uart->send_buf + rett, p_uart->send_len - rett);

        if (rc > 0)
        {
            rett += rc;
        }
        else
        {
            // 发生错误
            KITLOG(LOG_DRIVER_EN, ERROR_EN, "串口(%s) 发送错误! ret: %ld, errno: %s(%d)\n",
                   p_uart->address, ret, strerror(errno), errno);
            return 1;
        }
    }
    uartPrintLog("串口发送：", p_uart->send_buf, p_uart->send_len, p_uart->uartId);
    //tcdrain(p_uart->fd); // 确保数据实际发送出去
    return 0;
}

/*****************************************************************************
 * @brief       串口驱动接收
 * @param[in]   p_uart: 串口驱动结构体指针
 * @return      0-成功  1-失败
 *****************************************************************************/
int32_t drvUartRecv(uart_lib_t* p_uart)
{
    fd_set set;
    FD_ZERO(&set);
    FD_SET(p_uart->fd, &set);
    struct timeval time_out = {1, 0};

    static uint8_t retries = 0;    
    uint8_t expectedLength =  p_uart->recv_len;
    //expectedLength = 67;
    // expectedLength = 65;
    p_uart->recv_len = 0;
    //同一帧最多接收p_uart->error_cnt，收不到全量报文，强行退出
    while (p_uart->recv_len < expectedLength)
    {
        int ret = select(p_uart->fd + 1, &set, NULL, NULL, &time_out); // 改为读集合
        if (0 == ret)
        {
            // tcflush(p_uart->fd, TCIOFLUSH);
            uartPrintLog("-128串口接收：", p_uart->recv_buf, p_uart->recv_len, p_uart->uartId);
            return -128;
        }
        else if (ret < 0)
        {
            // tcflush(p_uart->fd, TCIOFLUSH);
            uartPrintLog("-1串口接收：", p_uart->recv_buf, p_uart->recv_len, p_uart->uartId);
            return -1;
        }
        
        ssize_t len = read(p_uart->fd, p_uart->recv_buf + p_uart->recv_len, expectedLength - p_uart->recv_len);
        if (len > 0)
        {
            p_uart->recv_len += len;
            // 检查是否接收到完整的帧
            if (p_uart->recv_len >= expectedLength)
            {
                retries = 0;
                tcflush(p_uart->fd, TCIOFLUSH);
                uartPrintLog("串口接收：", p_uart->recv_buf, p_uart->recv_len, p_uart->uartId);
                return 0;
            }
        }
        else if (len < 0)
        {
            KITLOG(LOG_DRIVER_EN, ERROR_EN, "串口(%d) 接收数据失败! ret: %zd, errno: %s(%d)\n",
                p_uart->uartId, len, strerror(errno), errno);
            return 1; // 接收失败
        }
        // 如果未达到期望长度，可以考虑重试接收
        if (p_uart->recv_len < expectedLength)
        {
            retries++;
            if(retries >= 3)
            {
                KITLOG(LOG_DRIVER_EN, ERROR_EN, "串口(%d) 全量数据接收失败! 期望长度: %d, 实际接收长度:%d",p_uart->uartId, expectedLength, p_uart->recv_len);
                // 清空串口接收缓冲区
                tcflush(p_uart->fd, TCIFLUSH);
                return -1;
            }
            usleep(100000); // 等待一段时间后重试
        }
    }
    return 0;
}
