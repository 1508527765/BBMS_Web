/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2026, gary. All rights reserved.
 * @file            kit_db.h
 * @brief           kit_db.h
 * @author          gary
 * @date            2026/02/01
 * @remark          整理了数据库操作方法的声明
 *****************************************************************************/

#ifndef __KIT_DB_H__
#define __KIT_DB_H__

#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include "kit_data.h"
#include "utarray.h"
#include "sqlite3.h"
#include "kit_time.h"

#define MAX_TIMESTAMP_COUNT 24 * 3600
#define MAX_POINT_ID 2000
#define MAX_POINT_COUNT 2000
#define INIT_DB
// sql返回数据类型，原有的db应用模块取所有数据，没有可扩展性，由于常用，放在这里
typedef struct
{
    char **dbresult;
    int nrow;
    int ncolumn;
    char *errmsg;
} db_table_data_t;

int kit_init_db(const char *schema_dir);
int kit_get_db_data(db_table_data_t *outData, const char *tableName, const char *columnsName[], int columnNum);
int kit_get_dev_db_data(UT_array **devPointInfo);
int kit_sync_dev_data();
int kit_get_protocol_db_data(const template_type_e templateType, const uint64_t relatedId, char protoContent[MAX_CONFIG_CONTENT_LEN]);
int kit_get_point_db_data(const uint64_t devDbId, UT_array **points);
int kit_get_tdata_config(UT_array **timedDataConfigs);
int kit_sync_point_data();
int kit_get_faults_config(UT_array **faultsDataConfigs);
int kit_get_event_config(UT_array **eventDataConfigs);
int kit_get_config_db_data(UT_array **DevConfigs);
int kit_get_di_do_set_arr(UT_array **diDoSets);
int kit_insert_timed_data(UT_array *timedDatas);
int kit_del_timed_data_by_date(int storage_interval, const char *date_time_string);
int kit_insert_faults_data(UT_array *faultsDatas);
int kit_insert_events_data(UT_array *faultsDatas);
int kit_del_faults_data_by_date(const char *date_time_string);
int kit_del_events_data_by_date(const char *date_time_string);
int kit_insert_power_data(UT_array *powerDatas);
int kit_del_power_data_by_date(const char *date_time_string);
int kit_insert_dsp_fault_trace(float* data, int data_length, int max_records);
int kit_insert_dsp_trace(float* data, int data_length, int max_records);
int kit_get_pcs_advance_db_data_old(UT_array **advanceInfo);
int kit_get_pcs_advance_db_data(pcs_advance_param_t *pcs_advance_param);
int kit_update_logic_advance_config_remote(int remote_value);
int kit_get_business_export_everyday_csv_files(UT_array **data_stroage_config, char *file_path, int dev_type, int dev_id, time_t day_start_ts);
int kit_update_pcs_soft_version(uint8_t id,const char *cpld_version, const char *dsp_cpu1_version, const char *dsp_cpu2_version,const char *pcs_version);


// 批量插入定时数据，快速数据
int kit_insert_record_data(UT_array *timedDatas);
int kit_del_record_data_by_date(int storage_interval, const char *date_time_string);
int kit_record_export_half_hour_csv_files(UT_array **data_stroage_config, char *file_path, int dev_type, int dev_id);

int kit_sync_record_point_data();
int kit_sync_record_dev_data();

// 故障记录智能批量插入（现代方式：一个函数搞定，自动管理事务和预编译）
int kit_insert_fault_record_data(uint8_t pcs_index, uint8_t fault_index, uint16_t can_id, uint16_t *can_data);

void kit_daily_incremental_vacuum();
void kit_check_emergency_vacuum();
void kit_after_batch_delete_cleanup();

/*****************************************************************
 * @brief 获取BMS系统配置
 * @param type 配置类型: 0-系统配置, 1-控制配置, 0xff-全部配置
 * @param config 输出参数，存储获取到的配置
 * @return 0-成功, 1-失败
 *****************************************************************/
int kit_get_bms_config(uint8_t type, bms_config_t *config);


/**
 * @brief 获取DO配置
 * @param config 输出参数，存储获取到的DO配置
 */
void kit_db_get_do_config(do_config_t *config);

/**
 * @brief 获取DI配置
 * @param config 输出参数，存储获取到的DI配置
 */
void kit_db_get_di_config(di_config_t *config);
#endif
