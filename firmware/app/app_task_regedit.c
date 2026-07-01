/*****************************************************************************
 * @copyright       1997-2050,AO DI LTD.
 * @file            app_task_regedit.c
 * @brief           xx功能
 * @author          Gary
 * @date            2024-12-05
 * @remark
 *****************************************************************************/
#define _XOPEN_SOURCE 700
#include <errno.h>
#include "drv_4g.h"
#include "drv_wifi.h"
#include "app_task_regedit.h"
#include "protocol_bms.h"

/*********************************************************************
 * @brief       SCU时间处理初始化函数，根据配置信息设置系统时间（NTP或手动）
 * @param[in]   rtc_dev_config:  设备配置信息 (包含NTP和手动时间设置参数，通常为JSON格式)
 * @return      0: 成功；1: 失败 (内存分配失败，JSON解析失败，NTP同步失败，或设置系统时间失败)
 *********************************************************************/
static int systimeInitHandle(con_dev_config_t *rtc_dev_config)
{
    int ret = 0;
    con_dev_rtc_config_t rtcInfo = {0};                                   // 使用局部变量，函数结束时自动释放内存
    cJSON *json_obj = cJSON_Parse((const char *)rtc_dev_config->content); // 解析JSON配置字符串

    if (json_obj == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "错误：解析JSON配置失败: %s\n", cJSON_GetErrorPtr());
        return 1;
    }

    cJSON *item;

    // 安全地获取JSON对象中的各个字段
    item = cJSON_GetObjectItem(json_obj, "isNtp");
    rtcInfo.isNtp = (item != NULL && cJSON_IsNumber(item)) ? item->valueint : 0;

    // 判断是否使用 NTP 自动对时
    if (rtcInfo.isNtp)
    {
        item = cJSON_GetObjectItem(json_obj, "port");
        rtcInfo.port = (item != NULL && cJSON_IsNumber(item)) ? item->valueint : 0;

        item = cJSON_GetObjectItem(json_obj, "address");
        strncpy((char *)rtcInfo.address, (item != NULL && cJSON_IsString(item)) ? item->valuestring : "", MAX_ADDR_LEN);
        rtcInfo.address[MAX_ADDR_LEN - 1] = '\0';                                   // 确保字符串以'\0'结尾
        ret = sync_ntp_timestamp((char *)rtcInfo.address, rtcInfo.port, "forlinx"); // 调用NTP对时函数
        if (ret != 0)
        {
            KITLOG(LOG_APP_EN, ERROR_EN, "错误：NTP同步失败: %s\n", strerror(errno));
        }
    }
    cJSON_Delete(json_obj); // 释放JSON对象

    return ret;
}

/*********************************************************************
 * @brief       初始化系统的IP、mack地址、网关等配置
 * @param[in]   eth_dev_config: 以太网口配置信息
 * @return      0-成功  1-失败
 *********************************************************************/
static int netInitHandble(con_dev_config_t *eth_dev_config)
{
    int ret = 0;
    con_dev_eth_config_t ethInfo = {0};                                   // 使用局部变量，函数结束时自动释放内存
    cJSON *json_obj = cJSON_Parse((const char *)eth_dev_config->content); // 解析JSON配置字符串

    if (json_obj == NULL)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "网口初始化时，错误：解析JSON配置失败: %s\n", cJSON_GetErrorPtr());
        KITLOG(LOG_APP_EN, ERROR_EN, "网口初始化时，错误：解析JSON配置失败: %s\n", cJSON_GetErrorPtr());
        return 1;
    }

    cJSON *item;

    // 安全地获取JSON对象中的各个字段
    item = cJSON_GetObjectItem(json_obj, "netId");
    ethInfo.netId = (item != NULL && cJSON_IsNumber(item)) ? item->valueint : 0;

    item = cJSON_GetObjectItem(json_obj, "isDHCP");
    ethInfo.isDHCP = (item != NULL && cJSON_IsNumber(item)) ? item->valueint : 0;

    item = cJSON_GetObjectItem(json_obj, "address");
    strncpy((char *)ethInfo.address, (item != NULL && cJSON_IsString(item)) ? item->valuestring : "", MAX_ADDR_LEN);
    ethInfo.address[MAX_ADDR_LEN - 1] = '\0'; // 确保字符串以'\0'结尾

    char temp[300] = {0}; // 命令缓冲区
    char buffer[256];     // 获取执行返回结果

    // 关闭netplan自动覆盖IP配置
    snprintf(temp, sizeof(temp), "echo \"%s\" | sudo -S systemctl stop systemd-networkd.service", "forlinx"); // 使用 DHCP 获取 IP
    ret = kit_popen_exec(temp, buffer, sizeof(buffer));
    if (ret != 0)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "网口初始化时，systemctl 执行失败: %s\n", temp);
        KITLOG(LOG_APP_EN, ERROR_EN, "网口初始化时，systemctl 执行失败: %s\n", temp);
        return 1;
    }

    // 判断是否使用 DHCP 动态分配 IP
    if (ethInfo.isDHCP == 1)
    {
        snprintf(temp, sizeof(temp), "echo \"%s\" | sudo -S udhcpc -i eth%d -n", "forlinx", ethInfo.netId); // 使用 DHCP 获取 IP
        ret = kit_popen_exec(temp, buffer, sizeof(buffer));
        if (ret != 0)
        {
            KITPTF(LOG_APP_EN, ERROR_EN, "网口初始化时，udhcpc 执行失败: %s\n", temp);
            KITLOG(LOG_APP_EN, ERROR_EN, "网口初始化时，udhcpc 执行失败: %s\n", temp);
            return 1;
        }
    }
    else
    {
        item = cJSON_GetObjectItem(json_obj, "ip");
        strncpy((char *)ethInfo.ip, (item != NULL && cJSON_IsString(item)) ? item->valuestring : "", MAX_IP_LEN);
        ethInfo.ip[MAX_IP_LEN - 1] = '\0'; // 确保字符串以'\0'结尾

        item = cJSON_GetObjectItem(json_obj, "mask");
        strncpy((char *)ethInfo.mask, (item != NULL && cJSON_IsString(item)) ? item->valuestring : "", MAX_IP_LEN);
        ethInfo.mask[MAX_IP_LEN - 1] = '\0'; // 确保字符串以'\0'结尾

        item = cJSON_GetObjectItem(json_obj, "gateway");
        strncpy((char *)ethInfo.gateway, (item != NULL && cJSON_IsString(item)) ? item->valuestring : "", MAX_IP_LEN);
        ethInfo.gateway[MAX_IP_LEN - 1] = '\0'; // 确保字符串以'\0'结尾

        snprintf(temp, sizeof(temp), "echo \"%s\" | sudo -S ifconfig eth%d %s netmask %s",
                 "forlinx", ethInfo.netId, ethInfo.ip, ethInfo.mask); // 静态分配 IP 地址和子网掩码

        KITPTF(LOG_APP_EN, INFO_EN, "执行IP设置：%s", temp);

        ret = kit_popen_exec(temp, buffer, sizeof(buffer));
        if (ret != 0)
        {
            KITPTF(LOG_APP_EN, ERROR_EN, "网口初始化时，ifconfig 执行失败: %s\n", temp);
            KITLOG(LOG_APP_EN, ERROR_EN, "网口初始化时，ifconfig 执行失败: %s\n", temp);
            return 1;
        }
    }
    cJSON_Delete(json_obj); // 释放JSON对象

    return ret;
}

/*********************************************************************
 * @brief       初始化can的配置入配置缓存
 * @param[in]   can_dev_config: can的配置信息
 * @return      0-成功  1-失败
 *********************************************************************/
static int canInitHandble(con_dev_config_t *can_dev_config)
{
    int ret = 0;
    con_dev_can_config_t itemInfo = {0};                                  // 使用局部变量，函数结束时自动释放内存
    cJSON *json_obj = cJSON_Parse((const char *)can_dev_config->content); // 解析JSON配置字符串

    if (json_obj == NULL)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "网口初始化时，错误：解析JSON配置失败: %s\n", cJSON_GetErrorPtr());
        KITLOG(LOG_APP_EN, ERROR_EN, "网口初始化时，错误：解析JSON配置失败: %s\n", cJSON_GetErrorPtr());
        return 1;
    }

    cJSON *item;

    // 安全地获取JSON对象中的各个字段
    item = cJSON_GetObjectItem(json_obj, "canId");
    itemInfo.canId = (item != NULL && cJSON_IsNumber(item)) ? item->valueint : 0;

    item = cJSON_GetObjectItem(json_obj, "baud");
    itemInfo.bitrate = (item != NULL && cJSON_IsNumber(item)) ? item->valueint : 0;

    // 设置内部buf
    ret = drvSetBaudBuf(itemInfo.canId, itemInfo.bitrate);
    cJSON_Delete(json_obj); // 释放JSON对象

    return ret;
}

/*********************************************************************
 * @brief       初始化系统Wifi配置
 * @param[in]   wifi_dev_config: wifi配置信息
 * @return      0-成功  1-失败
 *********************************************************************/
static int sysWifiInitHandle(con_dev_config_t *wifi_dev_config)
{
    int ret = 0;
    return ret;
}

/*********************************************************************
 * @brief       初始化系统4G配置
 * @param[in]   fourthg_dev_config：4G相关配置
 * @return      0-成功  1-失败
 *********************************************************************/
static int sys4GInitHandle(con_dev_config_t *fourthg_dev_config)
{
    int ret = 0;
    drv_4g_t fourthgInfo = {0}; // 使用局部变量，函数结束时自动释放内存
    return ret;
}

/*********************************************************************
 * @brief       初始化数据库
 * @param[in]   void
 * @return      0-成功  1-失败
 *********************************************************************/
int initSqliteDb(void) // 初始化系统时间
{
    int ret = 0;
    // 初始化数据库,"./config"是打包里的sql文件路径
    ret = kit_init_db("../initsql");
    return ret;
}

/*********************************************************************
 * @brief       初始化控制器配置
 * @param[in]   void
 * @return      0-成功  1-失败
 *********************************************************************/
int initConConfig(void)
{
    UT_array *itemDevConfigs = {0};
    int ret = 0;
    if (0 != kit_get_config_db_data(&itemDevConfigs))
    {
        printf("huoqushujvkshibai\n");
        return 1;
    }
    utarray_foreach(itemDevConfigs, con_dev_config_t *, p_itemDevConfig)
    {
        switch (p_itemDevConfig->type)
        {
        case kDev_Config_Uart:
            // 系统串口不需要初始化
            break;
        case kDev_Config_Net:
            // 初始化系统的IP、MAC地址、网关等配置
            netInitHandble(p_itemDevConfig);
            break;
        case kDev_Config_Can:
            canInitHandble(p_itemDevConfig);
            break;
        case kDev_Config_DI:
            break;
        case kDev_Config_DO:
            break;
        case kDev_Config_Rtc:
            // 初始化系统时间将rtc时钟同步到linux系统
            systimeInitHandle(p_itemDevConfig);
            break;
        default:
            printf("Unknown device config type: %d\n", p_itemDevConfig->type);
            // ret = 1;
            break;
        }
        if (ret != 0)
        {
            // 初始化失败，释放资源并返回错误
            utarray_free(itemDevConfigs);
            return 1;
        }
    }
    // 释放数组内存
    utarray_free(itemDevConfigs);

#ifdef CMAKE_BUILD_TYPE // 只有正式环境下才能执行
#if CMAKE_BUILD_TYPE == Release
    // gpio口初始化
    ret = drvGpioOpen();
    if (ret != 0)
    {
        return 1;
    }
#endif
#endif

    return ret;
}

/*********************************************************************
 * @brief       SCU初始化
 * @param[in]   arg：相关配置
 * @return      0-成功  1-失败
 *********************************************************************/
uint8_t initConSystem(void *arg)
{
    int ret = 0;

    ret = initSysSemaphore();
    if (ret != 0)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "信号量初始化失败！");
        KITLOG(LOG_APP_EN, ERROR_EN, "信号量初始化失败！");
        return ret;
    }

    ret = initSqliteDb();
    if (ret != 0)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "Sqlite数据库初始化失败！");
        KITLOG(LOG_APP_EN, ERROR_EN, "Sqlite数据库初始化失败！");
        return ret;
    }

    ret = initConConfig();
    if (ret != 0)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "盒子配置初始化失败！");
        KITLOG(LOG_APP_EN, ERROR_EN, "盒子配置初始化失败！");
        return ret;
    }

    ret = initDevInfo();
    if (ret != 0)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "读取设备配置、点位配置信息失败！");
        KITLOG(LOG_APP_EN, ERROR_EN, "读取设备配置、点位配置信息失败！");
        return ret;
    }

    ret = initRtdb(rtdbType, NULL, kEE_SHM_CREAT);
    if (ret != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "RTDB初始化失败！");
        KITPTF(LOG_APP_EN, ERROR_EN, "RTDB初始化失败！");
        return ret;
    }

    ret = initWebSign(kSign_ShMem, kEE_SHM_CREAT);
    if (ret != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "websign初始化失败！");
        KITPTF(LOG_APP_EN, ERROR_EN, "websign初始化失败！");
        return ret;
    }

    return ret;
}


/*********************************************************************
 * @brief       创建TMS CAN任务
 * @param[in]   proto：协议配置
 * @return      void
 * @remark      实际上来说，每个BBMS下只能有一个TMS设备，所以这里只创建一个CAN任务
 *              如果特殊情况，温度控制及协议解析要做变更，当前先按照一个设计处理
 *********************************************************************/
void creatTmsCanTaskEntry(proto_dev_point_map_t* proto)
{
    for (uint16_t i = 0; i < proto->devNum; i++)
    {
        protocol_can_tms_init(100, proto->devPointMapArr[i].devProtocol.canItem.canId);
    }
    creatTmsTempCtrlTaskEntry(proto->devNum);
    creatCanTaskEntry();
}



/*********************************************************************
 * @brief       创建modbus tcp默认转发任务
 * @param[in]   arg：相关配置
 * @return      void
 *********************************************************************/
void creatModbusTcpEmsServer()
{
    protocol_ems_modbus_tcp_init();
    creatModbusTcpStdSlaveTask(NULL);
}


/*********************************************************************
 * @brief       对上位机任务私有协议
 * @param[in]   arg：相关配置
 *********************************************************************/

void creatPrivateHmiTaskEntry(void)
{
    // 建立上位机任务
    registerUplinkCycleTimeCallback(generate_bbms_uplink_info);
    registerUplinkFaultCycleTimeCallback(generate_bbms_fault_uplink_info);
    registerUplinkRecvCallback(bbms_uplink_recv_callback);
    startUplinkTcpTask(NULL, 5002);
}




/*********************************************************************
 * @brief       创建任务的总入口
 * @param[in]   arg：相关配置
 * @return      void
 *********************************************************************/
void regeditThreadEntry(void *map_t) // 创建线程入口
{
    #if 1
    // 采集任务创建
    protocol_type_master_e masteType = kProto_Master_Start;
    for (masteType = kProto_Master_Start; masteType < kProto_Master_End; masteType++)
    {
        switch (masteType)
        {
        case kProto_ModbusRTU_Master:
            // 创建RTU采集任务
            creatUartModbusTaskEntry(&protoTable[kProto_ModbusRTU_Master]);
            break;
        case kProto_ModbusTCP_Master:
            // 创建TCP采集任务
            creatNetModbusTaskEntry(&protoTable[kProto_ModbusTCP_Master]);
            break;
        case kProto_RbmsPrivate_TCP_Master:
            // 注册 RBMS TCP 上行解析回调（须在 creatNetRbmsTaskEntry 之前）
            registerRbmsTcpMasterRecvCall(rbmsParseRecvDataFun);
            creatNetRbmsTaskEntry(&protoTable[kProto_RbmsPrivate_TCP_Master]);
            break;
        case kProto_RbmsPrivate_Can_Master: // RBMS CAN私有协议
            break;
        case kProto_TMSPrivateCan_Master:
            creatTmsCanTaskEntry(&protoTable[kProto_TMSPrivateCan_Master]);
            break;
        case kProto_ModbusTCPMulti_Master:
            // 创建采集modbus tcp multi id任务
            creatNetModbusMultTaskEntry(&protoTable[kProto_ModbusTCPMulti_Master]);

        default:
            break;
        }
    }
    #endif
    // 建立上位机任务
    creatPrivateHmiTaskEntry();

    #if 1
    // ems任务
    //creatModbusTcpEmsServer();

    // dido任务
    creatLogicDiTask();
    creatLogicDoTask();

    // 创建存储类任务
    creatDataStorageTask();

    // 创建簇控管理线程
    startBmsStackControlThread();

    // 创建定时器任务
    creatTimerTask();
    #endif
    return;
}
