/*****************************************************************************
 * @copyright       2024-2024, . POWER SUPPLY CO., LTD.
 * @file            kit_db.c
 * @brief           sqlite3数据库操作方法
 * @author          Gary
 * @date            2025/01/10
 * @remark          去掉sqlite3_get_table即将过期的方法，补充了设备、协议和点位表的查询方法
 *****************************************************************************/
#define _POSIX_C_SOURCE 200809L
#include "kit_db.h"
#include "kit_log.h"
#include "kit_string.h"
#include "utextend.h"
#include <dirent.h>
#include <pthread.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define CONFIG_SQLITE_PATH "/opt/bbms/db/sqlite3.db"    // 配置数据库
#define BUSINESS_SQLITE_PATH "/opt/bbms/db/business.db" // 业务数据库
#define RECORD_SQLITE_PATH "/opt/bbms/db/record.db"     // 业务数据库

static sqlite3 *config_db;   // 全局变量，数据库句柄
static sqlite3 *business_db; // 全局变量，业务数据库句柄
static sqlite3 *record_db;   // 全局变量，数据库句柄

pthread_mutex_t config_db_mutex = PTHREAD_MUTEX_INITIALIZER;   // 全局变量，配置数据库线程锁
pthread_mutex_t business_db_mutex = PTHREAD_MUTEX_INITIALIZER; // 全局变量，业务数据库线程锁
pthread_mutex_t record_db_mutex = PTHREAD_MUTEX_INITIALIZER;   // 全局变量，业务数据库线程锁

// 设备配置信息icd
static UT_icd dev_config_icd = {
    sizeof(con_dev_config_t),
    NULL,
    NULL,
    NULL,
};

// 设备信息的icd
static UT_icd dev_icd = {
    sizeof(dev_info_t), // 设备结构体大小
    NULL,               // 初始化函数
    NULL,               // 复制函数
    NULL,               // 销毁函数
};

// 点位信息icd
static UT_icd point_icd = {
    sizeof(point_t),
    NULL,
    NULL,
    NULL,
};

// 时序数据的icd
static UT_icd timed_data_config_icd = {
    sizeof(timed_data_config_t),
    NULL,
    NULL,
    NULL,
};

// 故障数据的icd
static UT_icd faults_data_config_icd = {
    sizeof(faults_data_config_t),
    NULL,
    NULL,
    NULL,
};

// DIDO逻辑设置信息的icd
static UT_icd di_do_logic_set_icd = {
    sizeof(di_do_logic_set_t),
    NULL,
    NULL,
    NULL,
};

// pcs高级设置的icd
static UT_icd pcs_advance_icd = {
    sizeof(pcs_advance_param_t), // 设备结构体大小
    NULL,                        // 初始化函数
    NULL,                        // 复制函数
    NULL,                        // 销毁函数
};
#include "cJSON.h"

static bool file_exists(const char *const path)
{
    struct stat buf = {0};
    if (-1 != stat(path, &buf) && S_ISDIR(buf.st_mode))
    {
        return true;
    }
    else
    {
        return false;
    }
}

// 查看数据库空间使用情况的函数
int check_database_space_info(sqlite3 *db, const char *db_name)
{
    sqlite3_stmt *stmt;
    int page_size, page_count, free_pages;

    // 获取页面大小
    if (sqlite3_prepare_v2(db, "PRAGMA page_size;", -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            page_size = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // 获取总页面数
    if (sqlite3_prepare_v2(db, "PRAGMA page_count;", -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            page_count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    // 获取空闲页面数（需要先开启incremental_vacuum模式）
    if (sqlite3_prepare_v2(db, "PRAGMA freelist_count;", -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            free_pages = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    KITLOG(LOG_KIT_EN, INFO_EN, "数据库 %s: 页面大小=%d, 总页面=%d, 空闲页面=%d, 空闲空间=%.2fMB",
           db_name, page_size, page_count, free_pages,
           (free_pages * page_size) / (1024.0 * 1024.0));

    return free_pages; // 返回空闲页面数，用于决定清理数量
}

// 智能增量清理函数
// 简化的智能增量清理函数
int smart_incremental_vacuum(sqlite3 *db, pthread_mutex_t *mutex, const char *db_name)
{
    char *err_msg = NULL;
    char sql[128];
    int free_pages = 0;
    sqlite3_stmt *stmt = NULL;

    pthread_mutex_lock(mutex); // 加锁

    // 获取空闲页面数
    int ret = sqlite3_prepare_v2(db, "PRAGMA freelist_count;", -1, &stmt, NULL);
    if (ret == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW)
    {
        free_pages = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (free_pages == 0)
    {
        KITLOG(LOG_KIT_EN, DEBUG_EN, "数据库 %s 没有需要清理的空闲页面", db_name);
        pthread_mutex_unlock(mutex); // 解锁
        return 0;
    }

    // 根据空闲页面数量决定清理策略
    int vacuum_pages;
    if (free_pages > 10000)
    {
        vacuum_pages = 2000;
    }
    else if (free_pages > 1000)
    {
        vacuum_pages = 500;
    }
    else if (free_pages > 100)
    {
        vacuum_pages = 100;
    }
    else
    {
        vacuum_pages = 0;
    }

    if (vacuum_pages > 0)
    {
        snprintf(sql, sizeof(sql), "PRAGMA incremental_vacuum(%d);", vacuum_pages);
        ret = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
        if (ret == SQLITE_OK)
        {
            KITLOG(LOG_KIT_EN, INFO_EN, "数据库 %s 清理了 %d/%d 个空闲页面", db_name, vacuum_pages,
                   free_pages);
        }
        else
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "数据库 %s 清理失败: %s", db_name, err_msg);
            if (err_msg)
                sqlite3_free(err_msg);
            pthread_mutex_unlock(mutex); // 解锁
            return 1;
        }
    }

    pthread_mutex_unlock(mutex); // 解锁
    return 0;
}

// 日常清理管理器
typedef struct
{
    time_t last_vacuum_time;
    int vacuum_count;
    int total_reclaimed_pages;
} VacuumManager;

static VacuumManager biz_vacuum_mgr = {0};
static VacuumManager rec_vacuum_mgr = {0};

// 日常清理入口函数
void kit_daily_incremental_vacuum()
{
    time_t now = time(NULL);

    // 业务数据库 - 每6小时清理一次，或者删除操作后立即清理
    if (now - biz_vacuum_mgr.last_vacuum_time > 6 * 3600)
    {
        KITLOG(LOG_KIT_EN, INFO_EN, "执行业务数据库日常清理");
        smart_incremental_vacuum(business_db, &business_db_mutex, "business");
        biz_vacuum_mgr.last_vacuum_time = now;
        biz_vacuum_mgr.vacuum_count++;
    }

    // 记录数据库 - 每半小时清理一次
    if (now - rec_vacuum_mgr.last_vacuum_time > 0.5 * 3600)
    {
        KITLOG(LOG_KIT_EN, INFO_EN, "执行记录数据库日常清理");
        smart_incremental_vacuum(record_db, &record_db_mutex, "record");
        rec_vacuum_mgr.last_vacuum_time = now;
        rec_vacuum_mgr.vacuum_count++;
    }
}

// 紧急清理检查
void kit_check_emergency_vacuum()
{
    sqlite3_stmt *stmt;
    int free_pages;

    pthread_mutex_lock(&business_db_mutex); // 给业务库操作上锁
    // 检查业务数据库
    if (sqlite3_prepare_v2(business_db, "PRAGMA freelist_count;", -1, &stmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            free_pages = sqlite3_column_int(stmt, 0);
            if (free_pages > 50000)
            { // 如果空闲页面超过5万
                KITLOG(LOG_KIT_EN, WARN_EN, "业务数据库空闲页面过多(%d)，执行紧急清理", free_pages);
                sqlite3_exec(business_db, "PRAGMA incremental_vacuum(10000);", NULL, NULL, NULL);
            }
        }
        sqlite3_finalize(stmt);
    }
    pthread_mutex_unlock(&business_db_mutex); // 解锁
}

// 在批量删除操作后立即调用
void kit_after_batch_delete_cleanup()
{
    KITLOG(LOG_KIT_EN, INFO_EN, "批量删除后立即执行清理");
    smart_incremental_vacuum(business_db, &business_db_mutex, "business");
    smart_incremental_vacuum(record_db, &record_db_mutex, "record");
}

static int path_cat(char *dst, int offset, int max_size, const char *src)
{
    int len = strlen(src);
    // 检查目标缓冲区是否足够容纳新拼接的路径
    if (offset + len >= max_size)
    {
        return -1;
    }
    // 将源路径复制到目标路径的指定位置
    strcpy(dst + offset, src);
    return offset + len;
}

static int read_file_string(const char *path, char **sql)
{
    FILE *fp = fopen(path, "r");
    if (fp == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "打开文件'%s'失败", path);
        return 1; // 打开文件失败
    }

    // 获取文件大小
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    // 分配足够的内存来存储文件内容
    *sql = malloc(size + 1);
    if (*sql == NULL)
    {
        fclose(fp);
        KITLOG(LOG_KIT_EN, ERROR_EN, "内存分配失败");
        return 1; // 内存分配失败
    }

    // 读取文件内容到内存中并检查读取结果
    size_t read_size = fread(*sql, 1, size, fp);
    if (read_size != size)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "读取文件内容失败");
        free(*sql);
        fclose(fp);
        return 1; // 读取文件内容失败
    }

    (*sql)[size] = '\0'; // 添加字符串终止符

    fclose(fp);
    return 0;
}

static bool ends_with(const char *str, const char *suffix)
{
    // 如果字符串的长度小于后缀的长度，直接返回 false
    if (strlen(str) < strlen(suffix))
    {
        return false;
    }
    // 比较字符串的结尾部分与后缀是否相等
    return strcmp(str + strlen(str) - strlen(suffix), suffix) == 0;
}

static int schema_sort_cmp(const void *a, const void *b)
{
    return strcmp(*(char **)a, *(char **)b);
}

static UT_array *collect_schemas(const char *dir)
{
    DIR *dirp = NULL;
    struct dirent *dent = NULL;
    UT_array *files = NULL;

    // 打开目录
    if ((dirp = opendir(dir)) == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "文件夹打开失败: %s", dir);
        return NULL;
    }

    // 初始化 UT_array 用于存储文件名
    utarray_new(files, &ut_str_icd);

    // 遍历目录中的所有文件，筛选出以 ".sql" 结尾的文件
    while (NULL != (dent = readdir(dirp)))
    {
        if (ends_with(dent->d_name, ".sql"))
        {
            char *file = strdup(dent->d_name); // strdup 自动分配并拷贝字符串
            if (file == NULL)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "strdup 失败");
                continue;
            }
            // 将副本指针推入 UT_array
            utarray_push_back(files, &file);
        }
    }

    // 关闭目录
    closedir(dirp);
    return files;
}

static int execute_sql_files(const char *dir, sqlite3 *db)
{
    char *sql = NULL;
    int ret = 0;

    if (!file_exists(dir))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "sql 文件 `%s` 不存在", dir);
        return 1;
    }

    // 获取目录下的所有 SQL 文件
    UT_array *files = collect_schemas(dir);
    if (NULL == files)
    {
        return 1; // 目录读取失败
    }

    // 对文件名进行排序
    utarray_sort(files, schema_sort_cmp);

    // 遍历每个 SQL 文件
    utarray_foreach(files, char **, file)
    {
        char *path = NULL;
        path = calloc(PATH_MAX_SIZE, sizeof(char));
        if (NULL == path)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "内存分配失败");
            return 1;
        }

        // 拼接文件路径
        if (PATH_MAX_SIZE <= path_cat(path, 0, PATH_MAX_SIZE, dir) ||
            PATH_MAX_SIZE <= path_cat(path, strlen(dir), PATH_MAX_SIZE, *file))
        { // 修正了路径拼接问题
            KITLOG(LOG_KIT_EN, ERROR_EN, "路径过长: %s", path);
            free(path);
            return 1;
        }

        // 读取 SQL 文件内容
        ret = read_file_string(path, &sql);
        if (0 != ret)
        {
            free(path);
            continue; // 读取失败，跳过该文件
        }

        // 执行 SQL 语句
        char *err_msg = NULL;
        ret = sqlite3_exec(db, sql, NULL, NULL, &err_msg);
        if (SQLITE_OK != ret)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "执行 %s 失败: (%d)%s", path, ret, err_msg);
            sqlite3_free(err_msg);
            ret = 1;
            free(path);
            free(sql);
            continue;
        }

        free(path);
        free(sql);
    }

    // 释放文件列表内存
    utarray_free(files);
    return ret;
}

static int init_db_handle(const char *schema_dir, sqlite3 *db)
{
    int ret = 0;

    // 设置忙时超时时间
    ret = sqlite3_busy_timeout(db, 120 * 1000);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "设置忙时超时失败: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 启用外键支持
    ret = sqlite3_exec(db, "PRAGMA foreign_keys=ON", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "数据库外键启动失败: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // 设置 WAL 模式
    ret = sqlite3_exec(db, "PRAGMA journal_mode=WAL", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "数据库 journal_mode WAL 启动失败: %s", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

#if 1
    // 执行目录中的所有 SQL 文件
    ret = execute_sql_files(schema_dir, db);
    if (ret != 0)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "执行 SQL 文件失败");
        sqlite3_close(db);
        return 1;
    }
#endif

    return 0;
}

int kit_init_db(const char *schema_dir)
{
    int ret = 0;
    char *err_msg = NULL;

    // 打开配置数据库，如果不存在则创建
    // ret = sqlite3_open(CONFIG_SQLITE_PATH, &config_db);
    ret = sqlite3_open_v2(CONFIG_SQLITE_PATH, &config_db,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_WAL, NULL);
    if (SQLITE_OK != ret)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "打开数据库 `%s` 失败: %s", CONFIG_SQLITE_PATH,
               sqlite3_errstr(ret));
        return 1;
    }
    char config_schema_dir[PATH_MAX_SIZE]; // 假设路径不会超过 PATH_MAX_SIZE 字符
    snprintf(config_schema_dir, PATH_MAX_SIZE, "%s/config/", schema_dir);
    ret = init_db_handle(config_schema_dir, config_db);
    if (0 != ret)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "初始化数据库 `%s` 失败", CONFIG_SQLITE_PATH);
        return 1;
    }

    // 打开业务数据库，如果不存在则创建
    // ret = sqlite3_open(BUSINESS_SQLITE_PATH, &business_db);
    ret = sqlite3_open_v2(BUSINESS_SQLITE_PATH, &business_db,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_WAL, NULL);
    if (SQLITE_OK != ret)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "打开数据库 `%s` 失败: %s", BUSINESS_SQLITE_PATH,
               sqlite3_errstr(ret));
        return 1;
    }

    // 使用INCREMENTAL模式 + 优化参数
    ret = sqlite3_exec(business_db, "PRAGMA auto_vacuum = INCREMENTAL;", NULL, NULL, &err_msg);
    if (SQLITE_OK != ret)
    {
        KITLOG(LOG_KIT_EN, WARN_EN, "设置业务数据库auto_vacuum失败: %s", err_msg);
        sqlite3_free(err_msg);
    }

    sqlite3_exec(business_db, "PRAGMA cache_size = 2000;", NULL, NULL, NULL); // 2MB缓存
    sqlite3_exec(business_db, "PRAGMA synchronous = NORMAL;", NULL, NULL, NULL);
    sqlite3_exec(business_db, "PRAGMA journal_size_limit = 32768;", NULL, NULL, NULL);

    char business_schema_dir[PATH_MAX_SIZE]; // 假设路径不会超过 PATH_MAX_SIZE 字符
    snprintf(business_schema_dir, PATH_MAX_SIZE, "%s/business/", schema_dir);
    ret = init_db_handle(business_schema_dir, business_db);
    if (0 != ret)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "初始化数据库 `%s` 失败", BUSINESS_SQLITE_PATH);
        return 1;
    }

    // 打开业务数据库，如果不存在则创建
    ret = sqlite3_open_v2(RECORD_SQLITE_PATH, &record_db,
                          SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_WAL, NULL);
    if (SQLITE_OK != ret)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "打开数据库 `%s` 失败: %s", RECORD_SQLITE_PATH,
               sqlite3_errstr(ret));
        return 1;
    }

    ret = sqlite3_exec(record_db, "PRAGMA auto_vacuum = INCREMENTAL;", NULL, NULL, &err_msg);
    if (SQLITE_OK != ret)
    {
        KITLOG(LOG_KIT_EN, WARN_EN, "设置记录数据库auto_vacuum失败: %s", err_msg);
        sqlite3_free(err_msg);
    }
    sqlite3_exec(record_db, "PRAGMA cache_size = 2000;", NULL, NULL, NULL);
    sqlite3_exec(record_db, "PRAGMA synchronous = NORMAL;", NULL, NULL, NULL);
    sqlite3_exec(record_db, "PRAGMA journal_size_limit = 32768;", NULL, NULL, NULL);

    char record_schema_dir[PATH_MAX_SIZE]; // 假设路径不会超过 PATH_MAX_SIZE 字符
    snprintf(record_schema_dir, PATH_MAX_SIZE, "%s/record/", schema_dir);
    ret = init_db_handle(record_schema_dir, record_db);
    if (0 != ret)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "初始化数据库 `%s` 失败", BUSINESS_SQLITE_PATH);
        return 1;
    }

#ifdef INIT_DB
    kit_sync_dev_data();   // 同步设备信息
    kit_sync_point_data(); // 同步点位信息

    // 同步record数据库点位和设备信息
    kit_sync_record_point_data(); // 同步点位信息
    kit_sync_record_dev_data();   // 同步设备信息
#endif

    return 0;
}

int kit_get_db_data(db_table_data_t *outData, const char *tableName, const char *columnsName[],
                    int columnNum)
{
    sqlite3_stmt *stmt = NULL;
    int ret = 0;
    char *sql = NULL;
    char *sql_columns = NULL;

    // 构建 SQL 查询语句
    if (columnsName == NULL || columnNum == 0)
    {
        sql = sqlite3_mprintf("SELECT * FROM %s", tableName);
    }
    else
    {
        sql_columns = kit_join_text(columnNum, columnsName, ",");
        if (sql_columns == NULL)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "构建列名失败");
            return 1;
        }
        sql = sqlite3_mprintf("SELECT %s FROM %s", sql_columns, tableName);
        free(sql_columns); // 释放拼接好的列名字符串
    }

    if (sql == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "构建 SQL 语句失败: sqlite3_mprintf 失败");
        return 1;
    }

    // 准备 SQL 语句
    sqlite3_free(sql); // 释放 SQL 字符串
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备语句失败: %s", sqlite3_errmsg(config_db));
        sqlite3_finalize(stmt); // 结束语句
        return 1;
    }

    // 获取查询结果
    outData->nrow = 0;
    outData->ncolumn = sqlite3_column_count(stmt); // 获取列数

    // 如果有列，继续处理
    if (outData->ncolumn > 0)
    {
        // 先检查有多少行
        while ((ret = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            outData->nrow++;
        }

        // 如果没有找到行
        if (outData->nrow == 0)
        {
            KITLOG(LOG_KIT_EN, INFO_EN, "没有找到匹配的行");
            sqlite3_finalize(stmt);
            return 0;
        }

        // 为结果表分配内存
        outData->dbresult =
            (char **)sqlite3_malloc(outData->nrow * outData->ncolumn * sizeof(char *));
        if (outData->dbresult == NULL)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "分配结果表内存失败");
            sqlite3_finalize(stmt);
            return 1;
        }

        // 重置语句，重新开始读取
        sqlite3_reset(stmt);

        // 读取数据行并存储结果
        int row = 0;
        while ((ret = sqlite3_step(stmt)) == SQLITE_ROW)
        {
            for (int i = 0; i < outData->ncolumn; i++)
            {
                const char *text = (const char *)sqlite3_column_text(stmt, i);
                outData->dbresult[row * outData->ncolumn + i] =
                    text ? sqlite3_mprintf("%s", text) : NULL;
            }
            row++;
        }

        if (ret != SQLITE_DONE)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "执行语句失败: %s", sqlite3_errmsg(config_db));
            sqlite3_finalize(stmt);
            return 1;
        }
    }

    // 结束语句
    sqlite3_finalize(stmt);

    return 0;
}

void kit_free_table_data(db_table_data_t *data)
{
    if (data == NULL)
    {
        // 如果 `data` 指针为空，则直接返回。
        return;
    }

    // 释放 `dbresult` 指向的内存，如果 `dbresult` 不为空。
    if (data->dbresult != NULL)
    {
        sqlite3_free_table(data->dbresult);
        data->dbresult = NULL;
    }

    // 释放 `errmsg` 指向的内存，如果 `errmsg` 不为空。
    if (data->errmsg != NULL)
    {
        sqlite3_free(data->errmsg);
        data->errmsg = NULL;
    }

    // 函数执行完毕，返回。
    return;
}
int kit_exec_db_data(char *tableName, char *sql[], int sql_num)
{
    sqlite3 *db = NULL; // 数据库连接指针
    int ret = 0;        // sqlite3_exec 函数返回值
    char *zErrMsg = 0;  // 错误信息指针

    // 循环执行每个 SQL 语句
    for (int i = 0; i < sql_num; i++)
    {
        ret = sqlite3_exec(db, sql[i], NULL, 0, &zErrMsg);
        if (ret != SQLITE_OK)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "执行 SQL 语句[%d] 失败: %s.", i, zErrMsg);
            sqlite3_free(zErrMsg); // 释放错误信息内存
            return 1;
        }
    }

    sqlite3_free(zErrMsg); // 释放错误信息内存
    return 0;              // 返回执行结果标志
}

int kit_get_dev_db_data(UT_array **devPointInfo)
{
    sqlite3_stmt *stmt = NULL;            // 预编译语句句柄
    const char *query;                    // 查询语句
    utarray_new(*devPointInfo, &dev_icd); // 创建新的 UT_array 用于存储设备点位信息

    // 构建查询语句
    query = "SELECT id, type, code, protocol_type, name, "
            "sort_order, template_id "
            "FROM device WHERE 1=1 "; // 基础查询语句
    // 添加结果排序
    query = sqlite3_mprintf("%s ORDER BY type, protocol_type, id ASC", query); // 按 id 升序排序

    // 准备 SQL 语句
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", query, sqlite3_errmsg(config_db));
        goto error; // 处理错误
    }

    // 执行查询并处理结果
    int step = sqlite3_step(stmt);
    while (SQLITE_ROW == step) // 遍历每一行结果
    {
        // 从数据库中获取每一列的值
        dev_info_t dev_info = {0}; // 初始化设备点位结构体
        dev_info.devDbId = sqlite3_column_int(stmt, 0);
        dev_info.devType = sqlite3_column_int(stmt, 1);
        dev_info.devId = sqlite3_column_int(stmt, 2);
        dev_info.protocolType = sqlite3_column_int(stmt, 3);
        snprintf((char *)dev_info.devName, MAX_NAME_BUF_LEN, "%s",
                 (const char *)sqlite3_column_text(stmt, 4));
        dev_info.sortOrder = sqlite3_column_int(stmt, 5);
        dev_info.templateId = sqlite3_column_int(stmt, 6);

        // 将设备点位信息添加到 UT_array 中
        utarray_push_back(*devPointInfo, &dev_info);

        // 继续处理下一行
        step = sqlite3_step(stmt);
    }

    // 检查是否完成所有行的处理
    if (SQLITE_DONE != step)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询未完成，错误信息: %s", sqlite3_errmsg(config_db));
        goto error; // 处理错误
    }

    // 清理资源
    sqlite3_finalize(stmt); // 释放预编译语句
    return 0;               // 成功返回

error:
    sqlite3_finalize(stmt);      // 确保释放预编译语句
    utarray_free(*devPointInfo); // 释放 UT_array
    devPointInfo = NULL;         // 设置为 NULL
    return 1;                    // 返回错误代码
}

int kit_sync_dev_data()
{
    sqlite3_stmt *stmt_select = NULL; // 用于 SELECT 语句
    sqlite3_stmt *stmt_insert = NULL; // 用于 INSERT 语句
    const char *sql_delete;
    const char *sql_select;
    const char *sql_insert;
    int ret;

    pthread_mutex_lock(&config_db_mutex);   // 给配置库操作上锁
    pthread_mutex_lock(&business_db_mutex); // 给业务库操作上锁

    // 清空 business_db 中的 device 表
    sql_delete = "DELETE FROM device";
    ret = sqlite3_exec(business_db, sql_delete, NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "清空 business_db 中的 device 表失败: %s",
               sqlite3_errmsg(business_db));
        goto error;
    }

    // 从 config_db 中选择数据
    sql_select = "SELECT id, sn, type, code, name, protocol_type, template_id, sort_order, remark, "
                 "status FROM device";
    ret = sqlite3_prepare_v2(config_db, sql_select, -1, &stmt_select, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", sql_select,
               sqlite3_errmsg(config_db));
        goto error;
    }

    // 准备插入语句
    sql_insert = "INSERT INTO device (id, sn, type, code, name, protocol_type, template_id, "
                 "sort_order, remark, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    ret = sqlite3_prepare_v2(business_db, sql_insert, -1, &stmt_insert, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备插入语句 `%s` 失败: %s", sql_insert,
               sqlite3_errmsg(business_db));
        goto error;
    }

    // 循环插入数据
    while ((ret = sqlite3_step(stmt_select)) == SQLITE_ROW)
    {
        // 绑定参数
        sqlite3_bind_int(stmt_insert, 1, sqlite3_column_int(stmt_select, 0));
        sqlite3_bind_text(stmt_insert, 2, (const char *)sqlite3_column_text(stmt_select, 1), -1,
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt_insert, 3, sqlite3_column_int(stmt_select, 2));
        sqlite3_bind_int(stmt_insert, 4, sqlite3_column_int(stmt_select, 3));
        sqlite3_bind_text(stmt_insert, 5, (const char *)sqlite3_column_text(stmt_select, 4), -1,
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt_insert, 6, sqlite3_column_int(stmt_select, 5));
        sqlite3_bind_int(stmt_insert, 7, sqlite3_column_int(stmt_select, 6));
        sqlite3_bind_int(stmt_insert, 8, sqlite3_column_int(stmt_select, 7));
        sqlite3_bind_text(stmt_insert, 9, (const char *)sqlite3_column_text(stmt_select, 8), -1,
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt_insert, 10, sqlite3_column_int(stmt_select, 9));

        // 执行插入
        ret = sqlite3_step(stmt_insert);
        sqlite3_reset(stmt_insert); // 重置语句以进行下一次插入
        if (ret != SQLITE_DONE)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "插入数据失败: %s", sqlite3_errmsg(business_db));
            goto error;
        }
    }

    // 检查是否有错误发生在sqlite3_step(stmt_select)中
    if (ret != SQLITE_DONE)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询数据失败: %s", sqlite3_errmsg(config_db));
        goto error;
    }

    // 清理资源
    sqlite3_finalize(stmt_select);
    sqlite3_finalize(stmt_insert);
    pthread_mutex_unlock(&config_db_mutex);   // 给配置库操作释放锁
    pthread_mutex_unlock(&business_db_mutex); // 给业务库操作释放锁
    return 0;

error:
    sqlite3_finalize(stmt_select);
    sqlite3_finalize(stmt_insert);
    pthread_mutex_unlock(&config_db_mutex);   // 给配置库操作释放锁
    pthread_mutex_unlock(&business_db_mutex); // 给业务库操作释放锁
    return 1;
}

int kit_sync_record_dev_data()
{
    sqlite3_stmt *stmt_select = NULL; // 用于 SELECT 语句
    sqlite3_stmt *stmt_insert = NULL; // 用于 INSERT 语句
    const char *sql_delete;
    const char *sql_select;
    const char *sql_insert;
    int ret;

    pthread_mutex_lock(&config_db_mutex); // 给配置库操作上锁
    pthread_mutex_lock(&record_db_mutex); // 给业务库操作上锁

    // 清空 record_db 中的 device 表
    sql_delete = "DELETE FROM device";
    ret = sqlite3_exec(record_db, sql_delete, NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "清空 record_db 中的 device 表失败: %s",
               sqlite3_errmsg(record_db));
        goto error;
    }

    // 从 config_db 中选择数据
    sql_select = "SELECT id, sn, type, code, name, protocol_type, template_id, sort_order, remark, "
                 "status FROM device";
    ret = sqlite3_prepare_v2(config_db, sql_select, -1, &stmt_select, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", sql_select,
               sqlite3_errmsg(config_db));
        goto error;
    }

    // 准备插入语句
    sql_insert = "INSERT INTO device (id, sn, type, code, name, protocol_type, template_id, "
                 "sort_order, remark, status) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";
    ret = sqlite3_prepare_v2(record_db, sql_insert, -1, &stmt_insert, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备插入语句 `%s` 失败: %s", sql_insert,
               sqlite3_errmsg(record_db));
        goto error;
    }

    // 循环插入数据
    while ((ret = sqlite3_step(stmt_select)) == SQLITE_ROW)
    {
        // 绑定参数
        sqlite3_bind_int(stmt_insert, 1, sqlite3_column_int(stmt_select, 0));
        sqlite3_bind_text(stmt_insert, 2, (const char *)sqlite3_column_text(stmt_select, 1), -1,
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt_insert, 3, sqlite3_column_int(stmt_select, 2));
        sqlite3_bind_int(stmt_insert, 4, sqlite3_column_int(stmt_select, 3));
        sqlite3_bind_text(stmt_insert, 5, (const char *)sqlite3_column_text(stmt_select, 4), -1,
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt_insert, 6, sqlite3_column_int(stmt_select, 5));
        sqlite3_bind_int(stmt_insert, 7, sqlite3_column_int(stmt_select, 6));
        sqlite3_bind_int(stmt_insert, 8, sqlite3_column_int(stmt_select, 7));
        sqlite3_bind_text(stmt_insert, 9, (const char *)sqlite3_column_text(stmt_select, 8), -1,
                          SQLITE_STATIC);
        sqlite3_bind_int(stmt_insert, 10, sqlite3_column_int(stmt_select, 9));

        // 执行插入
        ret = sqlite3_step(stmt_insert);
        sqlite3_reset(stmt_insert); // 重置语句以进行下一次插入
        if (ret != SQLITE_DONE)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "插入数据失败: %s", sqlite3_errmsg(record_db));
            goto error;
        }
    }

    // 检查是否有错误发生在sqlite3_step(stmt_select)中
    if (ret != SQLITE_DONE)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询数据失败: %s", sqlite3_errmsg(config_db));
        goto error;
    }

    // 清理资源
    sqlite3_finalize(stmt_select);
    sqlite3_finalize(stmt_insert);
    pthread_mutex_unlock(&config_db_mutex); // 给配置库操作释放锁
    pthread_mutex_unlock(&record_db_mutex); // 给业务库操作释放锁
    return 0;

error:
    sqlite3_finalize(stmt_select);
    sqlite3_finalize(stmt_insert);
    pthread_mutex_unlock(&config_db_mutex); // 给配置库操作释放锁
    pthread_mutex_unlock(&record_db_mutex); // 给业务库操作释放锁
    return 1;
}

int kit_get_protocol_db_data(const template_type_e templateType, const uint64_t relatedId,
                             char protoContent[MAX_CONFIG_CONTENT_LEN])
{
    sqlite3_stmt *stmt = NULL; // 预编译语句句柄
    char *query = sqlite3_mprintf(
        "SELECT content FROM protocol WHERE 1=1 AND template_type=? AND related_id=? LIMIT 1");

    // 准备 SQL 语句
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", query, sqlite3_errmsg(config_db));
        sqlite3_free(query); // 释放查询字符串
        return 1;            // 返回错误代码
    }

    // 绑定参数到预编译语句
    if (SQLITE_OK != sqlite3_bind_int(stmt, 1, templateType))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "绑定 template_type 失败: %s", sqlite3_errmsg(config_db));
        sqlite3_finalize(stmt);
        sqlite3_free(query); // 释放查询字符串
        return 1;            // 返回错误代码
    }
    if (SQLITE_OK != sqlite3_bind_int64(stmt, 2, relatedId))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "绑定 related_id 失败: %s", sqlite3_errmsg(config_db));
        sqlite3_finalize(stmt);
        sqlite3_free(query); // 释放查询字符串
        return 1;            // 返回错误代码
    }

    // 执行查询并处理结果
    int step = sqlite3_step(stmt);
    if (step == SQLITE_ROW || step == SQLITE_DONE)
    {
        snprintf(protoContent, MAX_CONFIG_CONTENT_LEN, "%s", sqlite3_column_text(stmt, 0));
    }
    else
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询未完成，错误信息: %s", sqlite3_errmsg(config_db));
        sqlite3_finalize(stmt);
        sqlite3_free(query); // 释放查询字符串
        return 1;            // 返回错误代码
    }

    // 清理资源
    sqlite3_finalize(stmt); // 释放预编译语句
    sqlite3_free(query);    // 释放查询字符串
    return 0;               // 成功返回
}

int kit_get_point_db_data(const uint64_t templateId, UT_array **points)
{
    sqlite3_stmt *stmt = NULL;        // 预编译语句句柄
    const char *query;                // 查询语句
    utarray_new(*points, &point_icd); // 创建新的 UT_array 用于存储设备点位信息

    // 构建查询语句
    query = "SELECT group_type, code, name, data_type, "
            "attribute, func, reg_addr, bit_pos, bit_num, endian, "
            "precision, ratio, offset, is_persisted, storage_interval, "
            "mutate_bound, default_value, min_value, max_value, dev_type,id "
            "FROM point WHERE 1=1 "; // 基础查询语句

    // 检查并添加设备Id条件
    query = sqlite3_mprintf("%s AND template_id=?", query); // 动态添加设备Id条件

    // 添加结果排序
    query = sqlite3_mprintf("%s ORDER BY func, reg_addr ASC", query); // 按 id 升序排序

    // 准备 SQL 语句
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", query, sqlite3_errmsg(config_db));
        goto error; // 处理错误
    }

    // 绑定参数到预编译语句
    int param_index = 1; // 参数索引
    if (SQLITE_OK != sqlite3_bind_int64(stmt, param_index++, templateId))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "绑定 `%s` 的参数 dev_id 失败: %s", query,
               sqlite3_errmsg(config_db));
        goto error; // 处理错误
    }

    //  执行查询并处理结果
    int step = sqlite3_step(stmt);
    while (SQLITE_ROW == step) // 遍历每一行结果
    {
        // 从数据库中获取每一列的值
        point_t point_info = {0}; // 初始化设备点位结构体
        point_info.groupType = (point_group_type_e)sqlite3_column_int(stmt, 0);
        point_info.pointId = (uint16_t)sqlite3_column_int(stmt, 1);
        snprintf((char *)point_info.pointName, MAX_POINT_NAME_LEN, "%s",
                 (const char *)sqlite3_column_text(stmt, 2));
        point_info.dataType = (data_type_e)sqlite3_column_int(stmt, 3);
        point_info.attribute = (point_attribute_e)sqlite3_column_int(stmt, 4);
        point_info.func = (uint32_t)sqlite3_column_int(stmt, 5);
        point_info.regAddr = (uint16_t)sqlite3_column_int(stmt, 6);
        point_info.bitPos = (uint8_t)sqlite3_column_int(stmt, 7);
        point_info.bitNum = (uint8_t)sqlite3_column_int(stmt, 8);
        point_info.endian = (data_endian_e)sqlite3_column_int(stmt, 9);
        point_info.precision = sqlite3_column_double(stmt, 10);
        point_info.ratio = sqlite3_column_double(stmt, 11);
        point_info.offset = sqlite3_column_double(stmt, 12);
        point_info.isPersisted = (uint8_t)sqlite3_column_int(stmt, 13);
        point_info.storageInterval = (storage_interval_type_e)sqlite3_column_int(stmt, 14);
        point_info.mutateBound = sqlite3_column_double(stmt, 15);
        point_info.defaultValue = sqlite3_column_double(stmt, 16);
        point_info.minValue = sqlite3_column_double(stmt, 17);
        point_info.maxValue = sqlite3_column_double(stmt, 18);
        point_info.devType = sqlite3_column_int(stmt, 19);
        point_info.pointDbId = sqlite3_column_int(stmt, 20);
        // point_info.devDbId = sqlite3_column_int(stmt, 20);

        // 将设备点位信息添加到 UT_array 中
        utarray_push_back(*points, &point_info);

        // 继续处理下一行
        step = sqlite3_step(stmt);
    }

    // 检查是否完成所有行的处理
    if (SQLITE_DONE != step)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询未完成，错误信息: %s", sqlite3_errmsg(config_db));
        goto error; // 处理错误
    }

    // 清理资源
    sqlite3_finalize(stmt); // 释放预编译语句
    return 0;               // 成功返回

error:
    sqlite3_finalize(stmt); // 确保释放预编译语句
    utarray_free(*points);  // 释放 UT_array
    points = NULL;          // 设置为 NULL
    return 1;               // 返回错误代码
}

int kit_sync_point_data()
{
    sqlite3_stmt *stmt_config = NULL;              // config_db 中的预编译语句句柄
    sqlite3_stmt *stmt_business = NULL;            // business_db 中的预编译语句句柄
    const char *query_config;                      // 从 config_db 查询点位信息的 SQL 语句
    const char *query_clear = "DELETE FROM point"; // 清空 business_db 中的 point 表的 SQL 语句
    int step;                                      // 查询结果步骤

    pthread_mutex_lock(&config_db_mutex);   // 给配置库操作上锁
    pthread_mutex_lock(&business_db_mutex); // 给业务库操作上锁

    // 清空 business_db 中的 point 表
    if (SQLITE_OK != sqlite3_exec(business_db, query_clear, NULL, NULL, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "清空 business_db 中的 point 表失败: %s",
               sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&config_db_mutex);   // 给配置库操作释放锁
        pthread_mutex_unlock(&business_db_mutex); // 给业务库操作释放锁
        return 1;                                 // 处理错误
    }

    // 构建查询语句从 config_db 获取所有点位信息
    query_config = "SELECT template_id, protocol_type, dev_type, group_type, code, name,ename, "
                   "data_type, attribute, func, reg_addr, bit_pos, bit_num, endian, precision, "
                   "ratio, offset, is_persisted, storage_interval, mutate_bound, default_value, "
                   "min_value, max_value, unit, is_show, id FROM point ORDER BY id ASC";

    // 准备从 config_db 执行查询语句
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query_config, -1, &stmt_config, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", query_config,
               sqlite3_errmsg(config_db));
        pthread_mutex_unlock(&config_db_mutex);   // 给配置库操作释放锁
        pthread_mutex_unlock(&business_db_mutex); // 给业务库操作释放锁
        return 1;                                 // 处理错误
    }

    // 准备向 business_db 插入数据的 SQL 语句
    const char *insert_query =
        "INSERT INTO point (template_id, protocol_type, dev_type, group_type, "
        "code, name, ename,data_type, attribute, func, reg_addr, bit_pos, bit_num, endian, "
        "precision, ratio, offset, is_persisted, storage_interval, mutate_bound, "
        "default_value, min_value, max_value, unit, is_show, id) VALUES (?, ?, ?, ?, ?, ?, ?, ?, "
        "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    // 准备向 business_db 插入数据
    if (SQLITE_OK != sqlite3_prepare_v2(business_db, insert_query, -1, &stmt_business, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备插入 `%s` 失败: %s", insert_query,
               sqlite3_errmsg(business_db));
        sqlite3_finalize(stmt_config);            // 确保释放预编译语句
        pthread_mutex_unlock(&config_db_mutex);   // 给配置库操作释放锁
        pthread_mutex_unlock(&business_db_mutex); // 给业务库操作释放锁
        return 1;                                 // 处理错误
    }

    // 执行查询并插入数据
    step = sqlite3_step(stmt_config);
    while (SQLITE_ROW == step) // 遍历每一行结果
    {
        // 从 config_db 中获取每一列的值
        int param_index = 1; // 参数索引

        // 绑定参数到插入语句
        if (SQLITE_OK != sqlite3_bind_int64(stmt_business, param_index++,
                                            sqlite3_column_int64(stmt_config, 0))) // template_id
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 1))) // protocol_type
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 2))) // dev_type
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 3))) // group_type
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 4))) // code
            goto error;
        if (SQLITE_OK != sqlite3_bind_text(stmt_business, param_index++,
                                           (const char *)sqlite3_column_text(stmt_config, 5), -1,
                                           SQLITE_STATIC)) // name
            goto error;
        if (SQLITE_OK != sqlite3_bind_text(stmt_business, param_index++,
                                           (const char *)sqlite3_column_text(stmt_config, 6), -1,
                                           SQLITE_STATIC)) // ename
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 7))) // data_type
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 8))) // attribute
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 9))) // func
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 10))) // reg_addr
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 11))) // bit_pos
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 12))) // bit_num
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 13))) // endian
            goto error;
        if (SQLITE_OK != sqlite3_bind_double(stmt_business, param_index++,
                                             sqlite3_column_double(stmt_config, 14))) // precision
            goto error;
        if (SQLITE_OK != sqlite3_bind_double(stmt_business, param_index++,
                                             sqlite3_column_double(stmt_config, 15))) // ratio
            goto error;
        if (SQLITE_OK != sqlite3_bind_double(stmt_business, param_index++,
                                             sqlite3_column_double(stmt_config, 16))) // offset
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 17))) // is_persisted
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 18))) // storage_interval
            goto error;
        if (SQLITE_OK !=
            sqlite3_bind_double(stmt_business, param_index++,
                                sqlite3_column_double(stmt_config, 19))) // mutate_bound
            goto error;
        if (SQLITE_OK !=
            sqlite3_bind_double(stmt_business, param_index++,
                                sqlite3_column_double(stmt_config, 20))) // default_value
            goto error;
        if (SQLITE_OK != sqlite3_bind_double(stmt_business, param_index++,
                                             sqlite3_column_double(stmt_config, 21))) // min_value
            goto error;
        if (SQLITE_OK != sqlite3_bind_double(stmt_business, param_index++,
                                             sqlite3_column_double(stmt_config, 22))) // max_value
            goto error;
        if (SQLITE_OK != sqlite3_bind_text(stmt_business, param_index++,
                                           (const char *)sqlite3_column_text(stmt_config, 23), -1,
                                           SQLITE_STATIC)) // unit
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 24))) // is_show
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 25))) // id
            goto error;

        // 执行插入操作
        if (SQLITE_DONE != sqlite3_step(stmt_business))
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "插入数据失败: %s", sqlite3_errmsg(business_db));
            goto error;
        }

        // 重置插入语句以便于重新绑定参数
        sqlite3_reset(stmt_business);

        // 继续处理下一行
        step = sqlite3_step(stmt_config);
    }

    // 检查是否完成所有行的处理
    if (SQLITE_DONE != step)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询未完成，错误信息: %s", sqlite3_errmsg(config_db));
        goto error; // 处理错误
    }

    // 清理资源
    sqlite3_finalize(stmt_config);            // 释放 config_db 预编译语句
    sqlite3_finalize(stmt_business);          // 释放 business_db 预编译语句
    pthread_mutex_unlock(&config_db_mutex);   // 给配置库操作释放锁
    pthread_mutex_unlock(&business_db_mutex); // 给业务库操作释放锁
    return 0;                                 // 成功返回

error:
    sqlite3_finalize(stmt_config);            // 确保释放预编译语句
    sqlite3_finalize(stmt_business);          // 确保释放预编译语句
    pthread_mutex_unlock(&config_db_mutex);   // 给配置库操作释放锁
    pthread_mutex_unlock(&business_db_mutex); // 给业务库操作释放锁
    return 1;                                 // 返回错误代码
}

int kit_sync_record_point_data()
{
    sqlite3_stmt *stmt_config = NULL;              // config_db 中的预编译语句句柄
    sqlite3_stmt *stmt_business = NULL;            // business_db 中的预编译语句句柄
    const char *query_config;                      // 从 config_db 查询点位信息的 SQL 语句
    const char *query_clear = "DELETE FROM point"; // 清空 business_db 中的 point 表的 SQL 语句
    int step;                                      // 查询结果步骤

    pthread_mutex_lock(&config_db_mutex); // 给配置库操作上锁
    pthread_mutex_lock(&record_db_mutex); // 给业务库操作上锁

    // 清空 business_db 中的 point 表
    if (SQLITE_OK != sqlite3_exec(record_db, query_clear, NULL, NULL, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "清空 business_db 中的 point 表失败: %s",
               sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&config_db_mutex); // 给配置库操作释放锁
        pthread_mutex_unlock(&record_db_mutex); // 给业务库操作释放锁
        return 1;                               // 处理错误
    }

    // 构建查询语句从 config_db 获取所有点位信息
    query_config = "SELECT template_id, protocol_type, dev_type, group_type, code, name,ename, "
                   "data_type, attribute, func, reg_addr, bit_pos, bit_num, endian, precision, "
                   "ratio, offset, is_persisted, storage_interval, mutate_bound, default_value, "
                   "min_value, max_value, unit, is_show, id FROM point ORDER BY id ASC";

    // 准备从 config_db 执行查询语句
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query_config, -1, &stmt_config, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", query_config,
               sqlite3_errmsg(config_db));
        pthread_mutex_unlock(&config_db_mutex);   // 给配置库操作释放锁
        pthread_mutex_unlock(&business_db_mutex); // 给业务库操作释放锁
        return 1;                                 // 处理错误
    }

    // 准备向 business_db 插入数据的 SQL 语句
    const char *insert_query =
        "INSERT INTO point (template_id, protocol_type, dev_type, group_type, "
        "code, name, ename,data_type, attribute, func, reg_addr, bit_pos, bit_num, endian, "
        "precision, ratio, offset, is_persisted, storage_interval, mutate_bound, "
        "default_value, min_value, max_value, unit, is_show, id) VALUES (?, ?, ?, ?, ?, ?, ?, ?, "
        "?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)";

    // 准备向 business_db 插入数据
    if (SQLITE_OK != sqlite3_prepare_v2(record_db, insert_query, -1, &stmt_business, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备插入 `%s` 失败: %s", insert_query,
               sqlite3_errmsg(record_db));
        sqlite3_finalize(stmt_config);          // 确保释放预编译语句
        pthread_mutex_unlock(&config_db_mutex); // 给配置库操作释放锁
        pthread_mutex_unlock(&record_db_mutex); // 给业务库操作释放锁
        return 1;                               // 处理错误
    }

    // 执行查询并插入数据
    step = sqlite3_step(stmt_config);
    while (SQLITE_ROW == step) // 遍历每一行结果
    {
        // 从 config_db 中获取每一列的值
        int param_index = 1; // 参数索引

        // 绑定参数到插入语句
        if (SQLITE_OK != sqlite3_bind_int64(stmt_business, param_index++,
                                            sqlite3_column_int64(stmt_config, 0))) // template_id
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 1))) // protocol_type
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 2))) // dev_type
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 3))) // group_type
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 4))) // code
            goto error;
        if (SQLITE_OK != sqlite3_bind_text(stmt_business, param_index++,
                                           (const char *)sqlite3_column_text(stmt_config, 5), -1,
                                           SQLITE_STATIC)) // name
            goto error;
        if (SQLITE_OK != sqlite3_bind_text(stmt_business, param_index++,
                                           (const char *)sqlite3_column_text(stmt_config, 6), -1,
                                           SQLITE_STATIC)) // ename
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 7))) // data_type
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 8))) // attribute
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 9))) // func
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 10))) // reg_addr
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 11))) // bit_pos
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 12))) // bit_num
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 13))) // endian
            goto error;
        if (SQLITE_OK != sqlite3_bind_double(stmt_business, param_index++,
                                             sqlite3_column_double(stmt_config, 14))) // precision
            goto error;
        if (SQLITE_OK != sqlite3_bind_double(stmt_business, param_index++,
                                             sqlite3_column_double(stmt_config, 15))) // ratio
            goto error;
        if (SQLITE_OK != sqlite3_bind_double(stmt_business, param_index++,
                                             sqlite3_column_double(stmt_config, 16))) // offset
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 17))) // is_persisted
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 18))) // storage_interval
            goto error;
        if (SQLITE_OK !=
            sqlite3_bind_double(stmt_business, param_index++,
                                sqlite3_column_double(stmt_config, 19))) // mutate_bound
            goto error;
        if (SQLITE_OK !=
            sqlite3_bind_double(stmt_business, param_index++,
                                sqlite3_column_double(stmt_config, 20))) // default_value
            goto error;
        if (SQLITE_OK != sqlite3_bind_double(stmt_business, param_index++,
                                             sqlite3_column_double(stmt_config, 21))) // min_value
            goto error;
        if (SQLITE_OK != sqlite3_bind_double(stmt_business, param_index++,
                                             sqlite3_column_double(stmt_config, 22))) // max_value
            goto error;
        if (SQLITE_OK != sqlite3_bind_text(stmt_business, param_index++,
                                           (const char *)sqlite3_column_text(stmt_config, 23), -1,
                                           SQLITE_STATIC)) // unit
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 24))) // is_show
            goto error;
        if (SQLITE_OK != sqlite3_bind_int(stmt_business, param_index++,
                                          sqlite3_column_int(stmt_config, 25))) // id
            goto error;
        // 执行插入操作
        if (SQLITE_DONE != sqlite3_step(stmt_business))
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "插入数据失败: %s", sqlite3_errmsg(record_db));
            goto error;
        }

        // 重置插入语句以便于重新绑定参数
        sqlite3_reset(stmt_business);

        // 继续处理下一行
        step = sqlite3_step(stmt_config);
    }

    // 检查是否完成所有行的处理
    if (SQLITE_DONE != step)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询未完成，错误信息: %s", sqlite3_errmsg(config_db));
        goto error; // 处理错误
    }

    // 清理资源
    sqlite3_finalize(stmt_config);          // 释放 config_db 预编译语句
    sqlite3_finalize(stmt_business);        // 释放 business_db 预编译语句
    pthread_mutex_unlock(&config_db_mutex); // 给配置库操作释放锁
    pthread_mutex_unlock(&record_db_mutex); // 给业务库操作释放锁
    return 0;                               // 成功返回

error:
    sqlite3_finalize(stmt_config);          // 确保释放预编译语句
    sqlite3_finalize(stmt_business);        // 确保释放预编译语句
    pthread_mutex_unlock(&config_db_mutex); // 给配置库操作释放锁
    pthread_mutex_unlock(&record_db_mutex); // 给业务库操作释放锁
    return 1;                               // 返回错误代码
}

int kit_get_tdata_config(UT_array **timedDataConfigs)
{
    sqlite3_stmt *stmt = NULL;
    char *query =
        sqlite3_mprintf("SELECT p.id, p.code, p.ename, p.storage_interval,p.is_persisted, d.name, "
                        "d.type, d.id, d.code FROM point p LEFT JOIN device d ON d.template_id = "
                        "p.template_id WHERE d.template_id IS NOT NULL ORDER BY d.type ASC");
    int ret;

    ret = sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", query, sqlite3_errmsg(config_db));
        sqlite3_free(query);
        return 1;
    }

    utarray_new(*timedDataConfigs, &timed_data_config_icd);

    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        timed_data_config_t data_info = {0};
        data_info.pointDbId = sqlite3_column_int(stmt, 0);
        data_info.pointId = sqlite3_column_int(stmt, 1);
        snprintf((char *)data_info.pointName, MAX_POINT_NAME_LEN, "%s",
                 (const char *)sqlite3_column_text(stmt, 2));
        data_info.storageInterval = (storage_interval_type_e)sqlite3_column_int(stmt, 3);
        data_info.isPersisted = (storage_interval_type_e)sqlite3_column_int(stmt, 4);
        snprintf((char *)data_info.devName, MAX_NAME_BUF_LEN, "%s",
                 (const char *)sqlite3_column_text(stmt, 5));
        data_info.devType = sqlite3_column_int(stmt, 6);
        data_info.devDbId = sqlite3_column_int(stmt, 7);
        data_info.devId = sqlite3_column_int(stmt, 8);

        utarray_push_back(*timedDataConfigs, &data_info);
    }

    if (ret != SQLITE_DONE)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询失败: %s", sqlite3_errmsg(config_db));
        sqlite3_finalize(stmt);
        sqlite3_free(query);
        utarray_free(*timedDataConfigs);
        *timedDataConfigs = NULL;
        return 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_free(query);
    return 0;
}

int kit_get_faults_config(UT_array **faultsDataConfigs)
{
    sqlite3_stmt *stmt = NULL;
    char *query = sqlite3_mprintf(
        "SELECT p.id, p.code, p.name, d.name, d.type, d.id, d.code FROM point p LEFT JOIN device d "
        "ON p.template_id = d.template_id WHERE p.group_type = ? ORDER BY d.code ASC");
    int ret;

    ret = sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", query, sqlite3_errmsg(config_db));
        sqlite3_free(query);
        return 1;
    }

    // 绑定参数
    if (SQLITE_OK != sqlite3_bind_int(stmt, 1, kPoint_SOE))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "绑定 group_type 失败: %s", sqlite3_errmsg(config_db));
        sqlite3_finalize(stmt);
        return 1; // 返回错误代码
    }

    utarray_new(*faultsDataConfigs, &faults_data_config_icd);

    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        faults_data_config_t data_info = {0};
        data_info.pointDbId = sqlite3_column_int(stmt, 0);
        data_info.pointId = sqlite3_column_int(stmt, 1);
        snprintf((char *)data_info.pointName, MAX_POINT_NAME_LEN, "%s",
                 (const char *)sqlite3_column_text(stmt, 2));
        snprintf((char *)data_info.devName, MAX_NAME_BUF_LEN, "%s",
                 (const char *)sqlite3_column_text(stmt, 3));
        data_info.devType = sqlite3_column_int(stmt, 4);
        data_info.devDbId = sqlite3_column_int(stmt, 5);
        data_info.devId = sqlite3_column_int(stmt, 6);
        data_info.faultsValue = 123.456;

        utarray_push_back(*faultsDataConfigs, &data_info);
    }

    if (ret != SQLITE_DONE)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询失败: %s", sqlite3_errmsg(config_db));
        sqlite3_finalize(stmt);
        sqlite3_free(query);
        utarray_free(*faultsDataConfigs);
        *faultsDataConfigs = NULL;
        return 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_free(query);
    return 0;
}

int kit_get_event_config(UT_array **eventDataConfigs)
{
    sqlite3_stmt *stmt = NULL;
    char *query = sqlite3_mprintf(
        "SELECT p.id, p.code, p.name, d.name, d.type, d.id, d.code FROM point p LEFT JOIN device d "
        "ON p.template_id = d.template_id WHERE p.group_type = ? ORDER BY d.code ASC");
    int ret;

    ret = sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", query, sqlite3_errmsg(config_db));
        sqlite3_free(query);
        return 1;
    }

    // 绑定参数
    if (SQLITE_OK != sqlite3_bind_int(stmt, 1, kPoint_EVENT))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "绑定 group_type 失败: %s", sqlite3_errmsg(config_db));
        sqlite3_finalize(stmt);
        return 1; // 返回错误代码
    }

    utarray_new(*eventDataConfigs, &faults_data_config_icd);

    while ((ret = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        faults_data_config_t data_info = {0};
        data_info.pointDbId = sqlite3_column_int(stmt, 0);
        data_info.pointId = sqlite3_column_int(stmt, 1);
        snprintf((char *)data_info.pointName, MAX_POINT_NAME_LEN, "%s",
                 (const char *)sqlite3_column_text(stmt, 2));
        snprintf((char *)data_info.devName, MAX_NAME_BUF_LEN, "%s",
                 (const char *)sqlite3_column_text(stmt, 3));
        data_info.devType = sqlite3_column_int(stmt, 4);
        data_info.devDbId = sqlite3_column_int(stmt, 5);
        data_info.devId = sqlite3_column_int(stmt, 6);
        data_info.faultsValue = 123.456;

        utarray_push_back(*eventDataConfigs, &data_info);
    }

    if (ret != SQLITE_DONE)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询失败: %s", sqlite3_errmsg(config_db));
        sqlite3_finalize(stmt);
        sqlite3_free(query);
        utarray_free(*eventDataConfigs);
        *eventDataConfigs = NULL;
        return 1;
    }

    sqlite3_finalize(stmt);
    sqlite3_free(query);
    return 0;
}

int kit_get_config_db_data(UT_array **DevConfigs)
{
    sqlite3_stmt *stmt = NULL; // 预编译语句句柄
    const char *query;         // 查询语句

    utarray_new(*DevConfigs, &dev_config_icd); // 创建新的 UT_array 用于存储设备点位信息

    // 构建查询语句
    query = "SELECT id, type, content "
            "FROM ems_device_config WHERE 1=1 "; // 基础查询语句

    // 添加结果排序
    query = sqlite3_mprintf("%s ORDER BY type ASC", query); // 按 type 升序排序

    // 准备 SQL 语句
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", query, sqlite3_errmsg(config_db));
        goto error; // 处理错误
    }

    // 执行查询并处理结果
    int step = sqlite3_step(stmt);
    while (SQLITE_ROW == step) // 遍历每一行结果
    {
        // 从数据库中获取每一列的值
        con_dev_config_t dev_config = {0}; // 初始化设备配置结构体
        dev_config.dbId = (uint16_t)sqlite3_column_int(stmt, 0);
        dev_config.type = (con_dev_config_type_e)sqlite3_column_int(stmt, 1);
        snprintf((char *)dev_config.content, MAX_CONFIG_CONTENT_LEN, "%s",
                 (const char *)sqlite3_column_text(stmt, 2));

        // 将设备配置信息添加到 UT_array 中
        utarray_push_back(*DevConfigs, &dev_config);

        // 继续处理下一行
        step = sqlite3_step(stmt);
    }

    // 检查是否完成所有行的处理
    if (SQLITE_DONE != step)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询未完成，错误信息: %s", sqlite3_errmsg(config_db));
        goto error; // 处理错误
    }

    // 清理资源
    sqlite3_finalize(stmt); // 释放预编译语句
    return 0;               // 成功返回

error:
    sqlite3_finalize(stmt);    // 确保释放预编译语句
    utarray_free(*DevConfigs); // 释放 UT_array
    DevConfigs = NULL;         // 设置为 NULL
    return 1;                  // 返回错误代码
}

int kit_get_di_do_set_arr(UT_array **diDoSets)
{
    sqlite3_stmt *stmt = NULL; // 预编译语句句柄
    const char *query;         // 查询语句

    query = "SELECT seq, type, cabinet_code, strategy_seq "
            "FROM di_do_set ";

    // 准备 SQL 语句
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询 `%s` 失败: %s", query, sqlite3_errmsg(config_db));
        return 1;
    }

    utarray_new(*diDoSets, &di_do_logic_set_icd); // 创建新的 UT_array 用于存储DIDO逻辑设置信息

    // 执行查询并处理结果
    int step = sqlite3_step(stmt);
    while (SQLITE_ROW == step) // 遍历每一行结果
    {
        // 从数据库中获取每一列的值
        di_do_logic_set_t di_do_set_logic_info = {0}; // 初始化DIDO逻辑设置的结构体
        di_do_set_logic_info.dIDOseq = sqlite3_column_int(stmt, 0);
        di_do_set_logic_info.dIDOType = (di_do_type_e)sqlite3_column_int(stmt, 1);
        di_do_set_logic_info.cabinetCode = sqlite3_column_int(stmt, 2);
        if (di_do_set_logic_info.dIDOType == kDi)
        {
            di_do_set_logic_info.strategySeqDi = (diFuc_e)sqlite3_column_int(stmt, 3);
        }
        else
        {
            di_do_set_logic_info.strategySeqDo = (doFuc_e)sqlite3_column_int(stmt, 3);
        }

        // 将DIDO逻辑设置添加到 UT_array 中
        utarray_push_back(*diDoSets, &di_do_set_logic_info);

        // 继续处理下一行
        step = sqlite3_step(stmt);
    }

    // 检查是否完成所有行的处理
    if (SQLITE_DONE != step)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询未完成，错误信息: %s", sqlite3_errmsg(config_db));
        goto error; // 处理错误
    }

    // 清理资源
    sqlite3_finalize(stmt); // 释放预编译语句
    return 0;               // 成功返回

error:
    sqlite3_finalize(stmt);  // 确保释放预编译语句
    utarray_free(*diDoSets); // 释放 UT_array
    diDoSets = NULL;         // 设置为 NULL
    return 1;                // 返回错误代码
}

int kit_insert_timed_data(UT_array *timedDatas)
{
    sqlite3_stmt *stmt = NULL; // 预编译语句句柄
    const char *sql = "INSERT INTO timed_data_storage (dev_type, dev_id, point_id, timed_value, "
                      "storage_interval, created_time) VALUES (?, ?, ?, ?, ?, strftime('%Y-%m-%d "
                      "%H:%M:%S', 'now', '+8 hours'))";
    int ret = 0;
    int total_count = utarray_len(timedDatas); // 总记录数
    int batch_size = 100;                      // 每批插入的条数
    int i;

    pthread_mutex_lock(&business_db_mutex); // 加锁，确保单线程访问
    // 开始事务
    ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;
    }

    // 预编译SQL语句
    ret = sqlite3_prepare_v2(business_db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL预编译失败: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;
    }

    // 批量插入
    for (i = 0; i < total_count; i++)
    {
        timed_data_storage_t *timedData = (timed_data_storage_t *)utarray_eltptr(timedDatas, i);

        // 绑定参数
        sqlite3_bind_int(stmt, 1, timedData->devType);
        sqlite3_bind_int(stmt, 2, timedData->devId);
        sqlite3_bind_int(stmt, 3, timedData->pointId);
        // sqlite3_bind_text(stmt, 4, (const char *)timedData->devName, -1, SQLITE_STATIC);
        // sqlite3_bind_text(stmt, 5, (const char *)timedData->pointName, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, timedData->timedValue);
        sqlite3_bind_int(stmt, 5, timedData->storageInterval);

        // 执行插入
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "插入数据失败: %s", sqlite3_errmsg(business_db));
            sqlite3_finalize(stmt);
            pthread_mutex_unlock(&business_db_mutex); // 解锁
            return 1;
        }

        // 重置语句，以便下次使用
        sqlite3_reset(stmt);

        // 每 batch_size 条记录提交一次事务
        if ((i + 1) % batch_size == 0)
        {
            ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
                sqlite3_finalize(stmt);
                pthread_mutex_unlock(&business_db_mutex); // 解锁
                return 1;
            }
            // 重新开始事务
            ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "无法重新开始事务: %s", sqlite3_errmsg(business_db));
                sqlite3_finalize(stmt);
                pthread_mutex_unlock(&business_db_mutex); // 解锁
                return 1;
            }
        }
    }

    // 提交剩余数据
    ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
    pthread_mutex_unlock(&business_db_mutex); // 解锁
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
        sqlite3_finalize(stmt);
        return 1;
    }

    // 释放预编译语句
    sqlite3_finalize(stmt);
    utarray_free(timedDatas);

    return 0; // 成功
}

int kit_del_timed_data_by_date(int storage_interval, const char *date_time_string)
{
    sqlite3_stmt *stmt = NULL; // SQLite语句句柄
    int ret = 0;               // 返回值
    char sql[256];             // SQL语句缓冲区

    // 参数校验，确保日期字符串不为NULL
    if (date_time_string == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "日期时间字符串为NULL");
        return 1;
    }

    // 使用sqlite3_snprintf构建SQL语句，防止SQL注入
    sqlite3_snprintf(sizeof(sql), sql, "DELETE FROM timed_data_storage WHERE created_time < '%q';",
                     date_time_string);

    pthread_mutex_lock(&business_db_mutex); // 加锁，确保单线程访问
    // 开始事务，确保数据一致性
    ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;                                 // 直接返回，不继续执行
    }

    // 准备SQL语句，避免直接使用sqlite3_exec
    ret = sqlite3_prepare_v2(business_db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL语句准备失败: %s SQL: %s", sqlite3_errmsg(business_db),
               sql);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        return 1;
    }

    // 执行SQL语句
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) // SQLITE_DONE表示成功执行完语句
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "删除数据失败: %s SQL: %s", sqlite3_errmsg(business_db), sql);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        sqlite3_finalize(stmt);                                   // 释放SQLite语句句柄
        return 1;
    }

    // 提交事务，确保操作生效
    ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        sqlite3_finalize(stmt);                                   // 释放SQLite语句句柄
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        return 1;
    }
    pthread_mutex_unlock(&business_db_mutex); // 解锁

    // 释放SQLite语句句柄，避免内存泄漏
    sqlite3_finalize(stmt);

    return 0; // 返回0表示成功
}

int kit_insert_faults_data(UT_array *faultsDatas)
{
    sqlite3_stmt *stmt = NULL; // 预编译语句句柄
    const char *sql =
        "INSERT INTO faults_record (dev_type, dev_id, point_id, faults_value, created_time) VALUES "
        "(?, ?, ?, ?, strftime('%Y-%m-%d %H:%M:%S', 'now', '+8 hours'))";
    int ret = 0;
    int total_count = utarray_len(faultsDatas); // 总记录数
    int batch_size = 100;                       // 每批插入的条数
    int i;

    pthread_mutex_lock(&business_db_mutex); // 加锁，确保单线程访问
    // 开始事务
    ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;
    }

    // 预编译SQL语句
    ret = sqlite3_prepare_v2(business_db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL预编译失败: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;
    }

    // 批量插入
    for (i = 0; i < total_count; i++)
    {
        faults_data_storage_t *faultsData = (faults_data_storage_t *)utarray_eltptr(faultsDatas, i);

        // 绑定参数
        sqlite3_bind_int(stmt, 1, faultsData->devType);
        sqlite3_bind_int(stmt, 2, faultsData->devId);
        sqlite3_bind_int(stmt, 3, faultsData->pointId);
        sqlite3_bind_double(stmt, 4, faultsData->faultsValue);

        // 执行插入
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "插入数据失败: %s", sqlite3_errmsg(business_db));
            sqlite3_finalize(stmt);
            pthread_mutex_unlock(&business_db_mutex); // 解锁
            return 1;
        }

        // 重置语句，以便下次使用
        sqlite3_reset(stmt);

        // 每 batch_size 条记录提交一次事务
        if ((i + 1) % batch_size == 0)
        {
            ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
                sqlite3_finalize(stmt);
                pthread_mutex_unlock(&business_db_mutex); // 解锁
                return 1;
            }
            // 重新开始事务
            ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "无法重新开始事务: %s", sqlite3_errmsg(business_db));
                sqlite3_finalize(stmt);
                pthread_mutex_unlock(&business_db_mutex); // 解锁
                return 1;
            }
        }
    }

    // 提交剩余数据
    ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
    pthread_mutex_unlock(&business_db_mutex); // 解锁
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
        sqlite3_finalize(stmt);
        return 1;
    }

    // 释放预编译语句
    sqlite3_finalize(stmt);
    utarray_free(faultsDatas);

    return 0; // 成功
}

int kit_insert_events_data(UT_array *faultsDatas)
{
    sqlite3_stmt *stmt = NULL; // 预编译语句句柄
    const char *sql =
        "INSERT INTO events_record (dev_type, dev_id, point_id, faults_value, created_time) VALUES "
        "(?, ?, ?, ?, strftime('%Y-%m-%d %H:%M:%S', 'now', '+8 hours'))";
    int ret = 0;
    int total_count = utarray_len(faultsDatas); // 总记录数
    int batch_size = 100;                       // 每批插入的条数
    int i;

    pthread_mutex_lock(&business_db_mutex); // 加锁，确保单线程访问
    // 开始事务
    ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;
    }

    // 预编译SQL语句
    ret = sqlite3_prepare_v2(business_db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL预编译失败: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;
    }

    // 批量插入
    for (i = 0; i < total_count; i++)
    {
        faults_data_storage_t *faultsData = (faults_data_storage_t *)utarray_eltptr(faultsDatas, i);

        // 绑定参数
        sqlite3_bind_int(stmt, 1, faultsData->devType);
        sqlite3_bind_int(stmt, 2, faultsData->devId);
        sqlite3_bind_int(stmt, 3, faultsData->pointId);
        sqlite3_bind_double(stmt, 4, faultsData->faultsValue);

        // 执行插入
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "插入数据失败: %s", sqlite3_errmsg(business_db));
            sqlite3_finalize(stmt);
            pthread_mutex_unlock(&business_db_mutex); // 解锁
            return 1;
        }

        // 重置语句，以便下次使用
        sqlite3_reset(stmt);

        // 每 batch_size 条记录提交一次事务
        if ((i + 1) % batch_size == 0)
        {
            ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
                sqlite3_finalize(stmt);
                pthread_mutex_unlock(&business_db_mutex); // 解锁
                return 1;
            }
            // 重新开始事务
            ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "无法重新开始事务: %s", sqlite3_errmsg(business_db));
                sqlite3_finalize(stmt);
                pthread_mutex_unlock(&business_db_mutex); // 解锁
                return 1;
            }
        }
    }

    // 提交剩余数据
    ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
    pthread_mutex_unlock(&business_db_mutex); // 解锁
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
        sqlite3_finalize(stmt);
        return 1;
    }

    // 释放预编译语句
    sqlite3_finalize(stmt);
    utarray_free(faultsDatas);

    return 0; // 成功
}

int kit_del_faults_data_by_date(const char *date_time_string)
{
    sqlite3_stmt *stmt = NULL; // SQLite语句句柄
    int ret = 0;               // 返回值
    char sql[256];             // SQL语句缓冲区

    // 参数校验，确保日期字符串不为NULL
    if (date_time_string == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "日期时间字符串为NULL");
        return 1;
    }

    // 使用sqlite3_snprintf构建SQL语句，防止SQL注入
    sqlite3_snprintf(sizeof(sql), sql, "DELETE FROM faults_record WHERE created_time < '%q';",
                     date_time_string);

    pthread_mutex_lock(&business_db_mutex); // 加锁，确保单线程访问
    // 开始事务，确保数据一致性
    ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;                                 // 直接返回，不继续执行
    }

    // 准备SQL语句，避免直接使用sqlite3_exec
    ret = sqlite3_prepare_v2(business_db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL语句准备失败: %s SQL: %s", sqlite3_errmsg(business_db),
               sql);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        return 1;
    }

    // 执行SQL语句
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) // SQLITE_DONE表示成功执行完语句
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "删除数据失败: %s SQL: %s", sqlite3_errmsg(business_db), sql);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        sqlite3_finalize(stmt);                                   // 释放SQLite语句句柄
        return 1;
    }

    // 提交事务，确保操作生效
    ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        sqlite3_finalize(stmt);                                   // 释放SQLite语句句柄
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        return 1;
    }
    pthread_mutex_unlock(&business_db_mutex); // 解锁

    // 释放SQLite语句句柄，避免内存泄漏
    sqlite3_finalize(stmt);

    return 0; // 返回0表示成功
}

int kit_del_events_data_by_date(const char *date_time_string)
{
    sqlite3_stmt *stmt = NULL; // SQLite语句句柄
    int ret = 0;               // 返回值
    char sql[256];             // SQL语句缓冲区

    // 参数校验，确保日期字符串不为NULL
    if (date_time_string == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "日期时间字符串为NULL");
        return 1;
    }

    // 使用sqlite3_snprintf构建SQL语句，防止SQL注入
    sqlite3_snprintf(sizeof(sql), sql, "DELETE FROM events_record WHERE created_time < '%q';",
                     date_time_string);

    pthread_mutex_lock(&business_db_mutex); // 加锁，确保单线程访问
    // 开始事务，确保数据一致性
    ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;                                 // 直接返回，不继续执行
    }

    // 准备SQL语句，避免直接使用sqlite3_exec
    ret = sqlite3_prepare_v2(business_db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL语句准备失败: %s SQL: %s", sqlite3_errmsg(business_db),
               sql);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        return 1;
    }

    // 执行SQL语句
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) // SQLITE_DONE表示成功执行完语句
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "删除数据失败: %s SQL: %s", sqlite3_errmsg(business_db), sql);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        sqlite3_finalize(stmt);                                   // 释放SQLite语句句柄
        return 1;
    }

    // 提交事务，确保操作生效
    ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        sqlite3_finalize(stmt);                                   // 释放SQLite语句句柄
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        return 1;
    }
    pthread_mutex_unlock(&business_db_mutex); // 解锁

    // 释放SQLite语句句柄，避免内存泄漏
    sqlite3_finalize(stmt);

    return 0; // 返回0表示成功
}

int kit_del_break_data_by_date(const char *date_time_string)
{
    char sql1[256], sql2[256];                 // SQL语句缓冲区
    sqlite3_stmt *stmt1 = NULL, *stmt2 = NULL; // SQLite语句句柄
    int ret = 0;                               // 返回值

    // 参数校验，确保日期时间字符串不为NULL
    if (date_time_string == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "日期时间字符串为NULL");
        return 1;
    }

    pthread_mutex_lock(&business_db_mutex); // 加锁，确保单线程访问
    // 开始事务，确保数据一致性
    ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;                                 // 直接返回，不继续执行
    }

    // 使用sqlite3_snprintf构建SQL语句，防止SQL注入
    sqlite3_snprintf(sizeof(sql1), sql1, "DELETE FROM break_record WHERE created_time < ?;");
    sqlite3_snprintf(sizeof(sql2), sql2, "DELETE FROM break_data_storage WHERE created_time < ?;");

    // 准备并执行第一个 DELETE 语句
    // 准备SQL语句
    ret = sqlite3_prepare_v2(business_db, sql1, -1, &stmt1, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL语句准备失败: %s SQL: %s", sqlite3_errmsg(business_db),
               sql1);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        return 1;
    }
    // 绑定参数
    ret = sqlite3_bind_text(stmt1, 1, date_time_string, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "绑定参数失败: %s", sqlite3_errmsg(business_db));
        sqlite3_finalize(stmt1);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL);
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;
    }
    // 执行SQL语句
    ret = sqlite3_step(stmt1);
    if (ret != SQLITE_DONE) // SQLITE_DONE表示成功执行完语句
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "删除数据失败: %s SQL: %s", sqlite3_errmsg(business_db), sql1);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        sqlite3_finalize(stmt1);                                  // 释放SQLite语句句柄
        return 1;
    }

    // 准备并执行第二个 DELETE 语句
    // 准备SQL语句
    ret = sqlite3_prepare_v2(business_db, sql2, -1, &stmt2, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL语句准备失败: %s SQL: %s", sqlite3_errmsg(business_db),
               sql2);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        return 1;
    }
    // 绑定参数
    ret = sqlite3_bind_text(stmt2, 1, date_time_string, -1, SQLITE_STATIC);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "绑定参数失败: %s", sqlite3_errmsg(business_db));
        sqlite3_finalize(stmt2);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL);
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;
    }
    // 执行SQL语句
    ret = sqlite3_step(stmt2);
    if (ret != SQLITE_DONE) // SQLITE_DONE表示成功执行完语句
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "删除数据失败: %s SQL: %s", sqlite3_errmsg(business_db), sql2);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        sqlite3_finalize(stmt2);                                  // 释放SQLite语句句柄
        return 1;
    }

    // 提交事务，确保操作生效
    ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        return 1;
    }
    pthread_mutex_unlock(&business_db_mutex); // 解锁

    // 释放SQLite语句句柄，避免内存泄漏
    sqlite3_finalize(stmt1);

    return 0; // 返回0表示成功
}

int kit_insert_power_data(UT_array *powerDatas)
{
    sqlite3_stmt *stmt = NULL; // 预编译语句句柄
    const char *sql =
        "INSERT INTO power_data_storage(dev_type, dev_code, point_code, value, created_time) "
        "VALUES(?, ?, ?, ?, strftime('%Y-%m-%d %H:%M:%S', 'now', '+8 hours'));";
    int ret = 0;
    int total_count = utarray_len(powerDatas); // 总记录数
    int batch_size = 100;                      // 每批插入的条数
    int i;

    pthread_mutex_lock(&business_db_mutex); // 加锁，确保单线程访问
    // 开始事务
    ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;
    }

    // 预编译SQL语句
    ret = sqlite3_prepare_v2(business_db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL预编译失败: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;
    }

    // 批量插入
    for (i = 0; i < total_count; i++)
    {
        power_data_storage_t *powerData = (power_data_storage_t *)utarray_eltptr(powerDatas, i);

        // 绑定参数
        sqlite3_bind_int(stmt, 1, powerData->devType);
        sqlite3_bind_int(stmt, 2, powerData->devId);
        sqlite3_bind_int(stmt, 3, powerData->pointId);
        sqlite3_bind_double(stmt, 4, powerData->value);

        // 执行插入
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "插入数据失败: %s", sqlite3_errmsg(business_db));
            pthread_mutex_unlock(&business_db_mutex); // 解锁
            sqlite3_finalize(stmt);
            return 1;
        }

        // 重置语句，以便下次使用
        sqlite3_reset(stmt);

        // 每 batch_size 条记录提交一次事务
        if ((i + 1) % batch_size == 0)
        {
            ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
                pthread_mutex_unlock(&business_db_mutex); // 解锁
                sqlite3_finalize(stmt);
                return 1;
            }
            // 重新开始事务
            ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "无法重新开始事务: %s", sqlite3_errmsg(business_db));
                pthread_mutex_unlock(&business_db_mutex); // 解锁
                sqlite3_finalize(stmt);
                return 1;
            }
        }
    }

    // 提交剩余数据
    ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
    pthread_mutex_unlock(&business_db_mutex); // 解锁
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
        sqlite3_finalize(stmt);
        return 1;
    }

    // 释放预编译语句
    sqlite3_finalize(stmt);
    utarray_free(powerDatas);

    return 0; // 成功
}

int kit_del_power_data_by_date(const char *date_time_string)
{
    sqlite3_stmt *stmt = NULL; // SQLite语句句柄
    int ret = 0;               // 返回值
    char sql[256];             // SQL语句缓冲区

    // 参数校验，确保日期字符串不为NULL
    if (date_time_string == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "日期时间字符串为NULL");
        return 1;
    }

    // 使用sqlite3_snprintf构建SQL语句，防止SQL注入
    sqlite3_snprintf(sizeof(sql), sql, "DELETE FROM power_data_storage WHERE created_time < '%q';",
                     date_time_string);

    pthread_mutex_lock(&business_db_mutex); // 加锁，确保单线程访问
    // 开始事务，确保数据一致性
    ret = sqlite3_exec(business_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(business_db));
        pthread_mutex_unlock(&business_db_mutex); // 解锁
        return 1;                                 // 直接返回，不继续执行
    }

    // 准备SQL语句，避免直接使用sqlite3_exec
    ret = sqlite3_prepare_v2(business_db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL语句准备失败: %s SQL: %s", sqlite3_errmsg(business_db),
               sql);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        return 1;
    }

    // 执行SQL语句
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) // SQLITE_DONE表示成功执行完语句
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "删除数据失败: %s SQL: %s", sqlite3_errmsg(business_db), sql);
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        sqlite3_finalize(stmt);                                   // 释放SQLite语句句柄
        return 1;
    }

    // 提交事务，确保操作生效
    ret = sqlite3_exec(business_db, "COMMIT;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(business_db));
        sqlite3_exec(business_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&business_db_mutex);                 // 解锁
        sqlite3_finalize(stmt);                                   // 释放SQLite语句句柄
        return 1;
    }
    pthread_mutex_unlock(&business_db_mutex); // 解锁

    // 释放SQLite语句句柄，避免内存泄漏
    sqlite3_finalize(stmt);

    return 0; // 返回0表示成功
}

#include <cjson/cJSON.h>
#define MAX_DATA_POINTS 500 // 最大数据点数
// 带max_records参数的循环插入函数
int kit_insert_dsp_trace(float *data, int data_length, int max_records)
{
    // 参数校验
    if (!config_db)
    {
        fprintf(stderr, "数据库未初始化\n");
        return SQLITE_ERROR;
    }

    if (!data || data_length <= 0 || data_length > MAX_DATA_POINTS)
    {
        fprintf(stderr, "无效数据参数\n");
        return SQLITE_RANGE;
    }

    if (max_records <= 0)
    {
        fprintf(stderr, "max_records必须大于0\n");
        return SQLITE_RANGE;
    }

    // 维护记录数量（如果max_records > 0）
    if (max_records > 0)
    {
        // 查询当前记录数
        const char *count_sql = "SELECT COUNT(*) FROM dsp_trace_curve";
        sqlite3_stmt *count_stmt;
        int rc = sqlite3_prepare_v2(config_db, count_sql, -1, &count_stmt, NULL);
        if (rc != SQLITE_OK)
            return rc;

        rc = sqlite3_step(count_stmt);
        if (rc != SQLITE_ROW)
        {
            sqlite3_finalize(count_stmt);
            return rc;
        }

        int current_count = sqlite3_column_int(count_stmt, 0);
        sqlite3_finalize(count_stmt);

        // 删除超出的旧记录（每次只删1条，保持简单）
        if (current_count >= max_records)
        {
            const char *delete_sql = "DELETE FROM dsp_trace_curve WHERE id IN "
                                     "(SELECT id FROM dsp_trace_curve ORDER BY time ASC LIMIT ?)";
            sqlite3_stmt *delete_stmt;
            rc = sqlite3_prepare_v2(config_db, delete_sql, -1, &delete_stmt, NULL);
            if (rc != SQLITE_OK)
                return rc;

            // 计算需要删除的数量（通常只删1条）
            int delete_count = current_count - max_records + 1;
            sqlite3_bind_int(delete_stmt, 1, delete_count);

            rc = sqlite3_step(delete_stmt);
            sqlite3_finalize(delete_stmt);
            if (rc != SQLITE_DONE)
                return rc;
        }
    }

    // 生成时间戳
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);

    // 创建JSON数据
    cJSON *json_array = cJSON_CreateArray();
    if (!json_array)
        return SQLITE_NOMEM;

    for (int i = 0; i < data_length; i++)
    {
        if (!cJSON_AddNumberToObject(json_array, NULL, data[i]))
        {
            cJSON_Delete(json_array);
            return SQLITE_NOMEM;
        }
    }

    char *json_str = cJSON_PrintUnformatted(json_array);
    if (!json_str)
    {
        cJSON_Delete(json_array);
        return SQLITE_NOMEM;
    }

    // 插入新记录
    const char *insert_sql = "INSERT INTO dsp_trace_curve (time, content) VALUES (?, ?)";
    sqlite3_stmt *insert_stmt;
    int rc = sqlite3_prepare_v2(config_db, insert_sql, -1, &insert_stmt, NULL);
    if (rc != SQLITE_OK)
    {
        cJSON_free(json_str);
        cJSON_Delete(json_array);
        return rc;
    }

    sqlite3_bind_text(insert_stmt, 1, timestamp, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 2, json_str, -1, cJSON_free);

    rc = sqlite3_step(insert_stmt);
    sqlite3_finalize(insert_stmt);
    cJSON_Delete(json_array);

    return rc;
}

int kit_insert_dsp_fault_trace(float *data, int data_length, int max_records)
{
    // 参数校验
    if (!config_db)
    {
        fprintf(stderr, "数据库未初始化\n");
        return SQLITE_ERROR;
    }

    if (!data || data_length <= 0 || data_length > MAX_DATA_POINTS)
    {
        fprintf(stderr, "无效数据参数\n");
        return SQLITE_RANGE;
    }

    if (max_records <= 0)
    {
        fprintf(stderr, "max_records必须大于0\n");
        return SQLITE_RANGE;
    }

    // 维护记录数量（如果max_records > 0）
    if (max_records > 0)
    {
        // 查询当前记录数
        const char *count_sql = "SELECT COUNT(*) FROM dsp_fault_trace_curve";
        sqlite3_stmt *count_stmt;
        int rc = sqlite3_prepare_v2(config_db, count_sql, -1, &count_stmt, NULL);
        if (rc != SQLITE_OK)
            return rc;

        rc = sqlite3_step(count_stmt);
        if (rc != SQLITE_ROW)
        {
            sqlite3_finalize(count_stmt);
            return rc;
        }

        int current_count = sqlite3_column_int(count_stmt, 0);
        sqlite3_finalize(count_stmt);

        // 删除超出的旧记录（每次只删1条，保持简单）
        if (current_count >= max_records)
        {
            const char *delete_sql =
                "DELETE FROM dsp_fault_trace_curve WHERE id IN "
                "(SELECT id FROM dsp_fault_trace_curve ORDER BY time ASC LIMIT ?)";
            sqlite3_stmt *delete_stmt;
            rc = sqlite3_prepare_v2(config_db, delete_sql, -1, &delete_stmt, NULL);
            if (rc != SQLITE_OK)
                return rc;

            // 计算需要删除的数量（通常只删1条）
            int delete_count = current_count - max_records + 1;
            sqlite3_bind_int(delete_stmt, 1, delete_count);

            rc = sqlite3_step(delete_stmt);
            sqlite3_finalize(delete_stmt);
            if (rc != SQLITE_DONE)
                return rc;
        }
    }

    // 生成时间戳
    time_t now = time(NULL);
    struct tm *tm_now = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_now);

    // 创建JSON数据
    cJSON *json_array = cJSON_CreateArray();
    if (!json_array)
        return SQLITE_NOMEM;

    for (int i = 0; i < data_length; i++)
    {
        if (!cJSON_AddNumberToObject(json_array, NULL, data[i]))
        {
            cJSON_Delete(json_array);
            return SQLITE_NOMEM;
        }
    }

    char *json_str = cJSON_PrintUnformatted(json_array);
    if (!json_str)
    {
        cJSON_Delete(json_array);
        return SQLITE_NOMEM;
    }

    // 插入新记录
    const char *insert_sql = "INSERT INTO dsp_fault_trace_curve (time, content) VALUES (?, ?)";
    sqlite3_stmt *insert_stmt;
    int rc = sqlite3_prepare_v2(config_db, insert_sql, -1, &insert_stmt, NULL);
    if (rc != SQLITE_OK)
    {
        cJSON_free(json_str);
        cJSON_Delete(json_array);
        return rc;
    }

    sqlite3_bind_text(insert_stmt, 1, timestamp, -1, SQLITE_STATIC);
    sqlite3_bind_text(insert_stmt, 2, json_str, -1, cJSON_free);

    rc = sqlite3_step(insert_stmt);
    sqlite3_finalize(insert_stmt);
    cJSON_Delete(json_array);

    return rc;
}

int kit_get_pcs_advance_db_data_old(UT_array **advanceInfo)
{
    sqlite3_stmt *stmt = NULL;
    char *query_fmt = NULL;

    utarray_new(*advanceInfo, &pcs_advance_icd);
    if (*advanceInfo == NULL)
    {
        return 1;
    }

    query_fmt = sqlite3_mprintf("SELECT id, type, name, content FROM logic_advance_config WHERE "
                                "type = 1 ORDER BY type, id ASC");

    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query_fmt, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询失败: %s", sqlite3_errmsg(config_db));
        goto error;
    }

    while (SQLITE_ROW == sqlite3_step(stmt))
    {
        pcs_advance_param_t pcs_info = {0};
        const char *content = (const char *)sqlite3_column_text(stmt, 3);
        cJSON *json = cJSON_Parse(content);
        if (!json)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "解析 JSON 失败: %s", cJSON_GetErrorPtr());
            goto error;
        }

        // 安全解析 JSON
        pcs_info.ctrl_mode = cJSON_GetObjectItem(json, "remote")->valueint;
        pcs_info.charge_direction = cJSON_GetObjectItem(json, "direction")->valueint;
        pcs_info.dist_type = cJSON_GetObjectItem(json, "dist_type")->valueint;
        pcs_info.pcs_target.ess_switch = cJSON_GetObjectItem(json, "power_on_off")->valueint;
        pcs_info.pcs_target.ess_active_power =
            cJSON_GetObjectItem(json, "active_power")->valuedouble;
        pcs_info.pcs_target.ess_reactive_power =
            cJSON_GetObjectItem(json, "reactive_power")->valuedouble;

        cJSON_Delete(json);
        utarray_push_back(*advanceInfo, &pcs_info);
    }

    if (SQLITE_DONE != sqlite3_errcode(config_db))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询未完成: %s", sqlite3_errmsg(config_db));
        goto error;
    }

    sqlite3_finalize(stmt);
    sqlite3_free(query_fmt);
    return 0;

error:
    if (stmt)
        sqlite3_finalize(stmt);
    if (query_fmt)
        sqlite3_free(query_fmt);
    if (*advanceInfo)
    {
        utarray_free(*advanceInfo);
        *advanceInfo = NULL;
    }
    return 1;
}


int kit_get_pcs_advance_db_data(pcs_advance_param_t *pcs_advance_param)
{
    sqlite3_stmt *stmt = NULL;
    char *query_fmt = NULL;
    int ret = 1; // 默认失败

    if (pcs_advance_param == NULL)
    {
        return 1;
    }

    // 初始化结构体
    memset(pcs_advance_param, 0, sizeof(pcs_advance_param_t));
    
    // 设置默认值
    pcs_advance_param->arch_mode = ePcs_Mode_Centralized; // 默认分布式
    memset(pcs_advance_param->bms_pcs_match, 0, sizeof(pcs_advance_param->bms_pcs_match));

    // 获取 type=1 的数据（手动设置）
    query_fmt = sqlite3_mprintf("SELECT content FROM logic_advance_config WHERE type = 1");
    
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query_fmt, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询手动设置失败: %s", sqlite3_errmsg(config_db));
        goto error;
    }
    
    if (SQLITE_ROW == sqlite3_step(stmt))
    {
        const char *content = (const char *)sqlite3_column_text(stmt, 0);
        cJSON *json = cJSON_Parse(content);
        if (!json)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "解析手动设置 JSON 失败: %s", cJSON_GetErrorPtr());
            goto error;
        }

        // 安全解析 JSON
        cJSON *remote = cJSON_GetObjectItem(json, "remote");
        cJSON *direction = cJSON_GetObjectItem(json, "direction");
        cJSON *dist_type = cJSON_GetObjectItem(json, "dist_type");
        cJSON *power_on_off = cJSON_GetObjectItem(json, "power_on_off");
        cJSON *active_power = cJSON_GetObjectItem(json, "active_power");
        cJSON *reactive_power = cJSON_GetObjectItem(json, "reactive_power");

        if (remote) pcs_advance_param->ctrl_mode = remote->valueint;
        if (direction) pcs_advance_param->charge_direction = direction->valueint;
        if (dist_type) pcs_advance_param->dist_type = dist_type->valueint;
        if (power_on_off) pcs_advance_param->pcs_target.ess_switch = power_on_off->valueint;
        if (active_power) pcs_advance_param->pcs_target.ess_active_power = active_power->valuedouble;
        if (reactive_power) pcs_advance_param->pcs_target.ess_reactive_power = reactive_power->valuedouble;

        cJSON_Delete(json);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_free(query_fmt);
    
    // 获取 type=2 的数据（控制架构）
    query_fmt = sqlite3_mprintf("SELECT content FROM logic_advance_config WHERE type = 2");
    
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query_fmt, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询控制架构失败: %s", sqlite3_errmsg(config_db));
        goto error;
    }
    
    if (SQLITE_ROW == sqlite3_step(stmt))
    {
        const char *content = (const char *)sqlite3_column_text(stmt, 0);
        cJSON *json = cJSON_Parse(content);
        if (json)
        {
            cJSON *centralized = cJSON_GetObjectItem(json, "Centralized ");
            cJSON *distributed = cJSON_GetObjectItem(json, "Distributed");
            
            // 根据数据库中的值设置架构模式
            if (distributed && distributed->valueint == 1)
            {
                pcs_advance_param->arch_mode = ePcs_Mode_Distributed;
            }
            else if (centralized && centralized->valueint == 1)
            {
                pcs_advance_param->arch_mode = ePcs_Mode_Centralized;
            }
            
            cJSON_Delete(json);
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_free(query_fmt);
    
    // 获取 type=3 的数据（BMS-PCS匹配关系）
    query_fmt = sqlite3_mprintf("SELECT content FROM logic_advance_config WHERE type = 3");
    
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query_fmt, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询BMS-PCS匹配失败: %s", sqlite3_errmsg(config_db));
        goto error;
    }
    
    if (SQLITE_ROW == sqlite3_step(stmt))
    {
        const char *content = (const char *)sqlite3_column_text(stmt, 0);
        cJSON *json = cJSON_Parse(content);
        if (json && cJSON_IsArray(json))
        {
            int array_size = cJSON_GetArraySize(json);
            int match_count = 0;
            
            // 限制不超过最大数量
            if (array_size > MAX_BMS_CLUSTER_NUM)
            {
                KITLOG(LOG_KIT_EN, WARN_EN, "BMS-PCS匹配数据超过最大数量 %d，只取前 %d 个", 
                      MAX_BMS_CLUSTER_NUM, MAX_BMS_CLUSTER_NUM);
                array_size = MAX_BMS_CLUSTER_NUM;
            }
            
            for (int i = 0; i < array_size; i++)
            {
                cJSON *item = cJSON_GetArrayItem(json, i);
                if (item)
                {
                    cJSON *batteryStack = cJSON_GetObjectItem(item, "BatteryStack");
                    cJSON *startPCSNumber = cJSON_GetObjectItem(item, "StartPCSNumber");
                    cJSON *endPCSNumber = cJSON_GetObjectItem(item, "EndPCSNumber");
                    
                    if (batteryStack && startPCSNumber && endPCSNumber)
                    {
                        pcs_advance_param->bms_pcs_match[match_count].bms_id = batteryStack->valueint;
                        pcs_advance_param->bms_pcs_match[match_count].pcs_range.pcs_start_id = startPCSNumber->valueint;
                        pcs_advance_param->bms_pcs_match[match_count].pcs_range.pcs_end_id = endPCSNumber->valueint;
                        match_count++;
                    }
                }
            }
            
            KITLOG(LOG_KIT_EN, DEBUG_EN, "成功加载 %d 个BMS-PCS匹配关系", match_count);
            cJSON_Delete(json);
        }
        else if (json)
        {
            KITLOG(LOG_KIT_EN, WARN_EN, "BMS-PCS匹配数据不是有效的JSON数组");
            cJSON_Delete(json);
        }
    }
    
    sqlite3_finalize(stmt);
    sqlite3_free(query_fmt);
    
    ret = 0; // 成功
    return ret;

error:
    if (stmt)
        sqlite3_finalize(stmt);
    if (query_fmt)
        sqlite3_free(query_fmt);
    return ret;
}

/// @brief 更新 logic_advance_config 表中 type=1 的 remote 参数
/// @param remote_value remote 参数的新值（0=本地控制，1=远程控制）
/// @return 0=成功，1=失败
/// @note 该函数会读取 type=1 的记录，更新 JSON 中的 remote 字段，然后写回数据库
int kit_update_logic_advance_config_remote(int remote_value)
{
    sqlite3_stmt *stmt = NULL;
    char *query_fmt = NULL;
    char *update_fmt = NULL;
    int ret = 1; // 默认失败
    
    // 加锁保护数据库操作
    pthread_mutex_lock(&config_db_mutex);
    
    // 查询 type=1 的记录，获取当前的 content
    query_fmt = sqlite3_mprintf("SELECT content FROM logic_advance_config WHERE type = 1");
    
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query_fmt, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询手动设置失败: %s", sqlite3_errmsg(config_db));
        goto error;
    }
    
    if (SQLITE_ROW != sqlite3_step(stmt))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "未找到 type=1 的记录");
        goto error;
    }
    
    // 获取当前的 content
    const char *content = (const char *)sqlite3_column_text(stmt, 0);
    if (!content)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "获取 content 失败");
        goto error;
    }
    
    // 解析 JSON
    cJSON *json = cJSON_Parse(content);
    if (!json)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "解析 JSON 失败: %s", cJSON_GetErrorPtr());
        goto error;
    }
    
    // 更新 remote 字段
    cJSON *remote_item = cJSON_GetObjectItem(json, "remote");
    if (remote_item)
    {
        // 如果 remote 字段已存在，更新它
        cJSON_SetIntValue(remote_item, remote_value);
    }
    else
    {
        // 如果 remote 字段不存在，创建它
        cJSON_AddNumberToObject(json, "remote", remote_value);
    }
    
    // 将 JSON 转换回字符串
    char *json_str = cJSON_PrintUnformatted(json);
    if (!json_str)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "JSON 序列化失败");
        cJSON_Delete(json);
        goto error;
    }
    
    // 释放查询语句
    sqlite3_finalize(stmt);
    stmt = NULL;
    sqlite3_free(query_fmt);
    query_fmt = NULL;
    
    // 执行 UPDATE 操作
    update_fmt = sqlite3_mprintf("UPDATE logic_advance_config SET content = ? WHERE type = 1");
    
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, update_fmt, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备更新语句失败: %s", sqlite3_errmsg(config_db));
        cJSON_free(json_str);
        cJSON_Delete(json);
        goto error;
    }
    
    // 绑定参数（使用 SQLITE_TRANSIENT 让 SQLite 复制字符串）
    sqlite3_bind_text(stmt, 1, json_str, -1, SQLITE_TRANSIENT);
    
    // 执行更新
    if (SQLITE_DONE != sqlite3_step(stmt))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "更新失败: %s", sqlite3_errmsg(config_db));
        cJSON_free(json_str);
        cJSON_Delete(json);
        goto error;
    }
    
    KITLOG(LOG_KIT_EN, INFO_EN, "成功更新 logic_advance_config type=1 的 remote 参数为 %d", remote_value);
    
    // 清理资源
    cJSON_free(json_str);
    cJSON_Delete(json);
    sqlite3_finalize(stmt);
    sqlite3_free(update_fmt);
    
    pthread_mutex_unlock(&config_db_mutex);
    return 0; // 成功

error:
    if (stmt)
        sqlite3_finalize(stmt);
    if (query_fmt)
        sqlite3_free(query_fmt);
    if (update_fmt)
        sqlite3_free(update_fmt);
    // 注意：json 和 json_str 在成功路径中已释放，在错误路径中如果创建了也会在各自的错误处理中释放
    pthread_mutex_unlock(&config_db_mutex);
    return ret;
}


// 获取业务表导出csv文件
/**
 * @brief       获取业务表数据并导出为 CSV 文件
 * @param[in]   data_stroage_config: 指向存储配置数组的指针，包含导出所需的数据存储配置信息
 * @param[in]   csv_path: 要导出的 CSV 文件的完整路径
 * @param[in]   dev_type: 设备类型，用于筛选业务表数据
 * @param[in]   dev_id: 设备 ID，用于筛选业务表数据
 * @param[in]   interval: 存储间隔类型，用于筛选业务表数据
 * @param[in]   day_start_ts: 起始时间戳，用于筛选指定时间段内的业务表数据
 * @return      操作结果，0 表示成功，非 0 表示失败
 */
int kit_get_business_export_everyday_csv_files(UT_array **data_stroage_config, char *file_path,
                                               int dev_type, int dev_id, time_t day_start_ts)
{
    FILE *fp = fopen(file_path, "w");
    if (!fp)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "创建文件%s失败", file_path);
        KITPTF(LOG_WM_EN, ERROR_EN, "创建文件%s失败", file_path);
        return 1;
    }

    // 记录当前设备存储周期为interval的测点数量
    int point_count = utarray_len(*data_stroage_config);

    // 构建 point_id → index 映射,以及构建表头
    uint8_t point_id_to_index[MAX_POINT_ID] = {0};

    memset(point_id_to_index, -1, sizeof(point_id_to_index));

    fprintf(fp, "timestamp");
    for (int i = 0; i < utarray_len(*data_stroage_config); ++i)
    {
        timed_data_config_t *p_config =
            (timed_data_config_t *)utarray_eltptr(*data_stroage_config, i);
        // if (p_config != NULL && p_config->devId == dev_id && p_config->devType == dev_type)
        if (p_config != NULL && p_config->devType == dev_type)
        {
            // 构建表头
            // printf("p_config->pointName = %s\n", p_config->pointName);
            fprintf(fp, ",%s", p_config->pointName);
            // 构建 point_id → index 映射
            int id = p_config->pointId;
            if (id >= 0 && id < MAX_POINT_ID)
                point_id_to_index[id] = i;
        }
    }
    fprintf(fp, "\n");

    // SQL 查询所有该设备该日期数据，按时间和点位排序
    const char *sql =
        "SELECT strftime('%s', created_time) as ts, point_id, timed_value FROM timed_data_storage "
        "WHERE dev_type=? AND dev_id=? AND created_time BETWEEN datetime(?, 'unixepoch', "
        "'localtime') AND datetime(?, 'unixepoch', 'localtime') "
        "ORDER BY ts, point_id;";

    sqlite3_stmt *stmt;
    pthread_mutex_lock(&business_db_mutex);
    if (sqlite3_prepare_v2(business_db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement\n");
        pthread_mutex_unlock(&business_db_mutex);
        fclose(fp);
        return 1;
    }
    sqlite3_bind_int(stmt, 1, dev_type);
    sqlite3_bind_int(stmt, 2, dev_id);
    // sqlite3_bind_int(stmt, 3, interval);
    sqlite3_bind_int64(stmt, 3, day_start_ts);
    sqlite3_bind_int64(stmt, 4, day_start_ts + 86400 - 1);

    time_t current_ts = 0;
    double values[MAX_POINT_COUNT];
    memset(values, 0, sizeof(values));
    int has_value[MAX_POINT_COUNT];
    memset(has_value, 0, sizeof(has_value));

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        time_t ts = (time_t)sqlite3_column_int64(stmt, 0);
        int point_id = sqlite3_column_int(stmt, 1);
        double val = sqlite3_column_double(stmt, 2);

        if (current_ts != 0 && ts != current_ts)
        {
            // 写入上一组时间戳行
            char ts_buf[32];
            kit_time_format_tamp(current_ts, ts_buf, sizeof(ts_buf));
            fprintf(fp, "%s", ts_buf);
            for (int i = 0; i < point_count; ++i)
            {
                fprintf(fp, ",");
                if (has_value[i])
                    fprintf(fp, "%.2f", values[i]);
                has_value[i] = 0;
            }
            fprintf(fp, "\n");
        }

        current_ts = ts;
        int idx = (point_id >= 0 && point_id < MAX_POINT_ID) ? point_id_to_index[point_id] : -1;
        if (idx >= 0 && idx < point_count)
        {
            values[idx] = val;
            has_value[idx] = 1;
        }
    }

    // 写最后一行
    if (current_ts != 0)
    {
        char ts_buf[32];
        kit_time_format_tamp(current_ts, ts_buf, sizeof(ts_buf));
        fprintf(fp, "%s", ts_buf);
        for (int i = 0; i < point_count; ++i)
        {
            fprintf(fp, ",");
            if (has_value[i])
                fprintf(fp, "%.2f", values[i]);
        }
        fprintf(fp, "\n");
    }

    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&business_db_mutex);
    fclose(fp);
    return 0;
}

int kit_update_pcs_soft_version_old(uint8_t id, const char *cpld_version,
                                    const char *dsp_cpu1_version, const char *dsp_cpu2_version,
                                    const char *pcs_version)
{
    int ret = 0;
    sqlite3_stmt *stmt = NULL;

    // 如果传入的是NULL或空字符串，替换成"/"
    const char *cpld_v = (cpld_version && cpld_version[0]) ? cpld_version : "/";
    const char *dsp1_v = (dsp_cpu1_version && dsp_cpu1_version[0]) ? dsp_cpu1_version : "/";
    const char *dsp2_v = (dsp_cpu2_version && dsp_cpu2_version[0]) ? dsp_cpu2_version : "/";
    const char *pcs_v = (pcs_version && pcs_version[0]) ? pcs_version : "/";

    const char *sql_update =
        "UPDATE pcs_soft_version "
        "SET cpld_version = ?, dsp_cpu1_version = ?, dsp_cpu2_version = ?, pcs_version = ? "
        "WHERE id = ?;";

    pthread_mutex_lock(&config_db_mutex); // 加锁，确保线程安全

    if (SQLITE_OK != sqlite3_prepare_v2(config_db, sql_update, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL预编译失败: %s", sqlite3_errmsg(config_db));
        ret = 1;
        goto cleanup;
    }

    // 绑定参数
    sqlite3_bind_text(stmt, 1, cpld_v, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, dsp1_v, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, dsp2_v, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, pcs_v, -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, id);

    // 执行SQL
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL执行失败: %s", sqlite3_errmsg(config_db));
        ret = 1;
        goto cleanup;
    }

cleanup:
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&config_db_mutex);
    return ret;
}

int kit_update_pcs_soft_version(uint8_t id, const char *cpld_version, const char *dsp_cpu1_version,
                                const char *dsp_cpu2_version, const char *pcs_version)
{
    int ret = 0;
    sqlite3_stmt *stmt = NULL;
    char name[32] = {0};

    // 根据id生成name
    if (id >= 1 && id <= 16)
    {
        snprintf(name, sizeof(name), "%d#PCS", id);
    }
    else if (id == 17)
    {
        snprintf(name, sizeof(name), "SCU");
    }
    else
    {
        // 其他id使用默认命名规则
        snprintf(name, sizeof(name), "%d#DEVICE", id);
    }

    // 如果传入的是NULL或空字符串，替换成"/"
    const char *cpld_v = (cpld_version && cpld_version[0]) ? cpld_version : "/";
    const char *dsp1_v = (dsp_cpu1_version && dsp_cpu1_version[0]) ? dsp_cpu1_version : "/";
    const char *dsp2_v = (dsp_cpu2_version && dsp_cpu2_version[0]) ? dsp_cpu2_version : "/";
    const char *pcs_v = (pcs_version && pcs_version[0]) ? pcs_version : "/";

    const char *sql_upsert = "INSERT OR REPLACE INTO pcs_soft_version (id, name, cpld_version, "
                             "dsp_cpu1_version, dsp_cpu2_version, pcs_version) "
                             "VALUES (?, ?, ?, ?, ?, ?);";

    pthread_mutex_lock(&config_db_mutex); // 加锁，确保线程安全

    if (SQLITE_OK != sqlite3_prepare_v2(config_db, sql_upsert, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL预编译失败: %s", sqlite3_errmsg(config_db));
        ret = 1;
        goto cleanup;
    }

    // 绑定参数
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_bind_text(stmt, 2, name, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, cpld_v, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, dsp1_v, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, dsp2_v, -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, pcs_v, -1, SQLITE_TRANSIENT);

    // 执行SQL
    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL执行失败: %s", sqlite3_errmsg(config_db));
        ret = 1;
        goto cleanup;
    }

    KITLOG(LOG_KIT_EN, INFO_EN, "成功更新/插入PCS软件版本记录，ID: %d, Name: %s", id, name);

cleanup:
    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&config_db_mutex);
    return ret;
}

// 批量插入定时数据，快速数据
int kit_insert_record_data(UT_array *timedDatas)
{
    sqlite3_stmt *stmt = NULL; // 预编译语句句柄
    const char *sql = "INSERT INTO timed_data_storage (dev_type, dev_id, point_id, timed_value, "
                      "storage_interval, created_time) VALUES (?, ?, ?, ?, ?, strftime('%Y-%m-%d "
                      "%H:%M:%S', 'now', '+8 hours'))";
    int ret = 0;
    int total_count = utarray_len(timedDatas); // 总记录数
    int batch_size = 100;                      // 每批插入的条数
    int i;

    pthread_mutex_lock(&record_db_mutex); // 加锁，确保单线程访问
    // 开始事务
    ret = sqlite3_exec(record_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(record_db));
        pthread_mutex_unlock(&record_db_mutex); // 解锁
        return 1;
    }

    // 预编译SQL语句
    ret = sqlite3_prepare_v2(record_db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL预编译失败: %s", sqlite3_errmsg(record_db));
        pthread_mutex_unlock(&record_db_mutex); // 解锁
        return 1;
    }

    // 批量插入
    for (i = 0; i < total_count; i++)
    {
        timed_data_storage_t *timedData = (timed_data_storage_t *)utarray_eltptr(timedDatas, i);

        // 绑定参数
        sqlite3_bind_int(stmt, 1, timedData->devType);
        sqlite3_bind_int(stmt, 2, timedData->devId);
        sqlite3_bind_int(stmt, 3, timedData->pointId);
        // sqlite3_bind_text(stmt, 4, (const char *)timedData->devName, -1, SQLITE_STATIC);
        // sqlite3_bind_text(stmt, 5, (const char *)timedData->pointName, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 4, timedData->timedValue);
        sqlite3_bind_int(stmt, 5, timedData->storageInterval);

        // 执行插入
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "插入数据失败: %s", sqlite3_errmsg(record_db));
            sqlite3_finalize(stmt);
            pthread_mutex_unlock(&record_db_mutex); // 解锁
            return 1;
        }

        // 重置语句，以便下次使用
        sqlite3_reset(stmt);

        // 每 batch_size 条记录提交一次事务
        if ((i + 1) % batch_size == 0)
        {
            ret = sqlite3_exec(record_db, "COMMIT;", NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(record_db));
                sqlite3_finalize(stmt);
                pthread_mutex_unlock(&record_db_mutex); // 解锁
                return 1;
            }
            // 重新开始事务
            ret = sqlite3_exec(record_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
            if (ret != SQLITE_OK)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "无法重新开始事务: %s", sqlite3_errmsg(record_db));
                sqlite3_finalize(stmt);
                pthread_mutex_unlock(&record_db_mutex); // 解锁
                return 1;
            }
        }
    }

    // 提交剩余数据
    ret = sqlite3_exec(record_db, "COMMIT;", NULL, NULL, NULL);
    pthread_mutex_unlock(&record_db_mutex); // 解锁
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(record_db));
        sqlite3_finalize(stmt);
        return 1;
    }

    // 释放预编译语句
    sqlite3_finalize(stmt);
    utarray_free(timedDatas);

    return 0; // 成功
}

static uint8_t deleteCount = 0;
int kit_del_record_data_by_date(int storage_interval, const char *date_time_string)
{
    sqlite3_stmt *stmt = NULL; // SQLite语句句柄
    int ret = 0;               // 返回值
    char sql[256];             // SQL语句缓冲区

    deleteCount++;
    // 使用sqlite3_snprintf构建SQL语句，防止SQL注入
    sqlite3_snprintf(sizeof(sql), sql, "DELETE FROM timed_data_storage WHERE created_time < '%q';",
                     date_time_string);

    pthread_mutex_lock(&record_db_mutex); // 加锁，确保单线程访问
    // 开始事务，确保数据一致性
    ret = sqlite3_exec(record_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "无法开始事务: %s", sqlite3_errmsg(record_db));
        pthread_mutex_unlock(&record_db_mutex); // 解锁
        return 1;                               // 直接返回，不继续执行
    }

    // 准备SQL语句，避免直接使用sqlite3_exec
    ret = sqlite3_prepare_v2(record_db, sql, -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "SQL语句准备失败: %s SQL: %s", sqlite3_errmsg(record_db), sql);
        sqlite3_exec(record_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&record_db_mutex);                 // 解锁
        return 1;
    }

    // 执行SQL语句
    ret = sqlite3_step(stmt);
    if (ret != SQLITE_DONE) // SQLITE_DONE表示成功执行完语句
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "删除数据失败: %s SQL: %s", sqlite3_errmsg(record_db), sql);
        sqlite3_exec(record_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        pthread_mutex_unlock(&record_db_mutex);                 // 解锁
        sqlite3_finalize(stmt);                                 // 释放SQLite语句句柄
        return 1;
    }

    // 提交事务，确保操作生效
    ret = sqlite3_exec(record_db, "COMMIT;", NULL, NULL, NULL);
    if (ret != SQLITE_OK)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(record_db));
        sqlite3_exec(record_db, "ROLLBACK;", NULL, NULL, NULL); // 回滚事务
        sqlite3_finalize(stmt);                                 // 释放SQLite语句句柄
        pthread_mutex_unlock(&record_db_mutex);                 // 解锁
        return 1;
    }
    pthread_mutex_unlock(&record_db_mutex); // 解锁

    // 释放SQLite语句句柄，避免内存泄漏
    sqlite3_finalize(stmt);

    return 0; // 返回0表示成功
}

int kit_record_export_half_hour_csv_files(UT_array **data_stroage_config, char *file_path,
                                          int dev_type, int dev_id)
{
    FILE *fp = fopen(file_path, "w");
    if (!fp)
    {
        KITLOG(LOG_WM_EN, ERROR_EN, "创建文件%s失败", file_path);
        KITPTF(LOG_WM_EN, ERROR_EN, "创建文件%s失败", file_path);
        return 1;
    }

    // 记录当前设备存储周期为interval的测点数量
    int point_count = utarray_len(*data_stroage_config);

    // 构建 point_id → index 映射,以及构建表头
    uint8_t point_id_to_index[MAX_POINT_ID] = {0};

    memset(point_id_to_index, -1, sizeof(point_id_to_index));

    fprintf(fp, "timestamp");
    for (int i = 0; i < utarray_len(*data_stroage_config); ++i)
    {
        timed_data_config_t *p_config =
            (timed_data_config_t *)utarray_eltptr(*data_stroage_config, i);
        if (p_config != NULL && p_config->devType == dev_type)
        {
            // 构建表头
            fprintf(fp, ",%s", p_config->pointName);
            // 构建 point_id → index 映射
            int id = p_config->pointId;
            if (id >= 0 && id < MAX_POINT_ID)
                point_id_to_index[id] = i;
        }
    }
    fprintf(fp, "\n");

    // SQL 查询所有该设备该日期数据，按时间和点位排序
    const char *sql =
        "SELECT strftime('%s', created_time) as ts, point_id, timed_value FROM timed_data_storage "
        "WHERE dev_type=? AND dev_id=? "
        "ORDER BY ts, point_id;";

    sqlite3_stmt *stmt;
    pthread_mutex_lock(&record_db_mutex);
    if (sqlite3_prepare_v2(record_db, sql, -1, &stmt, NULL) != SQLITE_OK)
    {
        fprintf(stderr, "Failed to prepare statement\n");
        pthread_mutex_unlock(&record_db_mutex);
        fclose(fp);
        return 1;
    }
    sqlite3_bind_int(stmt, 1, dev_type);
    sqlite3_bind_int(stmt, 2, dev_id);

    // printf("dev_type = %d,dev_id = %d\n", dev_type, dev_id);
    // printf("sql = %s\n", sql);
    time_t current_ts = 0;
    double values[MAX_POINT_COUNT];
    memset(values, 0, sizeof(values));
    int has_value[MAX_POINT_COUNT];
    memset(has_value, 0, sizeof(has_value));

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        time_t ts = (time_t)sqlite3_column_int64(stmt, 0);
        int point_id = sqlite3_column_int(stmt, 1);
        double val = sqlite3_column_double(stmt, 2);

        if (current_ts != 0 && ts != current_ts)
        {
            // 写入上一组时间戳行
            char ts_buf[32];
            kit_time_format_tamp(current_ts, ts_buf, sizeof(ts_buf));
            fprintf(fp, "%s", ts_buf);
            // printf("ts_buf = %s\n", ts_buf);
            for (int i = 0; i < point_count; ++i)
            {
                fprintf(fp, ",");
                if (has_value[i])
                    fprintf(fp, "%.2f", values[i]);
                has_value[i] = 0;
            }
            fprintf(fp, "\n");
        }

        current_ts = ts;
        int idx = (point_id >= 0 && point_id < MAX_POINT_ID) ? point_id_to_index[point_id] : -1;
        if (idx >= 0 && idx < point_count)
        {
            values[idx] = val;
            has_value[idx] = 1;
        }
    }

    // 写最后一行
    if (current_ts != 0)
    {
        char ts_buf[32];
        kit_time_format_tamp(current_ts, ts_buf, sizeof(ts_buf));
        fprintf(fp, "%s", ts_buf);
        for (int i = 0; i < point_count; ++i)
        {
            fprintf(fp, ",");
            if (has_value[i])
                fprintf(fp, "%.2f", values[i]);
        }
        fprintf(fp, "\n");
    }

    sqlite3_finalize(stmt);
    pthread_mutex_unlock(&record_db_mutex);
    fclose(fp);
    return 0;
}

// 故障记录批量插入全局变量（现代方式：智能批量插入，一个函数搞定）
static sqlite3_stmt *fault_record_stmt = NULL; // 预编译语句（第一次调用时初始化）
static int fault_record_count = 0;             // 当前事务中的记录数
static struct timespec transaction_start_time; // 事务开始时间
static bool transaction_active = false;        // 事务是否活跃

// 后台清理机制（现代方式：自动管理，无需外部调用）
static pthread_t cleanup_thread = 0;                              // 后台清理线程
static bool cleanup_thread_running = false;                       // 清理线程是否运行
static struct timespec last_call_time;                            // 最后一次调用时间
static pthread_mutex_t cleanup_mutex = PTHREAD_MUTEX_INITIALIZER; // 清理线程互斥锁
static int fault_record_cleanup_timeout_sec = 1;                  // 超时时间（秒），默认1秒
static pthread_mutex_t fault_record_db_mutex =
    PTHREAD_MUTEX_INITIALIZER; // 故障记录数据库专用互斥锁

// 后台清理线程函数（现代方式：基于最后调用时间的超时清理）
static void *fault_record_cleanup_thread(void *arg)
{
    (void)arg; // 避免未使用参数警告

    while (cleanup_thread_running)
    {
        sleep(1); // 每秒检查一次

        pthread_mutex_lock(&cleanup_mutex);
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        // 计算距离最后一次commit的时间差（秒）
        long time_diff_sec = current_time.tv_sec - last_call_time.tv_sec;

        // 如果超过配置的超时时间没有commit事务，执行清理
        if (time_diff_sec >= fault_record_cleanup_timeout_sec)
        {
            pthread_mutex_unlock(&cleanup_mutex);

            // 执行最后一次commit（现代方式：程序最后一次调用后的超时清理）
            pthread_mutex_lock(&fault_record_db_mutex);
            if (transaction_active)
            {
                int ret = sqlite3_exec(record_db, "COMMIT;", NULL, NULL, NULL);
                if (ret != SQLITE_OK)
                {
                    KITLOG(LOG_KIT_EN, ERROR_EN, "后台清理：执行最后一次commit失败: %s",
                           sqlite3_errmsg(record_db));
                }
                else
                {
                    KITLOG(LOG_KIT_EN, INFO_EN,
                           "后台清理：执行最后一次commit成功，提交了 %d 条记录（%d秒无新调用）",
                           fault_record_count, fault_record_cleanup_timeout_sec);
                }

                transaction_active = false;
                fault_record_count = 0;
            }
            pthread_mutex_unlock(&fault_record_db_mutex);

            // 执行最后一次commit后退出线程（现代方式：使命完成，线程退出）
            cleanup_thread_running = false;
            KITLOG(LOG_KIT_EN, INFO_EN, "后台清理线程使命完成，已退出");
            break;
        }

        pthread_mutex_unlock(&cleanup_mutex);
    }

    return NULL;
}

// 智能批量插入故障记录（现代方式：一个函数搞定所有逻辑）
int kit_insert_fault_record_data(uint8_t pcs_index, uint8_t fault_index, uint16_t can_id,
                                 uint16_t *can_data)
{
    int ret = 0;
    pthread_mutex_lock(&fault_record_db_mutex);

    // 第一部分：预编译部分，只有第一次调用才执行
    if (fault_record_stmt == NULL)
    {
        const char *sql = "INSERT INTO fault_recorder "
                          "(PCS_index, fault_index, can_id, fault_u16_valueA, fault_u16_valueB, "
                          "fault_u16_valueC, fault_u16_valueD) "
                          "VALUES (?, ?, ?, ?, ?, ?, ?)";

        ret = sqlite3_prepare_v2(record_db, sql, -1, &fault_record_stmt, NULL);
        if (ret != SQLITE_OK)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "故障记录SQL预编译失败: %s", sqlite3_errmsg(record_db));
            pthread_mutex_unlock(&fault_record_db_mutex);
            return 1;
        }
    }

    // 第二部分：开始事务，只有第一次，和commit之后，执行
    if (!transaction_active)
    {
        ret = sqlite3_exec(record_db, "BEGIN TRANSACTION;", NULL, NULL, NULL);
        if (ret != SQLITE_OK)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "开始事务失败: %s", sqlite3_errmsg(record_db));
            pthread_mutex_unlock(&fault_record_db_mutex);
            return 1;
        }

        transaction_active = true;
        fault_record_count = 0;
        clock_gettime(CLOCK_MONOTONIC, &transaction_start_time); // 记录事务开始时间

        // 启动后台清理线程（现代方式：第一次调用时启动，执行最后一次commit后退出）
        if (!cleanup_thread_running)
        {
            cleanup_thread_running = true;

            // 初始化最后commit时间（现代方式：第一次启动时设置初始时间）
            pthread_mutex_lock(&cleanup_mutex);
            clock_gettime(CLOCK_MONOTONIC, &last_call_time);
            pthread_mutex_unlock(&cleanup_mutex);

            int thread_ret =
                pthread_create(&cleanup_thread, NULL, fault_record_cleanup_thread, NULL);
            if (thread_ret != 0)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "创建后台清理线程失败: %d", thread_ret);
                cleanup_thread_running = false;
            }
            else
            {
                KITLOG(LOG_KIT_EN, INFO_EN,
                       "后台清理线程启动成功，将在程序最后一次调用后%d秒执行最后一次commit",
                       fault_record_cleanup_timeout_sec);
            }
        }
    }

    // 第三部分：批量插入数据，每次执行
    // 数据转换
    uint16_t dataA = (int16_t)(((can_data[0] << 8) + can_data[1]));
    uint16_t dataB = (int16_t)(((can_data[2] << 8) + can_data[3]));
    uint16_t dataC = (int16_t)(((can_data[4] << 8) + can_data[5]));
    uint16_t dataD = (int16_t)(((can_data[6] << 8) + can_data[7]));

    // 绑定参数（现代方式：直接绑定，无需reset）
    sqlite3_bind_int(fault_record_stmt, 1, pcs_index);
    sqlite3_bind_int(fault_record_stmt, 2, fault_index);
    sqlite3_bind_int(fault_record_stmt, 3, can_id);
    sqlite3_bind_double(fault_record_stmt, 4, dataA);
    sqlite3_bind_double(fault_record_stmt, 5, dataB);
    sqlite3_bind_double(fault_record_stmt, 6, dataC);
    sqlite3_bind_double(fault_record_stmt, 7, dataD);

    // 执行插入
    ret = sqlite3_step(fault_record_stmt);
    if (ret != SQLITE_DONE)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "插入故障数据失败: %s", sqlite3_errmsg(record_db));
        sqlite3_exec(record_db, "ROLLBACK;", NULL, NULL, NULL);
        transaction_active = false;
        fault_record_count = 0;
        pthread_mutex_unlock(&fault_record_db_mutex);
        return 1;
    }

    // 重置语句，为下次使用做准备（现代方式：必须手动重置才能重新绑定参数）
    sqlite3_reset(fault_record_stmt);

    fault_record_count++;

    // 第四部分：提交事务，距离开始事务，超过100ms或100条记录后，提交
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    // 计算时间差（毫秒）
    long time_diff_ms = (current_time.tv_sec - transaction_start_time.tv_sec) * 1000 +
                        (current_time.tv_nsec - transaction_start_time.tv_nsec) / 1000000;

    // 检查是否需要提交：超过100ms或100条记录
    if (time_diff_ms >= 100 || fault_record_count >= 100)
    {
        ret = sqlite3_exec(record_db, "COMMIT;", NULL, NULL, NULL);
        if (ret != SQLITE_OK)
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "提交事务失败: %s", sqlite3_errmsg(record_db));
            pthread_mutex_unlock(&fault_record_db_mutex);
            return 1;
        }

        KITLOG(LOG_KIT_EN, INFO_EN, "正常提交故障记录事务成功，提交了 %d 条记录",
               fault_record_count);
        transaction_active = false;
        fault_record_count = 0;

        // 更新最后commit时间（现代方式：每次commit后更新时间戳）
        pthread_mutex_lock(&cleanup_mutex);
        clock_gettime(CLOCK_MONOTONIC, &last_call_time);
        pthread_mutex_unlock(&cleanup_mutex);
    }

    pthread_mutex_unlock(&fault_record_db_mutex);
    return 0;
}

/*****************************************************************
 * @brief 获取BMS系统配置
 * @param type 配置类型: 0-系统配置, 1-控制配置, FF-全部配置
 * @param config 输出参数，存储获取到的配置
 * @return 0-成功, 1-失败
 *****************************************************************/
int kit_get_bms_config(uint8_t type, bms_config_t *config)
{
    sqlite3_stmt *stmt = NULL;
    const char *query;
    int ret = 0;
    
    if (config == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "参数config不能为空");
        return 1;
    }
    
    // 清空配置结构体
    memset(config, 0, sizeof(bms_config_t));
    
    // 根据类型获取配置
    if (type == 0 || type == 0xFF)
    {
        // 获取系统配置 (type=0)
        query = "SELECT content FROM bms_sys_config WHERE type = 0 AND name = 'BBMS_System_Config'";
        
        // 准备 SQL 语句
        if (SQLITE_OK != sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL))
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询系统配置失败: %s", sqlite3_errmsg(config_db));
            return 1;
        }
        
        // 执行查询
        int step = sqlite3_step(stmt);
        if (SQLITE_ROW == step)
        {
            const unsigned char *content = sqlite3_column_text(stmt, 0);
            if (content == NULL)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "系统配置内容为空");
                sqlite3_finalize(stmt);
                return 1;
            }
            
            // 解析JSON配置
            cJSON *json = cJSON_Parse((const char *)content);
            if (json == NULL)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "解析系统配置JSON失败: %s", (const char *)content);
                sqlite3_finalize(stmt);
                return 1;
            }
            
            // 解析系统配置
            bms_sys_config_t *sys_cfg = &config->sys_config;
            cJSON *item = NULL;
            
            item = cJSON_GetObjectItem(json, "bat_type");
            if (item && item->valuestring)
            {
                sys_cfg->bat_type = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "bat_model");
            if (item && item->valuestring)
            {
                strncpy(sys_cfg->bat_model, item->valuestring, sizeof(sys_cfg->bat_model) - 1);
            }
            
            item = cJSON_GetObjectItem(json, "bat_group_num");
            if (item && item->valuestring)
            {
                sys_cfg->bat_group_num = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "cluster_crtl_mode");
            if (item && item->valuestring)
            {
                sys_cfg->cluster_crtl_mode = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "min_online_cluster_num");
            if (item && item->valuestring)
            {
                sys_cfg->min_online_cluster_num = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "sys_shutdown_word");
            if (item && item->valuestring)
            {
                sys_cfg->sys_shutdown_word = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "allow_grid_volt_diff");
            if (item && item->valuestring)
            {
                sys_cfg->allow_grid_volt_diff = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "detach_pulse_time");
            if (item && item->valuestring)
            {
                sys_cfg->detach_pulse_time = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "cut_delay_time");
            if (item && item->valuestring)
            {
                sys_cfg->cut_delay_time = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "alert_led_time_interval");
            if (item && item->valuestring)
            {
                sys_cfg->alert_led_time_interval = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "insulation_enable");
            if (item && item->valuestring)
            {
                sys_cfg->insulation_enable = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "insulation_monitor_interval");
            if (item && item->valuestring)
            {
                sys_cfg->insulation_monitor_interval = (uint8_t)atoi(item->valuestring);
            }

            item = cJSON_GetObjectItem(json, "poweron_timeout_ms");
            if (item && item->valuestring)
            {
                sys_cfg->poweron_timeout_ms = (uint32_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "poweroff_timeout_ms");
            if (item && item->valuestring)
            {
                sys_cfg->poweroff_timeout_ms = (uint32_t)atoi(item->valuestring);
            }
            
            cJSON_Delete(json);
        }
        else
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "未找到系统配置记录");
            sqlite3_finalize(stmt);
            return 1;
        }
        sqlite3_finalize(stmt);
        stmt = NULL;
    }
    
    if (type == 1 || type == 0xFF)
    {
        // 获取控制配置 (type=1)
        query = "SELECT content FROM bms_sys_config WHERE type = 1 AND name = 'BBMS_Control'";
        
        // 准备 SQL 语句
        if (SQLITE_OK != sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL))
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询控制配置失败: %s", sqlite3_errmsg(config_db));
            return 1;
        }
        
        // 执行查询
        int step = sqlite3_step(stmt);
        if (SQLITE_ROW == step)
        {
            const unsigned char *content = sqlite3_column_text(stmt, 0);
            if (content == NULL)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "控制配置内容为空");
                sqlite3_finalize(stmt);
                return 1;
            }
            
            // 解析JSON配置
            cJSON *json = cJSON_Parse((const char *)content);
            if (json == NULL)
            {
                KITLOG(LOG_KIT_EN, ERROR_EN, "解析控制配置JSON失败: %s", (const char *)content);
                sqlite3_finalize(stmt);
                return 1;
            }
            
            // 解析控制配置
            bbms_ctrl_t *ctrl_cfg = &config->ctrl_config;
            cJSON *item = NULL;
            
            item = cJSON_GetObjectItem(json, "bat_conn");
            if (item && item->valuestring)
            {
                ctrl_cfg->bat_conn = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "ins_meas_en");
            if (item && item->valuestring)
            {
                ctrl_cfg->ins_meas_en = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "reserved");
            if (item && item->valuestring)
            {
                uint8_t reserved_val = (uint8_t)atoi(item->valuestring);
                ctrl_cfg->first_enable_rack_flg = reserved_val & 0x01U;
                ctrl_cfg->rever_con_detec_cmp_flg = (reserved_val >> 1) & 0x01U;
            }

            item = cJSON_GetObjectItem(json, "first_enable_rack_flg");
            if (item && item->valuestring)
            {
                ctrl_cfg->first_enable_rack_flg = (uint8_t)atoi(item->valuestring);
            }

            item = cJSON_GetObjectItem(json, "rever_con_detec_cmp_flg");
            if (item && item->valuestring)
            {
                ctrl_cfg->rever_con_detec_cmp_flg = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "bat_str_en");
            if (item && item->valuestring)
            {
                ctrl_cfg->bat_str_en = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "bank_hb");
            if (item && item->valuestring)
            {
                ctrl_cfg->bank_hb = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "str_en_rack");
            if (item && item->valuestring)
            {
                ctrl_cfg->str_en_rack = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "rack_exit_flag");
            if (item && item->valuestring)
            {
                ctrl_cfg->rack_exit_flag = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "ctrl_mode");
            if (item && item->valuestring)
            {
                ctrl_cfg->ctrl_mode = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "sys_arch_type");
            if (item && item->valuestring)
            {
                ctrl_cfg->sys_arch_type = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "bank_err_lvl");
            if (item && item->valuestring)
            {
                ctrl_cfg->bank_err_lvl = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "disch_pwr_lim");
            if (item && item->valuestring)
            {
                ctrl_cfg->disch_pwr_lim = (uint8_t)atoi(item->valuestring);
            }
            
            item = cJSON_GetObjectItem(json, "chg_pwr_lim");
            if (item && item->valuestring)
            {
                ctrl_cfg->chg_pwr_lim = (uint8_t)atoi(item->valuestring);
            }

            item = cJSON_GetObjectItem(json, "bbms_no");
            if (item && item->valuestring)
            {
                ctrl_cfg->bbms_no = (uint8_t)atoi(item->valuestring) & 0x0FU;
            }
            
            cJSON_Delete(json);
        }
        else
        {
            KITLOG(LOG_KIT_EN, ERROR_EN, "未找到控制配置记录");
            sqlite3_finalize(stmt);
            return 1;
        }
        sqlite3_finalize(stmt);
    }
    
    return 0;
}





/**
 * @brief 获取DI配置
 * @param config 输出参数，存储获取到的DI配置
 */
void kit_db_get_di_config(di_config_t *config)
{
    sqlite3_stmt *stmt = NULL;
    const char *query;
    
    if (config == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "参数config不能为空");
        return;
    }
    
    // 初始化配置结构体
    memset(config, 0, sizeof(di_config_t));
    
    // 查询DI配置，按seq排序
    query = "SELECT seq, cabinet_code, strategy_seq, valid_mode, remark FROM di_set_config ORDER BY seq";
    
    // 准备 SQL 语句
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询DI配置失败: %s", sqlite3_errmsg(config_db));
        return;
    }
    
    // 执行查询并处理结果
    int step = sqlite3_step(stmt);
    int index = 0;
    while (SQLITE_ROW == step)
    {
        if (index >= kDi_Signal_Stop)
        {
            KITLOG(LOG_KIT_EN, WARN_EN, "DI配置数量超过最大限制: %d", kDi_Signal_Stop);
            break;
        }
        
        // 获取数据库中的每一列值
        di_logic_conf_t *di_cfg = &config->di_logic_config[index];
        uint8_t seq = (uint8_t)sqlite3_column_int(stmt, 0);           // seq
        di_cfg->di_idx = (uint16_t)seq;                                 // di_idx 对应seq
        di_cfg->logic_idx = (uint16_t)sqlite3_column_int(stmt, 2);    // strategy_seq
        di_cfg->bms_idx = 0;                                           // 初始化为0，后续根据需要设置
        di_cfg->actual_value = 0;                                      // 初始化为0，后续更新
        di_cfg->signal_value = kDiSignalValue_None;                    // 初始化为无信号
        di_cfg->di_input_mode = (di_valid_mode_e)sqlite3_column_int(stmt, 3); // valid_mode
        di_cfg->delay_time = 0;                                        // 初始化为0，防抖时间
        
        // cabinet_code暂时不适用，可以根据需要赋值
        uint8_t cabinet_code = (uint8_t)sqlite3_column_int(stmt, 1);
        
        // 备注信息
        const unsigned char *remark = sqlite3_column_text(stmt, 4);
        if (remark != NULL)
        {
            KITLOG(LOG_KIT_EN, DEBUG_EN, "DI[%d] seq=%d, remark: %s", index, seq, remark);
        }
        
        index++;
        step = sqlite3_step(stmt);
    }
    
    config->di_config_num = index;
    
    // 检查是否完成所有行的处理
    if (SQLITE_DONE != step)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询DI配置未完成，错误信息: %s", sqlite3_errmsg(config_db));
    }
    
    // 清理资源
    sqlite3_finalize(stmt);
    
    KITLOG(LOG_KIT_EN, INFO_EN, "成功获取DI配置，共%d条", config->di_config_num);
}

/**
 * @brief 获取DO配置
 * @param config 输出参数，存储获取到的DO配置
 */
void kit_db_get_do_config(do_config_t *config)
{
    sqlite3_stmt *stmt = NULL;
    const char *query;
    
    if (config == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "参数config不能为空");
        return;
    }
    
    // 初始化配置结构体
    memset(config, 0, sizeof(do_config_t));
    
    // 查询DO配置，按seq排序
    query = "SELECT seq, cabinet_code, strategy_seq, ctrl_mode, remark FROM do_set_config ORDER BY seq";
    
    // 准备 SQL 语句
    if (SQLITE_OK != sqlite3_prepare_v2(config_db, query, -1, &stmt, NULL))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "准备查询DO配置失败: %s", sqlite3_errmsg(config_db));
        return;
    }
    
    // 执行查询并处理结果
    int step = sqlite3_step(stmt);
    int index = 0;
    while (SQLITE_ROW == step)
    {
        if (index >= kDo_Ctrl_Stop)
        {
            KITLOG(LOG_KIT_EN, WARN_EN, "DO配置数量超过最大限制: %d", kDo_Ctrl_Stop);
            break;
        }
        
        // 获取数据库中的每一列值
        do_logic_conf_t *do_cfg = &config->do_logic_config[index];
        uint8_t seq = (uint8_t)sqlite3_column_int(stmt, 0);           // seq
        do_cfg->do_idx = (uint16_t)seq;                                // do_idx 对应seq
        do_cfg->logic_idx = (uint16_t)sqlite3_column_int(stmt, 2);    // strategy_seq
        do_cfg->bms_idx = 0;                                           // 初始化为0，后续根据需要设置
        do_cfg->actual_value = 0;                                      // 初始化为0，后续更新
        do_cfg->do_output_mode = (do_ctrl_mode_e)sqlite3_column_int(stmt, 3); // ctrl_mode
        
        // cabinet_code暂时不适用，可以根据需要赋值
        uint8_t cabinet_code = (uint8_t)sqlite3_column_int(stmt, 1);
        
        // 备注信息
        const unsigned char *remark = sqlite3_column_text(stmt, 4);
        if (remark != NULL)
        {
            KITLOG(LOG_KIT_EN, DEBUG_EN, "DO[%d] seq=%d, remark: %s", index, seq, remark);
        }
        
        index++;
        step = sqlite3_step(stmt);
    }
    
    config->do_config_num = index;
    
    // 检查是否完成所有行的处理
    if (SQLITE_DONE != step)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "查询DO配置未完成，错误信息: %s", sqlite3_errmsg(config_db));
    }
    
    // 清理资源
    sqlite3_finalize(stmt);
    
    KITLOG(LOG_KIT_EN, INFO_EN, "成功获取DO配置，共%d条", config->do_config_num);
}