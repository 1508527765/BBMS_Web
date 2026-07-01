/*****************************************************************************
 * @copyright       2024-202,POWER SUPPLY CO., LTD.
 * @file            bsp_rtdb.h
 * @brief           rtdb组件
 * @author          GQ
 * @date            2025/08/30
 * @remark          初修订
 *****************************************************************************/
#include "bsp_rtdb.h"

rtdb_type_e rtdbType = Rtdb_ShMem;

uint8_t initWebSign(sign_share_type_e type, shm_creat_type_e rwType)
{
    if (type < kSign_Share_End && type >= kSign_Share_Start)
    {
        uint16_t ret = RTDB_ERROR;
        switch (type)
        {
        case Rtdb_ShMem:
            ret = initWebSignShm(rwType);
            break;
        case Rtdb_Redis:
            /*预留*/
            break;
        default:
            break;
        }
        return ret;
    }
    else
    {
        return RTDB_ERROR;
    }
}

/*****************************************************************************
 * @brief       读取web与c写配置变化的通知组件
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   signType: 通知的地址
 * @return
 *****************************************************************************/
uint16_t  readWebSign(sign_share_type_e type, cfg_web_sign_e signType)
{
    if (type < kSign_Share_End && type >= kSign_Share_Start)
    {
        uint16_t ret = RTDB_ERROR;
        switch (type)
        {
        case Rtdb_ShMem:
            ret = readWebSignShm(signType);
            break;
        case Rtdb_Redis:
            /*预留*/
            break;
        default:
            break;
        }
        return ret;
    }
    else
    {
        return RTDB_ERROR;
    }
}

/*****************************************************************************
 * @brief       写入web与c写配置变化的通知组件
 * @param[in]   type: 实时库的类型
 * @param[in]   signType: 通知的地址
 * @param[in]   val: 对应的值
 * @return      0-成功  1失败
 *****************************************************************************/
void writeWebSign(sign_share_type_e type, cfg_web_sign_e signType, uint16_t val)
{
    if (type < kSign_Share_End && type >= kSign_Share_Start)
    {
        switch (type)
        {
        case Rtdb_ShMem:
            writeWebSignShm(signType, val);
            break;
        case Rtdb_Redis:
            /*预留*/
            break;
        default:
            break;
        }
    }
}

/*****************************************************************************
 * @brief       初始化rtdb
 * @param[in]   type: 实时库的类型
 * @param[in]   arg: 所有测点属性的指针
 * @return      0-成功  1失败
 *****************************************************************************/
uint8_t initRtdb(rtdb_type_e type, void *arg, shm_creat_type_e shmType)
{
    if (type < Rtdb_End && type >= 0)
    {
        uint16_t ret = RTDB_ERROR;
        switch (type)
        {
        case Rtdb_ShMem:
            ret = initRtdbShm(arg, shmType);
            break;
        case Rtdb_Redis:
            //ret = initRtdbRedis(arg, shmType);
            break;
        default:
            break;
        }
        return ret;
    }
    else
    {
        return RTDB_ERROR;
    }
}

/*****************************************************************************
 * @brief       获取RTDB中的值
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   devType: 设备类型
 * @param[in]   devId: 设备序号 从0开始
 * @param[in]   pointId: 该设备中点号，从0开始
 * @return      获取值
 *****************************************************************************/
double getRtdbPointValue(rtdb_type_e type, uint16_t devType, uint16_t devId, uint16_t pointId)
{
    if (type < Rtdb_End && type >= 0)
    {
        switch (type)
        {
        case Rtdb_ShMem:
            return getShmPointValue(devType, devId, pointId);
            break;
        case Rtdb_Redis:
            //return getRedisPointValue(devType, devId, pointId, kP_value);
            break;
        default:
            break;
        }
    }
    else
    {
        KITPTF(LOG_APP_EN, INFO_EN, "实时库类型：%d 不存在", type);
        KITLOG(LOG_APP_EN, INFO_EN, "实时库类型：%d 不存在", type);
        return 0;
    }
    return 0;
}

/*****************************************************************************
 * @brief       向RTDB中写入值
 * @param[in]   type: 实时库的类型 默认选择Rtdb_ShMem
 * @param[in]   devType: 设备类型
 * @param[in]   devId: 设备序号 从0开始
 * @param[in]   pointId: 该设备中点号，从0开始
 * @param[in]   value: 向RTDB中写入的值
 *****************************************************************************/
void setRtdbPointValue(rtdb_type_e type, uint16_t devType, uint16_t devId, uint16_t pointId, double value)
{
    if (type < Rtdb_End && type >= 0)
    {
        switch (type)
        {
        case Rtdb_ShMem:
            setShmPointValue(devType, devId, pointId, value);
            break;
        case Rtdb_Redis:
            //setRedisPointValue(devType, devId, pointId, kP_value, value);
            break;
        default:
            break;
        }
    }
    else
    {
        KITPTF(LOG_APP_EN, INFO_EN, "实时库类型：%d 不存在", type);
        KITLOG(LOG_APP_EN, INFO_EN, "实时库类型：%d 不存在", type);
    }
}
