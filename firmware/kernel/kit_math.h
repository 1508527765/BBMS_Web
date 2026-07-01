/*****************************************************************************
 * @copyright       1997-2010,AO DI LTD.
 * @file            kit_math.h
 * @brief           xx功能
 * @author          Gary
 * @date            2024-12-03
 * @remark
 *****************************************************************************/

#ifndef INCLUDE_KIT_MARH_H_
#define INCLUDE_KIT_MARH_H_
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "kit_data.h"
#include "kit_log.h"

// #define EPSILON 0.000001

#define msleep(n) usleep(n * 1000)

#define KIT_ABS(a) (((a) < 0) ? (-(a)) : (a))
#define KIT_ABS_DIFF(a, b) (((a) > (b)) ? ((a) - (b)) : ((b) - (a)))
#define KIT_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define KIT_MIN(a, b) (((a) < (b)) ? (a) : (b))
#define KIT_UPPER_LIMIT(data, limit) (((data) > (limit)) ? (limit) : (data))
#define KIT_LOWER_LIMIT(data, limit) (((data) < (limit)) ? (limit) : (data))

#define KIT_CREAT_BIT(pos) ((uint32_t)0x01u << (pos))
#define KIT_GET_BIT(val, pos) ((val >> pos) & KIT_CREAT_BIT(0))

#define EQUAL_SECTION   1/1000

// 定义一个容差值用于浮点数比较
#define EPSILON 1e-6

#define IS_FLOAT_POSITIVE(x) ((x) > EPSILON)
#define IS_FLOAT_NEGATIVE(x) ((x) < -EPSILON)
#define IS_FLOAT_ZERO(x) (((x) > -EPSILON) && ((x) < EPSILON))
#define IS_FLOAT_EQUAL(a, b) (fabs((a) - (b)) < EPSILON)



// 宏定义，根据输入的浮点数返回相应的小数位数
#define GET_PRECISION(value) \
    (fabs((value) - 0.01) < EPSILON ? 2 : \
    fabs((value) - 0.1) < EPSILON ? 1 : \
    fabs((value) - 0.001) < EPSILON ? 3 : \
    fabs((value) - 1.0) < EPSILON ? 0 : 0)


int float_equal(float a, float b);
int double_equal(double a, double b);
float float_max(float a, float b);
float float_min(float a, float b);
bool float_diff(float a, float b);

uint8_t kit_check_sum8(uint8_t *buf, uint16_t len);
void kit_append_crc16(uint8_t *buf, uint32_t len);
uint16_t kit_check_crc16(const uint8_t *buf, uint32_t len);

void kit_set_buf(void *buf, uint32_t len, uint8_t value);
void kit_copy_buf(void *dest, const void *const src, uint32_t len);
bool kit_is_buf_same(const void *const dest, const void *const src, uint32_t len);

uint32_t kit_lt_read_buf(const uint8_t *const buf, uint32_t idx, uint32_t len);
uint32_t kit_bt_read_buf(const uint8_t *const buf, uint32_t idx, uint32_t len);
uint32_t kit_lt_add_read_buf(const uint8_t *const buf, uint32_t *idx, uint32_t len);
uint32_t kit_bt_add_read_buf(const uint8_t *const buf, uint32_t *idx, uint32_t len);

void kit_lt_write_buf(uint8_t *const buf, uint32_t idx, uint32_t value, uint32_t len);
void kit_add_lt_write_buf(uint8_t *buf, uint32_t *idx, uint32_t value, uint32_t len);
void kit_add_bt_write_buf(uint8_t *buf, uint32_t *idx, uint32_t value, uint32_t len);

void kit_add_lt_8u_write_buf(uint8_t *const buf, uint32_t *idx, uint8_t value);
void kit_add_lt_16u_write_buf(uint8_t *const buf, uint32_t *idx, uint16_t value);
void kit_add_lt_32u_write_buf(uint8_t *const buf, uint32_t *idx, uint32_t value);

int32_t kit_get_avg_filter_max_min(const int16_t *buf, uint32_t len);
int32_t kit_get_int32_avg_filter_max_min(const int32_t *buf, uint32_t len);
int32_t kit_get_avg_filter_max_min_diff(const int16_t *buf, uint32_t len, uint16_t *diff);
uint32_t kit_get_unsign_avg_filter_max_min(const uint16_t *buf, uint32_t len);
int32_t kit_get_dma_avg_filter_min_max(uint16_t *buf, uint32_t data_cnt, uint32_t idx, uint32_t sample_cnt);

uint32_t unsigned_add_signed_with_limit(uint32_t u_num, int32_t s_num, uint32_t upper_limit, uint32_t lower_limit);
void kit_bit_fill(uint8_t *buffer, uint8_t start_bit, uint8_t len, uint8_t data);
void kit_bit_fill_u16(uint16_t *buffer, uint8_t start_bit, uint8_t len, uint16_t data);
uint8_t kit_dec_to_bcd(uint8_t data);
uint8_t kit_dcb_to_dec(uint8_t data);

uint16_t kit_get_u16(uint8_t *data, data_endian_e type);
int16_t kit_get_s16(uint8_t *data, data_endian_e type);
uint32_t kit_get_u32(uint8_t *data, data_endian_e type);
int32_t kit_get_s32(uint8_t *data, data_endian_e type);
int64_t kit_get_s64(uint8_t *data, data_endian_e type);
uint64_t kit_get_u64(uint8_t *data, data_endian_e type);
float kit_get_float(uint8_t *data, data_endian_e type);
double kit_get_double(uint8_t *data, data_endian_e type);
uint8_t kit_byte_pos_bit(uint8_t *data, uint16_t index, uint16_t sizeLen);

// 是否存在变位
uint8_t kit_diff_change(float old_value, float new_value, float threshold);

uint8_t kit_fill_byte(uint8_t* data,data_type_e dataType, data_endian_e endian,double value);

/*********************************************************************
 * @brief    取出word中bit位bitpos开始取bitlen长度的bit数据
 * @param[in] bitPos bit开始位置从1开始。1表示bit0
 * @param[in] bitLen bit取出的长度
 * @param[in] word 原数据
 * @return 	 超出阈值 1 未超 0
 *********************************************************************/
uint8_t get_bit_word(uint8_t bitPos, uint8_t bitLen, uint32_t word);
#endif // INCLUDE_KIT_MARH_H_