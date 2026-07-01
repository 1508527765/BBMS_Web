/*****************************************************************************
 * @copyright       1997-2050,AO DI LTD. 
 * @file            kit_core.c
 * @brief           xx功能
 * @author          Gary
 * @date            2024/12/13
 * @remark          
 *****************************************************************************/

#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "kit_core.h"

sem_t event_semaphore_pcs_dist; // 功率分配使用
sem_t event_semaphore_pcs_reac_dist; // 无功分配使用
sem_t event_semaphore_pcs_switch; // 开关机控制使用

/*****************************************************************************
 * @brief           通过管道执行shell命令 (使用 system)
 * @param[in]       cmd：shell命令
 * @param[in]       buffer：读取执行结果
 * @param[in]       buffer_size：读取执行结果长度
 * @return          1-执行错误  0-执行成功
 *****************************************************************************/
int kit_popen_exec_old(const char *cmd, char* buffer, size_t buffer_size)
{
#if IF_RUNIN_VM == RUNIN_VM // 虚拟机环境，返回默认值
    return 0;
#endif
    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "popen failed");
        return 1;
    }

    // 读取执行结果
    size_t bytes_read = fread(buffer, 1, buffer_size - 1, fp); // 最多只读取buffer_size比特数据
    if (ferror(fp))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "文件读取失败: %s\n", strerror(errno));
        pclose(fp);
        return 1;
    }
    buffer[bytes_read] = '\0'; // 字符串要以NULL结束

    int status = pclose(fp);
    if (status == -1)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "pclose failed");
        return 1;
    }
    else if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "CMD命令 '%s' 执行失败，失败的编号： %d\n", cmd, WEXITSTATUS(status));
        return 1;
    }
    return 0;
}

int kit_popen_exec(const char *cmd, char* buffer, size_t buffer_size)
{
#if IF_RUNIN_VM == RUNIN_VM
    return 0;
#endif

    FILE *fp = popen(cmd, "r");
    if (fp == NULL)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "popen failed for command: %s", cmd);
        return 1;
    }

    size_t total_bytes = 0;
    size_t bytes_read = 0;
    int truncated = 0;
    
    // 安全读取
    while ((bytes_read = fread(buffer + total_bytes, 1, buffer_size - total_bytes - 1, fp)) > 0)
    {
        total_bytes += bytes_read;
        
        if (total_bytes >= buffer_size - 1) {
            truncated = 1;
            break;
        }
    }
    
    buffer[total_bytes] = '\0';

    // 如果发生截断，记录警告
    if (truncated) {
        KITLOG(LOG_KIT_EN, WARN_EN, "Command output truncated, need larger buffer (current: %zu)", buffer_size);
        // 丢弃剩余内容
        char trash[1024];
        while (fread(trash, 1, sizeof(trash), fp) > 0);
    }

    if (ferror(fp))
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "Read failed: %s", strerror(errno));
        pclose(fp);
        return 1;
    }

    int status = pclose(fp);
    if (status == -1)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "pclose failed");
        return 1;
    }
    else if (WIFEXITED(status) && WEXITSTATUS(status) != 0)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "Command failed: %s, exit code: %d", cmd, WEXITSTATUS(status));
        if (strlen(buffer) > 0) {
            KITLOG(LOG_KIT_EN, ERROR_EN, "Output: %s", buffer);
        }
        return 1;
    }
    
    return 0;
}

/*****************************************************************************
 * @brief           获取网络连接状态
 * @param[in]       cmd：shell命令
 * @return          1-网络不可连接  0-网络连接成功
 *****************************************************************************/
int kit_get_net_status(void)
{
    FILE *fp = popen("ping -c 1 -W 1 www.baidu.com > /dev/null 2>&1", "r");

    if (fp == NULL) 
    {
        perror("popen failed");
        return 1;  // 如果popen失败，认为网络不可用
    }

    // 等待命令执行完毕，获取返回状态
    int status = pclose(fp);
    
    // WEXITSTATUS获取子进程返回的状态码，ping成功返回0
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) 
    {
        return 0;  // 网络可用
    } 
    else 
    {
        return 1;  // 网络不可用
    }
}

/*****************************************************************************
 * @brief           获取SCU的SN号
 * @param[in]       serial：返回SN号
 * @return          1-执行错误  0-执行成功
 *****************************************************************************/
int kit_get_scu_sn(char serial[128])
{
    char buffer[3000]; // 接收执行返回值
    char buffer_copy[3000]; // 用于 strtok 的副本
    char *line = NULL;
    char *serialStart = NULL;

    // 使用 kit_popen_exec 执行"cat /proc/cpuinfo"
    if (kit_popen_exec("cat /proc/cpuinfo", buffer, sizeof(buffer)) != 0) 
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "cat /proc/cpuinfo 命令执行失败\n");
        return 1; // 返回错误
    }

    strcpy(buffer_copy, buffer); // 复制 buffer 到 buffer_copy
    // 使用 strtok 按行分割 buffer_copy
    line = strtok(buffer_copy, "\n");
    while (line != NULL) 
    {
        // 查找 "Serial" 字符串
        if ((serialStart = strstr(line, "Serial")) != NULL) 
        {
            // 提取序列号
            serialStart += strlen("Serial\t\t: "); // 跳过 "Serial          : "
            char *endOfSerial = strchr(serialStart, '\n'); // 查找换行符
            if (endOfSerial != NULL) 
            {
                *endOfSerial = '\0'; // 将换行符替换为字符串终止符
            }
            strncpy(serial, serialStart, 127); // 复制序列号到 serial，限制长度防止溢出
            serial[127] = '\0'; // 添加字符串终止符
            return 0; // 返回成功
        }
        line = strtok(NULL, "\n");
    }

    KITLOG(LOG_DRIVER_EN, ERROR_EN, "未找到序列号\n");
    return 1; // 返回错误
}

// 获取系统 CPU 时间
void getSysCpuTime(cpu_time_t *time)
{
    FILE *file = fopen("/proc/stat", "r");
    if (file == NULL)
    {
        perror("Error opening /proc/stat");
        exit(EXIT_FAILURE);
    }
    fscanf(file, "cpu %lu %lu %lu %lu %lu %lu %lu %lu",
           &time->user, &time->nice, &time->system, &time->idle,
           &time->iowait, &time->irq, &time->softirq, &time->steal);
    fclose(file);
}

// 获取指定进程的 CPU 时间
void getProCpuTime(cpu_time_t *time, pid_t pid)
{
    char filename[32];
    sprintf(filename, "/proc/%d/stat", pid);
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Error opening process stat file");
        exit(EXIT_FAILURE);
    }
    uint64_t utime, stime;
    fscanf(file, "%*d %*s %*c %*d %*d %*d %*d %*d %*u %*u %*u %*u %*u %lu %lu", &utime, &stime);
    time->user = utime;
    time->system = stime;
    fclose(file);
}

// 获取内存信息的函数
void getMemInfo(memory_info_t *info)
{
    FILE *file = fopen("/proc/meminfo", "r"); // 打开 /proc/meminfo 文件
    if (file == NULL)
    {
        perror("Error opening /proc/meminfo"); // 如果打开失败，打印错误信息并退出
        return;
    }
    // 从文件中读取内存信息，包括总内存、空闲内存、可用内存、缓冲区内存和缓存内存
    fscanf(file, "MemTotal: %lu kB\nMemFree: %lu kB\nMemAvailable: %lu kB\nBuffers: %lu kB\nCached: %lu kB",
           &info->total, &info->free, &info->available, &info->buffers, &info->cached);
    fclose(file); // 关闭文件
}

// 函数用于提取指定关键字后的整数值
int extractValue(const char *line, const char *keyword)
{
    char *pos = strstr(line, keyword);
    if (pos == NULL)
    {
        return -1;
    }
    pos += strlen(keyword);
    int value;
    sscanf(pos, "%d", &value);
    return value;
}

// 计算当前进程内存占用
double calculateProcMemUsage(memory_info_t info, pid_t pid)
{
    char filename[32];
    sprintf(filename, "/proc/%d/status", pid);
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        // perror("Error opening process status file");
        return 0.0;
    }
    char line[256];
    int rss = -1;
    while (fgets(line, sizeof(line), file))
    {
        if (rss == -1 && strstr(line, "VmRSS") != NULL)
        {
            rss = extractValue(line, "VmRSS:");
        }
    }
    fclose(file);
    if (rss != -1)
    {
        // printf("Current process resident memory size: %d kB\n", rss);
        return (double)rss / info.total * 100.0;
    }
    else
    {
        // printf("Failed to get process memory usage.\n");
        return 0.0;
    }
}

/*****************************************************************************
 * @brief           获取SCU的SN号
 * @param[in]       serial：返回SN号
 * @return          1-执行错误  0-执行成功
 *****************************************************************************/
int kit_get_ems_sn(char serial[50])
{
    char buffer[3000]; // 接收执行返回值
    char buffer_copy[3000]; // 用于 strtok 的副本
    char *line = NULL;
    char *serialStart = NULL;

    // 使用 kit_popen_exec 执行"cat /proc/cpuinfo|grep Serial"
    if (kit_popen_exec("cat /proc/cpuinfo|grep Serial", buffer, sizeof(buffer)) != 0) 
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "cat /proc/cpuinfo|grep Serial 命令执行失败\n");
        return 1; // 返回错误
    }

    strcpy(buffer_copy, buffer); // 复制 buffer 到 buffer_copy
    // 使用 strtok 按行分割 buffer_copy
    line = strtok(buffer_copy, "\n");
    while (line != NULL) 
    {
        // 查找 "Serial" 字符串
        if ((serialStart = strstr(line, "Serial")) != NULL) 
        {
            // 提取序列号
            serialStart += strlen("Serial\t\t: "); // 跳过 "Serial          : "
            char *endOfSerial = strchr(serialStart, '\n'); // 查找换行符
            if (endOfSerial != NULL) 
            {
                *endOfSerial = '\0'; // 将换行符替换为字符串终止符
            }
            strncpy(serial, serialStart, 127); // 复制序列号到 serial，限制长度防止溢出
            serial[127] = '\0'; // 添加字符串终止符
            return 0; // 返回成功
        }
        line = strtok(NULL, "\n");
    }

    KITLOG(LOG_DRIVER_EN, ERROR_EN, "未找到序列号\n");
    return 1; // 返回错误
}

/*********************************************************************
 * @brief       设置系统时间
 * @param[in]   sec: 1970年以来的秒数
 * @param[in]   linuxPass: linux密码
 * @return      0：成功；1：失败
 *********************************************************************/
int setSysTime(time_t sec, char linuxPass[50])
{
    struct timeval tv;
    tv.tv_sec = sec;
    tv.tv_usec = 0;

    char temp[300] = {0};                                      // 命令缓冲区
    char buffer[256] = {0};                                    // 执行接收命令直接的返回值
    snprintf(temp, sizeof(temp), "echo \"%s\" | sudo -S timedatectl set-ntp false", linuxPass); // 使用设置手动对时
    if (kit_popen_exec(temp, buffer, sizeof(buffer)) != 0)
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "执行设置手动对时失败: %s\n", temp);
        return 1;
    }

    // 设置系统时间
    if (settimeofday(&tv, NULL) == -1) 
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "设置系统时间失败"); // 使用 KITLOG 打印更详细的错误信息
        return 1;
    }

    // 使用 kit_popen_exec 执行 hwclock -w 命令
    snprintf(temp, sizeof(temp), "echo \"%s\" | sudo -S hwclock -w", linuxPass);
    if (kit_popen_exec(temp, buffer, sizeof(buffer)) != 0) 
    {
        KITLOG(LOG_DRIVER_EN, ERROR_EN, "hwclock -w 命令执行失败");
        return 1; // 返回错误
    }

    return 0; // 成功
}

/*********************************************************************
 * @brief       初始化信号量
 * @return      无
 *********************************************************************/
int initSysSemaphore(void)
{
    // 初始化信号量
    if (sem_init(&event_semaphore_pcs_dist, 0, 0) != 0)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "初始化 event_semaphore_pcs_dist 信号量失败: %s", strerror(errno));
        return 1;
    }
    // 初始化信号量
    if (sem_init(&event_semaphore_pcs_reac_dist, 0, 0) != 0)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "初始化 event_semaphore_pcs_reac_dist 信号量失败: %s", strerror(errno));
        return 1;
    }

    // 初始化信号量
    if (sem_init(&event_semaphore_pcs_switch, 0, 0) != 0)
    {
        KITLOG(LOG_KIT_EN, ERROR_EN, "初始化 event_semaphore_pcs_switch 信号量失败: %s", strerror(errno));
        return 1;
    }
    return 0;

    // 其他信号量初始化可以在这里添加
}

// 递归创建目录（类似 mkdir -p）
static void mkdir_p(const char *path)
{
    char tmp[512];
    char *p = NULL;
    size_t len;

    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);

    // 去除末尾的 '/'
    if (tmp[len - 1] == '/')
    {
        tmp[len - 1] = '\0';
    }

    // 逐级创建目录
    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/')
        {
            *p = '\0';
            mkdir(tmp, 0777); // 尝试创建目录（如果已存在则忽略）
            *p = '/';
        }
    }
    mkdir(tmp, 0777); // 创建最后一级目录
}

void kit_creat_sys_folder(const char *base_path, const char *date_str)
{
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", base_path, date_str);

    printf("Attempting to create directory: %s\n", full_path);
    mkdir_p(full_path);

    // 检查是否创建成功
    struct stat st;
    if (stat(full_path, &st) == 0 && S_ISDIR(st.st_mode))
    {
        printf("Successfully created directory: %s\n", full_path);
    }
    else
    {
        fprintf(stderr, "Failed to create directory: %s (%s)\n", full_path, strerror(errno));
    }
}

/// @brief 删除指定目录下，早于 days 天的子目录和文件
/// @param abs_basepath，必须是绝对路径
/// @param days，正整数
/// @return system返回值，没啥意义，反正定时执行，这次不成功，还有下次执行
int delete_old_dirs(const char *abs_basepath, int days) {
    if (!abs_basepath || strlen(abs_basepath) == 0)
    {
        printf("错误：目录路径为空\n");
        return -1;
    }
    
    if (days <= 0)
    {
        printf("错误：天数必须大于0\n");
        return -1;
    }
    
    char cmd[512];
    // 拼接 find 命令
    snprintf(cmd, sizeof(cmd),
             "find '%s' -mindepth 1 -maxdepth 1 -type d -mtime +%d -exec rm -rf -- {} +",
             abs_basepath, days);

    printf("执行命令: %s\n", cmd);
    int result = system(cmd);
    
    // 正确的system返回值判断
    if (result == -1) {
        printf("错误：无法执行系统命令\n");
        return -1;
    }
    else if (WIFEXITED(result) && WEXITSTATUS(result) == 0) {
        printf("目录清理完成\n");
        return 0;
    }
    else {
        printf("目录清理失败，返回码: %d\n", result);
        return -1;
    }
}

/*****************************************************************************
 * @brief           管理指定目录中的文件数量，保持指定数量的文件
 * @param[in]       dir_name: 目录路径
 * @param[in]       max_files: 最大文件数量
 * @return          0-成功  -1-失败
 * @remark          超过指定文件数量时删除时间最远的文件
 *****************************************************************************/
int manageDirectoryFileCount(const char *dir_name, int max_files)
{
    if (!dir_name || strlen(dir_name) == 0)
    {
        printf("错误：目录路径为空\n");
        return -1;
    }
    
    if (max_files <= 0)
    {
        printf("错误：最大文件数量必须大于0\n");
        return -1;
    }

    char cmd[512];
    int result;

    // 使用shell命令：ls -t 按时间排序，tail -n +N 跳过前N-1个，xargs rm 删除
    snprintf(cmd, sizeof(cmd), 
             "ls -t '%s' 2>/dev/null | tail -n +%d | xargs -r -I {} rm -f '%s'/{}", 
             dir_name, max_files + 1, dir_name);

    printf("执行命令: %s\n", cmd);
    result = system(cmd);

    // 正确的system返回值判断
    if (result == -1) {
        printf("错误：无法执行系统命令\n");
        return -1;
    }
    else if (WIFEXITED(result) && WEXITSTATUS(result) == 0) {
        printf("目录 %s 文件管理完成\n", dir_name);
        return 0;
    }
    else {
        printf("目录 %s 文件管理失败，返回码: %d\n", dir_name, result);
        return -1;
    }
}


/// @brief 判断目录是否存在
/// @param dir_path 目录路径
/// @return 1-存在且是目录 0-不存在 -1-存在但不是目录或访问错误
int isDirectoryExists(const char *dir_path)
{
    if (!dir_path || strlen(dir_path) == 0)
    {
        printf("错误：目录路径为空\n");
        return -1;
    }

    struct stat st = {0};
    int stat_result = stat(dir_path, &st);

    if (stat_result == -1)
    {
        if (errno == ENOENT)
        {
            // 目录不存在
            return 0;
        }
        else
        {
            // 其他错误（权限问题、路径无效等）
            printf("错误：无法访问目录 %s，错误信息: %s (错误码: %d)\n",
                   dir_path, strerror(errno), errno);
            return -1;
        }
    }
    else if (!S_ISDIR(st.st_mode))
    {
        // 路径存在但不是目录
        printf("错误：路径 %s 已存在但不是目录\n", dir_path);
        return -1;
    }

    return 1; // 目录存在且是目录
}

/// @brief 创建目录（如果不存在）/不支持递归创建
/// @param dir_path 目录路径
/// @return 0-成功 -1-失败 1-警告（目录不可写）
int createDirectoryIfNotExists(const char *dir_path)
{
    // 首先检查目录是否存在
    int exists = isDirectoryExists(dir_path);
    
    if (exists == 1)
    {
        // 目录已存在，检查权限
        if (access(dir_path, W_OK) != 0)
        {
            printf("警告：目录 %s 不可写，错误信息: %s\n", dir_path, strerror(errno));
            return 1; // 返回1表示警告
        }
        return 0; // 目录存在且可写
    }
    else if (exists == -1)
    {
        // 访问错误或路径不是目录
        return -1;
    }
    
    // 目录不存在，尝试创建
    if (mkdir(dir_path, 0777) == -1)
    {
        printf("错误：无法创建目录 %s，错误信息: %s (错误码: %d)\n",
               dir_path, strerror(errno), errno);
        return -1;
    }
    
    printf("信息：已成功创建目录 %s\n", dir_path);
    
    // // 创建后检查权限
    // if (access(dir_path, W_OK) != 0)
    // {
    //     printf("警告：新创建的目录 %s 不可写，错误信息: %s\n", dir_path, strerror(errno));
    //     return 1; // 返回1表示警告
    // }
    
    return 0; // 成功
}

