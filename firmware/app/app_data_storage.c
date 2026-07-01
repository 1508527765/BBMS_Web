/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            app_data_storage.c
 * @brief           数据存储模块源文件
 * @author          gary
 * @date            2025-07-10
 * @remark
 *****************************************************************************/
#include "app_data_storage.h"
#include <sys/time.h>

// 磁盘清理相关配置常量
#define DISK_CLEAN_THRESHOLD_PERCENT 80.0       // 磁盘使用率清理门限（百分比）
#define DISK_CLEAN_FILE_COUNT 5                 // 每次清理的文件数量
#define DEFAULT_STORAGE_PATH "/opt/bbms/storage" // 默认存储目录
#define MAX_PATH_LEN 4096                       // 最大路径长度

uint32_t exeCount = 0;                                             // 线程执行次数计数器，用于取余数
UT_array *timedDataConfigs, *faultsDataConfigs, *eventDataConfigs; // 时序数据配置、故障数据配置

// 时序数据的ICD结构定义
static UT_icd timed_data_storage_icd = {
    sizeof(timed_data_storage_t),
    NULL,
    NULL,
    NULL,
};

// 故障数据的ICD结构定义
static UT_icd faults_data_storage_icd = {
    sizeof(faults_data_storage_t),
    NULL,
    NULL,
    NULL,
};

/*****************************************************************************
 * @brief           封装的删除时序数据函数
 * @param[in]       interval: 存储间隔暂时默认全部周期一样，不做区分
 * @param[in]       timestamp: 删除之前数据时间戳
 *****************************************************************************/
static void deleteTimedData(int interval, time_t timestamp)
{
    struct tm tm_time;
    localtime_r(&timestamp, &tm_time);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_time);
    // 调用删除数据库函数
    int ret = kit_del_timed_data_by_date(interval, time_str);
    if (ret != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "数据删除失败, interval: %d, date: %s", interval, time_str);
    }
}

/*****************************************************************************
 * @brief           封装的删除故障数据函数
 * @param[in]       interval: 存储间隔
 * @param[in]       timestamp: 删除之前数据时间戳
 *****************************************************************************/
static void deleteFaultsData(time_t timestamp)
{
    struct tm tm_time;
    localtime_r(&timestamp, &tm_time);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_time);
    // 调用删除数据库函数
    int ret = kit_del_faults_data_by_date(time_str);
    if (ret != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "数据删除失败, date: %s", time_str);
    }
}

/*****************************************************************************
 * @brief           封装的删除故障数据函数
 * @param[in]       interval: 存储间隔
 * @param[in]       timestamp: 删除之前数据时间戳
 *****************************************************************************/
static void deleteEventsData(time_t timestamp)
{
    struct tm tm_time;
    localtime_r(&timestamp, &tm_time);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_time);
    // 调用删除数据库函数
    int ret = kit_del_events_data_by_date(time_str);
    if (ret != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "数据删除失败, date: %s", time_str);
    }
}

/*****************************************************************************
 * @brief           每天定时删除业务数据间隔
 *****************************************************************************/
static void delDbBusDataInterval()
{
    time_t now;
    time(&now);
    struct tm tm_time;
    localtime_r(&now, &tm_time);
    tm_time.tm_hour += 8; // 加8个小时，考虑时区

    time_t oneWeekAgo = now - 7 * 24 * 60 * 60;  // 7天前
    time_t twoWeekAgo = now - 14 * 24 * 60 * 60; // 7天前

    time_t t180dayAgo = now - 180 * 24 * 60 * 60; // 180天前

    deleteTimedData(0, twoWeekAgo);
    // 删除故障数据
    deleteFaultsData(t180dayAgo);

    // 删除事件数据
    deleteEventsData(t180dayAgo);

    kit_after_batch_delete_cleanup();
}

/*****************************************************************************
 * @brief           定时将RTDB数据保存到数据库
 *****************************************************************************/
static void saveRealDataInterval()
{
    UT_array *timedDatas = NULL;

    // 为时序数据动态数组分配空间
    utarray_new(timedDatas, &timed_data_storage_icd);
    for (int i = 0; i < utarray_len(timedDataConfigs); ++i)
    {
        timed_data_config_t *p_config = (timed_data_config_t *)utarray_eltptr(timedDataConfigs, i);
        if (p_config != NULL && p_config->isPersisted != 0) // 确保配置不为空且需要持久化
        {
            timed_data_storage_t data = {0};
            data.devType = p_config->devType;                 // 设备种类
            data.devDbId = p_config->devDbId;                 // 设备数据库Id
            data.storageInterval = p_config->storageInterval; // 存储间隔
            data.pointDbId = p_config->pointDbId;             // 点位Id
            data.pointId = p_config->pointId;                 // 点位编号
            data.devId = p_config->devId;                     // 设备编号
            data.timedValue = getRtdbPointValue(Rtdb_ShMem, data.devType, p_config->devId - 1,
                                                p_config->pointId); // 从实时库获取时序数据

            utarray_push_back(timedDatas, &data);
        }
    }

    // 插入时序数据；kit_insert_timed_data 成功时会释放 timedDatas，失败或空数组需在本函数释放
    if (utarray_len(timedDatas) > 0)
    {
        if (kit_insert_timed_data(timedDatas) != 0)
        {
            KITLOG(LOG_WM_EN, ERROR_EN, "kit_insert_timed_data 失败");
            utarray_free(timedDatas);
        }
    }
    else
    {
        utarray_free(timedDatas);
    }
}

/*****************************************************************************
 * @brief           定时将故障数据保存到数据库
 *****************************************************************************/
static void saveFaultsDataInterval()
{
    UT_array *faultsDatas = NULL;

    // 为故障数据动态数组分配空间
    utarray_new(faultsDatas, &faults_data_storage_icd);
    int fault_len = utarray_len(faultsDataConfigs);
    // printf("fault_len=%d\n", fault_len);
    for (int i = 0; i < utarray_len(faultsDataConfigs); ++i)
    {
        faults_data_config_t *p_config =
            (faults_data_config_t *)utarray_eltptr(faultsDataConfigs, i);
        if (p_config != NULL)
        {
            faults_data_storage_t data = {0};
            data.faultsValue = getRtdbPointValue(Rtdb_ShMem, p_config->devType, p_config->devId - 1,
                                                 p_config->pointId); // 从实时库获取时序数据
            // printf("设备类型，设备编号，点位编号，故障值=%d,%d,%d,%f\n", p_config->devType,
            // p_config->devId - 1, p_config->pointId, data.faultsValue);
            if (KIT_ABS_DIFF(data.faultsValue, p_config->faultsValue) < EPSILON) // 故障变化存储
            {
                continue;
            }
            // printf("故障值变化，准备存储\n");
            p_config->faultsValue = data.faultsValue; // 更新配置中的故障值
            data.devType = p_config->devType;         // 设备种类
            data.devId = p_config->devId;
            data.pointId = p_config->pointId;

            utarray_push_back(faultsDatas, &data);
        }
    }

    // 插入故障数据；kit_insert_faults_data 成功时会释放 faultsDatas，失败或空数组需在本函数释放
    if (utarray_len(faultsDatas) > 0)
    {
        if (kit_insert_faults_data(faultsDatas) != 0)
        {
            KITLOG(LOG_WM_EN, ERROR_EN, "kit_insert_faults_data 失败");
            utarray_free(faultsDatas);
        }
    }
    else
    {
        utarray_free(faultsDatas);
    }
}

static void saveEventDataInterval()
{
    UT_array *faultsDatas = NULL;

    // 为故障数据动态数组分配空间
    utarray_new(faultsDatas, &faults_data_storage_icd);
    for (int i = 0; i < utarray_len(eventDataConfigs); ++i)
    {
        faults_data_config_t *p_config =
            (faults_data_config_t *)utarray_eltptr(eventDataConfigs, i);
        if (p_config != NULL)
        {
            faults_data_storage_t data = {0};
            data.faultsValue = getRtdbPointValue(Rtdb_ShMem, p_config->devType, p_config->devId - 1,
                                                 p_config->pointId); // 从实时库获取时序数据
            if (KIT_ABS_DIFF(data.faultsValue, p_config->faultsValue) < EPSILON) // 故障变化存储
            {
                continue;
            }
            p_config->faultsValue = data.faultsValue; // 更新配置中的故障值
            data.devType = p_config->devType;         // 设备种类
            data.devId = p_config->devId;
            data.pointId = p_config->pointId;
            utarray_push_back(faultsDatas, &data);
        }
    }

    // 插入事件数据；kit_insert_events_data 成功时会释放 faultsDatas，失败或空数组需在本函数释放
    if (utarray_len(faultsDatas) > 0)
    {
        if (kit_insert_events_data(faultsDatas) != 0)
        {
            KITLOG(LOG_WM_EN, ERROR_EN, "kit_insert_faults_data 失败");
            utarray_free(faultsDatas);
        }
    }
    else
    {
        utarray_free(faultsDatas);
    }
}

/*****************************************************************************
 * @brief           封装的删除时序数据函数
 * @param[in]       interval: 存储间隔暂时默认全部周期一样，不做区分
 * @param[in]       timestamp: 删除之前数据时间戳
 *****************************************************************************/
static void deleteRecordData(int interval, time_t timestamp)
{
    struct tm tm_time;
    localtime_r(&timestamp, &tm_time);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", &tm_time);
    // 调用删除数据库函数
    int ret = kit_del_record_data_by_date(interval, time_str);
    if (ret != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "半小时数据数据删除失败");
    }
}

/*****************************************************************************
 * @brief           定时将RTDB数据保存到数据库
 *****************************************************************************/
static void saveRecordDataInterval()
{
    UT_array *timedDatas = NULL;

    // 为时序数据动态数组分配空间
    utarray_new(timedDatas, &timed_data_storage_icd);
    for (int i = 0; i < utarray_len(timedDataConfigs); ++i)
    {
        timed_data_config_t *p_config = (timed_data_config_t *)utarray_eltptr(timedDataConfigs, i);
        if (p_config != NULL && p_config->isPersisted != 0) // 确保配置不为空且需要持久化
        {
            timed_data_storage_t data = {0};
            data.devType = p_config->devType;                 // 设备种类
            data.devDbId = p_config->devDbId;                 // 设备数据库Id
            data.storageInterval = p_config->storageInterval; // 存储间隔
            data.pointDbId = p_config->pointDbId;             // 点位Id
            data.pointId = p_config->pointId;                 // 点位编号
            data.devId = p_config->devId;                     // 设备编号
            data.timedValue = getRtdbPointValue(Rtdb_ShMem, data.devType, p_config->devId - 1,
                                                p_config->pointId); // 从实时库获取时序数据

            utarray_push_back(timedDatas, &data);
        }
    }

    // 插入实时记录数据；kit_insert_record_data 成功时会释放 timedDatas，失败或空数组需在本函数释放
    if (utarray_len(timedDatas) > 0)
    {
        if (kit_insert_record_data(timedDatas) != 0)
        {
            KITLOG(LOG_WM_EN, ERROR_EN, "kit_insert_timed_data 失败");
            utarray_free(timedDatas);
        }
    }
    else
    {
        utarray_free(timedDatas);
    }
}

/*****************************************************************************
 * @brief           每1秒钟执行一次的任务
 * @param[in]       arg：线程执行参数
 *****************************************************************************/
static void *oneSecondTaskHandle(void *arg)
{
    struct timespec start, end;
    struct timespec interval = {.tv_sec = 6, .tv_nsec = 0};
    struct timespec tv;
    while (1)
    {
        clock_gettime(CLOCK_MONOTONIC, &start);
        if (exeCount % 60 == 0) // 10秒存储数据一次，如果想1min存储，将数据改为60s
        {
            saveRealDataInterval();
        }

        if (exeCount % 5 == 0) // 事件和记录每5秒检查一次
        {
            saveFaultsDataInterval(); // 保存故障
            saveEventDataInterval();  // 保存事件数据
        }

        if (exeCount % 86400 == 0) // 86400秒执行一次（一天）
        {
            delDbBusDataInterval(); // 每天定时删除业务数据间隔
        }

        // 执行次数
        exeCount = (exeCount + 1) % MAX_EXE_COUNT;
        sleep(1); // 每秒执行一次
    }

    return NULL;
}

//-------------------------------------追加处理------------------------------------------------
/*
// 接入控制器的所有设备类型
// 接入控制器的所有设备类型
// 接入控制器的所有设备类型
typedef enum
{
    kDev_Type_Start = 0,             // 起始值
    kDev_Type_Con = kDev_Type_Start, // 本机信息
    kDev_Type_BBMS,                  // BMS堆控
    kDev_Type_RBMS,                  // BMS簇控制器
    kDev_Type_PCS,                   // PCS 类型
    kDev_Type_Transfer,              // 变压器
    kDev_Type_Lqd,                   // 空调/液冷机组
    kDev_Type_Meter,                 // 电表设备
    kDev_Type_UPS,                   // UPS设备
    kDev_Type_FireDevice,            // 消防设备
    kDev_Type_dehumidifier,          // 除湿机
    kDev_Type_TempSensor,            // 温度传感器
    kDev_Type_HumiditySensor,        // 湿度传感器
    kDev_Type_WaterSensor,           // 水浸传感器
    kDev_Type_SmokeSensor,           // 烟雾传感器
    kDev_Type_OtherSensor1,          // 其他传感器1
    kDev_Type_OtherSensor2,          // 其他传感器2
    kDev_Type_OtherSensor3,          // 其他传感器3

    // 预留设备类型
    kDev_Type_Resv17,
    kDev_Type_Resv18,
    kDev_Type_Resv19,
    kDev_Type_Resv20,
    kDev_Type_Resv21,
    kDev_Type_Resv22,
    kDev_Type_Resv23,
    kDev_Type_Resv24,
    kDev_Type_Resv25,
    kDev_Type_Resv26,
    kDev_Type_Resv27,
    kDev_Type_Resv28,
    kDev_Type_Resv29,
    kDev_Type_End
} dev_type_e;
*/
// 设备名称,依据dev_type_e
const char *dev_name[] = {
    "本机信息",
    "BMS堆控",
    "BMS簇控制器",
    "PCS类型",
    "变压器",
    "空调/液冷机组",
    "电表设备",
    "UPS设备",
    "消防设备",
    "除湿机",
    "温度传感器",
    "湿度传感器",
    "水浸传感器",
    "烟雾传感器",
    "其他传感器1",
    "其他传感器2",
    "其他传感器3",
    "预留设备类型17",
    "预留设备类型18",
    "预留设备类型19",
    "预留设备类型20",
    "预留设备类型21",
    "预留设备类型22",
    "预留设备类型23",
    "预留设备类型24",
    "预留设备类型25",
    "预留设备类型26",
    "预留设备类型27",
    "预留设备类型28",
    "预留设备类型29",
};

void format_timestamp(time_t ts, char *buf, size_t bufsize)
{
    struct tm tm_info;
    localtime_r(&ts, &tm_info);
    strftime(buf, bufsize, "%Y-%m-%d %H:%M:%S", &tm_info);
}

// 时序数据的icd
static UT_icd timed_data_config_icd = {
    sizeof(timed_data_config_t),
    NULL,
    NULL,
    NULL,
};

uint16_t statistic_dev_stroage_point_num(UT_array **item, dev_type_e dev_type)
{
    // 为时序数据动态数组分配空间 timed_data_config_t timed_data_config_icd
    utarray_new(*item, &timed_data_config_icd);
    for (int i = 0; i < utarray_len(timedDataConfigs); ++i)
    {
        timed_data_config_t *p_config = (timed_data_config_t *)utarray_eltptr(timedDataConfigs, i);
        if (p_config != NULL && p_config->isPersisted != 0 && p_config->devType == dev_type &&
            p_config->devId == 1)
        {
            timed_data_config_t data_info = {0};
            // printf("测点名称为：%s\n", p_config->pointName);
            // data_info = *p_config;
            data_info.devType = p_config->devType;                 // 设备类型
            data_info.storageInterval = p_config->storageInterval; // 存储间隔
            data_info.devDbId = p_config->devDbId;                 // 设备数据库Id
            data_info.devId = p_config->devId;                     // 设备编号
            data_info.pointDbId = p_config->pointDbId;             // 点位数据库Id
            data_info.pointId = p_config->pointId;                 // 点位编号
            snprintf((char *)data_info.pointName, sizeof(data_info.pointName), "%s",
                     (char *)p_config->pointName); // 点位名称
            snprintf((char *)data_info.devName, sizeof(data_info.devName), "%s",
                     (char *)p_config->devName); // 设备名称

            // printf("data_info.pointName = %s\n", data_info.pointName);
            utarray_push_back(*item, &data_info);
        }
    }
    return utarray_len(*item);
}

void mw_export_csv_files(UT_array **data_stroage_config, int dev_type, int dev_id,
                         time_t day_start_ts, const char *date_str)
{
    /*初始化参数*/
    char folder_path[128] = {0}, csv_path[256] = {0}, var_path[40] = {0};

    /*文件路径拼接*/
    snprintf(folder_path, sizeof(var_path), "/opt/bbms/storage/%s", date_str);
    snprintf(var_path, sizeof(var_path), "%s", date_str);

    /*建立文件夹*/
    kit_creat_sys_folder("/opt/bbms/storage", var_path);

    /*创建csv文件*/
    snprintf(csv_path, sizeof(csv_path), "%s/%d#%s.csv", folder_path, dev_id, dev_name[dev_type]);

    if (kit_get_business_export_everyday_csv_files(data_stroage_config, csv_path, dev_type, dev_id,
                                                   day_start_ts) != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "获取业务表导出csv文件:%s失败", csv_path);
        KITPTF(LOG_WM_EN, ERROR_EN, "获取业务表导出csv文件:%s失败", csv_path);
    }
}

void mw_export_half_hour_csv_files(UT_array **data_stroage_config, int dev_type, int dev_id)
{
    /*初始化参数*/
    char csv_path[256] = {0};

    /*创建csv文件*/
    snprintf(csv_path, sizeof(csv_path), "/opt/bbms/record/%d#%s.csv", dev_id, dev_name[dev_type]);

    if (kit_record_export_half_hour_csv_files(data_stroage_config, csv_path, dev_type, dev_id) != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "获取业务表导出csv文件:%s失败", csv_path);
        KITPTF(LOG_WM_EN, ERROR_EN, "获取业务表导出csv文件:%s失败", csv_path);
    }
}

#include "bsp_rtdb.h"
#define GNERATE_HALF_HOUR_DATA 1
void *creat_data_export_thread(void *arg)
{
    // 初始化utarray配置存储
    UT_array *dataStroage[kDev_Type_End] = {NULL};
    uint16_t export_flag = 0;
    for (dev_type_e edev = kDev_Type_Start; edev < kDev_Type_End; edev++)
    {

        uint16_t point_num = statistic_dev_stroage_point_num(&dataStroage[edev], edev);
        if (point_num == 0)
        {
            utarray_free(dataStroage[edev]);
            dataStroage[edev] = NULL;
        }
    }

#if GNERATE_HALF_HOUR_DATA
    /*建立文件夹*/
    kit_creat_sys_folder("/opt/bbms/", "record");
    // 启动时，先导出一次半小时内的csv文件
    for (dev_type_e edev = kDev_Type_Start; edev < kDev_Type_End; edev++)
    {
        for (uint16_t num = 0; num < gStDevTypeNum[edev]; num++)
        {
            if (dataStroage[edev] == NULL)
            {
                continue;
            }
            mw_export_half_hour_csv_files(&dataStroage[edev], edev, num + 1);
        }
    }
#endif

    while (1)
    {
        time_t now = time(NULL);
        struct tm tm_now;
        localtime_r(&now, &tm_now);

        export_flag = readWebSign(kSign_ShMem, kSign_ExportCurDayLog);
        // 检查是否是 00:10
        if ((tm_now.tm_hour == 0 && tm_now.tm_min == 10) || (export_flag == 1))
        {
            if (tm_now.tm_hour == 0 && tm_now.tm_min == 10)
            {
                clean_disk_space_default_paths(); // 每天凌晨清理一次磁盘
            }

            // 获取昨天 00:00:00 的时间戳
            struct tm tm_yesterday = tm_now;
            tm_yesterday.tm_hour = 0;
            tm_yesterday.tm_min = 0;
            tm_yesterday.tm_sec = 0;

            if (export_flag == 1)
            {
                ; // 导出当日的数据，不需要修改日期
            }
            else if (export_flag == 0)
            {
                tm_yesterday.tm_mday -= 1;
            }

            time_t day_start = mktime(&tm_yesterday);

            char date_str[32];
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", &tm_yesterday);

            for (dev_type_e edev = kDev_Type_Start; edev < kDev_Type_End; edev++)
            {
                for (uint16_t num = 0; num < gStDevTypeNum[edev]; num++)
                {
                    if (dataStroage[edev] == NULL)
                    {
                        continue;
                    }
                    mw_export_csv_files(&dataStroage[edev], edev, num + 1, day_start, date_str);
                }
            }

            if (export_flag == 1)
            {
                // 导出当日的数据，不需要修改日期
                writeWebSign(kSign_ShMem, kSign_ExportCurDayLog, 100); // 清除导出标志，防止重复导出
                export_flag = 0;
            }

            // 等待1分钟避免重复导出
            sleep(60);
        }
        else
        {
            // 每30秒检测一次
            sleep(20);
        }
    }
    return NULL;
}

static void delRecordDataInterval()
{
    time_t now;
    time(&now);
    struct tm tm_time;
    localtime_r(&now, &tm_time);
    tm_time.tm_hour += 8; // 加8个小时，考虑时区

    time_t halfHourkAgo = now - 30 * 60; // 半小时前

    deleteRecordData(0, halfHourkAgo);
}

/*****************************************************************************
 * @brief           每1秒钟执行一次的任务
 * @param[in]       arg：线程执行参数
 *****************************************************************************/
static void *recordDataTaskHandle(void *arg)
{
    uint32_t dly_count = 0;
    while (1)
    {
        saveRecordDataInterval();
        if (dly_count % 30 == 0) // 30s执行一次
        {
            delRecordDataInterval(); // 每天定时删除业务数据间隔
        }

        // 执行次数
        dly_count = (dly_count + 1) % MAX_EXE_COUNT;
        sleep(1); // 每秒执行一次
    }

    return NULL;
}

void *creat_record_data_export_thread(void *arg)
{
    // 初始化utarray配置存储
    UT_array *dataStroage[kDev_Type_End] = {NULL};

    for (dev_type_e edev = kDev_Type_Start; edev < kDev_Type_End; edev++)
    {

        uint16_t point_num = statistic_dev_stroage_point_num(&dataStroage[edev], edev);
        if (point_num == 0)
        {
            utarray_free(dataStroage[edev]);
            dataStroage[edev] = NULL;
        }
    }

    while (1)
    {
        for (dev_type_e edev = kDev_Type_Start; edev < kDev_Type_End; edev++)
        {
            for (uint16_t num = 0; num < gStDevTypeNum[edev]; num++)
            {
                if (dataStroage[edev] == NULL)
                {
                    continue;
                }
                mw_export_half_hour_csv_files(&dataStroage[edev], edev, num + 1);
            }
        }
        // 启动执行一次即可
        break;
    }
    return NULL;
}

/*****************************************************************************
 * @brief           创建定时存储任务
 * @return          0-成功  1-失败
 *****************************************************************************/
#define ENABLE_EXPORT_CSV 1
void creatDataStorageTask()
{
    int ret = 0;
    pthread_t tid_one_sec, day_export, record_export, tid_record, tid_monitor; // 定义线程描述符

    // 从数据库里查询到所有时序数据存储的配置
    ret = kit_get_tdata_config(&timedDataConfigs);
    if (ret != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "获取时序数据存储的配置失败！");
        return (void)1;
    }

    // 从数据库获取故障点位配置
    ret = kit_get_faults_config(&faultsDataConfigs);
    if (ret != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "获取故障点位配置失败！");
        return (void)1;
    }

    // 从数据库获取故障点位配置
    ret = kit_get_event_config(&eventDataConfigs);
    if (ret != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "获取故障点位配置失败！");
        return (void)1;
    }

    // 创建每1秒钟执行一次的线程
    ret = pthread_create(&tid_one_sec, NULL, oneSecondTaskHandle, NULL);
    if (ret != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "创建每1秒钟执行一次的线程失败");
        return (void)1; // 创建失败，返回错误
    }

    // 创建实时记录数据任务线程,记录半小时需求数据
    ret = pthread_create(&tid_record, NULL, recordDataTaskHandle, NULL);
    if (ret != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "创建实时记录数据任务线程失败");
        return (void)1; // 创建失败，返回错误
    }

    // 创建数据库监控线程
    ret = pthread_create(&tid_monitor, NULL, database_monitor_thread, NULL);
    if (ret != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "创建数据库监控线程失败");
        return (void)1; // 创建失败，返回错误
    }

#if ENABLE_EXPORT_CSV
    // 创建每日导出csv文件功能
    if (pthread_create(&day_export, NULL, creat_data_export_thread, NULL) != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "创建每日导出csv文件功能");
        return (void)1; // 创建失败，返回错误
    }

#endif

    return (void)0;
}

/*****************************************************************************
 * @brief           清理磁盘空间，当磁盘使用率超过指定门限时删除最老的子文件夹
 * @param[in]       target_path: 目标文件夹路径
 * @param[in]       clean_count: 删除文件夹的个数
 * @param[in]       threshold_percent: 磁盘使用率门限（百分比）
 * @return          0-成功  1-失败
 *****************************************************************************/
int clean_disk_space(const char *target_path, int clean_count, double threshold_percent)
{
    if (!target_path || strlen(target_path) == 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "目标路径为空");
        KITPTF(LOG_WM_EN, DEBUG_EN, "目标路径为空");
        return 1;
    }

    if (clean_count <= 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "清理数量必须大于0");
        KITPTF(LOG_WM_EN, DEBUG_EN, "清理数量必须大于0");
        return 1;
    }

    if (threshold_percent < 0.0 || threshold_percent > 100.0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "磁盘门限必须在0-100之间");
        KITPTF(LOG_WM_EN, DEBUG_EN, "磁盘门限必须在0-100之间");
        return 1;
    }

    // 使用 df 命令检查磁盘使用率
    char df_cmd[MAX_PATH_LEN + 50];
    char df_output[512];

    snprintf(df_cmd, sizeof(df_cmd), "df '%s' | tail -1 | awk '{print $5}' | sed 's/%%//'",
             target_path);

    if (kit_popen_exec(df_cmd, df_output, sizeof(df_output)) != 0)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "获取磁盘使用率失败");
        KITPTF(LOG_WM_EN, DEBUG_EN, "获取磁盘使用率失败");
        return 1;
    }

    // 解析磁盘使用率
    double usage_percent = atof(df_output);
    KITLOG(LOG_WM_EN, INFO_EN, "磁盘使用率: %.1f%%", usage_percent);
    KITPTF(LOG_WM_EN, DEBUG_EN, "磁盘使用率: %.1f%%", usage_percent);

    // 如果使用率超过指定的门限，执行清理
    if (usage_percent > threshold_percent)
    {
        KITLOG(LOG_WM_EN, WARN_EN, "磁盘使用率超过%.1f%%，开始清理%d个最老的子文件夹",
               threshold_percent, clean_count);
        KITPTF(LOG_WM_EN, DEBUG_EN, "磁盘使用率超过%.1f%%，开始清理%d个最老的子文件夹",
               threshold_percent, clean_count);

        // 使用 find 命令查找指定数量的最老子目录并删除
        char find_cmd[MAX_PATH_LEN + 200];
        snprintf(find_cmd, sizeof(find_cmd),
                 "find '%s' -maxdepth 1 -mindepth 1 -type d -printf '%%T@ %%p\\n' | sort -n | head "
                 "-%d | awk '{print $2}' | xargs -r rm -rf",
                 target_path,  // 对应 %s
                 clean_count); // 对应 %d

        KITLOG(LOG_WM_EN, INFO_EN, "执行清理命令: %s", find_cmd);
        KITPTF(LOG_WM_EN, DEBUG_EN, "执行清理命令: %s", find_cmd);

        int result = system(find_cmd);
        if (result == -1)
        {
            KITLOG(LOG_WM_EN, ERROR_EN, "执行清理命令失败: %s", strerror(errno));
            KITPTF(LOG_WM_EN, DEBUG_EN, "执行清理命令失败: %s", strerror(errno));
            return 1;
        }
        else if (WIFEXITED(result) && WEXITSTATUS(result) == 0)
        {
            KITLOG(LOG_WM_EN, INFO_EN, "成功清理%d个最老的子文件夹，清理完成", clean_count);
            KITPTF(LOG_WM_EN, DEBUG_EN, "成功清理%d个最老的子文件夹，清理完成", clean_count);
            return 0;
        }
        else
        {
            KITLOG(LOG_WM_EN, ERROR_EN, "清理命令执行失败，返回码: %d", result);
            KITPTF(LOG_WM_EN, DEBUG_EN, "清理命令执行失败，返回码: %d", result);
            return 1;
        }
    }
    else
    {
        KITLOG(LOG_WM_EN, DEBUG_EN, "磁盘使用率正常，无需清理");
        KITPTF(LOG_WM_EN, DEBUG_EN, "磁盘使用率正常，无需清理");
        return 0;
    }
}

/*****************************************************************************
 * @brief           使用预定义路径清理磁盘空间
 * @return          0-成功  1-失败
 *****************************************************************************/
int clean_disk_space_default_paths()
{
    int result = 0;
    int total_result = 0;

    // 清理默认存储目录
    KITLOG(LOG_WM_EN, INFO_EN, "开始清理默认存储目录: %s", DEFAULT_STORAGE_PATH);
    result =
        clean_disk_space(DEFAULT_STORAGE_PATH, DISK_CLEAN_FILE_COUNT, DISK_CLEAN_THRESHOLD_PERCENT);
    if (result == 0)
    {
        KITLOG(LOG_WM_EN, INFO_EN, "存储目录磁盘清理完成");
    }
    else
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "存储目录磁盘清理失败");
        total_result = 1;
    }
    return total_result;
}

// 数据库空间监控线程
void *database_monitor_thread(void *arg)
{
    while (1)
    {
        // 每30分钟检查一次
        sleep(30 * 60);

        // 日常清理
        kit_daily_incremental_vacuum();

        // 紧急情况检查：如果空闲页面超过阈值，立即清理
        kit_check_emergency_vacuum();
    }
    return NULL;
}