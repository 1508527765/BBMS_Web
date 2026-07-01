

#ifndef __INCLUDE_KIT_TIME_H__
#define __INCLUDE_KIT_TIME_H__

#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <sys/time.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <bits/types/struct_timespec.h>
#include <time.h>

// 优先使用现代的标准头文件
#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE >= 199309L
#include <time.h>   // for clock_gettime
#else
// 备用方案：对于不支持clock_gettime的旧系统
#include <sys/time.h> // for gettimeofday
#endif

typedef struct {
    uint16_t year;    // RTC 年 (0~3000, 65535=无效)
    uint8_t  month;   // RTC 月 (0~12, 255=无效)
    uint8_t  day;     // RTC 日 (0~31, 255=无效)
    uint8_t  hour;    // RTC 时 (0~24, 255=无效)
    uint8_t  minute;  // RTC 分 (0~60, 255=无效)
    uint8_t  second;  // RTC 秒 (0~60, 255=无效)
}rtc_time_t;

uint8_t kit_isTimeout_by_now(struct timeval *lasttime, uint32_t timeout_ms);
uint32_t kit_time_diff_ms(struct timeval nowtime, struct timeval lasttime);
uint64_t get_systicks(struct timeval *tv);

void kit_time_format_tamp(time_t ts, char *buf, size_t bufsize);

uint64_t get_epoch_time_ms(void);
void kit_delay_ms(uint32_t ms);

rtc_time_t kit_get_rtc_time(void);

/** @return 0 成功；-1 失败（mktime/settimeofday） */
int kit_set_rtc_time(const rtc_time_t *rtc);
#endif
