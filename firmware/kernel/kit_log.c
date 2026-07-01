/*****************************************************************************
 * @copyright       1997-2010,AO DI LTD.
 * @file            bsp_log.c
 * @brief           日志功能
 * @author          Gary
 * @date            2024-12-02
 * @remark          初修订
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <fcntl.h>
#include <stdarg.h>
#include "kit_log.h"


// 输出到日志中使能开关 FATAL_EN,	ERROR_EN,	WARN_EN,	INFO_EN,	DEBUG_EN
u_int8_t g_LogEnable[Log_End][Level_End] =
{
    {LOG_APP_EN, FATAL_EN, ERROR_EN, WARN_EN, INFO_EN, DEBUG_EN},
    {LOG_CFGPARSE_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_MODBUS_EN, FATAL_EN, ERROR_EN, WARN_EN, INFO_EN, DEBUG_EN},
    {LOG_DB_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_IEC104_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_LOGIC_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_DRIVER_EN, FATAL_EN, ERROR_EN, WARN_EN, INFO_EN, DEBUG_EN},
    {LOG_KIT_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_WM_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_MQTT_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_SYSBAK_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_SM_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_BMS_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_ARGPARSE_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_DAEMON_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
};

// 输出至终端调试打印使能开关 //FATAL_EN,	ERROR_EN,	WARN_EN,	INFO_EN,	DEBUG_EN
u_int8_t g_PrintEnable[Log_End][Level_End] =
{
    {LOG_APP_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_CFGPARSE_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_MODBUS_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_DB_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_IEC104_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_LOGIC_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_DRIVER_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_KIT_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_WM_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_MQTT_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_SYSBAK_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_SM_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_BMS_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_ARGPARSE_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
    {LOG_DAEMON_EN, FATAL_EN, ERROR_EN, WARN_EN, DISEABLE, DEBUG_EN},
};

uint8_t remove_file_flag = 0; // 为了避免多线程操作竞争

// void LogV(char* file1, char* file2, const char* pszFmt, va_list argp)
// {
//     FILE* flog;

//     char logstr[MAX_LINE_SIZE + 1];
//     char datestr[16];
//     char timestr[16];
//     char mss[4];

//     struct timeval current_time;
//     gettimeofday(&current_time, NULL);
//     struct tm* now;
//     now = localtime(&current_time.tv_sec);
//     if (NULL == pszFmt || 0 == pszFmt[0])
//         return;
//     vsnprintf(logstr, MAX_LINE_SIZE, pszFmt, argp);
//     sprintf(datestr, "%04d-%02d-%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
//     sprintf(timestr, "%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
//     sprintf(mss, "%03d", (int)current_time.tv_usec / 1000);

//     flog = fopen(file1, "a");
//     if (NULL != flog)
//     {

//         fprintf(flog, "%s %s.%s %s\n", datestr, timestr, mss, logstr);

//         if (ftell(flog) > MAX_LOG_SIZE)
//         {
//             fclose(flog);
//             if (rename(file1, file2))
//             {
//                 remove(file1);
//             }
//         }
//         else
//         {
//             fclose(flog);
//         }
//     }
// }

void LogV(char* file1, char* file2, const char* pszFmt, va_list argp)
{
    FILE* flog;

    char logstr[MAX_LINE_SIZE + 1];
    char datestr[32];
    char timestr[16];
    char mss[12];

    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    struct tm* now;
    now = localtime(&current_time.tv_sec);
    if (NULL == pszFmt || 0 == pszFmt[0])
        return;

    vsnprintf(logstr, MAX_LINE_SIZE, pszFmt, argp);
    snprintf(datestr, sizeof(datestr), "%04d-%02d-%02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday);
    snprintf(timestr, sizeof(timestr), "%02d:%02d:%02d", now->tm_hour, now->tm_min, now->tm_sec);
    snprintf(mss, sizeof(mss), "%03d", (int)current_time.tv_usec / 1000);

    flog = fopen(file1, "a");
    if (NULL != flog)
    {
        fprintf(flog, "%s %s.%s %s\n", datestr, timestr, mss, logstr);
        if (ftell(flog) > MAX_LOG_SIZE)
        {
            fclose(flog);
            if (rename(file1, file2))
            {
                remove(file1);
            }
        }
        else
        {
            fclose(flog);
        }
    }
}

void LogPrintf(const char* file, const char* func, const int line, u_int8_t module, u_int8_t level, const char* fmt, ...)
{
    if (g_LogEnable[module][level] & level) // 功能分块对应的日志类型有效，且输入的日志类型也有效
    {
        char msg_buf[MAX_LINE_SIZE + 1] = { 0 };
        sprintf(msg_buf, "[%s : %s : %d] ", file, func, line);
        strcat(msg_buf + strlen(msg_buf), fmt);
        char file1[512], file2[512]; // 文件长度要足够大，能够存放文件路径，否则会导致日志文件保存失败
        memset(file1, 0, sizeof(file1));
        memset(file2, 0, sizeof(file2));
        switch (level)
        {
            case FATAL_EN:
                strcat(file1, PATH);
                strcat(file1, FATAL_LOG);
                strcat(file2, PATH);
                strcat(file2, FATAL_LOG_BACKUP);
                break;
            case ERROR_EN:
                strcat(file1, PATH);
                strcat(file1, ERROR_LOG);
                strcat(file2, PATH);
                strcat(file2, ERROR_LOG_BACKUP);
                break;
            case WARN_EN:
                strcat(file1, PATH);
                strcat(file1, WARN_LOG);
                strcat(file2, PATH);
                strcat(file2, WARN_LOG_BACKUP);
                break;
            case INFO_EN:
                strcat(file1, PATH);
                strcat(file1, INFO_LOG);
                strcat(file2, PATH);
                strcat(file2, INFO_LOG_BACKUP);
                break;
            case DEBUG_EN:
                strcat(file1, PATH);
                strcat(file1, DEBUG_LOG);
                strcat(file2, PATH);
                strcat(file2, DEBUG_LOG_BACKUP);
                break;
            default:
                break;
        }

        va_list argp;

        va_start(argp, fmt);
        LogV(file1, file2, msg_buf, argp);
        va_end(argp);
    }
}

void DebugPrintf(const char* file, const char* func, const int line, u_int8_t module, u_int8_t level, const char* fmt, ...)
{
    // printf("---------%d------------\n",level);
    if (g_PrintEnable[module][level] & level) // 功能分块对应的日志类型有效，且输入的日志类型也有效
    {
        char msg_buf[MAX_LINE_SIZE + 1];
        sprintf(msg_buf, "[%s : %s : %d]", file, func, line);

        switch (level)
        {
            case FATAL_EN:
                sprintf(msg_buf + strlen(msg_buf), "%s", " FATAL ");
                break;
            case ERROR_EN:
                sprintf(msg_buf + strlen(msg_buf), "%s", " ERROR ");
                break;
            case WARN_EN:
                sprintf(msg_buf + strlen(msg_buf), "%s", " WARN ");
                break;
            case INFO_EN:
                sprintf(msg_buf + strlen(msg_buf), "%s", " INFO ");
                break;
            case DEBUG_EN:
                sprintf(msg_buf + strlen(msg_buf), "%s", " DEBUG ");
                break;
            default:
                break;
        }
        strcat(msg_buf + strlen(msg_buf), fmt);

        char logstr[MAX_LINE_SIZE + 1];
        va_list argp;
        va_start(argp, fmt);
        vsnprintf(logstr, MAX_LINE_SIZE, msg_buf, argp);
        va_end(argp);

        printf("%s\n", logstr);
    }
}

void PrintFileLog(char* filepath, uint16_t fileMaxsize, char* format, va_list args)
{
    // 打开文件
    FILE* logFile = fopen(filepath, "a");

    // 检查文件是否成功打开
    if (logFile != NULL)
    {
        vfprintf(logFile, format, args);

        // 关闭文件
        fclose(logFile);

        // 检查文件是否超过指定大小，超过则备份
        struct stat fileStat = { 0 };
        if (stat(filepath, &fileStat) != 0)
        {
            perror("Error getting file status");
            return; // 返回0表示无法获取文件大小
        }
        if (fileMaxsize <= 0 || fileMaxsize > 100)
        {
            fileMaxsize = 10;
        }

        if (fileStat.st_size > fileMaxsize * 1024 * 1024)
        {
            if (remove_file_flag == 0) // 避免多线程竞争
            {
                remove_file_flag = 1;
                // 检查备份文件是否存在，如果存在则删除
                char backupFilePath[256] = { 0 };
                sprintf(backupFilePath, "%s_back.log", filepath);
                if (remove(backupFilePath) != 0)
                {
                    // 如果删除失败，可能文件不存在，忽略错误
                    perror("Error removing existing backup file, maybe not exist");
                }
                // 使用rename函数将文件重命名
                if (rename(filepath, backupFilePath) != 0)
                {
                    perror("Error renaming file");
                    remove_file_flag = 0;
                    return;
                }
                remove(filepath);
                remove_file_flag = 0;
            }
        }
    }
}

void PrintLog(uint16_t logEnable, char* level, const char* function, const uint32_t line, char* filepath, uint16_t fileMaxsize, char* format, ...)
{
    if (logEnable <= 0 || logEnable > 3)
    {
        return;
    }
    struct timeval current_time;
    char timeString[30]; // 用于存储日期时间的字符串，包括毫秒和结尾的空字符

    gettimeofday(&current_time, NULL);

    // 将秒和微秒转换为日期时间格式
    strftime(timeString, sizeof(timeString), "%Y-%m-%d %H:%M:%S", localtime(&current_time.tv_sec));

    // 将毫秒添加到时间字符串中
    sprintf(timeString + strlen(timeString), ".%03ld", current_time.tv_usec / 1000);

    // printf("当前日期和时间（包含毫秒）是：%s\n", timeString);
    char textFormat[2000] = { 0 };
    sprintf(textFormat, "[%s: %s  %s: %d]: %s\n", level, timeString, function, line, format);

    switch (logEnable)
    {
        case Log_Off:
        {
            return;
        }
        case Log_File:
        {
            va_list args;
            va_start(args, format);
            PrintFileLog(filepath, fileMaxsize, textFormat, args);
            va_end(args);
            return;
        }
        case Log_Std:
        {
            va_list args;
            va_start(args, format);
            vprintf(textFormat, args);
            va_end(args);
            return;
        }
        case Log_File_Std:
        {
            va_list args;
            va_start(args, format);
            PrintFileLog(filepath, fileMaxsize, textFormat, args);
            va_end(args);
            va_start(args, format);
            vprintf(textFormat, args);
            va_end(args);

            return;
        }
        default:
            return;
    }
}
