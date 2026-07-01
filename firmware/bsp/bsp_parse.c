/*****************************************************************************
 * @copyright       1997-2010,  POWER SUPPLY CO., LTD.
 * @file            bsp_parse.c
 * @brief           解析所有配置文件的入口
 * @author          Gary
 * @remark
 *****************************************************************************/

#include "bsp_parse.h"

// 设备管理数组全局变量
proto_dev_point_map_t protoTable[kProto_Master_End] = {0};

static void parse_protocol_type_TCP(cJSON *json_obj, dev_protocol_u *protocol_u);
static void parse_protocol_type_RTU(cJSON *json_obj, dev_protocol_u *protocol_u);
static dev_protocol_u parse_json(const char protoContent[MAX_CONFIG_CONTENT_LEN],
                                 protocol_type_master_e protocolType);

// 接入控制器的所有设备类型数量
/*// 接入控制器的所有设备类型
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
} dev_type_e; */
uint16_t gStDevTypeNum[kDev_Type_End] = {0};
uint16_t gStDevTypePointNum[kDev_Type_End] = {
    kCon_DataEnd,             // 本机点位数量
    kBbms_Data_End,           // BMS堆控点位数量
    kRbms_Data_End,           // BMS簇控制器点位数量
    kPCS_Data_End,            // PCS点位数量
    kTran_Data_End,           // 变压器点位数量
    kLqd_Data_End,            // 液冷机组点位数量
    kMeter_Data_End,          // 电表点位数量
    kUPS_Data_End,            // UPS点位数量
    kFireDevice_Data_End,     // 消防设备点位数量
    kDehumidifier_Data_End,   // 除湿机点位数量
    kTempSensor_Data_End,     // 温度传感器点位数量
    kHumiditySensor_Data_End, // 湿度传感器点位数量
    kWaterSensor_Data_End,    // 水浸传感器点位数量
    kSmokeSensor_Data_End,    // 烟雾传感器点位数量
    kOtherSensor1_Data_End,   // 其他传感器1点位数量
    kOtherSensor2_Data_End,   // 其他传感器2点位数量
    kOtherSensor3_Data_End,   // 其他传感器3点位数量
    kResv17_Data_End,         // 预留设备类型17点位数量
    kResv17_Data_End,         // 预留设备类型18点位数量
    kResv17_Data_End,         // 预留设备类型19点位数量
    kResv17_Data_End,         // 预留设备类型20点位数量
    kResv17_Data_End,         // 预留设备类型21点位数量
    kResv17_Data_End,         // 预留设备类型22点位数量
    kResv17_Data_End,         // 预留设备类型23点位数量
    kResv17_Data_End,         // 预留设备类型24点位数量
    kResv17_Data_End,         // 预留设备类型25点位数量
    kResv17_Data_End,         // 预留设备类型26点位数量
    kResv17_Data_End,         // 预留设备类型27点位数量
    kResv17_Data_End,         // 预留设备类型28点位数量
    kResv17_Data_End,         // 预留设备类型29点位数量
};

/*********************************************************************
 * @brief       累加设备数量
 * @param[in]   typeNo: 协议类型
 * @return      devNum：设备数量
 *********************************************************************/
void addDevType_Num(dev_type_e typeNo)
{
    if (typeNo >= kDev_Type_End)
    {
        return;
    }
    gStDevTypeNum[typeNo]++;
}

/*********************************************************************
 * @brief       读取系统配置的设备数量
 * @return      0-成功  1-失败
 *********************************************************************/
int initDevInfo()
{
    UT_array *devPointInfo = NULL;
    // 获取数据库ID、设备类型、设备编号、协议类型
    if (0 != kit_get_dev_db_data(&devPointInfo))
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "kit_get_dev_db_data 执行失败！");
        return 1;
    }

    // 遍历设备
    utarray_foreach(devPointInfo, dev_info_t *, p_dev)
    {
        UT_array *points = NULL;
        char protoContent[MAX_CONFIG_CONTENT_LEN];
        // 通过设备数据库Id获取到对应协议配置内容
        if (0 != kit_get_protocol_db_data(kTemplate_Type_Device, p_dev->devDbId, protoContent))
        {
            printf("load protoContent fail\n");
            if (devPointInfo != NULL)
            {
                utarray_free(devPointInfo); // 释放已分配的内存
                devPointInfo = NULL;
            }
            return 1;
        }
        // 解析协议配置内容
        p_dev->devProtocol = parse_json(protoContent, p_dev->protocolType);
        // 通过设备使用的模板Id获取到对应设备的所有点位
        if (0 != kit_get_point_db_data(p_dev->templateId, &points))
        {
            printf("load protoContent fail");
            if (devPointInfo != NULL)
            {
                utarray_free(devPointInfo); // 释放已分配的内存
                devPointInfo = NULL;
            }
            return 1;
        }
        // 获取点位数量
        p_dev->pointNum = utarray_len(points);

        if (p_dev->pointNum <= 0)
        {
            if (points != NULL)
            {
                utarray_free(points); // 释放已分配的内存
                points = NULL;
            }
            continue;
        }

        p_dev->pointArr = calloc(p_dev->pointNum, sizeof(point_t));
        if (p_dev->pointArr == NULL)
        {
            printf("Memory allocation for pointArr failed\n");
            if (points != NULL)
            {
                utarray_free(points); // 释放已分配的内存
                points = NULL;
            }
            if (devPointInfo != NULL)
            {
                utarray_free(devPointInfo); // 释放已分配的内存
                devPointInfo = NULL;
            }
            return 1;
        }
        int i = 0;
        // 遍历点位
        utarray_foreach(points, point_t *, tag)
        {
            if (i < p_dev->pointNum)
            {
                p_dev->pointArr[i] = *tag; // 使用结构体直接赋值
                // strncpy((char *)p_dev->pointArr[i].pointName, (char *)tag->pointName, MAX_POINT_NAME_LEN);
                i++;
            }
        }

        if (points != NULL)
        {
            utarray_free(points); // 释放已分配的内存
            points = NULL;
        }

        // 将设备信息存储到 protoTable 中
        protocol_type_master_e protocolType = p_dev->protocolType;
        proto_dev_point_map_t *protoEntry = &protoTable[protocolType];

        // 检查并动态扩展设备数组
        if (protoEntry->devNum == 0)
        {
            protoEntry->devPointMapArr = (dev_info_t *)calloc(1, sizeof(dev_info_t));
            if (protoEntry->devPointMapArr == NULL)
            {
                printf("Memory allocation failed for devPointMapArr\n");
                if (devPointInfo != NULL)
                {
                    utarray_free(devPointInfo); // 释放已分配的内存
                    devPointInfo = NULL;
                }
                return 1;
            }
        }
        else
        {
            // 动态扩展设备数组的大小以存储新设备
            dev_info_t *newArr = (dev_info_t *)realloc(
                protoEntry->devPointMapArr, (protoEntry->devNum + 1) * sizeof(dev_info_t));
            if (newArr == NULL)
            {
                printf("Memory allocation failed during realloc");
                if (devPointInfo != NULL)
                {
                    utarray_free(devPointInfo); // 释放已分配的内存
                    devPointInfo = NULL;
                }
                return 1;
            }
            protoEntry->devPointMapArr = newArr;
        }

        // 防止配置端并没有配置任何的测点也导致设备加1
        if (p_dev->pointNum > 0)
        {
            addDevType_Num(p_dev->devType);
            // 将设备信息复制到协议设备数组
            memcpy(&protoEntry->devPointMapArr[protoEntry->devNum], p_dev, sizeof(dev_info_t));
            protoEntry->devNum++;
        }
    }

    if (devPointInfo != NULL)
    {
        utarray_free(devPointInfo); // 释放已分配的内存
        devPointInfo = NULL;
    }
    return 0;
}

/*********************************************************************
 * @brief       读取高级设置的信息
 * @return      0-成功  1-失败
 *********************************************************************/
int initAdvancedSettingInfo()
{
    UT_array *advancedSettings = NULL;
    // 获取高级设置项

    return 0;
}

/*********************************************************************
 * @brief       解析协议类型 TCP 的 JSON 字符串，并将解析结果赋值给联合体
 * @param[in]   json_obj: JSON 对象
 * @param[out]  protocol_u: 联合体指针，存储解析结果
 * @return      none
 *********************************************************************/
static void parse_protocol_type_TCP(cJSON *json_obj, dev_protocol_u *protocol_u)
{
    cJSON *host = cJSON_GetObjectItem(json_obj, "ip");
    cJSON *port = cJSON_GetObjectItem(json_obj, "port");
    cJSON *netId = cJSON_GetObjectItem(json_obj, "netId");
    cJSON *timeout = cJSON_GetObjectItem(json_obj, "timeout");
    cJSON *uId = cJSON_GetObjectItem(json_obj, "uId");
    if (host && cJSON_IsString(host))
    {
        // 赋值给联合体的 tcpClientLib 成员的 host 字段
        strcpy((char *)protocol_u->tcpClientLib.ip, host->valuestring);
    }
    if (port && cJSON_IsNumber(port))
    {
        // 赋值给联合体的 tcpClientLib 成员的 port 字段
        protocol_u->tcpClientLib.port = port->valueint;
    }
    if (netId && cJSON_IsNumber(netId))
    {
        // 赋值给联合体的 tcpClientLib 成员的 port 字段
        protocol_u->tcpClientLib.netId = netId->valueint;
    }
    if (timeout && cJSON_IsNumber(timeout))
    {
        // 赋值给联合体的 tcpClientLib 成员的 timeout 字段
        protocol_u->tcpClientLib.timeout = timeout->valueint;
    }
    if (uId && cJSON_IsNumber(uId))
    {
        protocol_u->tcpClientLib.uId = uId->valueint;
    }
}

/*********************************************************************
 * @brief       解析协议类型 TCP 的 JSON 字符串，并将解析结果赋值给联合体
 * @param[in]   json_obj: JSON 对象
 * @param[out]  protocol_u: 联合体指针，存储解析结果
 * @return      none
 *********************************************************************/
static void parse_protocol_Rbms_TCP(cJSON *json_obj, dev_protocol_u *protocol_u)
{
    cJSON *host = cJSON_GetObjectItem(json_obj, "ip");
    cJSON *port = cJSON_GetObjectItem(json_obj, "port");
    cJSON *netId = cJSON_GetObjectItem(json_obj, "netId");
    cJSON *timeout = cJSON_GetObjectItem(json_obj, "timeout");
    cJSON *uId = cJSON_GetObjectItem(json_obj, "uId");
    if (host && cJSON_IsString(host))
    {
        // 赋值给联合体的 tcpRbmsItem 成员的 host 字段
        strcpy((char *)protocol_u->tcpRbmsItem.ip, host->valuestring);
    }
    if (port && cJSON_IsNumber(port))
    {
        // 赋值给联合体的 tcpRbmsItem 成员的 port 字段
        protocol_u->tcpRbmsItem.port = port->valueint;
    }
    if (netId && cJSON_IsNumber(netId))
    {
        // 赋值给联合体的 tcpRbmsItem 成员的 port 字段
        protocol_u->tcpRbmsItem.netId = netId->valueint;
    }
    if (timeout && cJSON_IsNumber(timeout))
    {
        // 赋值给联合体的 tcpRbmsItem 成员的 timeout 字段
        protocol_u->tcpRbmsItem.timeout = timeout->valueint;
    }
    if (uId && cJSON_IsNumber(uId))
    {
        protocol_u->tcpRbmsItem.uId = uId->valueint;
    }
}

/*********************************************************************
 * @brief       解析协议类型 RTU 的 JSON 字符串，并将解析结果赋值给联合体
 * @param[in]   json_obj: JSON 对象
 * @param[out]  protocol_u: 联合体指针，存储解析结果
 * @return      none
 *********************************************************************/
static void parse_protocol_type_RTU(cJSON *json_obj, dev_protocol_u *protocol_u)
{
    cJSON *address = cJSON_GetObjectItem(json_obj, "address");
    cJSON *timeout = cJSON_GetObjectItem(json_obj, "timeout");
    cJSON *stop = cJSON_GetObjectItem(json_obj, "stop");
    cJSON *parity = cJSON_GetObjectItem(json_obj, "parity");
    cJSON *baud = cJSON_GetObjectItem(json_obj, "baud");
    cJSON *data = cJSON_GetObjectItem(json_obj, "data");
    cJSON *uartId = cJSON_GetObjectItem(json_obj, "uartId");
    cJSON *uId = cJSON_GetObjectItem(json_obj, "uId");

    if (address && cJSON_IsString(address))
    {
        // 赋值给联合体的 uartLib 成员的 address 字段
        strncpy((char *)protocol_u->uartLib.address, address->valuestring, MAX_ADDR_LEN - 1);
        protocol_u->uartLib.address[MAX_ADDR_LEN - 1] = '\0'; // 确保以 null 结尾
    }
    if (uId && cJSON_IsNumber(uId))
    {
        // 赋值给联合体的 uartLib 成员的 timeout 字段
        protocol_u->uartLib.uId = uId->valueint;
    }
    if (timeout && cJSON_IsNumber(timeout))
    {
        // 赋值给联合体的 uartLib 成员的 timeout 字段
        protocol_u->uartLib.timeout = timeout->valueint;
    }
    if (stop && cJSON_IsNumber(stop))
    {
        // 赋值给联合体的 uartLib 成员的 stop 字段
        protocol_u->uartLib.stop = stop->valueint;
    }
    if (parity && cJSON_IsNumber(parity))
    {
        // 赋值给联合体的 uartLib 成员的 parity 字段
        protocol_u->uartLib.parity = parity->valueint;
    }
    if (baud && cJSON_IsNumber(baud))
    {
        // 赋值给联合体的 uartLib 成员的 baud 字段
        protocol_u->uartLib.baud = baud->valueint;
    }
    if (data && cJSON_IsNumber(data))
    {
        // 赋值给联合体的 uartLib 成员的 data 字段
        protocol_u->uartLib.data = data->valueint;
    }
    if (uartId && cJSON_IsNumber(uartId))
    {
        // 赋值给联合体的 uartLib 成员的 串口号 字段
        protocol_u->uartLib.uartId = uartId->valueint;
    }
}

/*********************************************************************
 * @brief       解析协议类型 CAN 的 JSON 字符串，并将解析结果赋值给联合体
 * @param[in]   json_obj: JSON 对象
 * @param[out]  protocol_u: 联合体指针，存储解析结果
 * @return      none
 *********************************************************************/
static void parse_protocol_type_can(cJSON *json_obj, dev_protocol_u *protocol_u)
{

    cJSON *canid = cJSON_GetObjectItem(json_obj, "canid");
    cJSON *uId = cJSON_GetObjectItem(json_obj, "uId");
    cJSON *baud = cJSON_GetObjectItem(json_obj, "baud");
    cJSON *timeout = cJSON_GetObjectItem(json_obj, "timeout");

    if (uId && cJSON_IsNumber(uId))
    {
        // 赋值给联合体的 uartLib 成员的 timeout 字段
        protocol_u->canItem.uId = uId->valueint;
    }
    if (baud && cJSON_IsNumber(baud))
    {
        // 赋值给联合体的 uartLib 成员的 timeout 字段
        protocol_u->canItem.bitrate = baud->valueint;
    }

    if (timeout && cJSON_IsNumber(timeout))
    {
        // 赋值给联合体的 uartLib 成员的 timeout 字段
        protocol_u->canItem.timeout = timeout->valueint;
    }
    if (canid && cJSON_IsNumber(canid))
    {
        protocol_u->canItem.canId = (uint8_t)canid->valueint;
    }
    else
    {
        cJSON *canIdUpper = cJSON_GetObjectItem(json_obj, "canId");
        if (canIdUpper && cJSON_IsNumber(canIdUpper))
        {
            protocol_u->canItem.canId = (uint8_t)canIdUpper->valueint;
        }
    }
}

/*********************************************************************
 * @brief       解析协议类型 RBMS CAN 的 JSON 字符串，并将解析结果赋值给联合体
 * @param[in]   json_obj: JSON 对象
 * @param[out]  protocol_u: 联合体指针，存储解析结果
 * @return      none
 *********************************************************************/
static void parse_protocol_rbms_can(cJSON *json_obj, dev_protocol_u *protocol_u)
{

    cJSON *canid = cJSON_GetObjectItem(json_obj, "canid");
    cJSON *uId = cJSON_GetObjectItem(json_obj, "uId");
    cJSON *baud = cJSON_GetObjectItem(json_obj, "baud");
    cJSON *timeout = cJSON_GetObjectItem(json_obj, "timeout");

    if (uId && cJSON_IsNumber(uId))
    {
        protocol_u->canRbmsItem.uId = uId->valueint;
    }
    if (baud && cJSON_IsNumber(baud))
    {
        protocol_u->canRbmsItem.bitrate = baud->valueint;
    }

    if (timeout && cJSON_IsNumber(timeout))
    {
        protocol_u->canRbmsItem.timeout = timeout->valueint;
    }
    if (canid && cJSON_IsNumber(canid))
    {
        protocol_u->canRbmsItem.canId = canid->valueint;
    }
}

/*********************************************************************
 * @brief       解析 JSON 字符串
 * @param[in]   protoContent: JSON 字符串
 * @param[in]   protocolType: 协议类型
 * @return      none
 *********************************************************************/
static dev_protocol_u parse_json(const char protoContent[MAX_CONFIG_CONTENT_LEN],
                                 protocol_type_master_e protocolType)
{
    // 解析 JSON 字符串
    cJSON *json_obj = cJSON_Parse(protoContent);
    if (json_obj == NULL)
    {
        fprintf(stderr, "Error parsing JSON\n");
        dev_protocol_u emptyProtocol = {0}; // 如果解析失败，返回一个空的联合体
        return emptyProtocol;
    }

    dev_protocol_u tempProtocol_u = {0};

    switch (protocolType)
    {
    case kProto_ModbusTCP_Master:
    case kProto_ModbusTCPMulti_Master:
        parse_protocol_type_TCP(json_obj, &tempProtocol_u);
        break;
    case kProto_ModbusRTU_Master:
        parse_protocol_type_RTU(json_obj, &tempProtocol_u);
        break;

    // RBMS TCP私有协议
    case kProto_RbmsPrivate_TCP_Master:
        parse_protocol_Rbms_TCP(json_obj, &tempProtocol_u);
        break;

    // RBMS CAN私有协议
    case kProto_RbmsPrivate_Can_Master:
        parse_protocol_rbms_can(json_obj, &tempProtocol_u);
        break;

    case kProto_TMSPrivateCan_Master:
        parse_protocol_type_can(json_obj, &tempProtocol_u);
        break;

    default:
        fprintf(stderr, "Unsupported protocol type: %d\n", protocolType);
        break;
    }

    // 释放 JSON 解析对象
    cJSON_Delete(json_obj);
    return tempProtocol_u; // 返回解析后的联合体
}
/*********************************************************************
 * @brief       用指定内容替换源字符串中的指定位置
 * @param[in]   source: 源字符串
 * @param[in]   replacement: 需要写入源字符串的字符串
 * @param[in]   wordIndex: 被分隔符分割的子串索引，例如a/b/c/d,abcd的索引依次为1234
 * @param[in]   divide: 分隔符
 * @return      修改后的字符串
 *********************************************************************/
char *replaceSubStrByIndexAndTag(const char *source, const char *replacement, int wordIndex,
                                 char divide)
{
    if (wordIndex <= 0)
    {
        // 如果wordIndex无效，返回source的副本
        size_t source_len = strlen(source);
        char *result = (char *)malloc(source_len + 1);
        if (result)
        {
            strcpy(result, source);
        }
        return result;
    }

    const char *start_position = source, *end_position = NULL;
    int current_word = 0;

    // 定位要替换部分的起始和结束位置
    for (const char *p = source; *p != '\0'; p++)
    {
        if (*p == divide)
        {
            current_word++;
            if (current_word == wordIndex - 1)
            {
                start_position = p + 1;
            }
            else if (current_word == wordIndex)
            {
                end_position = p;
                break;
            }
        }
    }

    // 如果在字符串中找不到足够多的分隔符
    if (current_word < wordIndex)
    {
        end_position = source + strlen(source);
    }

    // 计算新字符串的长度
    size_t new_length = (start_position - source) + strlen(replacement) + strlen(end_position);

    // 为新字符串分配内存
    char *result = (char *)malloc(new_length + 1);
    if (!result)
    {
        return NULL; // 处理内存分配错误
    }

    // 复制第一部分
    strncpy(result, source, start_position - source);
    result[start_position - source] = '\0'; // null终止新字符串

    // 连接替换字符串
    strcat(result, replacement);

    // 连接第二部分
    strcat(result, end_position);

    return result;
}

/*********************************************************************
 * @brief       根据设备类型和设备ID获取设备的点位数组
 * @param[in]   devType 设备类型
 * @param[in]   devId 设备ID
 * @return      返回匹配设备的点位数组，未找到返回 NULL
 *********************************************************************/
point_t *get_pointArr(dev_type_e devType, uint16_t devId)
{
    for (int i = 0; i < kProto_Master_End; i++)
    {
        if (protoTable[i].devPointMapArr == NULL)
        {
            continue; // 跳过空协议
        }
        for (int j = 0; j < protoTable[i].devNum; j++)
        {
            if (protoTable[i].devPointMapArr[j].devType == devType &&
                protoTable[i].devPointMapArr[j].devId == devId)
            {
                return protoTable[i].devPointMapArr[j].pointArr; // 返回匹配设备的 pointArr
            }
        }
    }
    return NULL; // 未找到匹配设备，返回 NULL
}

/*********************************************************************
 * @brief       根据设备类型和设备ID获取设备的点位数组
 * @param[in]   devType 设备类型
 * @param[in]   devId 设备ID
 * @return      返回匹配设备的点位数组，未找到返回 NULL
 *********************************************************************/
dev_info_t *get_devPointMapArr(dev_type_e devType, uint16_t devArrayId)
{
    for (int i = 0; i < kProto_Master_End; i++)
    {
        if (protoTable[i].devPointMapArr == NULL)
        {
            continue; // 跳过空协议
        }
        for (int j = 0; j < protoTable[i].devNum; j++)
        {
            if (protoTable[i].devPointMapArr[j].devType == devType &&
                (protoTable[i].devPointMapArr[j].devId == devArrayId + 1))
            {
                return &protoTable[i].devPointMapArr[j]; // 返回匹配设备的 pointArr
            }
        }
    }
    return NULL; // 未找到匹配设备，返回 NULL
}