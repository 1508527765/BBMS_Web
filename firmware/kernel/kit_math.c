/*****************************************************************************
 * @copyright       2024-202,AO DI LTD.
 * @file            xxx.c
 * @brief           xxxx
 * @author          xx
 * @date            2024/12/30
 * @remark          初修订
 *****************************************************************************/
#include <stdio.h>
#include <math.h>
#include "kit_math.h"

int float_equal(float a, float b)
{
    if (KIT_ABS_DIFF(a, b) < EPSILON)
        return 1;
    else
        return 0;
}

int double_equal(double a, double b)
{
    if (KIT_ABS_DIFF(a, b) < EPSILON)
        return 1;
    else
        return 0;
}

// #define MAX(x, y) (((x) > (y)) ? (x) : (y))
float float_max(float a, float b)
{
    if (float_equal(a, b))
    {
        return a;
    }
    float diff = a - b;
    if (diff > EPSILON)
    {
        return a;
    }
    return b;
}

// #define MIN(x, y) (((x) < (y)) ? (x) : (y))
float float_min(float a, float b)
{
    if (float_equal(a, b))
    {
        return a;
    }
    float diff = a - b;
    if (diff < -EPSILON)
    {
        return a;
    }
    return b;
}

/*****************************************************************************
 * @brief        判断两个数差距是否超过1%
 * @param[in]    a b 两个参数
 * @return       bool 超过返回true 不超过false
 *****************************************************************************/
bool float_diff(float a, float b)
{
    // 处理特殊情况：两个值都为0
    if (a == 0 && b == 0)
    {
        return false;
    }

    // 处理相等情况
    if (float_equal(a, b))
    {
        return false;
    }

    // 计算绝对差值
    double absolute_difference = fabs(a - b);

    // 使用平均值进行计算
    double average = (fabs(a) + fabs(b)) / 2.0;

    // 如果平均值为0，直接返回false
    if (average == 0)
    {
        return false;
    }

    // 计算差距比率
    double difference_ratio = absolute_difference / average;
    return difference_ratio >= EQUAL_SECTION;
}

const uint8_t crc_byte_hi[] =
{ 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01,
 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00,
 0xC1, 0x81, 0x40 };

const uint8_t crc_byte_lo[] =
{ 0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05,
 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A,
 0xCA, 0xCB, 0x0B, 0xC9, 0x09, 0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B,
 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14,
 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3, 0x11,
 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36,
 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF,
 0x3F, 0x3E, 0xFE, 0xFA, 0x3A, 0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28,
 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D,
 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26, 0x22,
 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63,
 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C,
 0xAC, 0xAD, 0x6D, 0xAF, 0x6F, 0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69,
 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE,
 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5, 0x77,
 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50,
 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55,
 0x95, 0x94, 0x54, 0x9C, 0x5C, 0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A,
 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B,
 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C, 0x44,
 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41,
 0x81, 0x80, 0x40 };

const uint32_t bit_array[32] =
{
    0x00000001,
    0x00000002,
    0x00000004,
    0x00000008,
    0x00000010,
    0x00000020,
    0x00000040,
    0x00000080,
    0x00000100,
    0x00000200,
    0x00000400,
    0x00000800,
    0x00001000,
    0x00002000,
    0x00004000,
    0x00008000,
    0x00010000,
    0x00020000,
    0x00040000,
    0x00080000,
    0x00100000,
    0x00200000,
    0x00400000,
    0x00800000,
    0x01000000,
    0x02000000,
    0x04000000,
    0x08000000,
    0x10000000,
    0x20000000,
    0x40000000,
    0x80000000,
};

uint8_t kit_check_sum8(uint8_t* buf, uint16_t len)
{
    uint32_t sum = 0;

    while (len-- > 0)
        sum += buf[len];

    return sum;
}

uint16_t kit_check_crc16(const uint8_t* buf, uint32_t len)
{
    uint8_t tmp, crc_low = 0xFF, crc_high = 0xFF;

    while (len-- > 0)
    {
        tmp = crc_low ^ (*buf++);
        crc_low = crc_high ^ crc_byte_hi[tmp];
        crc_high = crc_byte_lo[tmp];
    }
    return ((uint16_t)crc_high << 8 | crc_low);
}

void kit_append_crc16(uint8_t* buf, uint32_t len)
{
    uint16_t crc = kit_check_crc16(buf, len);
    kit_lt_write_buf(buf, len, crc, 2);
}

int32_t kit_get_avg_filter_max_min(const int16_t* buf, uint32_t len)
{
    uint32_t i;
    int32_t sum, min, max, tmp = 0;

    if ((buf != NULL) && (len > 2))
    {
        max = min = sum = buf[0];
        for (i = 1; i < len; i++)
        {
            tmp = buf[i];

            if (tmp > max)
                max = tmp;
            else if (tmp < min)
                min = tmp;

            sum += tmp;
        }
        tmp = (sum - max - min) / (int32_t)(len - 2);
    }
    return tmp;
}

int32_t kit_get_int32_avg_filter_max_min(const int32_t* buf, uint32_t len)
{
    uint32_t i;
    int32_t sum, min, max, tmp = 0;

    if ((buf != NULL) && (len > 2))
    {
        max = min = sum = buf[0];
        for (i = 1; i < len; i++)
        {
            tmp = buf[i];

            if (tmp > max)
                max = tmp;
            else if (tmp < min)
                min = tmp;

            sum += tmp;
        }
        tmp = (sum - max - min) / (int32_t)(len - 2);
    }
    return tmp;
}

int32_t kit_get_avg_filter_max_min_diff(const int16_t* buf, uint32_t len, uint16_t* diff)
{
    uint32_t i;
    int32_t sum, min, max, tmp = 0;

    if ((buf != NULL) && (len > 2))
    {
        max = min = sum = buf[0];
        for (i = 1; i < len; i++)
        {
            tmp = buf[i];

            if (tmp > max)
                max = tmp;
            else if (tmp < min)
                min = tmp;

            sum += tmp;
        }
        tmp = (sum - max - min) / (int32_t)(len - 2);
        *diff = max - min;
    }
    return tmp;
}

uint32_t kit_get_unsign_avg_filter_max_min(const uint16_t* buf, uint32_t len)
{
    uint32_t i, sum, min, max, tmp = 0;

    if ((buf != NULL) && (len > 2))
    {
        max = min = sum = buf[0];
        for (i = 1; i < len; i++)
        {
            tmp = buf[i];

            if (tmp > max)
                max = tmp;
            else if (tmp < min)
                min = tmp;

            sum += tmp;
        }
        tmp = (sum - max - min) / (int32_t)(len - 2);
    }
    return tmp;
}

int32_t kit_get_dma_avg_filter_min_max(uint16_t* buf, uint32_t data_cnt, uint32_t idx, uint32_t sample_cnt)
{
    uint32_t i;
    int32_t sum, max, min, tmp = 0;

    if ((buf != NULL) && (sample_cnt > 2))
    {
        buf = buf + idx;
        max = min = sum = buf[0];
        for (i = 1; i < sample_cnt; i++)
        {
            tmp = *(buf + i * data_cnt);

            if (tmp > max)
                max = tmp;
            else if (tmp < min)
                min = tmp;
            sum += tmp;
        }
        tmp = (sum - max - min) / (int32_t)(sample_cnt - 2);
    }
    return tmp;
}

void kit_copy_buf(void* dest, const void* const src, uint32_t len)
{
    if (((dest != NULL) && (src != NULL)))
    {
        while (len-- > 0)
        {
            ((uint8_t*)dest)[len] = ((uint8_t*)src)[len];
        }
    }
}

bool kit_is_buf_same(const void* const dest, const void* const src, uint32_t len)
{
    bool res = true;

    while (len-- > 0)
    {
        if (((uint8_t*)dest)[len] != ((uint8_t*)src)[len])
        {
            res = false;
            break;
        }
    }
    return res;
}

void kit_set_buf(void* buf, uint32_t len, uint8_t value)
{
    while (len-- > 0)
    {
        ((uint8_t*)buf)[len] = value;
    }
}

uint32_t kit_lt_read_buf(const uint8_t* const buf, uint32_t idx, uint32_t len)
{
    uint32_t i, data = 0;

    for (i = 0; i < len; i++)
    {
        data |= ((uint32_t)buf[idx + i]) << (i << 3);
    }

    return data;
}

uint32_t kit_add_lt_read_buf(const uint8_t* const buf, uint32_t* idx, uint32_t len)
{
    uint32_t i, data = 0;

    for (i = 0; i < len; i++)
    {
        data |= ((uint32_t)buf[(*idx)++]) << (i << 3);
    }

    return data;
}

void kit_lt_write_buf(uint8_t* const buf, uint32_t idx, uint32_t value, uint32_t len)
{
    while (len-- > 0)
        buf[idx + len] = (uint8_t)(value >> (len << 3));
}

void kit_add_lt_8u_write_buf(uint8_t* const buf, uint32_t* idx, uint8_t value)
{
    buf[(*idx)++] = value;
}

void kit_add_lt_16u_write_buf(uint8_t* const buf, uint32_t* idx, uint16_t value)
{
    buf[(*idx)++] = value;
    buf[(*idx)++] = value >> 8;
}

void kit_add_lt_32u_write_buf(uint8_t* const buf, uint32_t* idx, uint32_t value)
{
    buf[(*idx)++] = value;
    buf[(*idx)++] = value >> 8;
    buf[(*idx)++] = value >> 16;
    buf[(*idx)++] = value >> 24;
}

void kit_add_bt_write_buf(uint8_t* const buf, uint32_t* idx, uint32_t value, uint32_t len)
{
    uint32_t i;

    for (i = 0; i < len; i++)
    {
        buf[(*idx)++] = (uint8_t)(value >> ((len - i - 1) << 3));
    }
}

uint32_t kit_bt_read_buf(const uint8_t* const buf, uint32_t idx, uint32_t len)
{
    uint32_t i, data = 0;

    for (i = 0; i < len; i++)
    {
        data |= ((uint32_t)buf[idx + i]) << ((len - i - 1) << 3);
    }

    return data;
}

uint32_t kit_bt_add_read_buf(const uint8_t* const buf, uint32_t* idx, uint32_t len)
{
    uint32_t i, data = 0;

    for (i = 0; i < len; i++)
    {
        data |= ((uint32_t)buf[(*idx)++]) << ((len - i - 1) << 3);
    }

    return data;
}

uint32_t unsigned_add_signed_with_limit(uint32_t u_num, int32_t s_num, uint32_t upper_limit, uint32_t lower_limit)
{
    uint32_t temp;
    if (s_num > 0)
    {
        temp = (uint32_t)(s_num)+u_num;
        if (temp > upper_limit)
            temp = upper_limit;
    }
    else
    {
        if (u_num < (uint32_t)(0 - s_num))
            temp = lower_limit;
        else
            temp = (uint32_t)(s_num + u_num);
    }

    return temp;
}

uint8_t kit_dec_to_bcd(uint8_t data)
{
    uint8_t tmp = 0;

    while (data >= 10)
    {
        tmp++;
        data -= 10;
    }

    return ((tmp << 4) | data);
}

uint8_t kit_dcb_to_dec(uint8_t data)
{
    uint8_t tmp = 0;

    tmp = ((uint8_t)(data & (uint8_t)0xF0) >> (uint8_t)0x04) * 10;
    return (tmp + (data & (uint8_t)0x0F));
}

void kit_bit_fill(uint8_t* buffer, uint8_t start_bit, uint8_t len, uint8_t data)
{
    uint8_t tmp = 0xFF;
    uint8_t rtmp = *buffer;
    tmp >>= (8 - len);
    tmp <<= start_bit;
    tmp = ~tmp;
    rtmp &= tmp;
    tmp = 0xFF;
    tmp >>= (8 - len);
    data &= tmp;
    data <<= start_bit;
    rtmp |= data;
    *buffer = rtmp;
}

void kit_bit_fill_u16(uint16_t* buffer, uint8_t start_bit, uint8_t len, uint16_t data)
{
    uint16_t tmp = 0xFFFF;
    uint16_t rtmp = *buffer;
    tmp >>= (16 - len);
    tmp <<= start_bit;
    tmp = ~tmp;
    rtmp &= tmp;
    tmp = 0xFFFF;
    tmp >>= (16 - len);
    data &= tmp;
    data <<= start_bit;
    rtmp |= data;
    *buffer = rtmp;
}

/*********************************************************************
 * @brief    数据类型无符号16位数据的解析,并更新数据至设备数据表
 * @return 		返回无符号16位数据
 *********************************************************************/
uint16_t kit_get_u16(uint8_t* data, data_endian_e type)
{
    uint16_t u16_data = 0;
    if (type == E_AB)
    {
        u16_data = (uint16_t)data[0] << 8 | data[1];
    }
    else if (type == E_BA)
    {
        u16_data = (uint16_t)data[1] << 8 | data[0];
    }
    return u16_data;
}

/*********************************************************************
 * @brief    数据类型使符号16位数据的解析
 * @return 		返回有符号16位数据
 *********************************************************************/
int16_t kit_get_s16(uint8_t* data, data_endian_e type)
{
    int16_t i16_data = 0;
    if (type == E_AB)
    {
        i16_data = (int16_t)data[0] << 8 | data[1];
    }
    else if (type == E_BA)
    {
        i16_data = (int16_t)data[1] << 8 | data[0];
    }
    return i16_data;
}
/*********************************************************************
 * @brief    数据类型无符号32位数据的解析
 * @return 		返回无符号32位数据
 *********************************************************************/
uint32_t kit_get_u32(uint8_t* data, data_endian_e type)
{
    uint32_t u32_data = 0;
    if (type == E_ABCD || type == E_AB)
    {
        u32_data = (uint32_t)data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    }
    else if (type == E_CDAB || type == E_BA)
    {
        u32_data = (uint32_t)data[2] << 24 | data[3] << 16 | data[0] << 8 | data[1];
    }
    else if (type == E_BADC)
    {
        u32_data = (uint32_t)data[1] << 24 | data[0] << 16 | data[3] << 8 | data[2];
    }
    else if (type == E_DCBA)
    {
        u32_data = (uint32_t)data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];
    }
    return u32_data;
}

/*********************************************************************
 * @brief    数据类型有符号位数据的解析
 * @return 		返回符号有符号32位数据
 *********************************************************************/
int32_t kit_get_s32(uint8_t* data, data_endian_e type)
{
    int32_t i32_data = 0;
    if (type == E_ABCD || type == E_AB)
    {
        i32_data = (int32_t)data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3];
    }
    else if (type == E_CDAB || type == E_BA)
    {
        i32_data = (int32_t)data[2] << 24 | data[3] << 16 | data[0] << 8 | data[1];
    }
    else if (type == E_BADC)
    {
        i32_data = (int32_t)data[1] << 24 | data[0] << 16 | data[3] << 8 | data[2];
    }
    else if (type == E_DCBA)
    {
        i32_data = (int32_t)data[3] << 24 | data[2] << 16 | data[1] << 8 | data[0];
    }
    return i32_data;
}

/*********************************************************************
 * @brief    数据类型有符号位数据的解析
 * @return 		返回符号无符号64位数据
 *********************************************************************/
uint64_t kit_get_u64(uint8_t* data, data_endian_e type)
{
    uint64_t result = 0;

    if (type == E_HGFEDCBA)
    {
        // 小端模式
        for (int i = 0; i < 8; i++)
        {
            result |= ((uint64_t)data[i] << (i * 8));
        }
    }
    else if (type == E_ABCDEFGH)
    {
        // 大端模式
        for (int i = 0; i < 8; i++)
        {
            result |= ((uint64_t)data[i] << ((7 - i) * 8));
        }
    }
    else if (type == E_BADCFEHG)
    {
        // 大端字-小端字节（Big-Endian Word, Little-Endian Byte）
        for (int i = 0; i < 4; i++)
        {
            result |= ((uint64_t)data[i * 2 + 1] << ((7 - i * 2) * 8)); // 高位字节
            result |= ((uint64_t)data[i * 2] << ((6 - i * 2) * 8));     // 低位字节
        }
    }
    else if (type == E_GHEFCDAB)
    {
        // 小端字-大端字节（Little-Endian Word, Big-Endian Byte）
        for (int i = 0; i < 4; i++)
        {
            result |= ((uint64_t)data[i * 2 + 1] << (i * 2 * 8));   // 高位字节
            result |= ((uint64_t)data[i * 2] << ((i * 2 + 1) * 8)); // 低位字节
        }
    }
    return result;
}

/*********************************************************************
 * @brief    数据类型有符号位数据的解析
 * @return 		返回符号有符号64位数据
 *********************************************************************/
int64_t kit_get_s64(uint8_t* data, data_endian_e type)
{
    int64_t result = 0;

    if (type == E_HGFEDCBA)
    {
        // 小端模式
        for (int i = 0; i < 8; i++)
        {
            result |= ((int64_t)data[i] << (i * 8));
        }
    }
    else if (type == E_ABCDEFGH)
    {
        // 大端模式
        for (int i = 0; i < 8; i++)
        {
            result |= ((int64_t)data[i] << ((7 - i) * 8));
        }
    }
    else if (type == E_BADCFEHG)
    {
        // 大端字-小端字节（Big-Endian Word, Little-Endian Byte）
        for (int i = 0; i < 4; i++)
        {
            result |= ((int64_t)data[i * 2 + 1] << ((7 - i * 2) * 8)); // 高位字节
            result |= ((int64_t)data[i * 2] << ((6 - i * 2) * 8));     // 低位字节
        }
    }
    else if (type == E_GHEFCDAB)
    {
        // 小端字-大端字节（Little-Endian Word, Big-Endian Byte）
        for (int i = 0; i < 4; i++)
        {
            result |= ((int64_t)data[i * 2 + 1] << (i * 2 * 8));   // 高位字节
            result |= ((int64_t)data[i * 2] << ((i * 2 + 1) * 8)); // 低位字节
        }
    }
    return result;
}

/*********************************************************************
 * @brief    数据类型float32位数据的解析
 * @return 		返回float32位数据
 *********************************************************************/
float kit_get_float(uint8_t* data, data_endian_e type)
{
    union HexToF
    {
        float floatdata;
        uint8_t hex[4];
    } HtoF;

    if (type == E_ABCD)
    {
        HtoF.hex[0] = *(data);
        HtoF.hex[1] = *(data + 1);
        HtoF.hex[2] = *(data + 2);
        HtoF.hex[3] = *(data + 3);
    }
    else if (type == E_CDAB)
    {
        HtoF.hex[0] = *(data + 2);
        HtoF.hex[1] = *(data + 3);
        HtoF.hex[2] = *(data + 0);
        HtoF.hex[3] = *(data + 1);
    }
    else if (type == E_BADC)
    {
        HtoF.hex[0] = *(data + 1);
        HtoF.hex[1] = *(data);
        HtoF.hex[2] = *(data + 3);
        HtoF.hex[3] = *(data + 2);
    }
    else if (type == E_DCBA)
    {
        HtoF.hex[0] = *(data + 3);
        HtoF.hex[1] = *(data + 2);
        HtoF.hex[2] = *(data + 1);
        HtoF.hex[3] = *(data);
    }
    return HtoF.floatdata;
}

/*********************************************************************
 * @brief    数据类型double位数据的解析
 * @return 		返回double位数据
 *********************************************************************/
double kit_get_double(uint8_t* data, data_endian_e type)
{
    uint64_t int_repr = 0;
    double result;

    // 根据不同的端序模式处理字节序
    switch (type)
    {
        case E_ABCDEFGH: // 大端模式（Big Endian）
            for (int i = 0; i < 8; i++)
            {
                int_repr |= ((uint64_t)data[i] << ((7 - i) * 8));
            }
            break;

        case E_HGFEDCBA: // 小端模式（Little Endian）
            for (int i = 0; i < 8; i++)
            {
                int_repr |= ((uint64_t)data[i] << (i * 8));
            }
            break;

        case E_BADCFEHG: // 大端字-小端字节（Big-Endian Word, Little-Endian Byte）
            for (int i = 0; i < 4; i++)
            {
                int_repr |= ((uint64_t)data[i * 2 + 1] << ((7 - i * 2) * 8)); // 高位字节
                int_repr |= ((uint64_t)data[i * 2] << ((6 - i * 2) * 8));     // 低位字节
            }
            break;

        case E_GHEFCDAB: // 小端字-大端字节（Little-Endian Word, Big-Endian Byte）
            for (int i = 0; i < 4; i++)
            {
                int_repr |= ((uint64_t)data[i * 2 + 1] << (i * 2 * 8));   // 高位字节
                int_repr |= ((uint64_t)data[i * 2] << ((i * 2 + 1) * 8)); // 低位字节
            }
            break;

        default: // 默认大端模式（Big Endian）
            for (int i = 0; i < 8; i++)
            {
                int_repr |= ((uint64_t)data[i] << ((7 - i) * 8));
            }
            break;
    }
    // 将 uint64_t 表示转换为 double
    memcpy(&result, &int_repr, sizeof(result));
    return result;
}

/*********************************************************************
 * @brief    获取data中第index个bit位置的值
 * @return 	 返回double位数据
 *********************************************************************/
uint8_t kit_byte_pos_bit(uint8_t* data, uint16_t index, uint16_t dataSize)
{
    uint16_t i = index / 8;
    uint16_t k = index % 8;
    if (i < dataSize)
    {
        return KIT_GET_BIT(data[i], k);
    }
    else
    {
        KITLOG(LOG_KIT_EN, INFO_EN, "数据超过范围");
        KITPTF(LOG_KIT_EN, INFO_EN, "数据超过范围");
    }
    return 0;
}

/*********************************************************************
 * @brief    判断新值和旧值之间的差异是否超过指定的范围
 * @return 	 超出阈值 1 未超 0
 *********************************************************************/
uint8_t kit_diff_change(float old_value, float new_value, float threshold)
{
    float difference = fabs(new_value - old_value);
    return difference > threshold;
}

/*
    E_ABCD,		// 大端模式
    E_CDAB,		// 小端字、大端字节
    E_BADC,		// 大端字、小端字节
    E_DCBA,		// 小端模式
*/
uint8_t kit_fill_byte(uint8_t* data, data_type_e dataType, data_endian_e endian, double value)
{
    uint8_t idx = 0;
    switch (dataType)
    {
        //针对配置错误的U8，I8来说，也默认可正常下发
        case Uint8:
        case Int8:
        case Uint16:
        {
            uint16_t u16 = (uint16_t)value;
            if (endian == E_AB)
            {
                data[idx++] = u16 >> 8;
                data[idx++] = u16;
            }
            else if (E_BA)
            {
                data[idx++] = u16;
                data[idx++] = u16 >> 8;
            }
            break;
        }
        case Int16:
        {
            int16_t s16 = (int16_t)value;
            if (endian == E_AB)
            {
                data[idx++] = s16 >> 8;
                data[idx++] = s16;
            }
            else if (E_BA)
            {
                data[idx++] = s16;
                data[idx++] = s16 >> 8;
            }
            break;
        }
        case Uint32:
        case Int32:
            /* code */
            break;
        case Uint64:
        case Int64:
            /* code */
            break;

        case Float32:
        case Float64:
            /* code */
            break;

        default:
            break;
    }

    return idx;
}


/*********************************************************************
 * @brief    取出word中bit位bitpos开始取bitlen长度的bit数据
 * @param[in] bitPos bit开始位置从1开始。1表示bit0
 * @param[in] bitLen bit取出的长度
 * @param[in] word 原数据
 * @return 	 超出阈值 1 未超 0
 *********************************************************************/
uint8_t get_bit_word(uint8_t bitPos, uint8_t bitLen, uint32_t word)
{
    if (bitPos == 0)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "bitPos = %d,数据错误", bitPos);
        KITPTF(LOG_KIT_EN, ERROR_EN, "bitPos = %d,数据错误", bitPos);
        bitPos = 1;
    }
    uint32_t mask = (1 << bitLen) - 1;
    return (word >> (bitPos - 1)) & mask;
}
