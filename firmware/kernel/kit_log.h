
/*****************************************************************************
 * @copyright       1997-2010,AO DI LTD.
 * @file            kit_log.h
 * @brief           日志功能
 * @author          Gary
 * @date            2024-12-02
 * @remark
 *****************************************************************************/
#ifndef BSP_LOG_H
#define BSP_LOG_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "stdbool.h"


#define PATH "/opt/bbms/log/" 

#define FATAL_LOG "fatal.log"
#define ERROR_LOG "error.log"
#define WARN_LOG "warn.log"
#define INFO_LOG "info.log"
#define DEBUG_LOG "debug.log"

#define FATAL_LOG_BACKUP "fatal_backup.log"
#define ERROR_LOG_BACKUP "error_backup.log"
#define WARN_LOG_BACKUP "warn_backup.log"
#define INFO_LOG_BACKUP "info_backup.log"
#define DEBUG_LOG_BACKUP "debug_backup.log"

#define MAX_LEN_PER_LOG 256 

// 定义宏，根据输入的精度值（如0.01或0.1）选择打印精度
#define PRINT_BY_PRECISION(value, precision) \
    do { \
        if (precision == 0.01) { \
            printf("%.2f", value); \
        } else if (precision == 0.1) { \
            printf("%.1f", value); \
        } else { \
            printf("%f", value);  /* 默认输出完整的浮点数 */ \
        } \
    } while (0)


//根据precision小数位数打印，如：precision = 1则1.123打印为1.1
#define PRINT_DOUBLE(value, precision) printf("%.*f", precision, value);

/*
 * usr config one log size
 */
#define MAX_LOG_SIZE 50000000


#define MAX_LINE_SIZE 65535

typedef enum
{
	LOG_EN_START = 0,
	LOG_APP_EN = LOG_EN_START,
	LOG_CFGPARSE_EN,
	LOG_MODBUS_EN,
	LOG_DB_EN,
	LOG_IEC104_EN,
	LOG_LOGIC_EN,
	LOG_DRIVER_EN,
	LOG_KIT_EN,
	LOG_WM_EN,
	LOG_MQTT_EN,
	LOG_SYSBAK_EN,
	LOG_SM_EN,
	LOG_BMS_EN,
	LOG_ARGPARSE_EN,
	LOG_DAEMON_EN,
	Log_End
} LogSDK_E;


#define DISEABLE 0

typedef enum
{
	Level_Start = 1,
	FATAL_EN = Level_Start, // 使能必须从1开始，0为不使能
	ERROR_EN,
	WARN_EN,
	INFO_EN,
	DEBUG_EN,
	Level_End
} LogLevel_E;

/*
 * usr config log SDK & LEVEL enable in config.c
 *
 * [SDK Module Enable][Level Enable]
 * [SDK Module Enable]:
 * 		Mqtt : 1
 * 		RTDB : 2
 * [Level Enable]:
 * 		FATALEER : 1
 * 		ERROR 	 : 2
 * 		WARN 	 : 3
 * 		INFO 	 : 4
 * 		DEBUG	 : 5
 */
extern u_int8_t g_LogEnable[Log_End][Level_End]; // in log file

extern u_int8_t g_PrintEnable[Log_End][Level_End]; // in debug cmd
/*
 * usr in log api
 * module:SDK    in LogSDK_E
 * level :level  in LogLevel_E
 * 	__FILE__ 		宏在预编译时会替换成当前的源文件名
 *	__LINE__		宏在预编译时会替换成当前的行号
 *	__FUNCTION__	宏在预编译时会替换成当前的函数名称
 */
#define KITLOG(module, level, format, ...) LogPrintf(__FILE__, __FUNCTION__, __LINE__, module, level, format, ##__VA_ARGS__) // in log file
void LogPrintf(const char *file, const char *func, const int line, u_int8_t module, u_int8_t level, const char *fmt, ...);

#define KITPTF(module, level, format, ...) DebugPrintf(__FILE__, __FUNCTION__, __LINE__, module, level, format, ##__VA_ARGS__) // in log file
void DebugPrintf(const char *file, const char *func, const int line, u_int8_t module, u_int8_t level, const char *fmt, ...);

// 日志开关，0:不启用，1:启用文件打印，2:启用控制台打印，3：启用文件打印和控制台打印
typedef enum
{
	Enum_LogType_Start,
	Log_Off = Enum_LogType_Start,
	Log_File,
	Log_Std,
	Log_File_Std,
	Enum_LogType_End = Log_File_Std,
} LogEnableType;
void PrintLog(uint16_t logEnable, char *level, const char *function, const uint32_t line, char *filepath, uint16_t fileMaxsize, char *format, ...);

#endif // BSP_LOG_H
