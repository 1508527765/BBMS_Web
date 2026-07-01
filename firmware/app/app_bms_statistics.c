/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, Gary. All rights reserved.
 * @file            app_bms_statistics.c
 * @brief           BMS数据统计
 * @author          gary
 * @date            2026-03-23
 * @remark
 *****************************************************************************/

#include "app_bms_statistics.h"
bms_bank_oper_status_t bms_bank_oper_status = {0}; // 全局BMS电池包运行状态变量
bms_soc_config_t bms_soc_config = {0};
bms_soh_config_t bms_soh_config = {0};

/*****************************************************************************
 * @brief       获取rbms数量
 *****************************************************************************/
uint16_t get_rbms_num(void)
{
    // 从RTDB获取RBMS数量的示例
    uint16_t rbms_count = 0;
    rbms_count = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_Con, 0, kCon_RBMSNum);
    return rbms_count;
}


bool get_rbms_online(uint16_t rbms_idx)
{
    if(rbms_idx >= get_rbms_num())
    {
        return false;
    }
    // 从RTDB获取RBMS是否在线的示例
    bool rbms_online = false;
    rbms_online = (bool)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rbms_idx, kRbms_Online_State);
    return rbms_online;
}

uint8_t bms_bank_rbms_fault_or(uint16_t fault_id)
{
    uint16_t rbms_num = get_rbms_num();
    uint16_t i;

    if (fault_id >= kRbmsFault_End)
    {
        return 0;
    }

    for (i = 0; i < rbms_num; i++)
    {
        if (!get_rbms_online(i))
        {
            continue;
        }

        if (get_rbms_fault_bit((uint8_t)i, (rbms_fault_type)fault_id) != 0)
        {
            return 1;
        }
    }

    return 0;
}

static uint16_t get_rbms_soc(uint16_t rbms_idx)
{
    // 从RTDB获取RBMS SOC值的示例
    uint16_t rbms_soc = 0;
    rbms_soc = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rbms_idx, kRbms_SoC);
    return rbms_soc;
}


static uint16_t get_rbms_soh(uint16_t rbms_idx)
{
    // 从RTDB获取RBMS SOH值的示例
    uint16_t rbms_soh = 0;
    rbms_soh = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rbms_idx, kRbms_SoH);
    return rbms_soh;
}

/*****************************************************************************
 * @brief       更新SOC计算数据
 *****************************************************************************/
void bms_update_soc_calc_data(bms_soc_config_t *config)
{
    if (config == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "bms_update_soc_calc_data: config is NULL");
        return;
    }
    config->rbms_count = get_rbms_num();

    for(uint16_t i = 0; i < config->rbms_count; i++)
    {
        config->rbms_soc[i] = get_rbms_soc(i);
    }
    config->soc_lower_limit = 3000;
    config->soc_upper_limit = 9900;
    config->soc_formula_numerator = 10000;
}

/*****************************************************************************
 * @brief       更新SOH计算数据
 *****************************************************************************/
void bms_update_soh_calc_data(bms_soh_config_t *config)
{
    if (config == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "bms_update_soh_calc_data: config is NULL");
        return;
    }
    config->rbms_count = get_rbms_num();

    for(uint16_t i = 0; i < config->rbms_count; i++)
    {
        config->rbms_soh[i] = get_rbms_soh(i);
    }
}

/**************************************************************************
 * @brief           计算SOC统计结果
    @param[in]       config: 包含RBMS数量、每个RBMS的SOC值、SOC上下界、公式分子系数、电池类型和温度补偿使能等配置参数
    @return          计算得到的SOC值（百分比×100）
    @details:
                    **磷酸铁锂电池（LFP）**：
                    - 下界阈值：30%（LFP 在低 SOC 时放电能力强，但仍需留余度）
                    - 上界阈值：99%（LFP 过充耐受性较强，但为安全考虑仍保留缓冲）
                    - 特性：循环寿命长（> 6000 次），高温性能好，可工作范围相对宽（10%-95%）
                    - 应用：长期储能、频繁循环应用

                    **三元锂电池（NCM）**：
                    - 下界阈值：35%（NCM 在低 SOC 时输出特性下降明显，保留更大裕度）
                    - 上界阈值：98%（NCM 对过充更敏感，上界更严格）
                    - 特性：能量密度高，低温性能好，循环寿命短于 LFP（< 2000 次）
                    - 应用：短期储能、快速响应应用
*************************************************************************/
uint16_t bms_soc_statistics(bms_soc_config_t *config)
{
    if (config == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "bms_soc_statistics: config is NULL");
        return 0;
    }
    if (config->rbms_count == 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "bms_soc_statistics: rbms_count is 0");
        return 0;
    }

    //更新数据
    bms_update_soc_calc_data(config);

    uint16_t soc_new = 0, min_soc = 10000, max_soc = 0; // 初始化为0%
    uint16_t soc_range = config->soc_upper_limit - config->soc_lower_limit;

    for (uint16_t i = 0; i < config->rbms_count; i++)
    {
        if (config->rbms_soc[i] < min_soc)
        {
            min_soc = config->rbms_soc[i];
        }
        if (config->rbms_soc[i] > max_soc)
        {
            max_soc = config->rbms_soc[i];
        }
    }

    if (min_soc <= config->soc_lower_limit)
    {
        return min_soc; // 低 SOC 保护
    }

    if (max_soc >= config->soc_upper_limit)
    {
        return max_soc; // 高 SOC 保护
    }

    soc_new = (min_soc - config->soc_lower_limit) * soc_range / (config->soc_upper_limit + (min_soc - config->soc_lower_limit) - max_soc) + config->soc_lower_limit;

    config->bank_soc = soc_new;
    return soc_new;
}



/**************************************************************************
 * @brief           计算SOC统计结果
    @param[in]       config: 包含RBMS数量、每个RBMS的SOH值等配置参数
    @return          计算得到的SOH值，平均值
    @details:
                    
*************************************************************************/
uint16_t bms_soh_statistics(bms_soh_config_t *config)
{
    if (config == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "bms_soh_statistics: config is NULL");
        return 0;
    }
    if (config->rbms_count == 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "bms_soh_statistics: rbms_count is 0");
        return 0;
    }

    bms_update_soh_calc_data(config);

    uint16_t soh_new = 0, en_count = 0;
    for (uint16_t i = 0; i < config->rbms_count; i++)
    {
        if (config->rbms_en[i] == 1)
        {
            soh_new += config->rbms_soh[i];
            en_count++;
        }
    }
    config->bank_soh = soh_new / en_count;
    return config->bank_soh;
}

/*****************************************************************************
 * @brief       获取BMS充放电状态
 *****************************************************************************/ 
 bms_charge_status_e get_bms_charge_status(void)
{
    // 从RTDB获取BMS充放电状态的示例
    bms_charge_status_e charge_status = kBms_ChargeStatus_Start;
    charge_status = (bms_charge_status_e)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, 0, kBbms_ChSt);
    return charge_status;
}



//计算Bank系统运行状态
void bms_bank_oper_status_update(void)
{
    bms_bank_oper_status.control_mode = bbms_ctrl.ctrl_mode;
    bms_bank_oper_status.address_allocation = 1; // 后续填充--等待需求

    if(bbms_ctrl.bat_conn == 0)
    {
        bms_bank_oper_status.operate_control_cmd = 0;
    }
    else if(bbms_ctrl.bat_conn == 1)
    {
        bms_bank_oper_status.operate_control_cmd = 1; // NORMAL CONNECT
    }
    else if(bbms_ctrl.bat_conn == 2)
    {
        bms_bank_oper_status.operate_control_cmd = 1; // MANUAL CONNECT
    }
    else if(bbms_ctrl.bat_conn == 3)
    {
        bms_bank_oper_status.operate_control_cmd = 2; // DISCONNECT
    }
    else
    {
        bms_bank_oper_status.operate_control_cmd = 0; // 无效值处理，默认为NO REQ
    }

    bms_bank_oper_status.fault_clear = 0; //  后续填充--等待需求
    bms_bank_oper_status.reserved = 0; //  预留字段，当前未使用
}


/*****************************************************************************
 * @brief       更新BBMS电池状态
 * @param       bbms_idx  BBMS电池包索引
 * @return      bms_bank_status_e 电池状态
 * @remark      根据BBMS控制结构体中的bat_conn字段，判断电池状态并返回相应的枚举值
 *              只要有一个RBMS未连接，Bank状态即为Not Connected
 *              只要有一个RBMS已连接，Bank状态即为Connected
 *              只要有一个RBMS处于SOC保护状态，Bank状态即为SOC Protection
 *              只要有一个RBMS处于故障状态，Bank状态即为Fault
 *              其他情况，Bank状态即为Allocation
 *****************************************************************************/
bms_bank_status_e bms_bank_status_update(uint16_t bbms_idx)
{
    bms_bank_status_e bank_status = kBms_BankStatus_Start;
    bank_status = kBms_BankStatus_Allocation;

    uint16_t rbms_num = get_rbms_num(); // 获取RBMS数量的函数，需根据实际情况实现
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        //如果不在线，跳过
        if(get_rbms_online(i) == 0)
        {
            continue;
        }

        bms_bank_status_e rbms_conn_status = (bms_bank_status_e)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SysOperSt);
        if(rbms_conn_status == kBms_BankStatus_NotConnected) // 假设0表示未连接，1表示已连接
        {
            bank_status = kBms_BankStatus_NotConnected;
            break; // 只要有一个RBMS未连接，Bank状态即为Not Connected
        }
        else if(rbms_conn_status == kBms_BankStatus_Connected)
        {
            bank_status = kBms_BankStatus_Connected;
        }
        else if(rbms_conn_status == kBms_BankStatus_SOCProtection)
        {
            bank_status = kBms_BankStatus_SOCProtection;
            break; // 只要有一个RBMS处于SOC保护状态，Bank状态即为SOC Protection
        }
        else if(rbms_conn_status == kBms_BankStatus_Fault)
        {
            bank_status = kBms_BankStatus_Fault;
            break; // 只要有一个RBMS处于故障状态，Bank状态即为Fault
        }
        else
        {
            // 其他状态不影响当前Bank状态，继续检查下一个RBMS
        }
    }

    return bank_status;
}


/*****************************************************************************
 * @brief       计算BBMS电池包充放电状态
 * @return      bms_charge_status_e 充放电状态
 * @remark      根据所有RBMS的充放电状态，判断BBMS的充放电状态并返回相应的枚举值
 *              只要有一个RBMS放电中，BBMS的充放电状态即为放电中
 *              只要有一个RBMS充电中，BBMS的充放电状态即为充电中
 *              其他情况，BBMS的充放电状态即为Not Connected
 *****************************************************************************/
bms_charge_status_e bms_bank_charge_status_update(void)
{
    uint16_t rbms_num = get_rbms_num(); // 获取RBMS数量的函数，需根据实际情况实现
    bms_charge_status_e charge_status = kBms_ChargeStatus_Start;
    charge_status = kBms_ChargeStatus_NotConnected;

    for(uint16_t i = 0; i < rbms_num; i++)
    {
        //如果不在线，跳过
        if(get_rbms_online(i) == 0)
        {
            continue;
        }

        bms_charge_status_e rbms_charge_status = (bms_charge_status_e)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ChaSt);
        if(rbms_charge_status == kBms_ChargeStatus_Discharge) // 假设3表示放电中
        {
            charge_status = kBms_ChargeStatus_Discharge;
            break; // 只要有一个RBMS放电中，充放电状态即为放电中
        }
        else if(rbms_charge_status == kBms_ChargeStatus_Charge) // 假设4表示充电中
        {
            charge_status = kBms_ChargeStatus_Charge;
            break; // 只要有一个RBMS充电中，充放电状态即为充电中
        }
        else if(rbms_charge_status == kBms_ChargeStatus_FullDischarge) // 假设2表示充电完成
        {
            charge_status = kBms_ChargeStatus_FullDischarge;
            // 不break，继续检查是否有充电中的RBMS，以确定最终状态
        }
        else if(rbms_charge_status == kBms_ChargeStatus_FullCharge) // 假设5表示充电完成
        {
            charge_status = kBms_ChargeStatus_FullCharge;
            // 不break，继续检查是否有放电中的RBMS，以确定最终状态
        }
        else
        {
            // 其他状态不影响当前充放电状态，继续检查下一个RBMS
        }
    }

    return charge_status;
}

// 更新BBMS设置逆变器状态
uint16_t  bms_bank_set_inverter_st_update(uint16_t bbms_idx)
{
    uint16_t set_inverter_st = 0;
    return set_inverter_st;
}


// 更新BBMS最小并网簇数
uint16_t bms_bank_min_rack_enaNbr_update(void)
{
    uint16_t min_rack_enaNbr = 0;
    return  bbms_sys_cfg.min_online_cluster_num;
}



// 更新BBMS存在严重故障的Rack位置
uint16_t bms_bank_rack_flt_lvl1_nbr_update(void)
{
    uint16_t rack_flt_lvl1_nbr = 0;
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        uint8_t rbms_fault = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_BMSMaxFltLevel);
        if(rbms_fault == kFaultLevel_1)
        {
            rack_flt_lvl1_nbr |= (1 << i);
        }
    }
    return rack_flt_lvl1_nbr;
}

/*****************************************************************************
 * @brief       更新BBMS接触器状态
 * @return      uint16_t 接触器状态
 *****************************************************************************/
typedef struct
{
    uint16_t epo_status            : 1;    // Bit0  急停故障状态 0=正常 1=故障
    uint16_t cmbst_gas_status      : 1;    // Bit1  可燃气体故障状态 0=正常 1=故障
    uint16_t fire_warn_status      : 1;    // Bit2  消防三级故障状态 0=正常 1=故障
    uint16_t fire_fault_status     : 1;    // Bit3  消防二级故障状态 0=正常 1=故障
    uint16_t fire_actuator_status  : 1;    // Bit4  消防一级故障状态 0=正常 1=故障
    uint16_t qf_switch_status      : 1;    // Bit5  断路器QF故障状态 0=正常 1=故障
    uint16_t qs_status            : 1;    // Bit6  隔离开关QS故障状态 0=正常 1=故障
    uint16_t spd_status            : 1;    // Bit7  浪涌反馈故障状态 0=正常 1=故障

    uint16_t green_light           : 1;    // Bit8  绿灯 0=灭 1=亮
    uint16_t yellow_light          : 1;    // Bit9  黄灯 0=灭 1=亮
    uint16_t red_light             : 1;    // Bit10 红灯 0=灭 1=亮
    uint16_t reserved1             : 1;    // Bit11 保留

    uint16_t door_status           : 1;    // Bit12 门禁故障状态 0=正常 1=故障
    uint16_t aerosol_status        : 1;    // Bit13 气溶胶故障状态 0=正常 1=故障
    uint16_t expprf_fan_status     : 1;    // Bit14 防爆风机状态 0=关闭 1=打开
    uint16_t expprf_fan_fault      : 1;    // Bit15 防爆风机故障状态 0=正常 1=故障
} bms_system_status_bits_t;

typedef union
{
    uint16_t value;                  // CAN收到的原始16位数据
    bms_system_status_bits_t bit;    // 按位解析
} bms_bank_contact_status_t;


uint16_t  bms_bank_contact_st_update(uint16_t bbmsIdx)
{
    bms_bank_contact_status_t contact_st = {0};
    contact_st.value = 0;

    contact_st.bit.epo_status = bms_get_m_fault_status(kBmsMFault_EmergStopFault);
    contact_st.bit.cmbst_gas_status = bms_get_m_fault_status(kBmsMFault_CombGasAlarm);
    contact_st.bit.fire_warn_status = bms_get_m_fault_status(kBmsMFault_FireFault);
    contact_st.bit.fire_fault_status = bms_get_m_fault_status(kBmsMFault_FireModerateAlarm);
    contact_st.bit.fire_actuator_status = bms_get_m_fault_status(kBmsMFault_FireExtremeAlarm);
    contact_st.bit.qf_switch_status = bms_get_m_fault_status(kBmsMFault_QF1CircuitBrkFault);

    uint16_t rbms_num = get_rbms_num();

    //计算隔离关QS故障状态 0=正常 1=故障，统计rbms而来
    uint8_t qs_status = 0;
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        uint8_t rbms_fault = get_rbms_fault_bit(i, kRbmsFault_IslSwitchOpen);
        if(rbms_fault != 0)
        {
            qs_status = 1;
            break;
        }
    }
    contact_st.bit.qs_status = qs_status;

    contact_st.bit.spd_status = bms_get_m_fault_status(kBmsMFault_SPD2SurgeFault);  
    contact_st.bit.green_light = get_do_output_status(kDo_Ctrl_Green_Light);
    contact_st.bit.yellow_light = get_do_output_status(kDo_Ctrl_Yellow_Light);
    contact_st.bit.red_light = get_do_output_status(kDo_Ctrl_Red_Light);
    contact_st.bit.reserved1 = 0;
    contact_st.bit.door_status = get_do_output_status(kDo_Ctrl_PCS_EStop);


    //气溶胶故障状态计算 0=正常 1=故障，统计rbms而来
    uint8_t aerosol_status = 0;
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        uint8_t rbms_fault1 = get_rbms_fault_bit(i, kRbmsFault_Aerosol1);
        uint8_t rbms_fault2 = get_rbms_fault_bit(i, kRbmsFault_Aerosol2);
        if(rbms_fault1 != 0 || rbms_fault2 != 0 )
        {
            aerosol_status = 1;
            break;
        }
    }
    contact_st.bit.aerosol_status = aerosol_status;

    //后续需要确认防爆风机状态和故障状态区分，数据源在哪里，当前只有一个DI接入的
    contact_st.bit.expprf_fan_status = bms_get_m_fault_status(kBmsMFault_ExproFanAlarm);
    contact_st.bit.expprf_fan_fault = bms_get_m_fault_status(kBmsMFault_ExproFanAlarm);

    return contact_st.value;
}



/*********************************************
 * @brief 计算系统可充能量
 * @param bbms_idx BBMS索引
 * @return float 系统可充能量
 * @note 统计rbms中的所有可充能量，赋值到max_chrg_cap中
 **********************************************/
float bms_bank_max_chrg_cap_update(uint16_t bbms_idx)
{
    float max_chrg_cap = 0.0f;
    // 计算系统可充能量
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_chrg_cap += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoEC);
        }
    }

    return max_chrg_cap;
}



/*********************************************
 * @brief 计算系统可放能量
 * @param bbms_idx BBMS索引
 * @return float 系统可放能量
 **********************************************/
float bms_bank_max_dis_chrg_cap_update(uint16_t bbms_idx)
{
    float max_dis_chrg_cap = 0.0f;
    // 计算系统可放能量
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_dis_chrg_cap += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoED);
        }
    }

    return max_dis_chrg_cap;
}



//持续充电功率限制
static float bms_bank_max_chrg_power_limit_update(uint16_t bbms_idx)
{
    float max_chrg_power_limit = 0.0f;
    // 计算持续充电功率限制
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_chrg_power_limit += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoFPulseChrgPwr);
        }
    }
    return max_chrg_power_limit;
}


//脉冲充电功率限制 (kW)
static float bms_bank_max_chrg_power_limit_60s_update(uint16_t bbms_idx)
{
    float max_chrg_power_limit_60s = 0.0f;
    // 计算脉冲充电功率限制
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_chrg_power_limit_60s += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoFPulseChrgPwr);
        }
    }
    return max_chrg_power_limit_60s;
}



//持续放电功率限制
static float bms_bank_max_dis_chrg_power_limit_update(uint16_t bbms_idx)
{
    float max_dis_chrg_power_limit = 0.0f;
    // 计算持续放电功率限制
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_dis_chrg_power_limit += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoFContDisChrgPwr);
        }
    }
    return max_dis_chrg_power_limit;
}


//脉冲放电功率限制 (kW)
static float bms_bank_max_dis_chrg_power_limit_60s_update(uint16_t bbms_idx)
{
    float max_dis_chrg_power_limit_60s = 0.0f;
    // 计算脉冲放电功率限制
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_dis_chrg_power_limit_60s += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoFPulseDisChrgPwr);
        }
    }
    return max_dis_chrg_power_limit_60s;
}


//持续充电电流限制 (A)
static float bms_bank_max_chrg_cur_limit_update(uint16_t bbms_idx)
{
    float max_chrg_cur_limit = 0.0f;
    // 计算持续充电电流限制
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_chrg_cur_limit += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoFContChrgCurr);
        }
    }
    return max_chrg_cur_limit;
}

//脉冲充电电流限制 (A)
static float bms_bank_max_chrg_cur_limit_60s_update(uint16_t bbms_idx)
{
    float max_chrg_cur_limit_60s = 0.0f;
    // 计算脉冲充电电流限制
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_chrg_cur_limit_60s += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoFPulseChrgCurr);
        }
    }
    return max_chrg_cur_limit_60s;
}

//持续放电电流限制 (A)
static float bms_bank_max_dis_chrg_cur_limit_update(uint16_t bbms_idx)
{
    float max_dis_chrg_cur_limit = 0.0f;
    // 计算持续放电电流限制
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_dis_chrg_cur_limit += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoFContDisChrgCurr);
        }
    }
    return max_dis_chrg_cur_limit;
}


//脉冲放电电流限制 (A)
static float bms_bank_max_dis_chrg_cur_limit_60s_update(uint16_t bbms_idx)
{
    float max_dis_chrg_cur_limit_60s = 0.0f;
    // 计算脉冲放电电流限制
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_dis_chrg_cur_limit_60s += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoFPulseDisChrgCurr);
        }
    }
    return max_dis_chrg_cur_limit_60s;
}

//Bank湿度 (RH)  Bank湿度 (RH)          -- 后续确认,当前取除湿机湿度
static float bms_bank_hum_sensor_rh_update(uint16_t bbms_idx)
{
    float hum_sensor_rh = 0.0f;
    // 计算Bank湿度 (RH)
    uint16_t dehumidifier_num = getRtdbPointValue(Rtdb_ShMem, kDev_Type_Con, 0, kCon_dehumidifierNum);
    for(uint16_t i = 0; i < dehumidifier_num; i++)
    {
        uint8_t online = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_dehumidifier, i, kDehumidifier_Online);
        if(online)
        {
            hum_sensor_rh = getRtdbPointValue(Rtdb_ShMem, kDev_Type_dehumidifier, i, kDehumidifier_CurrentHumidity);
        }
    }
    return hum_sensor_rh;
}


//Bank温度 (℃)  Bank温度 (℃)          -- 后续确认,当前取除湿机温度
static float bms_bank_tmp_sensor_rh_update(uint16_t bbms_idx)
{
    float tmp_sensor_rh = 0.0f;
    // 计算Bank温度 (℃)
    uint16_t dehumidifier_num = getRtdbPointValue(Rtdb_ShMem, kDev_Type_Con, 0, kCon_dehumidifierNum);
    for(uint16_t i = 0; i < dehumidifier_num; i++)
    {
        uint8_t online = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_dehumidifier, i, kDehumidifier_Online);
        if(online)
        {
            tmp_sensor_rh = getRtdbPointValue(Rtdb_ShMem, kDev_Type_dehumidifier, i, kDehumidifier_CurrentTemp);
        }
    }
    return tmp_sensor_rh;
}



//Bank心跳信号
static uint16_t bms_bank_heartbeat_update(uint16_t bbms_idx)
{
    // 获取Bank心跳信号
    uint16_t bank_bms_heart = getRtdbPointValue(Rtdb_ShMem, kDev_Type_Con, 0, kCon_RunHeartCount);
    return bank_bms_heart;
}

//Bank DC线电压 (V)，获取rbms中最大的DC线电压
static float bms_bank_dc_bus_v_update(uint16_t bbms_idx)
{
    float dc_bus_v = 0.0f, max_dc_bus_v = 0.0f;
    // 计算BankDC线电压 (V)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            dc_bus_v = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_DCBusV);
            if(dc_bus_v > max_dc_bus_v)
            {
                max_dc_bus_v = dc_bus_v;
            }
        }
    }
    return max_dc_bus_v;
}


//Bank DC线电流 (A)，获取rbms中DC线电流之和
static float bms_bank_dc_bus_cur_update(uint16_t bbms_idx)
{
    float dc_bus_cur = 0.0f;
    // 计算BankDC线电流 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            dc_bus_cur += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_A_HighAccu);
        }
    }
    return dc_bus_cur;
}


//Bank 总功率 (kW)，获取rbms中总功率之和
static float bms_bank_total_power_update(uint16_t bbms_idx)
{
    float total_power = 0.0f;
    // 计算Bank总功率 (kW)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            float dc_bus_v = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_DCBusV);
            float dc_bus_cur = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_A_HighAccu);
            float rbms_total_power = dc_bus_v * dc_bus_cur;
            total_power += rbms_total_power;
        }
    }
    return total_power/1000.0f;
}

//Bank 最大Rack电流 (A)，获取rbms中最大的Rack电流
static float bms_bank_max_rack_cur_update(uint16_t bbms_idx)
{
    float max_rack_cur = 0.0f;
    // 计算Bank最大Rack电流 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_rack_cur = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_A_HighAccu);
            if(max_rack_cur > max_rack_cur)
            {
                max_rack_cur = max_rack_cur;
            }
        }
    }
    return max_rack_cur;
}


//Bank 最大电流Rack位置 (A)，获取rbms中最大的电流Rack位置
static uint16_t bms_bank_max_cur_rack_nr_update(uint16_t bbms_idx)
{
    float max_cur_rack_nr = 0.0f;
    uint16_t max_cur_rack_nr_idx = 0;
    // 计算Bank最大电流Rack位置 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_cur_rack_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_A_HighAccu);
            if(max_cur_rack_nr > max_cur_rack_nr)
            {
                max_cur_rack_nr = max_cur_rack_nr;
                max_cur_rack_nr_idx = i;
            }
        }
    }
    return max_cur_rack_nr_idx;
}

//Bank 最小Rack电流 (A)，获取rbms中最小的Rack电流
static float bms_bank_min_rack_cur_update(uint16_t bbms_idx)
{
    float min_rack_cur = 65535.00f;
    // 计算Bank最小Rack电流 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            min_rack_cur = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_A_HighAccu);
            if(min_rack_cur < min_rack_cur)
            {
                min_rack_cur = min_rack_cur;
            }
        }
    }
    return min_rack_cur;
}   

//Bank 最小电流Rack位置 (A)，获取rbms中最小的电流Rack位置
static uint16_t bms_bank_min_cur_rack_nr_update(uint16_t bbms_idx)
{
    float min_cur_rack_nr = 65535.00f;
    uint16_t min_cur_rack_nr_idx = 0;
    // 计算Bank最小电流Rack位置 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            min_cur_rack_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_A_HighAccu);
            if(min_cur_rack_nr < min_cur_rack_nr)
            {
                min_cur_rack_nr = min_cur_rack_nr;
                min_cur_rack_nr_idx = i;
            }
        }
    }
    return min_cur_rack_nr_idx;
}

//Bank 最大电芯电压 (mV)，获取rbms中最大的电芯电压
static float bms_bank_max_cell_v_update(uint16_t bbms_idx)
{
    float max_cell_v = 0.0f;
    // 计算Bank最大电芯电压 (mV)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_cell_v = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_CellVMax);
            if(max_cell_v > max_cell_v)
            {
                max_cell_v = max_cell_v;
            }
        }
    }
    return max_cell_v;
}

//Bank 最大电芯电压Rack位置 (A)，获取rbms中最大的电芯电压Rack位置
static uint16_t bms_bank_max_v_cell_rack_nr_update(uint16_t bbms_idx)
{
    float max_v_cell_rack_nr = 0.0f;
    uint16_t max_v_cell_rack_nr_idx = 0;
    // 计算Bank最大电芯电压Rack位置 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_v_cell_rack_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_CellVMax);
            if(max_v_cell_rack_nr > max_v_cell_rack_nr)
            {
                max_v_cell_rack_nr = max_v_cell_rack_nr;
                max_v_cell_rack_nr_idx = i;
            }
        }
    }
    return max_v_cell_rack_nr_idx;
}

//Bank 最大电芯电压模组位置 (A)，获取rbms中最大的电芯电压模组位置
static uint16_t bms_bank_max_v_cell_tray_nr_update(uint16_t bbms_idx)
{
    float max_v_cell_tray_nr = 0.0f;
    uint16_t max_v_cell_tray_nr_idx = 0;
    // 计算Bank最大电芯电压模组位置 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_v_cell_tray_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_CellVMax);
            if(max_v_cell_tray_nr > max_v_cell_tray_nr)
            {
                max_v_cell_tray_nr = max_v_cell_tray_nr;
                max_v_cell_tray_nr_idx = i;
            }
        }
    }
    // 获取最大电芯电压模组位置 (A)
    uint16_t max_v_cell_tray_nr_pack = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, max_v_cell_tray_nr_idx, kRbms_MaxCellVMdulIdxNbr);
    return max_v_cell_tray_nr_pack;
}

//Bank 最大电芯电压模组内位置 (A)，获取rbms中最大的电芯电压模组内位置
static uint16_t bms_bank_max_v_cell_nr_update(uint16_t bbms_idx)
{
    float max_v_cell_tray_nr = 0.0f;
    uint16_t max_v_cell_tray_nr_idx = 0;
    // 计算Bank最大电芯电压模组位置 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_v_cell_tray_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_CellVMax);
            if(max_v_cell_tray_nr > max_v_cell_tray_nr)
            {
                max_v_cell_tray_nr = max_v_cell_tray_nr;
                max_v_cell_tray_nr_idx = i;
            }
        }
    }
    // 获取最大电芯电压模组位置 (A)
    uint16_t max_v_cell_tray_nr_rack = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, max_v_cell_tray_nr_idx, kRbms_MaxCellVMdulInnerIdxNbr);
    return max_v_cell_tray_nr_rack;
}


//Bank 最小电芯电压 (mV)，获取rbms中最小的电芯电压
static float bms_bank_min_cell_v_update(uint16_t bbms_idx)
{
    float min_cell_v = 65535.00f;
    // 计算Bank最小电芯电压 (mV)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            min_cell_v = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_CellVMin);
            if(min_cell_v < min_cell_v)
            {
                min_cell_v = min_cell_v;
            }
        }
    }
    return min_cell_v;
}

//Bank 最小电芯电压Rack位置 (A)，获取rbms中最小的电芯电压Rack位置
static uint16_t bms_bank_min_v_cell_rack_nr_update(uint16_t bbms_idx)
{
    float min_v_cell_rack_nr = 0.0f;
    uint16_t min_v_cell_rack_nr_idx = 0;
    // 计算Bank最小电芯电压Rack位置 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            min_v_cell_rack_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_CellVMin);
            if(min_v_cell_rack_nr < min_v_cell_rack_nr)
            {
                min_v_cell_rack_nr = min_v_cell_rack_nr;
                min_v_cell_rack_nr_idx = i;
            }
        }
    }
    // 获取最小电芯电压Rack位置 (A)
    uint16_t min_v_cell_rack_nr_pack = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, min_v_cell_rack_nr_idx, kRbms_CellVMinPstn);
    return min_v_cell_rack_nr_pack;
}

//Bank 最小电芯电压模组位置 (A)，获取rbms中最小的电芯电压模组位置
static uint16_t bms_bank_min_v_cell_tray_nr_update(uint16_t bbms_idx)
{
    float min_v_cell_rack_nr = 0.0f;
    uint16_t min_v_cell_rack_nr_idx = 0;
    // 计算Bank最小电芯电压Rack位置 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            min_v_cell_rack_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_CellVMin);
            if(min_v_cell_rack_nr < min_v_cell_rack_nr)
            {
                min_v_cell_rack_nr = min_v_cell_rack_nr;
                min_v_cell_rack_nr_idx = i;
            }
        }
    }
    // 获取最小电芯电压Rack位置 (A)
    uint16_t min_v_cell_rack_nr_pack = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, min_v_cell_rack_nr_idx, kRbms_MinCellVMdulIdxNbr);
    return min_v_cell_rack_nr_pack;
}

//Bank 最小电芯电压模组内位置 (A)，获取rbms中最小的电芯电压模组内位置
static uint16_t bms_bank_min_v_cell_nr_update(uint16_t bbms_idx)
{
    float min_v_cell_rack_nr = 0.0f;
    uint16_t min_v_cell_rack_nr_idx = 0;
    // 计算Bank最小电芯电压Rack位置 (A)
    uint16_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            min_v_cell_rack_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_CellVMin);
            if(min_v_cell_rack_nr < min_v_cell_rack_nr)
            {
                min_v_cell_rack_nr = min_v_cell_rack_nr;
                min_v_cell_rack_nr_idx = i;
            }
        }
    }
    // 获取最小电芯电压Rack位置 (A)
    uint16_t min_v_cell_rack_nr_pack = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, min_v_cell_rack_nr_idx, kRbms_MinCellVMdulInnerIdxNbr);
    return min_v_cell_rack_nr_pack;
}

//Bank 平均电芯电压 (mV)，获取rbms中平均的电芯电压
static float bms_bank_avg_cell_v_update(uint16_t bbms_idx)
{
    float avg_cell_v = 0.0f;
    // 计算Bank平均电芯电压 (mV)
    uint16_t rbms_num = get_rbms_num();
    uint16_t online_rbms_num = 0;
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            online_rbms_num++;
            avg_cell_v += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_CellVAvg);
        }
    }
    avg_cell_v /= online_rbms_num;
    return avg_cell_v;
}

//Bank 最高高压箱温度Rack位置 (A)，获取rbms中最高的高压箱温度Rack位置
static uint16_t bms_bank_max_hv_box_temp_rack_nr_update(uint16_t bbms_idx)
{
    float max_temp_cur = 0.0f;
    uint16_t max_temp_cur_idx = 0;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_temp_cur = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_HvBoxMaxTemp);
            if(max_temp_cur < max_temp_cur)
            {
                max_temp_cur = max_temp_cur;
                max_temp_cur_idx = i;
            }
        }
    }

    // 获取最高高压箱温度Rack位置 (A)
    return max_temp_cur_idx;
}

//Bank 最低高压箱温度Rack位置 (A)，获取rbms中最低的高压箱温度Rack位置
static uint16_t bms_bank_min_hv_box_temp_rack_nr_update(uint16_t bbms_idx)
{
    float min_temp_cur = 0.0f;
    uint16_t min_temp_cur_idx = 0;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            min_temp_cur = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_HvBoxMinTemp);
            if(min_temp_cur < min_temp_cur)
            {
                min_temp_cur = min_temp_cur;
                min_temp_cur_idx = i;
            }
        }
    }

    // 获取最低高压箱温度Rack位置 (A)
    return min_temp_cur_idx;
}


//Bank 最高电芯温度Rack位置 (A)
static uint16_t bms_bank_max_t_cell_rack_nr_update(uint16_t bbms_idx)
{
    float max_temp_cur = 0.0f;
    uint16_t max_temp_cur_idx = 0;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_temp_cur = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ModTmpMax);
            if(max_temp_cur < max_temp_cur)
            {
                max_temp_cur = max_temp_cur;
                max_temp_cur_idx = i;
            }
        }
    }

    // 获取最高电芯温度Rack位置 (A)
    return max_temp_cur_idx;
}


//Bank 最低电芯温度Rack位置 (A)，获取rbms中最低的电芯温度Rack位置
static uint16_t bms_bank_min_t_cell_rack_nr_update(uint16_t bbms_idx)
{
    float min_temp_cur = 0.0f;
    uint16_t min_temp_cur_idx = 0;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            min_temp_cur = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ModTmpMin);
            if(min_temp_cur < min_temp_cur)
            {
                min_temp_cur = min_temp_cur;
                min_temp_cur_idx = i;
            }
        }
    }
    // 获取最低电芯温度Rack位置 (A)
    return min_temp_cur_idx;
}


//Bank 最高电芯温度模组位置 (A)
static uint16_t bms_bank_max_t_cell_tray_nr_update(uint16_t bbms_idx)
{
    float max_temp_cur = 0.0f;
    uint16_t max_temp_cur_idx = bms_bank_max_t_cell_rack_nr_update(bbms_idx);
    uint16_t max_t_cell_tray_nr_pack = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, max_temp_cur_idx, kRbms_MaxTMdulIdxNbr);  

    // 获取最高电芯温度模组位置 (A)
    return max_t_cell_tray_nr_pack;
}



//Bank 最低电芯温度模组位置 
static uint16_t bms_bank_min_t_cell_tray_nr_update(uint16_t bbms_idx)
{
    float min_temp_cur = 0.0f;
    uint16_t min_temp_cur_idx = bms_bank_min_t_cell_rack_nr_update(bbms_idx);
    uint16_t min_t_cell_tray_nr_pack = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, min_temp_cur_idx, kRbms_MinTMdulIdxNbr);  

    // 获取最低电芯温度模组位置 (A)
    return min_t_cell_tray_nr_pack;
}



//Bank 最高电芯温度模组内位置 
static uint16_t bms_bank_max_t_cell_nr_update(uint16_t bbms_idx)
{
    float max_temp_cur = 0.0f;
    uint16_t max_temp_cur_idx = bms_bank_max_t_cell_rack_nr_update(bbms_idx);

    uint16_t max_t_cell_nr_pack = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, max_temp_cur_idx, kRbms_MaxCellVMdulInnerIdxNbr);  

    // 获取最高电芯温度模组内位置 (A)
    return max_t_cell_nr_pack;
}


//Bank 最低电芯温度模组内位置 
static uint16_t bms_bank_min_t_cell_nr_update(uint16_t bbms_idx)
{
    float min_temp_cur = 0.0f;
    uint16_t min_temp_cur_idx = bms_bank_min_t_cell_rack_nr_update(bbms_idx);
    uint16_t min_t_cell_nr_pack = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, min_temp_cur_idx, kRbms_MinCellVMdulInnerIdxNbr);  

    // 获取最低电芯温度模组内位置 (A)
    return min_t_cell_nr_pack;
}

/****************************************************
 * @brief 获取禁止升级标志位
 * @param None
 * @return uint16_t � 禁止升级标志位
 * @remark 如果有电流在运行，大于10A，禁止升级  
 *     
 ****************************************************/
static uint16_t bms_bank_proh_upgrade_flg_update(uint16_t bbms_idx)
{
    uint16_t proh_upgrade_flg = 0;
    
    // 如果有电流在运行，获取bbms电流
    float bbms_cur =  getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_TotalDCCur);

    // 如果bbms电流大于0，禁止升级
    if(bbms_cur >= 10.0f)
    {
        proh_upgrade_flg = 1;
    }

    return proh_upgrade_flg;
}

/*****************************************************************************
 * @brief       获取系统BMS中最高故障等级
 * @param[in]   bbms_idx: 为模拟堆预留，从1开始
 * @return      最高故障等级
 *****************************************************************************/
uint16_t bms_get_total_fault_level(uint16_t bbms_idx)
{
    uint16_t max_flt_level = 7;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
           uint16_t rbms_level = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_BMSMaxFltLevel);
           if(rbms_level < max_flt_level)
           {
               max_flt_level = rbms_level;
           }
        }
    }

    //获取液冷机组最高故障等级
    uint16_t liquid_cooling_max_flt_level = getRtdbPointValue(Rtdb_ShMem, kDev_Type_Lqd, 0, kRbms_TMS_BBMSFltLevel);
    if(liquid_cooling_max_flt_level < max_flt_level)
    {
        max_flt_level = liquid_cooling_max_flt_level;
    }
    
    //获取a核&M核心故障最高故障等级
    uint16_t a_fault_max_flt_level = bms_get_bank_max_flt_level(bbms_idx);
    if(a_fault_max_flt_level < max_flt_level)
    {
        max_flt_level = a_fault_max_flt_level;
    }


    return max_flt_level;
}


//Bank ID编码状态
static uint16_t bms_bank_id_learn_st_update(uint16_t bbms_idx)
{
    uint16_t id_learn_st = 0;
    return id_learn_st;
}

//Bank ID编码失败位置
static uint16_t bms_bank_id_learn_fai_location_nr_update(uint16_t bbms_idx)
{
    uint16_t id_learn_fai_location_nr = 0;
    return id_learn_fai_location_nr;
}


//Bank 正对地绝缘阻值，取自rbmskRbms_IsoRPos中最小值
static float bms_bank_iso_r_pos_update(uint16_t bbms_idx)
{
    float iso_r_pos = 65535.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
           uint16_t rbms_iso_r_pos = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_IsoRPos);
           if(rbms_iso_r_pos < iso_r_pos)
           {
               iso_r_pos = rbms_iso_r_pos;
           }
        }
    }

    return iso_r_pos;
}

//Bank 负对地绝缘阻值，取自rbmskRbms_IsoRNeg中最小值
static float bms_bank_iso_r_neg_update(uint16_t bbms_idx)
{
    float iso_r_neg = 65535.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
           uint16_t rbms_iso_r_neg = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_IsoRNeg);
           if(rbms_iso_r_neg < iso_r_neg)
           {
               iso_r_neg = rbms_iso_r_neg;
           }
        }
    }

    return iso_r_neg;
}


//Bank 累计充电容量，取自rbmskRbms_AccuChrgC中之和
static float bms_bank_accu_chrg_c_update(uint16_t bbms_idx)
{
    float accu_chrg_c = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            accu_chrg_c += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_AccuChrgAh2);
        }
    }

    return accu_chrg_c;
}


//Bank 累计充电能量，取自rbmskRbms_AccuChrgE中之和
static float bms_bank_accu_chrg_e_update(uint16_t bbms_idx)
{
    float accu_chrg_e = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            accu_chrg_e += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_AccuChrgkWh2);
        }
    }

    return accu_chrg_e;
}


//Bank 累计放电容量，取自rbmskRbms_AccuDisChrgC中之和
static float bms_bank_accu_dis_chrg_c_update(uint16_t bbms_idx)
{
    float accu_dis_chrg_c = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            accu_dis_chrg_c += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_AccuDisChrgAh2);
        }
    }

    return accu_dis_chrg_c;
}


//Bank 累计放电能量，取自rbmskRbms_AccuDisChrgE中之和
static float bms_bank_accu_dis_chrg_e_update(uint16_t bbms_idx)
{
    float accu_dis_chrg_e = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            accu_dis_chrg_e += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_AccuDisChrgkWh2);
        }
    }

    return accu_dis_chrg_e;
}

//Bank 最近一次堆充电能量，取自rbmskRbms_LstChEkWh中之和
static float bms_bank_lst_chrg_e_update(uint16_t bbms_idx)
{
    float lst_chrg_e = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            lst_chrg_e += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_LstChEkWh);
        }
    }

    return lst_chrg_e;
}


//Bank 最近一次堆放电能量，取自rbmskRbms_LstDchEkWh中之和
static float bms_bank_lst_dch_e_update(uint16_t bbms_idx)
{
    float lst_dch_e = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            lst_dch_e += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_LstDchEkWh);
        }
    }

    return lst_dch_e;
}


//簇使能数量，后续确认，当前使用rbms中的高压失败原因计算
static float bms_bank_rack_enaNbr_update(uint16_t bbms_idx)
{
    uint16_t rack_enaNbr = 0;
    uint8_t rbms_num = get_rbms_num();
    bms_con_fail_e con_fail = kRbms_ConFail_BatStrDis;
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            uint16_t rbms_con_fail = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ConFail);
            rack_enaNbr += ((rbms_con_fail >> con_fail) & 0x01);
        }
    }

    return rack_enaNbr;
}


//簇在线数量，后续确认，当前使用rbms中的高压失败原因计算
static float bms_bank_rack_onlineNbr_update(uint16_t bbms_idx)
{
    uint16_t rack_onlineNbr = 0;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            rack_onlineNbr++;
        }
    }

    return rack_onlineNbr;
}


//簇高压失败原因，后续确认，当前rbms中的高压失败原因直接或运算
static float bms_bank_con_fail_update(uint16_t bbms_idx)
{
    uint16_t con_fail = 0;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            uint16_t rbms_con_fail = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ConFail);
            con_fail |= rbms_con_fail;
        }
    }

    return con_fail;
}


//簇最大充容量，
static float bms_bank_max_chrg_cap_ah_update(uint16_t bbms_idx)
{
    float max_chrg_cap = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_chrg_cap += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ResChAvlCapAh);
        }
    }

    return max_chrg_cap;
}


//簇最大放容量，
static float bms_bank_max_dis_chrg_cap_ah_update(uint16_t bbms_idx)
{
    float max_dis_chrg_cap = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            max_dis_chrg_cap += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ResDischAvlCapAh);
        }
    }

    return max_dis_chrg_cap;
}


//簇最大高压箱温度，
static float bms_bank_max_hv_box_t_deg_c_update(uint16_t bbms_idx)
{
    float max_hv_box_t_deg_c = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            float rbms_max_hv_box_t_deg_c = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_HvBoxMaxTemp);
            if(rbms_max_hv_box_t_deg_c > max_hv_box_t_deg_c)
            {
                max_hv_box_t_deg_c = rbms_max_hv_box_t_deg_c;
            }
        }
    }

    return max_hv_box_t_deg_c;
}


//簇最小高压箱温度，
static float bms_bank_min_hv_box_t_deg_c_update(uint16_t bbms_idx)
{
    float min_hv_box_t_deg_c = 65535.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            float rbms_min_hv_box_t_deg_c = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_HvBoxMinTemp);
            if(rbms_min_hv_box_t_deg_c < min_hv_box_t_deg_c)
            {
                min_hv_box_t_deg_c = rbms_min_hv_box_t_deg_c;
            }
        }
    }   
    return min_hv_box_t_deg_c;
}


//簇最大电芯温度，
static float bms_bank_max_cell_t_deg_c_update(uint16_t bbms_idx)
{
    float max_cell_t_deg_c = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            float rbms_max_cell_t_deg_c = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ModTmpMax);
            if(rbms_max_cell_t_deg_c > max_cell_t_deg_c)
            {
                max_cell_t_deg_c = rbms_max_cell_t_deg_c;
            }
        }
    }
    return max_cell_t_deg_c;
}


//簇最小电芯温度，
static float bms_bank_min_cell_t_deg_c_update(uint16_t bbms_idx)
{
    float min_cell_t_deg_c = 65535.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            float rbms_min_cell_t_deg_c = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ModTmpMin);
            if(rbms_min_cell_t_deg_c < min_cell_t_deg_c)
            {
                min_cell_t_deg_c = rbms_min_cell_t_deg_c;
            }
        }
    }   
    return min_cell_t_deg_c;
}

//簇平均电芯温度，
static float bms_bank_avg_cell_t_deg_c_update(uint16_t bbms_idx)
{
    float avg_cell_t_deg_c = 0.0f;
    uint16_t rbms_nbr = 0;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            rbms_nbr++;
            avg_cell_t_deg_c += getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ModTmpAvg);
        }
    }
    if(rbms_nbr > 0)
    {
        avg_cell_t_deg_c /= rbms_nbr;
    }
    return avg_cell_t_deg_c;
}


//簇最大极柱温度，
static float bms_bank_max_pole_t_deg_c_update(uint16_t bbms_idx)
{
    float max_pole_t_deg_c = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            float rbms_max_pole_t_deg_c = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_MaxPoleTDegC);
            if(rbms_max_pole_t_deg_c > max_pole_t_deg_c)
            {
                max_pole_t_deg_c = rbms_max_pole_t_deg_c;
            }
        }
    }
    return max_pole_t_deg_c;
}



//簇最小极柱温度，
static float bms_bank_min_pole_t_deg_c_update(uint16_t bbms_idx)
{
    float min_pole_t_deg_c = 65535.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            float rbms_min_pole_t_deg_c = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_MinPoleTDegC);
            if(rbms_min_pole_t_deg_c < min_pole_t_deg_c)
            {
                min_pole_t_deg_c = rbms_min_pole_t_deg_c;
            }
        }
    }   
    return min_pole_t_deg_c;
}


//簇最大极柱温度Rack位置，
static uint16_t bms_bank_max_pole_t_rack_nr_update(uint16_t bbms_idx)
{
    uint16_t max_pole_t_rack_nr = 0;
    uint8_t rbms_num = get_rbms_num();
    uint8_t rbms_nbr = 0;
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            uint16_t rbms_max_pole_t_rack_nr = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_MaxPoleTIdxNbr);
            if(rbms_max_pole_t_rack_nr > max_pole_t_rack_nr)
            {
                max_pole_t_rack_nr = rbms_max_pole_t_rack_nr;
                rbms_nbr = i;
            }
        }
    }
    return rbms_nbr;
}



//簇最小极柱温度Rack位置，
static uint16_t bms_bank_min_pole_t_rack_nr_update(uint16_t bbms_idx)
{
    uint16_t min_pole_t_rack_nr = 65535;
    uint8_t rbms_num = get_rbms_num();
    uint8_t rbms_nbr = 0;
    for(uint16_t i = 0; i < rbms_num; i++)
    {
        if(get_rbms_online(i))
        {
            uint16_t rbms_min_pole_t_rack_nr = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_MinPoleTIdxNbr);
            if(rbms_min_pole_t_rack_nr < min_pole_t_rack_nr)
            {
                min_pole_t_rack_nr = rbms_min_pole_t_rack_nr;
                rbms_nbr = i;
            }
        }
    }
    return rbms_nbr;
}


//簇最大极柱温度模组位置，
static uint16_t bms_bank_max_pole_t_mdul_nr_update(uint16_t bbms_idx)
{
    uint16_t max_pole_t_mdul_nr = 0;
    uint8_t rbms_idx = bms_bank_max_pole_t_rack_nr_update(bbms_idx);
    if(rbms_idx < 0)
    {
        return max_pole_t_mdul_nr;
    }
    max_pole_t_mdul_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rbms_idx, kRbms_MaxPoleTMdulIdxNbr);
    return max_pole_t_mdul_nr;
}

//簇最小极柱温度模组位置，
static uint16_t bms_bank_min_pole_t_mdul_nr_update(uint16_t bbms_idx)
{
    uint16_t min_pole_t_mdul_nr = 0;
    uint8_t rbms_idx = bms_bank_min_pole_t_rack_nr_update(bbms_idx);
    if(rbms_idx < 0)
    {
        return min_pole_t_mdul_nr;
    }
    min_pole_t_mdul_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rbms_idx, kRbms_MinPoleTMdulIdxNbr);
    return min_pole_t_mdul_nr;
}



//簇最大极柱温度模组内位置，
static uint16_t bms_bank_max_pole_t_mdul_inner_nr_update(uint16_t bbms_idx)
{
    uint16_t max_pole_t_mdul_inner_nr = 0;
    uint8_t rbms_idx = bms_bank_max_pole_t_rack_nr_update(bbms_idx);
    if(rbms_idx < 0)
    {
        return max_pole_t_mdul_inner_nr;
    }
    max_pole_t_mdul_inner_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rbms_idx, kRbms_MaxPoleTMdulInnerIdxNbr);
    return max_pole_t_mdul_inner_nr;
}


//簇最小极柱温度模组内位置，
static uint16_t bms_bank_min_pole_t_mdul_inner_nr_update(uint16_t bbms_idx)
{
    uint16_t min_pole_t_mdul_inner_nr = 0;
    uint8_t rbms_idx = bms_bank_min_pole_t_rack_nr_update(bbms_idx);
    if(rbms_idx < 0)
    {
        return min_pole_t_mdul_inner_nr;
    }
    min_pole_t_mdul_inner_nr = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rbms_idx, kRbms_MinPoleTMdulInnerIdxNbr);
    return min_pole_t_mdul_inner_nr;
}


//RTC时间与生产日期有效性标志
static uint16_t bms_bank_rtc_time_vld_flg_update(uint16_t bbms_idx)
{
    uint16_t rtc_time_vld_flg = 0;
    return rtc_time_vld_flg;
}

//电池使用时长(month)
static uint16_t bms_bank_cell_used_month_update(uint16_t bbms_idx)
{
    uint16_t cell_used_month = 0;
    return cell_used_month;
}

//电池自放电率(%)，rbms中的电芯自放电率的最大值
static float bms_bank_cell_discharge_rate_pct_update(uint16_t bbms_idx)
{
    float cell_discharge_rate_pct = 0.0f;
    uint8_t rbms_num = get_rbms_num();
    for(uint16_t i = 0; i < rbms_num; i++)  
    {
        if(get_rbms_online(i))
        {
            for(uint16_t j = 0; j < kRbms_CellSdrate_End - kRbms_CellSdrate_Start; j++) //后续确认是否需要循环电芯 
            {
                float rbms_cell_discharge_rate_pct = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_CellSdrate_Start + j);
                if(rbms_cell_discharge_rate_pct > cell_discharge_rate_pct && rbms_cell_discharge_rate_pct <= 100.0f)
                {
                    cell_discharge_rate_pct = rbms_cell_discharge_rate_pct;
                }
            }
        }
    }

    return cell_discharge_rate_pct;
}

//Bank BMS故障统计，后续确认，当前使用rbms中的BMS故障统计
uint8_t bms_get_rbms_fault_statistics(uint8_t fault_idx)
{
    uint8_t rbms_fault_statistics = 0;
    for(uint16_t i = 0; i < get_rbms_num(); i++)
    {
        if(get_rbms_online(i))
        {
            rbms_fault_statistics |= (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_Fault_Start + fault_idx);
        }
    }

    return rbms_fault_statistics;
}





//band bms数据统计
void bms_bank_statistics_update(uint16_t bbms_idx)
{
    //Bank工作状态更新并写入RTDB
    bms_bank_status_e bank_status = kBms_BankStatus_Start;
    bank_status = bms_bank_status_update(bbms_idx); //后续补充完整-需求需要HC确认
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_BatSt, (double)bank_status);

    //Bank充放电状态
    bms_charge_status_e charge_status = bms_bank_charge_status_update();//后续补充完整-需求需要HC确认
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_ChSt, (double)charge_status);

    //更新Bank系统运行状态
    bms_bank_oper_status_update(); //后续补充完整-需求需要HC确认
    uint8_t bank_bms_oper_status = 0;
    memcpy(&bank_bms_oper_status, &bms_bank_oper_status, sizeof(bms_bank_oper_status_t));
    // 将bank_bms_oper_status写入RTDB，供其他线程读取
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SysOperSt, (double)bank_bms_oper_status);

    //允许运行的最小Rack数量
    uint16_t min_rack_enaNbr = bms_bank_min_rack_enaNbr_update(); 
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_PmtRkFltNum, (double)min_rack_enaNbr);

    //设置逆变器状态
    uint16_t set_inverter_st = bms_bank_set_inverter_st_update(bbms_idx);  //后续补充完整-需求确认
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SetInverterSt, (double)set_inverter_st);

    //存在严重故障的Rack位置
    uint16_t rack_flt_lvl1_nbr = bms_bank_rack_flt_lvl1_nbr_update();  
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_RackFltLvl1Nbr, (double)rack_flt_lvl1_nbr);

    //Bank接触器状态
    uint16_t contact_st = bms_bank_contact_st_update(bbms_idx);  //后续补充完整-需求确认
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_CntrSt, (double)contact_st);

    //Bank SOC 值
    uint16_t bank_soc = bms_soc_statistics(&bms_soc_config);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SOC, (double)bank_soc);
    
    //Bank SOH 值
    float bank_soh = bms_soh_statistics(&bms_soh_config);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SOH, (double)bank_soh);

    //系统可充能量
    float max_chrg_cap = bms_bank_max_chrg_cap_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxChrgE, (double)max_chrg_cap);

    //系统可放能量
    float max_dis_chrg_cap = bms_bank_max_dis_chrg_cap_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxDisChrgE, (double)max_dis_chrg_cap);

    //持续充电功率限制
    float max_chrg_power_limit = bms_bank_max_chrg_power_limit_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxContChrgPower, (double)max_chrg_power_limit);

    //脉冲充电功率限制 (kW)
    float max_chrg_power_limit_60s = bms_bank_max_chrg_power_limit_60s_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_Max60sChrgPower, (double)max_chrg_power_limit_60s);       

    //持续放电功率限制
    float max_dis_chrg_power_limit = bms_bank_max_dis_chrg_power_limit_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxContDisChrgPower, (double)max_dis_chrg_power_limit);

    // kBbms_Max60sDisChrgPower,    // Pulse Discharge Power Limit (kW)  脉冲放电功率限制 (kW)
    float max_dis_chrg_power_limit_60s = bms_bank_max_dis_chrg_power_limit_60s_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_Max60sDisChrgPower, (double)max_dis_chrg_power_limit_60s);       

    // kBbms_MaxContChrgCur,        // Const Charge Curr Limit (A)  持续充电电流限制 (A)
    float max_chrg_cur_limit = bms_bank_max_chrg_cur_limit_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxContChrgCur, (double)max_chrg_cur_limit);

    // kBbms_Max60sChrgCur,         // Pulse Charge Curr Limit (A)  脉冲充电电流限制 (A)
    float max_chrg_cur_limit_60s = bms_bank_max_chrg_cur_limit_60s_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_Max60sChrgCur, (double)max_chrg_cur_limit_60s);

    // kBbms_MaxContDisChrgCur,     // Const Discharge Curr Limit (A)  持续放电电流限制 (A)
    float max_dis_chrg_cur_limit = bms_bank_max_dis_chrg_cur_limit_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxContDisChrgCur, (double)max_dis_chrg_cur_limit);

    // kBbms_Max60sDisChrgCur,      // Pulse Discharge Curr Limit (A)  脉冲放电电流限制 (A)
    float max_dis_chrg_cur_limit_60s = bms_bank_max_dis_chrg_cur_limit_60s_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_Max60sDisChrgCur, (double)max_dis_chrg_cur_limit_60s);
    
    // Bank Humidity (RH)  Bank湿度 (RH)          -- 后续确认,当前取除湿机湿度
    float hum_sensor_rh = bms_bank_hum_sensor_rh_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_HumSensorRH, (double)hum_sensor_rh);
    
    // kBbms_TmpSensorRH,           // Bank Temp (℃)  Bank温度 (℃)
    float tmp_sensor_rh = bms_bank_tmp_sensor_rh_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_TmpSensorRH, (double)tmp_sensor_rh);

    // kBbms_WtrSt,                 // Bank Water Sensor Status  Bank水浸状态
    uint8_t wtr_st = bms_get_m_fault_status(kBmsMFault_WaterLeakAlarm);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_WtrSt, (double)wtr_st);   

    // kBbms_StrCtrlHb,             // Bank BMS Heartbeat Bank BMS心跳信号
    uint16_t str_ctrl_hb = bms_bank_heartbeat_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_StrCtrlHb, (double)str_ctrl_hb);

    // DC Bus Voltage (V) 母线电压 (V) ，获取rbms中最大的DC线电压
    float dc_bus_v = bms_bank_dc_bus_v_update(bbms_idx); 
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_DCBusV, (double)dc_bus_v);
    // DC Bus Current (A) 母线电流 (A) ，获取rbms中DC线电流之和
    float dc_bus_cur = bms_bank_dc_bus_cur_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_TotalDCCur, (double)dc_bus_cur);

    // kBbms_TotalPower,            // Total Power (kW) 总功率 (kW)
    float total_power = bms_bank_total_power_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_TotalPower, (double)total_power);

    // kBbms_MaxRackCur,            // Max Rack Current (A) 最大Rack电流 (A)
    float max_rack_cur = bms_bank_max_rack_cur_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxRackCur, (double)max_rack_cur);

    // kBbms_MaxCurRackNr,          // Max Current Rack Position 最大电流Rack位置
    uint16_t max_cur_rack_nr = bms_bank_max_cur_rack_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxCurRackNr, (double)max_cur_rack_nr);

    // kBbms_MinRackCur,            // Min Rack Current (A) 最小Rack电流 (A)
    float min_rack_cur = bms_bank_min_rack_cur_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinRackCur, (double)min_rack_cur);

    // kBbms_MinCurRackNr,          // Min Current Rack Position 最小电流Rack位置
    uint16_t min_cur_rack_nr = bms_bank_min_cur_rack_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinCurRackNr, (double)min_cur_rack_nr);

    // kBbms_MaxCellV,              // Max Cell Voltage (mV) 最大电芯电压 (mV)
    float max_cell_v = bms_bank_max_cell_v_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxCellV, (double)max_cell_v);

    // kBbms_MaxVCellRackNr,        // Max Cell Voltage Rack Position 最大电芯电压Rack位置
    uint16_t max_v_cell_rack_nr = bms_bank_max_v_cell_rack_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxVCellRackNr, (double)max_v_cell_rack_nr);

    // kBbms_MaxVCellTrayNr,        // Max Cell Voltage Module Position 最大电芯电压模组位置
    uint16_t max_v_cell_tray_nr = bms_bank_max_v_cell_tray_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxVCellTrayNr, (double)max_v_cell_tray_nr);  

    // kBbms_MaxVCellNr,            // Max Cell Voltage Module Inner Position 最大电芯电压模组内位置
    uint16_t max_v_cell_nr = bms_bank_max_v_cell_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxVCellNr, (double)max_v_cell_nr);  

    // kBbms_MinCellV,              // Min Cell Voltage (mV) 最小电芯电压 (mV)
    float min_cell_v = bms_bank_min_cell_v_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinCellV, (double)min_cell_v);

    // kBbms_MinVCellRackNr,        // Min Cell Voltage Rack Position 最小电芯电压Rack位置
    uint16_t min_v_cell_rack_nr = bms_bank_min_v_cell_rack_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinVCellRackNr, (double)min_v_cell_rack_nr);

    // Min Cell Voltage Module Position 最小电芯电压模组位置
    uint16_t min_v_cell_tray_nr = bms_bank_min_v_cell_tray_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinVCellTrayNr, (double)min_v_cell_tray_nr);

    // Min Cell Voltage Position 最小电芯电压模组内位置
    uint16_t min_v_cell_nr = bms_bank_min_v_cell_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinVCellNr, (double)min_v_cell_nr);   

    // Average Cell Voltage (mV) 平均电芯电压 (mV)
    float avg_cell_v = bms_bank_avg_cell_v_update(bbms_idx);  //每个rbms的平均电芯电压之和的平均
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_AvgCellV, (double)avg_cell_v);

    // kBbms_MaxHvBoxTRackNr,       // Max Hv Box Temp Rack Position 最高高压箱温度Rack位置
    uint16_t max_hv_box_temp_rack_nr = bms_bank_max_hv_box_temp_rack_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxHvBoxTRackNr, (double)max_hv_box_temp_rack_nr);

    // kBbms_MaxTCellRackNr,        // Max Cell Temp Rack Position 最高电芯温度Rack位置
    uint16_t max_t_cell_rack_nr = bms_bank_max_t_cell_rack_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxTCellRackNr, (double)max_t_cell_rack_nr);

    // kBbms_MaxTCellTrayNr,        // Max Cell Temp Module Position 最高电芯温度模组位置
    uint16_t max_t_cell_tray_nr = bms_bank_max_t_cell_tray_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxTCellTrayNr, (double)max_t_cell_tray_nr);

    // kBbms_MaxTCellNr,            // Max Cell Temp Module Inner Position  最高电芯温度模组内位置
    uint16_t max_t_cell_nr = bms_bank_max_t_cell_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxTCellNr, (double)max_t_cell_nr);

    // kBbms_MinHvBoxTRackNr,       // Min Hv Box Temp Rack Position 最低高压箱温度Rack位置
    uint16_t min_hv_box_temp_rack_nr = bms_bank_min_hv_box_temp_rack_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinHvBoxTRackNr, (double)min_hv_box_temp_rack_nr);

    // kBbms_MinTCellRackNr,        // Min Cell Temp Rack Position 最低电芯温度Rack位置
    uint16_t min_t_cell_rack_nr = bms_bank_min_t_cell_rack_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinTCellRackNr, (double)min_t_cell_rack_nr);

    // kBbms_MinTCellTrayNr,        // Min Cell Temp Module Position 最低电芯温度模组位置
    uint16_t min_t_cell_tray_nr = bms_bank_min_t_cell_tray_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinTCellTrayNr, (double)min_t_cell_tray_nr);

    // kBbms_MinTCellNr,            // Min Cell Temp Module Inner Position  最低电芯温度模组内位置
    uint16_t min_t_cell_nr = bms_bank_min_t_cell_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinTCellNr, (double)min_t_cell_nr);

    // kBbms_ProhUpgradeFlg,        // Prohibited Upgrade Flag 禁止升级标志位
    uint16_t proh_upgrade_flg = bms_bank_proh_upgrade_flg_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_ProhUpgradeFlg, (double)proh_upgrade_flg);    

    // kBbms_BMSMaxFltLevel,        // System Fault Level 系统故障等级
    uint16_t max_flt_level = bms_get_total_fault_level(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_BMSMaxFltLevel, (double)max_flt_level);
    
    // kBbms_IDLearnSt,             // Bank ID Learn Status  Bank ID编码状态
    uint16_t id_learn_st = bms_bank_id_learn_st_update(bbms_idx); //后续确认，需求未知
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_IDLearnSt, (double)id_learn_st);

    // kBbms_IDLearnFaiLocationNbr, // Bank ID Learn Fail Position Bank ID编码失败位置
    uint16_t id_learn_fai_location_nr = bms_bank_id_learn_fai_location_nr_update(bbms_idx); //后续确认，需求未知
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_IDLearnFaiLocationNbr, (double)id_learn_fai_location_nr);
    
    // kBbms_IsoRPos,               // Positive to GND Insulation Resistance (kOhm) 正对地绝缘阻值 (kOhm)
    float iso_r_pos = bms_bank_iso_r_pos_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_IsoRPos, (double)iso_r_pos);  

    // kBbms_IsoRNeg,               // Negative to GND Insulation Resistance (kOhm) 负对地绝缘阻值 (kOhm)
    float iso_r_neg = bms_bank_iso_r_neg_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_IsoRNeg, (double)iso_r_neg);  

    // kBbms_AccuChrgC,             // Accu Charge Cap (Ah) 累计充电容量 (Ah)
    float accu_chrg_c = bms_bank_accu_chrg_c_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_AccuChrgC, (double)accu_chrg_c);  

    // kBbms_AccuChrgE,             // Accu Charge Eng (kWh) 累计充电能量 (kWh)
    float accu_chrg_e = bms_bank_accu_chrg_e_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_AccuChrgE, (double)accu_chrg_e);  

    // kBbms_AccuDisChrgC,          // Accu Discharge Cap (Ah) 累计放电容量 (Ah)
    float accu_dis_chrg_c = bms_bank_accu_dis_chrg_c_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_AccuDisChrgC, (double)accu_dis_chrg_c);  

    // kBbms_AccuDisChrgE,          // Accu Discharge Eng (kWh) 累计放电能量 (kWh)
    float accu_dis_chrg_e = bms_bank_accu_dis_chrg_e_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_AccuDisChrgE, (double)accu_dis_chrg_e);  

    // kBbms_LstChEkWh,             // Bank Last Charge Energy (kWh) 最近一次堆充电能量 (kWh)
    float lst_chrg_e = bms_bank_lst_chrg_e_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_LstChEkWh, (double)lst_chrg_e);  

    // kBbms_LstDchEkWh,            // Bank Last Discharge Energy (kWh) 最近一次堆放电能量 (kWh)
    float lst_dch_e = bms_bank_lst_dch_e_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_LstDchEkWh, (double)lst_dch_e);  

    // kBbms_RackEnaNbr,            // Rack Enable Number Rack使能数量
    uint16_t rack_enaNbr = bms_bank_rack_enaNbr_update(bbms_idx); //后续确认，当前使用rbms中的高压失败原因计算
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_RackEnaNbr, (double)rack_enaNbr);  

    // kBbms_RackOnlineNbr,         // Rack Online Number Rack在线数量
    uint16_t rack_onlineNbr = bms_bank_rack_onlineNbr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_RackOnlineNbr, (double)rack_onlineNbr);  


    // kBbms_ConFail,               // HV Connect Fail Reason 高压连接失败原因
    uint16_t con_fail = bms_bank_con_fail_update(bbms_idx); //后续确认，当前rbms中的高压失败原因直接或运算
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_ConFail, (double)con_fail);  

    // kBbms_MaxChrgCap,            // Remaining Charge Cap (Ah) 可充容量 (Ah)
    float max_chrg_cap_ah = bms_bank_max_chrg_cap_ah_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxChrgCap, (double)max_chrg_cap_ah);  

    // kBbms_MaxDisChrgCap,         // Remaining Discharge Cap (Ah) 可放容量 (Ah)
    float max_dis_chrg_cap_ah = bms_bank_max_dis_chrg_cap_ah_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxDisChrgCap, (double)max_dis_chrg_cap_ah);  

    // kBbms_MaxHvBoxTDegC,         // Max Hv Box Temp (℃) 最高高压箱温度 (℃)
    float max_hv_box_t_deg_c = bms_bank_max_hv_box_t_deg_c_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxHvBoxTDegC, (double)max_hv_box_t_deg_c);  

    // kBbms_MinHvBoxTDegC,         // Min Hv Box Temp (℃) 最低高压箱温度 (℃)
    float min_hv_box_t_deg_c = bms_bank_min_hv_box_t_deg_c_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinHvBoxTDegC, (double)min_hv_box_t_deg_c);  

    // kBbms_MaxCellT,              // Max Cell Temp (℃)  最高电芯温度 (℃)
    float max_cell_t_deg_c = bms_bank_max_cell_t_deg_c_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxCellT, (double)max_cell_t_deg_c);  

    // kBbms_MinCellT,              // Min Cell Temp (℃)  最低电芯温度 (℃)
    float min_cell_t_deg_c = bms_bank_min_cell_t_deg_c_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinCellT, (double)min_cell_t_deg_c);  

    // kBbms_AvgCellT,              // Average Cell Temp (℃)  平均电芯温度 (℃)
    float avg_cell_t_deg_c = bms_bank_avg_cell_t_deg_c_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_AvgCellT, (double)avg_cell_t_deg_c);  

    // kBbms_MaxPoleT,              // Max Pole Temp (℃) 最高极柱温度 (℃)
    float max_pole_t_deg_c = bms_bank_max_pole_t_deg_c_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxPoleT, (double)max_pole_t_deg_c);  

    // kBbms_MinPoleT,              // Min Pole Temp (℃) 最低极柱温度 (℃)
    float min_pole_t_deg_c = bms_bank_min_pole_t_deg_c_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinPoleT, (double)min_pole_t_deg_c);  

    // kBbms_MaxPoleTRackNr,        // Max Pole Temp Rack Position 最高极柱温度Rack位置
    uint16_t max_pole_t_rack_nr = bms_bank_max_pole_t_rack_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxPoleTRackNr, (double)max_pole_t_rack_nr);  

    // kBbms_MaxPoleTMdulNr,        // Max Pole Temp Module Position 最高极柱温度模组位置
    uint16_t max_pole_t_mdul_nr = bms_bank_max_pole_t_mdul_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxPoleTMdulNr, (double)max_pole_t_mdul_nr);  

    // kBbms_MaxPoleTMdulInnerNr,   // Max Pole Temp Module Inner Position  最高极柱温度模组内位置
    uint16_t max_pole_t_mdul_inner_nr = bms_bank_max_pole_t_mdul_inner_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MaxPoleTMdulInnerNr, (double)max_pole_t_mdul_inner_nr);  

    // kBbms_MinPoleTRackNr,        // Min Pole Temp Rack Position 最低极柱温度Rack位置
    uint16_t min_pole_t_rack_nr = bms_bank_min_pole_t_rack_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinPoleTRackNr, (double)min_pole_t_rack_nr);  

    // kBbms_MinPoleTMdulNr,        // Min Pole Temp Module Position 最低极柱温度模组位置
    uint16_t min_pole_t_mdul_nr = bms_bank_min_pole_t_mdul_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinPoleTMdulNr, (double)min_pole_t_mdul_nr);  

    // kBbms_MinPoleTMdulInnerNr,   // Min Pole Temp Module Inner Position  最低极柱温度模组内位置
    uint16_t min_pole_t_mdul_inner_nr = bms_bank_min_pole_t_mdul_inner_nr_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_MinPoleTMdulInnerNr, (double)min_pole_t_mdul_inner_nr);  

    //kBbms_SbEMCR_RTCnCMTimeVldFlg = kBbms_A_Selfdr_Start,                  // RTC时间与生产日期有效性标志 -- 后续确认，当前默认无效
    uint16_t rtc_time_vld_flg = bms_bank_rtc_time_vld_flg_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SbEMCR_RTCnCMTimeVldFlg, (double)rtc_time_vld_flg);  

    //kBbms_ScEMCR_CellUsedMonth,                     // 电池使用时长(month) -- 后续确认，当前默认无效
    float cell_used_month = bms_bank_cell_used_month_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_ScEMCR_CellUsedMonth, (double)cell_used_month);  

    //kBbms_ScEMCR_CellDischargeRatePct,              // 电池自放电率(%) -- 后续确认，当前取rbms中的电芯自放电率的最大值
    float cell_discharge_rate_pct = bms_bank_cell_discharge_rate_pct_update(bbms_idx);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_ScEMCR_CellDischargeRatePct, (double)cell_discharge_rate_pct);  

    // kBbms_FaultList_Start,             // Fault List 故障列表  25字节
    for(uint16_t fault_array_id = 0; fault_array_id < kBbms_FaultList_End - kBbms_FaultList_Start; fault_array_id++)
    {
        uint8_t fault_byte_value = bms_get_m_bank_idx_byte_value(fault_array_id); //后续确认，当前从rbms中获取统计的故障状态或的关系
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_FaultList_Start + fault_array_id, (double)fault_byte_value);
    }

    // kBbms_BBMSNo, Bank Number 堆编号 (BBMS_SumInfo / BBMS_Fault byte26)
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_BBMSNo, (double)bbms_idx);

    // kBbms_ACore_FaultList_Start = kBbms_FaultList_End,              //A核障列表结束
    for(uint16_t fault_array_id = 0; fault_array_id < kBbms_ACore_FaultList_End - kBbms_ACore_FaultList_Start; fault_array_id++)
    {
        uint8_t fault_byte_value = bms_get_a_bank_idx_byte_value(fault_array_id); //后续确认，当前直接从bank的故障状态中获取，后续根据需求调整
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_ACore_FaultList_Start + fault_array_id, (double)fault_byte_value);
    }

    //BBMS故障越限次数->需要汇总rbms数据 后续确认，当前采用上位机设置的值
    for(uint16_t flt_ov_ti_nbr_id = 0; flt_ov_ti_nbr_id < 200; flt_ov_ti_nbr_id++)
    {
        uint16_t flt_ov_ti_nbr_value = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, bbms_idx, kBbms_HMI_RackAllFltHistAccuCaliVal_0 + flt_ov_ti_nbr_id); //后续确认，当前直接从rbms中获取统计的故障越限次数，后续根据需求调整
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_BBMS_FltOvTiNbr_Start + flt_ov_ti_nbr_id, (double)flt_ov_ti_nbr_value);
    }

    // // 等间隔位置电芯SOC数组[16] -- 后续确认，暂时不知道是具体哪些电芯的SOC需要上报，当前先预留16个位置，后续根据需求调整
    // kBbms_SaSOCB_BankCellSOCPctxT_0,                // 等间隔位置电芯SOC[0] (%)
    // kBbms_SaSOCB_BankCellSOCPctxT_15 = kBbms_SaSOCB_BankCellSOCPctxT_0 + 15,               // 等间隔位置电芯SOC[15] (%)


    // // 等间隔位置电芯SOC状态数组[16] -- 后续确认，暂时不知道是具体哪些电芯的SOC状态需要上报，当前先预留16个位置，后续根据需求调整
    // kBbms_SaSOCB_BankCellSOCStatexT_0,              // 等间隔位置电芯SOC状态[0] (Nbr)
    // kBbms_SaSOCB_BankCellSOCStatexT_15 = kBbms_SaSOCB_BankCellSOCStatexT_0 + 15,             // 等间隔位置电芯SOC状态[15] (Nbr)

    // // Rack实时SOC数组[最大12]
    // kBbms_SaSOCB_RackRealSOCPct_0,                  // Rack实时SOC[0] (%)
    // kBbms_SaSOCB_RackRealSOCPct_11 = kBbms_SaSOCB_RackRealSOCPct_0 + 11,                 // Rack实时SOC[11] (%)
    uint16_t rack_num = get_rbms_num();
    for(uint16_t rack_soc_id = 0; rack_soc_id < rack_num; rack_soc_id++)
    {
        float rack_real_soc_pct = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_soc_id, kRbms_SoC); //后续确认，当前直接从rbms中获取SOC，后续根据需求调整
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SaSOCB_RackRealSOCPct_0 + rack_soc_id, (double)rack_real_soc_pct);
    }

    // // Rack最大SOC数组[最大12]
    // kBbms_SaSOCB_RackMaxSOCPct_0,                   // Rack最大SOC[0] (%)
    // kBbms_SaSOCB_RackMaxSOCPct_11 = kBbms_SaSOCB_RackMaxSOCPct_0 + 11,                 // Rack最大SOC[11] (%)
    for(uint16_t rack_soc_id = 0; rack_soc_id < rack_num; rack_soc_id++)
    {
        float rack_max_soc_pct = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_soc_id, kRbms_SysMaxSOC); //后续确认，当前直接从rbms中获取最大SOC，后续根据需求调整
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SaSOCB_RackMaxSOCPct_0 + rack_soc_id, (double)rack_max_soc_pct);
    }

    // // Rack最小SOC数组[最大12]
    // kBbms_SaSOCB_RackMinSOCPct_0,                   // Rack最小SOC[0] (%)
    // kBbms_SaSOCB_RackMinSOCPct_11 = kBbms_SaSOCB_RackMinSOCPct_0 + 11,                 // Rack最小SOC[11] (%)
    for(uint16_t rack_soc_id = 0; rack_soc_id < rack_num; rack_soc_id++)
    {
        float rack_min_soc_pct = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_soc_id, kRbms_SysMinSOC); //后续确认，当前直接从rbms中获取最小SOC，后续根据需求调整
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SaSOCB_RackMinSOCPct_0 + rack_soc_id, (double)rack_min_soc_pct);
    }

    // kBbms_A_SOHInfo_Start = kBbms_A_SOCInfo_End,   // 单芯SOH信息开始枚举
    // // 等间隔位置电芯容量数组[16]
    // kBbms_SaSOHB_BankCellCapAhxT_0,                // 等间隔位置电芯容量[0] (Ah)
    // kBbms_SaSOHB_BankCellCapAhxT_15 = kBbms_SaSOHB_BankCellCapAhxT_0 + 15,
    // // 等间隔位置电芯DFCL容量数组[16]
    // kBbms_SaSOHB_DFCLCapResultAhxT_0,               // 等间隔位置电芯DFCL容量[0] (Ah)
    // kBbms_SaSOHB_DFCLCapResultAhxT_15 = kBbms_SaSOHB_DFCLCapResultAhxT_0 + 15,
    // // 等间隔位置电芯MFCL容量数组[16]
    // kBbms_SaSOHB_MFCLCapResultAhxT_0,               // 等间隔位置电芯MFCL容量[0] (Ah)
    // kBbms_SaSOHB_MFCLCapResultAhxT_15 = kBbms_SaSOHB_MFCLCapResultAhxT_0 + 15,

    // // Rack实时容量数组[最大12] -- 后续确认，当前采用最近一次充电容量，后续根据需求调整
    // kBbms_SaSOHB_RackRealCapAh_0,                   // Rack实时容量[0] (Ah)
    // kBbms_SaSOHB_RackRealCapAh_11 = kBbms_SaSOHB_RackRealCapAh_0 + 11,
    for(uint16_t rack_cap_id = 0; rack_cap_id < rack_num; rack_cap_id++)
    {
        float rack_real_cap_ah = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_cap_id, kRbms_LstChCapAh); 
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SaSOHB_RackRealCapAh_0 + rack_cap_id, (double)rack_real_cap_ah);
    }

    // // Rack最大容量数组[最大12]
    // kBbms_SaSOHB_RackMaxCapAh_0,                    // Rack最大容量[0] (Ah)
    // kBbms_SaSOHB_RackMaxCapAh_11 = kBbms_SaSOHB_RackMaxCapAh_0 + 11,
    for(uint16_t rack_cap_id = 0; rack_cap_id < rack_num; rack_cap_id++)
    {
        float rack_real_cap_ah = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_cap_id, kRbms_RealSysCapAh); 
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SaSOHB_RackMaxCapAh_0 + rack_cap_id, (double)rack_real_cap_ah);
    }

    // // Rack最小容量数组[最大12] -- 后续确认，当前采用最近一次放电容量，后续根据需求调整
    // kBbms_SaSOHB_RackMinCapAh_0,                    // Rack最小容量[0] (Ah)
    // kBbms_SaSOHB_RackMinCapAh_11 = kBbms_SaSOHB_RackMinCapAh_0 + 11,
    for(uint16_t rack_cap_id = 0; rack_cap_id < rack_num; rack_cap_id++)
    {
        float rack_real_cap_ah = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, rack_cap_id, kRbms_LstDchCapAh); 
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_SaSOHB_RackMinCapAh_0 + rack_cap_id, (double)rack_real_cap_ah);
    }






    // kBbms_A_SOHInfo_End,                            // 单芯SOH信息结束枚举

}








