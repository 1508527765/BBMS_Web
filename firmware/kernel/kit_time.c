
#include "kit_time.h"
#include <errno.h>  // 用于 errno 和 EINTR

uint32_t kit_time_diff_ms(struct timeval nowtime, struct timeval lasttime)
{
    uint32_t scount = (nowtime.tv_sec - lasttime.tv_sec) * 1000;
    uint32_t mscount = (nowtime.tv_usec - lasttime.tv_usec) / 1000;

    return scount + mscount;
}

/**
 * @brief: 计算当前时间和上次时间差，若超过timeout_ms，返回真，并更新lasttime
 * @param {timeval} *lasttime
 * @param {uint32_t} timeout_ms
 * @return {*}
 * @author: XX
 * @Date: 2024-03-10
 */
uint8_t kit_isTimeout_by_now(struct timeval *lasttime, uint32_t timeout_ms)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    if (kit_time_diff_ms(now, *lasttime) >= timeout_ms)
    {
        *lasttime = now;
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief: 获取系统从开始启动以来的tick
 * @param {timeval} *tv
 * @return {*}  ms的值
 * @author: XX
 * @Date: 2024-07-07
 */
uint64_t get_systicks(struct timeval *tv)
{
    struct timespec ts;
    // 获取自系统启动以来的滴答数
    if (clock_gettime(1, &ts) == -1) // CLOCK_MONOTONIC
    {
        perror("clock_gettime");
        return 0;
    }
    if (tv != NULL)
    {
        tv->tv_sec = ts.tv_sec;
        tv->tv_usec = ts.tv_nsec / 1000;
    }

    return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

void kit_time_format_tamp(time_t ts, char *buf, size_t bufsize)
{
    struct tm tm_info;
    localtime_r(&ts, &tm_info);
    strftime(buf, bufsize, "%Y-%m-%d %H:%M:%S", &tm_info);
}

uint64_t get_epoch_time_ms(void)
{
    uint64_t milliseconds = 0;

// 优先使用 clock_gettime (POSIX.1-2001)
#if defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0) && defined(CLOCK_REALTIME)
    struct timespec ts;
    
    // 获取实时时间（从Epoch开始）
    if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
        // 将秒转换为毫秒，纳秒转换为毫秒
        milliseconds = (uint64_t)ts.tv_sec * 1000ULL + (uint64_t)(ts.tv_nsec / 1000000ULL);
        return milliseconds;
    }
    // 如果clock_gettime失败，回退到备用方案
#endif

// 备用方案：使用 gettimeofday (较旧但广泛支持)
#if !(defined(_POSIX_TIMERS) && (_POSIX_TIMERS > 0))
    struct timeval tv;
    
    if (gettimeofday(&tv, NULL) == 0) {
        // 将秒转换为毫秒，微秒转换为毫秒
        milliseconds = (uint64_t)tv.tv_sec * 1000ULL + (uint64_t)(tv.tv_usec / 1000ULL);
        return milliseconds;
    }
#endif

    // 如果所有方法都失败，返回0（理论上不应该发生）
    return 0;
}

/// @brief 延时函数，将毫秒转换为nanosleep使用的timespec
/// @param ms 毫秒
/// @return 无
void kit_delay_ms(uint32_t ms)
{
    struct timespec req;
    req.tv_sec  = ms / 1000;
    req.tv_nsec = (ms % 1000) * 1000000UL;

    struct timespec rem;

    while (nanosleep(&req, &rem) == -1 && errno == EINTR)
    {
        req = rem;   // 继续剩余时间
    }
}


rtc_time_t kit_get_rtc_time(void)
{
    rtc_time_t rtc_time;
    time_t now = time(NULL);
    struct tm tm_info;
    localtime_r(&now, &tm_info);

    rtc_time.year = tm_info.tm_year + 1900; // tm_year是从1900年开始的
    rtc_time.month = tm_info.tm_mon + 1;    // tm_mon是从0开始的
    rtc_time.day = tm_info.tm_mday;
    rtc_time.hour = tm_info.tm_hour;
    rtc_time.minute = tm_info.tm_min;
    rtc_time.second = tm_info.tm_sec;

    return rtc_time;
}

int kit_set_rtc_time(const rtc_time_t *rtc)
{
    struct tm tm_info;
    struct timeval tv;
    time_t sec;

    if (rtc == NULL)
    {
        return -1;
    }

    memset(&tm_info, 0, sizeof(tm_info));
    tm_info.tm_year = (int)rtc->year - 1900;
    tm_info.tm_mon = (int)rtc->month - 1;
    tm_info.tm_mday = (int)rtc->day;
    tm_info.tm_hour = (int)rtc->hour;
    tm_info.tm_min = (int)rtc->minute;
    tm_info.tm_sec = (int)rtc->second;
    tm_info.tm_isdst = -1;

    sec = mktime(&tm_info);
    if (sec == (time_t)-1)
    {
        return -1;
    }

    tv.tv_sec = sec;
    tv.tv_usec = 0;
    if (settimeofday(&tv, NULL) != 0)
    {
        return -1;
    }

    return 0;
}