/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2026, gary. All rights reserved.
 * @file            bsp_redis.c
 * @brief           xx功能
 * @author          Gary
 * @date            2026/02/01
 * @remark
 *****************************************************************************/
#include "bsp_redis.h"

#if 0
// Redis 连接对象
redisContext *redis_conn = NULL;

// 与点位配置的结构体保持一致
const char *fieldNames[kPoint_End] = {
    "pointDbId", "pointId", "storageInterval", "devDbId", "devId",
    "regAddr", "func", "groupType", "devType", "dataType",
    "attribute", "endian", "bitPos", "bitNum", "isPersisted",
    "precision", "ratio", "offset", "mutateBound", "value",
    "minValue", "maxValue", "defaultValue", "pointName"};

// 初始化 Redis 数据库
int initRtdbRedis(void *arg, shm_creat_type_e shmType)
{
    // 连接 Redis
    redis_conn = redisConnect(REDIS_SERVER_IP, REDIS_SERVER_PORT);
    if (redis_conn == NULL || redis_conn->err)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "Redis 连接错误: %s", redis_conn->errstr);
        KITLOG(LOG_APP_EN, ERROR_EN, "Redis 连接错误: %s", redis_conn->errstr);
        return RTDB_ERROR;
    }
    KITPTF(LOG_APP_EN, INFO_EN, "成功连接到 Redis 服务器");
    KITLOG(LOG_APP_EN, INFO_EN, "成功连接到 Redis 服务器");

    // 清空当前 Redis 数据库
    redisReply *reply = redisCommand(redis_conn, "FLUSHDB");
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "清空 Redis 数据库失败: %s", reply ? reply->str : "无法执行命令");
        KITLOG(LOG_APP_EN, ERROR_EN, "清空 Redis 数据库失败: %s", reply ? reply->str : "无法执行命令");
        if (reply)
            freeReplyObject(reply);
        redisFree(redis_conn);
        return RTDB_ERROR;
    }
    KITPTF(LOG_APP_EN, INFO_EN, "成功清空 Redis 数据库");
    KITLOG(LOG_APP_EN, INFO_EN, "成功清空 Redis 数据库");

    // 初始化 Redis 数据库
    reply = redisCommand(redis_conn, "SET devTypeNum %d", kDev_Type_End);
    if (reply)
    {
        freeReplyObject(reply);
        usleep(1000);
    }

    int offset = kP_pointDbId;
    // 设置设备类型的设备数量
    for (int devType = 0; devType < kDev_Type_End; devType++)
    {
        printf(" 111111111 devType: %d, devNum: %d, pointNum: %d \r\n", devType, gStDevTypeNum[devType], gStDevTypePointNum[devType]);

        gStDevTypeNum[devType] = (gStDevTypeNum[devType] == 0 ? 1 : gStDevTypeNum[devType]);
        reply = redisCommand(redis_conn, "SET devType:%d:devNum %d", devType, gStDevTypeNum[devType]);
        if (reply)
        {
            freeReplyObject(reply);
            usleep(1000);
        }
        // 设置设备点位数量
        for (int devId = 0; devId < gStDevTypeNum[devType]; devId++)
        {
            // 获取解析的数据
            point_t *src = get_pointArr(devType, devId);
            if (src != NULL)
            {
                // 初始化每个点位
                for (int pointId = 0; pointId < gStDevTypePointNum[devType]; pointId++)
                {

                    // 获取每个点位数据
                    point_t *point = &src[pointId];
                    offset = 0;

                    // 使用带有 pointId 的 Redis 键
                    // pointDbId
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %u", devType, devId, pointId, fieldNames[offset++], point->pointDbId);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // pointId
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %u", devType, devId, pointId, fieldNames[offset++], point->pointId);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // storageInterval
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %d", devType, devId, pointId, fieldNames[offset++], point->storageInterval);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // devDbId
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %u", devType, devId, pointId, fieldNames[offset++], point->devDbId);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // devId
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %u", devType, devId, pointId, fieldNames[offset++], point->devId);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // regAddr
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %u", devType, devId, pointId, fieldNames[offset++], point->regAddr);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // func
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %d", devType, devId, pointId, fieldNames[offset++], point->func);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // groupType
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %d", devType, devId, pointId, fieldNames[offset++], point->groupType);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // devType
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %d", devType, devId, pointId, fieldNames[offset++], point->devType);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // dataType
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %d", devType, devId, pointId, fieldNames[offset++], point->dataType);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // attribute
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %d", devType, devId, pointId, fieldNames[offset++], point->attribute);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // endian
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %d", devType, devId, pointId, fieldNames[offset++], point->endian);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // bitPos
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %u", devType, devId, pointId, fieldNames[offset++], point->bitPos);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // bitNum
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %u", devType, devId, pointId, fieldNames[offset++], point->bitNum);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // isPersisted
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %u", devType, devId, pointId, fieldNames[offset++], point->isPersisted);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // precision
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %f", devType, devId, pointId, fieldNames[offset++], point->precision);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // ratio
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %f", devType, devId, pointId, fieldNames[offset++], point->ratio);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // offset++
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %f", devType, devId, pointId, fieldNames[offset++], point->offset);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // mutateBound
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %f", devType, devId, pointId, fieldNames[offset++], point->mutateBound);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // value
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %f", devType, devId, pointId, fieldNames[offset++], point->value);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // minValue
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %f", devType, devId, pointId, fieldNames[offset++], point->minValue);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // maxValue
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %f", devType, devId, pointId, fieldNames[offset++], point->maxValue);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // defaultValue
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %f", devType, devId, pointId, fieldNames[offset++], point->defaultValue);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // pointName
                    reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %s", devType, devId, pointId, fieldNames[offset++], point->pointName);
                    if (reply)
                        freeReplyObject(reply);
                    usleep(1000);

                    // 打印日志
                    KITPTF(LOG_APP_EN, DEBUG_EN, "设备类型 %d 设备 ID %d 点位 ID %d 的数据已初始化", devType, devId, pointId);
                    KITLOG(LOG_APP_EN, DEBUG_EN, "设备类型 %d 设备 ID %d 点位 ID %d 的数据已初始化", devType, devId, pointId);
                }
            }

            KITPTF(LOG_APP_EN, DEBUG_EN, "设备类型 %d 设备 ID %d 的点位数量已设置为 %d", devType, devId, gStDevTypePointNum[devType]);
            KITLOG(LOG_APP_EN, DEBUG_EN, "设备类型 %d 设备 ID %d 的点位数量已设置为 %d", devType, devId, gStDevTypePointNum[devType]);
        }
        KITPTF(LOG_APP_EN, DEBUG_EN, "设备类型 %d 的设备数量已设置为 %d", devType, (gStDevTypeNum[devType] == 0 ? 1 : gStDevTypeNum[devType]));
        KITLOG(LOG_APP_EN, DEBUG_EN, "设备类型 %d 的设备数量已设置为 %d", devType, (gStDevTypeNum[devType] == 0 ? 1 : gStDevTypeNum[devType]));
    }

    // 关闭 Redis 连接
    // redisFree(redis_conn);
    // KITPTF(LOG_APP_EN, INFO_EN, "Redis 连接已关闭");
    // KITLOG(LOG_APP_EN, INFO_EN, "Redis 连接已关闭");

    return RTDB_OK;
}

double getRedisPointValue(uint16_t devType, uint16_t devId, uint16_t pointId, point_e pointField)
{
    if (pointField >= kPoint_End)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "无效的点位字段: %d", pointField);
        return 0.0; // 默认值
    }

    // 修改 Redis 键，加入 pointId
    redisReply *reply = redisCommand(redis_conn, "HGET devType:%d:devId:%d:pointId:%d %s", devType, devId, pointId, fieldNames[pointField]);

    if (reply == NULL || reply->str == NULL || reply->type == REDIS_REPLY_NIL)
    {
        KITPTF(LOG_APP_EN, INFO_EN, "获取设备类型：%d，设备号：%d，点位ID：%d，字段：%s，不存在", devType, devId, pointId, fieldNames[pointField]);
        KITLOG(LOG_APP_EN, INFO_EN, "获取设备类型：%d，设备号：%d，点位ID：%d，字段：%s，不存在", devType, devId, pointId, fieldNames[pointField]);
        if (reply)
            freeReplyObject(reply);
        return 0.0; // 默认值
    }

    double value = atof(reply->str);
    freeReplyObject(reply);
    return value;
}

void setRedisPointValue(uint16_t devType, uint16_t devId, uint16_t pointId, point_e pointField, double value)
{
    if (pointField >= kPoint_End)
    {
        KITPTF(LOG_APP_EN, ERROR_EN, "无效的点位字段: %d", pointField);
        return;
    }

    // 修改 Redis 键，加入 pointId
    redisReply *reply = redisCommand(redis_conn, "HSET devType:%d:devId:%d:pointId:%d %s %f", devType, devId, pointId, fieldNames[pointField], value);

    if (reply == NULL || reply->type == REDIS_REPLY_ERROR)
    {
        KITPTF(LOG_APP_EN, INFO_EN, "写入设备类型：%d，设备号：%d，点位ID：%d，字段：%s，失败", devType, devId, pointId, fieldNames[pointField]);
        KITLOG(LOG_APP_EN, INFO_EN, "写入设备类型：%d，设备号：%d，点位ID：%d，字段：%s，失败", devType, devId, pointId, fieldNames[pointField]);
    }

    if (reply)
        freeReplyObject(reply);
}
#endif