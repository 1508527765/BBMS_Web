/*****************************************************************************
 * @copyright       1997-2050, gary. POWER SUPPLY CO., LTD.
 * @file            app_bms_fault.c
 * @brief           故障检测功能
 * @author          gary
 * @date            2026-04-22
 * @remark
 *****************************************************************************/

#include "app_bms_fault.h"
//降流系数
static uint16_t derating_ratio = 100;
extern bbms_ctrl_t bbms_ctrl;

RBMS_Fault_t rbms_fault[MAX_CLUSTER_NUM];  // RBMS故障全局变量

/****************************************************全局数据定义区**************************************************************/

BBMS_M_Fault_t bms_m_fault;
BBMS_A_Fault_t bms_a_fault;

double m_fault_values[kBmsMFault_End - kBmsMFault_Start] = {0.0};
double a_fault_values[kBmsAFault_End - kBmsAFault_Start] = {0.0};

static void sync_a_fault_to_rtdb(uint16_t bbms_idx);
static void sync_m_fault_to_rtdb(uint16_t bbms_idx);

/*****************************************************************************
 * M核故障状态数组（运行时）
 *****************************************************************************/
static fault_state_t m_fault_states[kBmsMFault_End - kBmsMFault_Start] = {0};


/*****************************************************************************
 * A核故障状态数组（运行时）
 *****************************************************************************/
static fault_state_t a_fault_states[kBmsAFault_End - kBmsAFault_Start] = {0};

/*****************************************************************************
 * M核故障配置数组 -- 数据待补充
 *****************************************************************************/
static const fault_config_t m_fault_configs[kBmsMFault_End - kBmsMFault_Start] = {
    /* 故障ID                                 故障使能                          故障等级          处理措施                  触发阈值     恢复阈值  触发持续时间(ms) 恢复持续时间(ms) 比较条件 降流比例（0-100）*/
    // //bms类故障
    {kBmsMFault_AllRackCommLost,            kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     30000,               0,               kCompare_GreaterEqual ,0}, // 全部Rack通信丢失
    {kBmsMFault_OneRackCommLost,            kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     30000,               0,               kCompare_GreaterEqual ,0}, // 一个Rack通信丢失
    //系统类
    {kBmsMFault_AllCluDeltaTempOver,        kFaultEnable_Enabled,        kFaultLevel_2,        kFaultHandle_Derating,      15.0,      10.0,     10000,             10000,               kCompare_GreaterEqual ,50}, // 所有簇温差过大故障
    /*故障列表，后续补充下配置*/
    {kBmsMFault_CluDeltaCurrOverLv1,        kFaultEnable_Disabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      70.0,      0.0,     5000,              0,               kCompare_GreaterEqual ,0}, // 电池簇间环流越限1级
    {kBmsMFault_CluDeltaCurrOverLv2,        kFaultEnable_Disabled,        kFaultLevel_2,        kFaultHandle_Derating,      50.0,      30.0,     5000,             5000,               kCompare_GreaterEqual ,50}, // 电池簇间环流越限2级
    {kBmsMFault_CluDeltaCurrOverLv3,        kFaultEnable_Enabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      30.0,      20.0,     5000,             5000,               kCompare_GreaterEqual ,100}, // 电池簇间环流越限3级
    // //bms故障
    {kBmsMFault_PCSCommLost,                kFaultEnable_Disabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     10000,              0,               kCompare_GreaterEqual ,0}, // PCS通信丢失故障
    // //系统类故障
    {kBmsMFault_CluVoltDiffExces,           kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      20.0,      15.0,    3000,               3000,               kCompare_GreaterEqual ,0}, // 簇间压差过大故障
    {kBmsMFault_IDLearnFail,                kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     100,                 100,               kCompare_GreaterEqual ,0}, // IDLearm编码失败
    // //外设类故障
    {kBmsMFault_WaterCoolModeMismatch,      kFaultEnable_Enabled,        kFaultLevel_2,        kFaultHandle_Derating,      1.0,      0.0,     300000,              120000,               kCompare_GreaterEqual ,50}, // 水冷机组工作模式不匹配故障
    {kBmsMFault_WaterCoolLvl1Fault,         kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     5000,                0,               kCompare_GreaterEqual ,0}, // 水冷机一级故障
    {kBmsMFault_WaterCoolLvl2Fault,         kFaultEnable_Enabled,        kFaultLevel_2,        kFaultHandle_Derating,      1.0,      0.0,     5000,                2000,               kCompare_GreaterEqual ,50}, // 水冷机二级故障
    {kBmsMFault_TMSCommLost,                kFaultEnable_Enabled,        kFaultLevel_2,        kFaultHandle_Derating,      1.0,      0.0,     10000,               10000,               kCompare_GreaterEqual ,50}, // TMS通讯丢失
    // //bms故障
    {kBmsMFault_24VVoltOutRange,            kFaultEnable_Disabled,        kFaultLevel_2,        kFaultHandle_Derating,      32.0,      24.0,     1000,             1000,               kCompare_GreaterEqual ,50}, // 24V电压超范围故障
    {kBmsMFault_12VVoltOutRange,            kFaultEnable_Disabled,        kFaultLevel_2,        kFaultHandle_Derating,      18.0,      12.0,     1000,              1000,               kCompare_GreaterEqual ,50}, // 12V电压超范围故障
    // //系统类故障
    {kBmsMFault_OverPower,                  kFaultEnable_Disabled,        kFaultLevel_3,       kFaultHandle_AlarmOnly,      1.0,      0.0,     10000,              10000,               kCompare_GreaterEqual ,100}, // 过功率故障
    {kBmsMFault_RackSOCDiffExces,           kFaultEnable_Enabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      40.0,     25.0,     10000,             0,               kCompare_GreaterEqual ,100}, // Rack间SOC差距过大故障
    // //外设类故障
    {kBmsMFault_DoorAccessFault,            kFaultEnable_Enabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     3000,               3000,               kCompare_GreaterEqual ,100}, // 门禁故障
    {kBmsMFault_QF1CircuitBrkFault,         kFaultEnable_Enabled,        kFaultLevel_2,        kFaultHandle_Derating,      1.0,      0.0,     3000,                3000,               kCompare_GreaterEqual ,50}, // QF1断路器故障
    {kBmsMFault_SPD2SurgeFault,            kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,                 0,               kCompare_GreaterEqual ,0}, // SPD2过压故障
    {kBmsMFault_WaterLeakAlarm,             kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     30000,               0,               kCompare_GreaterEqual ,0}, // 水浸告警故障
    {kBmsMFault_CombGasAlarm,               kFaultEnable_Enabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     3000,               3000,               kCompare_GreaterEqual ,100}, // 可燃气体告警故障
    {kBmsMFault_ExproFanAlarm,              kFaultEnable_Enabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     3000,               3000,               kCompare_GreaterEqual ,100}, // 防爆风机告警故障
    {kBmsMFault_EmergStopFault,             kFaultEnable_Disabled,       kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,                0,               kCompare_GreaterEqual ,0}, // 急停故障
    {kBmsMFault_FireExtremeAlarm,           kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,               0,               kCompare_GreaterEqual ,0}, // 消防严重告警
    {kBmsMFault_FireModerateAlarm,          kFaultEnable_Enabled,        kFaultLevel_2,        kFaultHandle_Derating,      1.0,      0.0,     3000,                3000,               kCompare_GreaterEqual ,50}, // 消防中度告警
    {kBmsMFault_FireFault,                  kFaultEnable_Enabled,        kFaultLevel_2,        kFaultHandle_Derating,      1.0,      0.0,     3000,                3000,               kCompare_GreaterEqual ,50}, // 消防故障
    {kBmsMFault_UPSCommLost,                kFaultEnable_Enabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     30000,              5000,               kCompare_GreaterEqual ,100}, // UPS通讯节点丢失
    {kBmsMFault_EnergyMeterCommLost,        kFaultEnable_Enabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     30000,              5000,               kCompare_GreaterEqual ,100}, // 能量表通讯节点丢失
    {kBmsMFault_PCSEmergStopFault,          kFaultEnable_Disabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,               0,               kCompare_GreaterEqual ,0}, // PCS急停故障
    {kBmsMFault_ContainerEmergStop,         kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,                0,               kCompare_GreaterEqual ,0}, // 集装箱急停故障
    {kBmsMFault_DistrCabEmergStop,          kFaultEnable_Disabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,               0,               kCompare_GreaterEqual ,0}, // 配电柜急停故障
    {kBmsMFault_EMSCommLost,                kFaultEnable_Disabled,       kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     30000,              0,               kCompare_GreaterEqual ,0}, // EMS通讯节点丢失
    {kBmsMFault_BatCompDehumidCommLost,     kFaultEnable_Enabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     30000,              5000,               kCompare_GreaterEqual ,100}, //电池舱除湿机通讯节点丢失故障
    {kBmsMFault_MainsPowerAbnLvl1,          kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     6840000,            30000,               kCompare_GreaterEqual ,0}, // 市电异常一级故障
    {kBmsMFault_FireAlarmActAbn,            kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,                0,               kCompare_GreaterEqual ,0}, // 消防火警启动异常
    {kBmsMFault_CtrlCabDCSurgeFault,        kFaultEnable_Disabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,               0,               kCompare_GreaterEqual ,0}, // 中控柜直流SPD浪涌故障
    {kBmsMFault_BackupPowerSPD1Fault,       kFaultEnable_Disabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,               0,               kCompare_GreaterEqual ,0}, // 备电SPD1故障
    {kBmsMFault_CustExtEmergStop,           kFaultEnable_Disabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,               0,               kCompare_GreaterEqual ,0}, // 客户外部急停
    {kBmsMFault_WaterCoolLvl3Fault,         kFaultEnable_Enabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     3000,               3000,               kCompare_GreaterEqual ,100}, // 水冷机三级故障
    // //bms类故障
    {kBmsMFault_ACoreCommFault,             kFaultEnable_Enabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     30000,              30000,               kCompare_GreaterEqual ,100}, // A核通讯故障
    {kBmsMFault_OneRackCanCommLost,         kFaultEnable_Disabled,       kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     30000,             30000,               kCompare_GreaterEqual ,100}, // 至少一个RBMS的CAN通讯丢失
    {kBmsMFault_AllRackCanCommLost,         kFaultEnable_Disabled,       kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     30000,             30000,               kCompare_GreaterEqual ,100}, // 所有RBMS的CAN通讯丢失
    // //外设类故障
    {kBmsMFault_ConvFuseFault,              kFaultEnable_Disabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,                0,               kCompare_GreaterEqual ,0}, // 汇流熔断器故障
    {kBmsMFault_CtrlCabACCommLost,          kFaultEnable_Enabled,        kFaultLevel_3,         kFaultHandle_AlarmOnly,      1.0,      0.0,     30000,              5000,               kCompare_GreaterEqual ,100}, // 中控柜空调/除湿机通讯节点丢失
    {kBmsMFault_CtrlCabTravelSwFault,       kFaultEnable_Disabled,        kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     3000,               3000,               kCompare_GreaterEqual ,100}, // 中控柜行程开关故障
    {kBmsMFault_FireFightCommLost,          kFaultEnable_Disabled,       kFaultLevel_3,        kFaultHandle_AlarmOnly,      1.0,      0.0,     30000,              3000,               kCompare_GreaterEqual ,100}, // 消防通讯节点丢失故障
    {kBmsMFault_MainsPowerAbnLvl3,          kFaultEnable_Enabled,        kFaultLevel_3,         kFaultHandle_AlarmOnly,      1.0,      0.0,     3000,               3000,               kCompare_GreaterEqual ,100},// 市电异常三级故障
    // //系统类故障
    {kBmsMFault_HVBoxConvReverse,           kFaultEnable_Enabled,        kFaultLevel_1,        kFaultHandle_Shutdown,      1.0,      0.0,     3000,                0,               kCompare_GreaterEqual ,0}, // 高压盒与汇流柜反接故障
};


/*****************************************************************************
 * A核故障配置数组 -- 数据待补充
 *****************************************************************************/
static const fault_config_t a_fault_configs[kBmsAFault_End - kBmsAFault_Start] = {
    /* 故障ID                         故障使能                  故障等级          处理措施          触发阈值  恢复阈值  触发持续时间(ms) 恢复持续时间(ms)      比较条件 降流比例（0-100）*/
    {kBmsAFault_SSDFault,              kFaultEnable_Enabled, kFaultLevel_3,     kFaultHandle_AlarmOnly,     1.0,      0.0,        3000,          3000,        kCompare_GreaterEqual ,100},
    {kBmsAFault_SDFault,               kFaultEnable_Enabled, kFaultLevel_3,     kFaultHandle_AlarmOnly,     1.0,      0.0,        3000,          3000,        kCompare_GreaterEqual ,100},
    {kBmsAFault_MCoreCommFault,        kFaultEnable_Enabled, kFaultLevel_3,     kFaultHandle_AlarmOnly,      1.0,      0.0,        30000,         30000,       kCompare_GreaterEqual ,100},
    {kBmsAFault_EMSCommLost,           kFaultEnable_Enabled, kFaultLevel_3,     kFaultHandle_AlarmOnly,      1.0,      0.0,        30000,         30000,       kCompare_GreaterEqual ,100},
    {kBmsAFault_OneRackLanCommLost,    kFaultEnable_Disabled, kFaultLevel_3,     kFaultHandle_AlarmOnly,      1.0,      0.0,        30000,         30000,       kCompare_GreaterEqual ,100},
    {kBmsAFault_AllRackLanCommLost,    kFaultEnable_Disabled, kFaultLevel_3,     kFaultHandle_AlarmOnly,      1.0,      0.0,        30000,         30000,       kCompare_GreaterEqual ,100},
    {kBmsAFault_OneRackCommLost,       kFaultEnable_Enabled, kFaultLevel_3,     kFaultHandle_AlarmOnly,      1.0,      0.0,        30000,         30000,       kCompare_GreaterEqual ,100},
    {kBmsAFault_AllRackCommLost,       kFaultEnable_Enabled, kFaultLevel_3,     kFaultHandle_AlarmOnly,      1.0,      0.0,        30000,         30000,       kCompare_GreaterEqual ,100},
};



/*****************************************************************************
 * @brief       获取Bank最高故障等级
 * @param[in]   bbms_idx: 为模拟堆预留，从1开始
 * @return      最高故障等级
 *****************************************************************************/
fault_level_e bms_get_bank_max_flt_level(uint16_t bbms_idx)
{
    fault_level_e max_flt_level = kFaultLevel_No_Fault;

    //m_fault_configs数组中获取故障故障等级
    for(uint16_t i = 0; i < kBmsMFault_End - kBmsMFault_Start; i++)
    {
        if(m_fault_values[i] == 1)
        {
            fault_level_e fault_level = m_fault_configs[i].level;
            if(fault_level < max_flt_level)
            {
                max_flt_level = fault_level;
            }
        }
    }

    //a_fault_configs数组中获取故障故障等级
    for(uint16_t i = 0; i < kBmsAFault_End - kBmsAFault_Start; i++)
    {
        if(a_fault_values[i] == 1)
        {
            fault_level_e fault_level = a_fault_configs[i].level;
            if(fault_level < max_flt_level)
            {
                max_flt_level = fault_level;
            }
        }
    }


    return max_flt_level;
}


/*****************************************************************************
 * @brief       获取rbms数量
 *****************************************************************************/
// static uint16_t get_rbms_num(void)
// {
//     // 从RTDB获取RBMS数量的示例
//     uint16_t rbms_count = 0;
//     rbms_count = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_Con, 0, kCon_RBMSNum);
//     return rbms_count;
// }


/*****************************************************************************
 * @brief       更新M核故障值 -- 数据待补充
 *****************************************************************************/
void bms_update_m_fault_value(void)
{
    //bms类故障
    uint16_t rbms_num = get_rbms_num();
    uint16_t oneRackLanCommLostCount = 0;
    for (size_t i = 0; i < rbms_num; i++)
    {
        uint8_t rbms_online_status = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_Online_State);
        if (rbms_online_status == 1) // 假设1表示在线，0表示离线
        {
            oneRackLanCommLostCount++;
        }
    }
    if(oneRackLanCommLostCount == 0)
    {
        m_fault_values[kBmsMFault_AllRackCommLost] = 1;  // 所有RBMS通讯丢失
    }
    else if(oneRackLanCommLostCount < rbms_num)
    {
        m_fault_values[kBmsMFault_AllRackCommLost] = 0;  // 所有RBMS通讯丢失
        m_fault_values[kBmsMFault_OneRackCommLost] = 1;  // 至少一个RBMS通讯丢失
    }
    else if(oneRackLanCommLostCount == rbms_num)
    {
        m_fault_values[kBmsMFault_AllRackCommLost] = 0;  // 所有RBMS通讯丢失
        m_fault_values[kBmsMFault_OneRackCommLost] = 0;  // 至少一个RBMS通讯丢失
    }


    // //系统类故障

    //统计簇温差最大值，最小值
    double maxCluDeltaTemp = -1000.0;
    double minCluDeltaTemp = 1000.0;
    for (size_t i = 0; i < rbms_num; i++)
    {
        double cluDeltaTempMax = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_HvBoxMaxTemp);
        if(cluDeltaTempMax > maxCluDeltaTemp)
        {
            maxCluDeltaTemp = cluDeltaTempMax;
        }
        double cluDeltaTempMin = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_HvBoxMinTemp);
        if(cluDeltaTempMin < minCluDeltaTemp)
        {
            minCluDeltaTemp = cluDeltaTempMin;
        }
    }
    double cluDeltaTemp = maxCluDeltaTemp - minCluDeltaTemp;
    m_fault_values[kBmsMFault_AllCluDeltaTempOver] = cluDeltaTemp;  // 所有簇温差过大故障

    //获取簇间环流最大值，最小值
    double maxCluDeltaCurr = -1000.0;
    double minCluDeltaCurr = 1000.0;
    for (size_t i = 0; i < rbms_num; i++)
    {
        double cluDeltaCurrMax = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_A_HighAccu);
        if(cluDeltaCurrMax > maxCluDeltaCurr)
        {
            maxCluDeltaCurr = cluDeltaCurrMax;
        }
        double cluDeltaCurrMin = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_A_HighAccu);
        if(cluDeltaCurrMin < minCluDeltaCurr)
        {
            minCluDeltaCurr = cluDeltaCurrMin;
        }
    }
    double cluDeltaCurr = maxCluDeltaCurr - minCluDeltaCurr;
    m_fault_values[kBmsMFault_CluDeltaCurrOverLv1] = cluDeltaCurr;  // 电池簇间环流越限1级
    m_fault_values[kBmsMFault_CluDeltaCurrOverLv2] = cluDeltaCurr;  // 电池簇间环流越限2级
    m_fault_values[kBmsMFault_CluDeltaCurrOverLv3] = cluDeltaCurr;  // 电池簇间环流越限3级
    
    //bms故障
    m_fault_values[kBmsMFault_PCSCommLost] = 0;  // PCS通信丢失故障 -- 后续追加

    //统计rbms之间的直流电压差
    double maxCluVoltDiff = -1000.0;
    for (size_t i = 0; i < rbms_num; i++)
    {
        for (size_t j = i + 1; j < rbms_num; j++)
        {
            double cluVoltDiff = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_DCBusV) - getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, j, kRbms_DCBusV);
            if(cluVoltDiff > maxCluVoltDiff)
            {
                maxCluVoltDiff = cluVoltDiff;
            }
        }
    }

    m_fault_values[kBmsMFault_CluVoltDiffExces] = maxCluVoltDiff;  // 簇间压差过大故障


    m_fault_values[kBmsMFault_IDLearnFail] = 0;  // IDLearm编码失败--后续追加

    // //外设类故障
    m_fault_values[kBmsMFault_WaterCoolModeMismatch] = 0;    // 水冷机组工作模式不匹配故障--后续追加
    m_fault_values[kBmsMFault_WaterCoolLvl1Fault] = 0;       // 水冷机一级故障--后续追加
    m_fault_values[kBmsMFault_WaterCoolLvl2Fault] = 0;       // 水冷机二级故障--后续追加

    uint16_t tms_online_status = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_Lqd, 0, kLqd_Online);
    m_fault_values[kBmsMFault_TMSCommLost] = (tms_online_status == 0) ? 1 : 0;              // TMS通讯丢失

    // //bms故障
    m_fault_values[kBmsMFault_24VVoltOutRange] = 0;          // 24V电压超范围故障--没有采样， 故障永远为否
    m_fault_values[kBmsMFault_12VVoltOutRange] = 0;          // 12V电压超范围故障--=没有采样， 故障永远为否
    
    // //系统类故障
    bms_charge_status_e charge_status = get_bms_charge_status();
    float Current = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kRbms_A_HighAccu);
    float maxChargeCurrent = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kRbms_SoFContChrgCurr);
    float maxDischargeCurrent = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kRbms_SoFContDisChrgCurr);

    if(charge_status == kBms_ChargeStatus_Discharge)
    {
        if(KIT_ABS(Current) > maxDischargeCurrent)
        {
            m_fault_values[kBmsMFault_OverPower] = 1;                // 过功率故障--放电状态且电流超过放电最大电流
        }
        else
        {
            m_fault_values[kBmsMFault_OverPower] = 0;                // 过功率故障--放电状态但电流未超过放电最大电流
        }
    } 
    else if(charge_status == kBms_ChargeStatus_Charge)
    {
        if(KIT_ABS(Current) > maxChargeCurrent)
        {
            m_fault_values[kBmsMFault_OverPower] = 1;                // 过功率故障--充电状态且电流超过充电最大电流
        }
        else
        {
            m_fault_values[kBmsMFault_OverPower] = 0;                // 过功率故障--充电状态但电流未超过充电最大电流
        }
    }
    else
    {
        if(KIT_ABS(Current) > KIT_MAX(maxChargeCurrent, maxDischargeCurrent))
        {
            m_fault_values[kBmsMFault_OverPower] = 1;                // 过功率故障--非充放电状态但电流超过最大电流
        }
        else
        {
            m_fault_values[kBmsMFault_OverPower] = 0;                // 过功率故障--非充放电状态且电流未超过最大电流
        }
    }

     //统计rbms之间的soc差距
    

    double maxSocDiff = -1000.0;
    for (size_t i = 0; i < rbms_num; i++)
    {
        for (size_t j = i + 1; j < rbms_num; j++)
        {
            double socDiff = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SoC) - getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, j, kRbms_SoC);
            if(socDiff > maxSocDiff)
            {
                maxSocDiff = socDiff;
            }
        }
    }
    m_fault_values[kBmsMFault_RackSOCDiffExces] = maxSocDiff;  // kBmsMFault_RackSOCDiffExces,         // Rack间SOC差距过大故障

    // //外设类故障

    m_fault_values[kBmsMFault_DoorAccessFault] = get_di_signal_value(kDi_Signal_Door_AlarmIII);                                     // 门禁故障
    m_fault_values[kBmsMFault_QF1CircuitBrkFault] = get_di_signal_value(kDi_Signal_QF1_Feedback_AlarmII);                           // QF1断路器故障
    m_fault_values[kBmsMFault_SPD2SurgeFault] = get_di_signal_value(kDi_Signal_ACSPD1_AlarmI);                                      // SPD2浪涌故障
    m_fault_values[kBmsMFault_WaterLeakAlarm] = get_di_signal_value(kDi_Signal_Water_AlarmI);                                       // 水浸告警故障
    m_fault_values[kBmsMFault_CombGasAlarm] = get_di_signal_value(kDi_Signal_Gas_AlarmI);                                           // 可燃气体告警故障
    m_fault_values[kBmsMFault_ExproFanAlarm] = get_di_signal_value(kDi_Signal_Fan_FaultII);                                         // 防爆风机告警故障
    m_fault_values[kBmsMFault_EmergStopFault] = get_di_signal_value(kDi_Signal_Container_EStopI);                                   // 急停故障
    m_fault_values[kBmsMFault_FireExtremeAlarm] = get_di_signal_value(kDi_Signal_Fire_AlarmI);                                      // 消防严重告警
    m_fault_values[kBmsMFault_FireModerateAlarm] = get_di_signal_value(kDi_Signal_Fire_AlarmII);                                    // 消防中度告警
    m_fault_values[kBmsMFault_FireFault] = get_di_signal_value(kDi_Signal_Fire_Fault);                                              // 消防故障
    m_fault_values[kBmsMFault_UPSCommLost] = getRtdbPointValue(Rtdb_ShMem, kDev_Type_UPS, 0, kUPS_Online);                          // UPS通讯节点丢失故障
    m_fault_values[kBmsMFault_EnergyMeterCommLost] = getRtdbPointValue(Rtdb_ShMem, kDev_Type_Meter, 0, kMeter_Online);              // 电表通讯节点丢失故障
    m_fault_values[kBmsMFault_PCSEmergStopFault] = get_di_signal_value(kDi_Signal_PCS_EStopI);                                      // PCS急停故障 -- 文档缺失--后续追加
    m_fault_values[kBmsMFault_ContainerEmergStop] = get_di_signal_value(kDi_Signal_Container_EStopI);                               // 集装箱急停故障
    m_fault_values[kBmsMFault_DistrCabEmergStop] = get_di_signal_value(kDi_Signal_Power_EStopI);                                    // 配电柜急停故障 -- 文档缺失--使用配电柜急停一级告警代替
    m_fault_values[kBmsMFault_EMSCommLost] = 0;                                                                                     // EMS通讯节点丢失故障 -- 后续追加
    m_fault_values[kBmsMFault_BatCompDehumidCommLost] = getRtdbPointValue(Rtdb_ShMem, kDev_Type_dehumidifier, 0, 0);                // 电池舱除湿机通讯节点丢失故障 -- 后续追加
    m_fault_values[kBmsMFault_MainsPowerAbnLvl1] = getRtdbPointValue(Rtdb_ShMem, kDev_Type_dehumidifier, 0, 0);                     // 市电异常一级故障 -- 后续追加-在UPS模型中
    m_fault_values[kBmsMFault_FireAlarmActAbn] = get_di_signal_value(kDi_Signal_Fire_AlarmI);                                       // 消防火警启动异常 -- 文档缺失，暂用消防一级告警代替
    m_fault_values[kBmsMFault_CtrlCabDCSurgeFault] = get_di_signal_value(kDi_Signal_BMS_DCSPD_FaultI);                              // 中控柜直流SPD浪涌故障 
    m_fault_values[kBmsMFault_BackupPowerSPD1Fault] = get_di_signal_value(kDi_Signal_BackupPowerSPD1Fault);                         // 备电SPD1故障 -- 文档没有--用DI信号备电SPD1故障一级告警代替
    m_fault_values[kBmsMFault_CustExtEmergStop] = get_di_signal_value(kDi_Signal_CustExtEmergStop);                                 // 客户外部急停 -- 文档没有--用DI信号客户外部急停一级告警代替
    m_fault_values[kBmsMFault_WaterCoolLvl3Fault] = getRtdbPointValue(Rtdb_ShMem, kDev_Type_Lqd, 0, kLqd_FaultLevel) > 2 ? 1 : 0;   // 水冷机三级故障

    // //bms类故障
    m_fault_values[kBmsMFault_ACoreCommFault] = 0;           // A核通讯故障
    m_fault_values[kBmsMFault_OneRackCanCommLost] = 0;       // 至少一个RBMS的CAN通讯丢失
    m_fault_values[kBmsMFault_AllRackCanCommLost] = 0;       // 所有RBMS的CAN通讯丢失

    // //外设类故障
    m_fault_values[kBmsMFault_ConvFuseFault] = get_di_signal_value(kDi_Signal_Fuse_FaultI);                                               // 汇流熔断器故障一级告警
    m_fault_values[kBmsMFault_CtrlCabACCommLost] =  getRtdbPointValue(Rtdb_ShMem, kDev_Type_Lqd, 0, kLqd_Online) > 0 ? 0 : 1;             // 中控柜空调/除湿机通讯节点丢失故障 -- 使用水冷机在线状态代替
    m_fault_values[kBmsMFault_CtrlCabTravelSwFault] = get_di_signal_value(kDi_Signal_Crtl_Switch_AlarmIII);                               // 中控柜行程开关故障
    m_fault_values[kBmsMFault_FireFightCommLost] = get_di_signal_value(kDi_Signal_Crtl_Switch_AlarmIII);                                  // 消防通讯节点丢失故障
    m_fault_values[kBmsMFault_MainsPowerAbnLvl3] = getRtdbPointValue(Rtdb_ShMem, kDev_Type_UPS, 0, kUPS_MainsPowerAbnFault) > 0 ? 1 : 0;  // 市电异常三级故障 -- 在UPS模型中
    
    // //系统类故障
    m_fault_values[kBmsMFault_HVBoxConvReverse] = 0;  // 高压盒与汇流柜反接故障 -- 文档缺失，后续补充


}


/*****************************************************************************
 * @brief       更新A核故障值 -- 数据待补充
 *****************************************************************************/
void bms_update_a_fault_value(void)
{

    // 更新所有故障值,后续追加
    a_fault_values[kBmsAFault_SSDFault] = 0;
    a_fault_values[kBmsAFault_SSDFault] = 0;    // SSD故障
    a_fault_values[kBmsAFault_SDFault] = 0;     // SD故障
    a_fault_values[kBmsAFault_MCoreCommFault] = 0;          // M核通讯故障


    a_fault_values[kBmsAFault_EMSCommLost] = 0;             // EMS通讯节点丢失故障 --- 后续补充

    //统计rbms的在线状态
    uint16_t rbms_num = get_rbms_num();
    uint16_t oneRackLanCommLostCount = 0;
    for (size_t i = 0; i < rbms_num; i++)
    {
        uint8_t rbms_online_status = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_Online_State);
        if (rbms_online_status == 1) // 假设1表示在线，0表示离线
        {
            oneRackLanCommLostCount++;
        }
    }
    if(oneRackLanCommLostCount < rbms_num)
    {
        a_fault_values[kBmsAFault_OneRackLanCommLost] = 1; // 至少一个RBMS的LAN通讯丢失
    }
    else
    {
        a_fault_values[kBmsAFault_OneRackLanCommLost] = 0;
    }

    if(oneRackLanCommLostCount == 0)
    {
        a_fault_values[kBmsAFault_AllRackLanCommLost] = 1; // 所有RBMS的LAN通讯丢失
    }
    else
    {
        a_fault_values[kBmsAFault_AllRackLanCommLost] = 0;
    }
  
    a_fault_values[kBmsAFault_OneRackCommLost] = a_fault_values[kBmsAFault_OneRackLanCommLost]; // 至少一个RBMS通讯丢失 先代替后续can通讯补充完毕后再修改
    a_fault_values[kBmsAFault_AllRackCommLost] = a_fault_values[kBmsAFault_AllRackLanCommLost];// 所有RBMS通讯丢失 先代替后续can通讯补充完毕后再修改
}




/*****************************************************************************
 * @brief       获取M核故障数据源值
 *****************************************************************************/
static double get_m_fault_data_value(uint16_t fault_id)
{
    if (fault_id >= kBmsMFault_End || fault_id < kBmsMFault_Start) 
    {
        return 0.0;
    }

    return m_fault_values[fault_id];
}


/*****************************************************************************
 * @brief       获取A核故障数据源值
 *****************************************************************************/
static double get_a_fault_data_value(uint16_t fault_id)
{
    if (fault_id >= kBmsAFault_End || fault_id < kBmsAFault_Start) 
    {
        return 0.0;
    }

    return a_fault_values[fault_id];
}


/*****************************************************************************
 * @brief       比较数据值是否符合条件
 *****************************************************************************/
static bool compare_value(double value, double threshold, compare_condition_e cond)
{
    switch (cond) {
        case kCompare_GreaterThan:     return value > threshold;
        case kCompare_LessThan:        return value < threshold;
        case kCompare_Equal:           return fabs(value - threshold) < 0.0001;
        case kCompare_NotEqual:        return fabs(value - threshold) >= 0.0001;
        case kCompare_GreaterEqual:    return value >= threshold;
        case kCompare_LessEqual:       return value <= threshold;
        default:                       return false;
    }
}


/*****************************************************************************
 * @brief       获取当前时间戳(ms)
 *****************************************************************************/
static uint32_t get_current_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}


/*****************************************************************************
 * @brief       检测单个M核故障
 *****************************************************************************/
static void detect_m_fault(uint16_t fault_id)
{
    if (fault_id >= kBmsMFault_End || fault_id < kBmsMFault_Start) 
    {
        return;
    }

    uint16_t index = fault_id - kBmsMFault_Start;
    const fault_config_t *config = &m_fault_configs[index];

    // 检查故障使能设置
    if(config->enable == kFaultEnable_Disabled)
    {
        return;
    }

    fault_state_t *state = &m_fault_states[index];

    double current_value = get_m_fault_data_value(fault_id);
    uint32_t current_time = get_current_time_ms();

    // 检查是否满足故障触发条件
    bool trigger_condition = compare_value(current_value, config->trigger_threshold, config->compare_cond);

    if (trigger_condition) 
    {
        if (!state->trigger_timer_active) 
        {
            state->trigger_timer_active = true;
            state->fault_start_time = current_time;
            state->recover_timer_active = false;
        }

        // 检查是否达到触发时间
        if (current_time - state->fault_start_time >= config->trigger_time_ms) 
        {
            state->is_fault = true;
        }
    } 
    else 
    {
        state->trigger_timer_active = false;

        // 如果当前处于故障状态，检查恢复条件
        if (state->is_fault) 
        {
            bool recover_condition = compare_value(current_value, config->recover_threshold, config->compare_cond);

            if (recover_condition) 
            {
                if (!state->recover_timer_active) 
                {
                    state->recover_timer_active = true;
                    state->recover_start_time = current_time;
                }

                // 检查是否达到恢复时间
                if (current_time - state->recover_start_time >= config->recover_time_ms) 
                {
                    state->is_fault = false;
                    state->recover_timer_active = false;
                }
            } 
            else 
            {
                state->recover_timer_active = false;
            }
        }
    }
}


/*****************************************************************************
 * @brief       检测单个A核故障
 *****************************************************************************/
static void detect_a_fault(uint16_t fault_id)
{
    if (fault_id >= kBmsAFault_End || fault_id < kBmsAFault_Start) 
    {
        return;
    }

    uint16_t index = fault_id - kBmsAFault_Start;
    const fault_config_t *config = &a_fault_configs[index];

    // 检查故障使能设置
    if(config->enable == kFaultEnable_Disabled)
    {
        return;
    }

    fault_state_t *state = &a_fault_states[index];

    double current_value = get_a_fault_data_value(fault_id);
    uint32_t current_time = get_current_time_ms();

    // 检查是否满足故障触发条件
    bool trigger_condition = compare_value(current_value, config->trigger_threshold, config->compare_cond);

    if (trigger_condition) 
    {
        if (!state->trigger_timer_active) 
        {
            state->trigger_timer_active = true;
            state->fault_start_time = current_time;
            state->recover_timer_active = false;
        }

        // 检查是否达到触发时间
        if (current_time - state->fault_start_time >= config->trigger_time_ms) 
        {
            state->is_fault = true;
        }
    } 
    else 
    {
        state->trigger_timer_active = false;

        // 如果当前处于故障状态，检查恢复条件
        if (state->is_fault) 
        {
            bool recover_condition = compare_value(current_value, config->recover_threshold, config->compare_cond);

            if (recover_condition) 
            {
                if (!state->recover_timer_active) 
                {
                    state->recover_timer_active = true;
                    state->recover_start_time = current_time;
                }

                // 检查是否达到恢复时间
                if (current_time - state->recover_start_time >= config->recover_time_ms) 
                {
                    state->is_fault = false;
                    state->recover_timer_active = false;
                }
            } 
            else 
            {
                state->recover_timer_active = false;
            }
        }
    }
}


/*****************************************************************************
 * @brief       更新M核故障位图
 *****************************************************************************/
static void update_m_fault_bitmap(void)
{
    memset(&bms_m_fault, 0, sizeof(BBMS_M_Fault_t));

    for (uint16_t i = 0; i < kBmsMFault_End - kBmsMFault_Start; i++) 
    {
        if (m_fault_states[i].is_fault) 
        {
            uint8_t byte_index = i / 8;
            uint8_t bit_index = i % 8;
            bms_m_fault.byte[byte_index] |= (1 << bit_index);
        }
    }
}


/*****************************************************************************
 * @brief       更新A核故障位图
 *****************************************************************************/
static void update_a_fault_bitmap(void)
{
    memset(&bms_a_fault, 0, sizeof(BBMS_A_Fault_t));

    for (uint16_t i = 0; i < kBmsAFault_End - kBmsAFault_Start; i++) 
    {
        if (a_fault_states[i].is_fault) 
        {
            bms_a_fault.byte |= (1 << i);
        }
    }
}


/*****************************************************************************
 * @brief       执行故障处理措施（独立处理每个故障）
 *****************************************************************************/
static void execute_fault_handle(bms_m_fault_type_e fault_type)
{
    uint16_t index = fault_type - kBmsMFault_Start;
    fault_handle_e handle_type = m_fault_configs[index].handle_type;

    switch (handle_type) {
        case kFaultHandle_None:
            break;

        case kFaultHandle_AlarmOnly:
            // 仅报警处理
            KITLOG(LOG_APP_EN, WARN_EN, "bbms m_fault %d triggered, only alarm", fault_type);
            break;

        case kFaultHandle_Derating:
        {
            // 降流处理，1、先计算当故障降流系数，2、最终要得到最小的值
            uint16_t cur_derating_ratio = m_fault_configs[index].derating_ratio;
            if(cur_derating_ratio < 0 || cur_derating_ratio > 100)
            {
                cur_derating_ratio = 100; // 如果配置的降流系数无效，则默认为不降流
            }
            // 得到降流最低系数
            derating_ratio = KIT_MIN(derating_ratio, cur_derating_ratio);
            
            KITLOG(LOG_APP_EN, WARN_EN, "bbms m_fault %d triggered, derating, ratio: %d", fault_type, derating_ratio);
            break;
        }
        case kFaultHandle_Shutdown:
            // 关机处理-所有的rbms都关机
            for(uint16_t i = 0; i < get_rbms_num(); i++)
            {
                //获取当前的rbms状态，如果不是关机则下发关机指令
                if(bms_get_rbms_is_off_status(i + 1) != 1)
                {
                    generateRbmsPowerOffCmd(i + 1);
                }

            }
            KITLOG(LOG_APP_EN, WARN_EN, "bbms m_fault %d triggered, shutdown all rbms", fault_type);
            //同时降流为0

            break;

        default:
            break;
    }
}


/*****************************************************************************
 * @brief       初始化故障检测功能
 *****************************************************************************/
void bms_fault_init(void)
{
    memset(&bms_m_fault, 0, sizeof(BBMS_M_Fault_t));
    memset(&bms_a_fault, 0, sizeof(BBMS_A_Fault_t));
    memset(m_fault_states, 0, sizeof(m_fault_states));
    memset(a_fault_states, 0, sizeof(a_fault_states));
}


/*****************************************************************************
 * @brief       获取M核故障状态
 *****************************************************************************/
bool bms_get_m_fault_status(bms_m_fault_type_e fault_type)
{
    if (fault_type >= kBmsMFault_End || fault_type < kBmsMFault_Start) {
        return false;
    }
    return m_fault_states[fault_type - kBmsMFault_Start].is_fault;
}


/*****************************************************************************
 * @brief       获取A核故障状态
 *****************************************************************************/
bool bms_get_a_fault_status(bms_a_fault_type_e fault_type)
{
    if (fault_type >= kBmsAFault_End || fault_type < kBmsAFault_Start) {
        return false;
    }
    return a_fault_states[fault_type - kBmsAFault_Start].is_fault;
}


/*****************************************************************************
 * @brief       获取M核故障处理措施
 *****************************************************************************/
fault_handle_e bms_get_m_fault_handle(bms_m_fault_type_e fault_type)
{
    if (fault_type >= kBmsMFault_End || fault_type < kBmsMFault_Start) {
        return kFaultHandle_None;
    }
    return m_fault_configs[fault_type - kBmsMFault_Start].handle_type;
}


/*****************************************************************************
 * @brief       获取A核故障处理措施
 *****************************************************************************/
fault_handle_e bms_get_a_fault_handle(bms_a_fault_type_e fault_type)
{
    if (fault_type >= kBmsAFault_End || fault_type < kBmsAFault_Start) {
        return kFaultHandle_None;
    }
    return a_fault_configs[fault_type - kBmsAFault_Start].handle_type;
}



/*****************************************************************************
 * @brief       从实时库同步rbms故障信息,实时库中为最新的数据，周期调用
*****************************************************************************/
void rbms_fault_sync_data(void)
{
    uint16_t rbms_num = get_rbms_num();  // 获取当前RBMS数量
    if(rbms_num > MAX_CLUSTER_NUM) 
    {
        rbms_num = MAX_CLUSTER_NUM;  // 防止越界
    }
    uint8_t rtdb_value = 0;  
    for(uint16_t i = 0; i < rbms_num; i++) 
    {
        for (uint8_t byte_index = 0; byte_index < sizeof(rbms_fault[i].faultBits); byte_index++) 
        {
            rtdb_value = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_Fault_Start + byte_index);
            rbms_fault[i].faultBits[byte_index] = rtdb_value;
        }
    }
}


/*****************************************************************************
 * @brief       获取rbms故障
 * @param[in]   fault_id: 故障类型
 * @retval      故障位状态: 1-故障发生, 0-无故障, 0xFF-无效故障ID
 *****************************************************************************/
uint8_t get_rbms_fault_bit(uint8_t cluster_id, rbms_fault_type fault_id)
{
    // 检查cluster_id是否有效
    if (cluster_id >= MAX_CLUSTER_NUM) 
    {
        return 0xFF;  // 无效cluster_id
    }
    
    // 检查故障ID是否有效
    if (fault_id <= kRbmsFault_Start || fault_id >= kRbmsFault_End) 
    {
        return 0xFF;  // 无效故障ID
    }
    
    // 计算所在字节和位偏移
    uint8_t byte_index = fault_id / 8;
    uint8_t bit_offset = fault_id % 8;
    
    // 读取对应位
    return (rbms_fault[cluster_id].faultBits[byte_index] >> bit_offset) & 0x01;
}

/*****************************************************************************
 * @brief       更新rbms故障
 * @param[in]   fault_id: 故障类型
 * @param[in]   bit_value: 要设置的值 (0-清除故障, 1-设置故障)
 * @retval      0-成功, 1-无效故障ID, 2-无效bit值
 *****************************************************************************/
uint8_t update_rbms_fault_bit(uint8_t cluster_id, rbms_fault_type fault_id, uint8_t bit_value)
{
    // 检查cluster_id是否有效
    if (cluster_id >= MAX_CLUSTER_NUM) 
    {
        return 1;  // 无效cluster_id
    }
    
    // 检查故障ID是否有效
    if (fault_id <= kRbmsFault_Start || fault_id >= kRbmsFault_End) 
    {
        return 1;  // 无效故障ID
    }
    
    // 检查bit_value是否有效
    if (bit_value != 0 && bit_value != 1) 
    {
        return 2;  // 无效bit值
    }
    
    // 计算所在字节和位偏移
    uint8_t byte_index = fault_id / 8;
    uint8_t bit_offset = fault_id % 8;
    
    // 更新对应位
    if (bit_value) 
    {
        // 设置故障位为1
        rbms_fault[cluster_id].faultBits[byte_index] |= (1 << bit_offset);
    } 
    else 
    {
        // 清除故障位为0
        rbms_fault[cluster_id].faultBits[byte_index] &= ~(1 << bit_offset);
    }
    
    return 0;  // 成功
}

/*******************************************************************************************************
 * @brief       充电功率限制
 * @param[in]   max_power: 最大功率
 * @param[in]   derating_ratio: 降流系数
 * @retval      限制后的功率
 *  kBbms_MaxContChrgPower,      // Const Charge Power Limit (kW)  持续充电功率限制 (kW)
    kBbms_Max60sChrgPower,       // Pulse Charge Power Limit (kW)  脉冲充电功率限制 (kW)
    kBbms_MaxContDisChrgPower,   // Const Discharge Power Limit (kW)  持续放电功率限制 (kW)
    kBbms_Max60sDisChrgPower,    // Pulse Discharge Power Limit (kW)  脉冲放电功率限制 (kW)
    kBbms_MaxContChrgCur,        // Const Charge Curr Limit (A)  持续充电电流限制 (A)
    kBbms_Max60sChrgCur,         // Pulse Charge Curr Limit (A)  脉冲充电电流限制 (A)
    kBbms_MaxContDisChrgCur,     // Const Discharge Curr Limit (A)  持续放电电流限制 (A)
    kBbms_Max60sDisChrgCur,      // Pulse Discharge Curr Limit (A)  脉冲放电电流限制 (A)
*******************************************************************************************************/
void bms_charge_power_limit(uint16_t derating_ratio)
{
    double max_charge_power = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_Max60sChrgPower);

    double max_charge_current = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_Max60sChrgCur);

    max_charge_power = max_charge_power * derating_ratio / 100;
    max_charge_current = max_charge_current * derating_ratio / 100;

    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_MaxContChrgPower, max_charge_power);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_MaxContChrgCur, max_charge_current);
}


/*******************************************************************************************************
 * @brief       放电功率限制
 * @param[in]   max_power: 最大功率
 * @param[in]   derating_ratio: 降流系数
 * @retval      限制后的功率
*******************************************************************************************************/
void bms_discharge_power_limit(uint16_t derating_ratio)
{
    double max_discharge_power = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_Max60sDisChrgPower);

    double max_discharge_power_current = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_Max60sDisChrgCur);

    max_discharge_power = max_discharge_power * derating_ratio / 100;
    max_discharge_power_current = max_discharge_power_current * derating_ratio / 100;

    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_MaxContDisChrgPower, max_discharge_power);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_MaxContDisChrgCur, max_discharge_power_current);
}


/*****************************************************************************
 * @brief       获取rbms高压状态
 * @param[in]   cluster_id: RBMS簇ID，从1开始
 * @retval      电源状态: true-开机，false-关机
 *****************************************************************************/
bms_charge_status_e bms_get_rbms_power_status(uint16_t cluster_id)
{
    // 检查cluster_id是否有效
    if (cluster_id >= MAX_CLUSTER_NUM) 
    {
        return 0;  // 无效cluster_id
    }
    
    // 检查rbms运行状态
    uint8_t rbms_status = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, 0, kRbms_SysOperSt);
    bms_charge_status_e charge_status = (bms_charge_status_e)rbms_status;
    return charge_status;
}


/*****************************************************************************
 * @brief       获取rbms是否关机
 * @param[in]   cluster_id: RBMS簇ID，从1开始
 * @retval      电源状态: true-开机，false-关机
 *****************************************************************************/
uint8_t bms_get_rbms_is_off_status(uint16_t cluster_id)
{
    // 检查cluster_id是否有效
    if (cluster_id >= MAX_CLUSTER_NUM) 
    {
        return 0;  // 无效cluster_id
    }
    
    // 检查rbms运行状态
    uint8_t rbms_status = getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, cluster_id - 1, kRbms_SysOperSt);
    bms_charge_status_e charge_status = (bms_charge_status_e)rbms_status;

    //rbms有效情况下，判断是否关机


    if(charge_status ==  kBms_ChargeStatus_NotConnected)
    {
        return 1; // 关机
    }
    else
    {
        return 0; // 开机
    }
}


/*****************************************************************************
 * @brief       故障检测主循环（周期调用）
 *****************************************************************************/
void bms_fault_detect_loop(void)
{
    // 更新RBMS故障状态
    rbms_fault_sync_data();

    //更新数据源数据
    bms_update_m_fault_value();
    bms_update_a_fault_value();

    // 检测所有M核故障
    for (uint16_t i = 0; i < kBmsMFault_End - kBmsMFault_Start; i++) 
    {
        bool pre_fault = m_fault_states[i].is_fault;
        detect_m_fault(i + kBmsMFault_Start);

        // 故障状态变化时，执行处理措施
        if (!pre_fault && m_fault_states[i].is_fault) 
        {
            execute_fault_handle(i + kBmsMFault_Start);
        }
    }

    // 降流处理
    bms_charge_power_limit(derating_ratio);
    bms_discharge_power_limit(derating_ratio);

    // 检测所有A核故障
    for (uint16_t i = 0; i < kBmsAFault_End - kBmsAFault_Start; i++) 
    {
        detect_a_fault(i + kBmsAFault_Start);
    }

    // 更新故障位图
    update_m_fault_bitmap();
    update_a_fault_bitmap();
    sync_m_fault_to_rtdb(0U);
    sync_a_fault_to_rtdb(0U);
}


/*****************************************************************************
 * @brief       获取M核故障数组状态
 * @param[in]   fault_type: 故障类型
 * @return      true-故障中  false-正常
 *****************************************************************************/
uint8_t bms_get_m_bank_idx_byte_value(uint16_t fault_array_id)
{
    // 检查fault_array_id是否有效
    if (fault_array_id >= sizeof(bms_m_fault.byte))
    {
        return 0U;
    }

    return bms_m_fault.byte[fault_array_id];
}

static void sync_m_fault_to_rtdb(uint16_t bbms_idx)
{
    const uint16_t fault_byte_count = kBbms_FaultList_End - kBbms_FaultList_Start;

    for (uint16_t i = 0U; i < fault_byte_count; i++)
    {
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_FaultList_Start + i,
                          (double)bms_get_m_bank_idx_byte_value(i));
    }
}


/*****************************************************************************
 * @brief       获取A核故障状态
 * @param[in]   fault_type: 故障类型
 * @return      true-故障中  false-正常
 *****************************************************************************/
uint8_t bms_get_a_bank_idx_byte_value(uint16_t fault_array_id)
{
    // 检查fault_array_id是否有效
    if (fault_array_id == 0U)
    {
        return bms_a_fault.byte;
    }

    return 0U;
}

static void sync_a_fault_to_rtdb(uint16_t bbms_idx)
{
    const uint16_t fault_byte_count = kBbms_ACore_FaultList_End - kBbms_ACore_FaultList_Start;

    for (uint16_t i = 0U; i < fault_byte_count; i++)
    {
        setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_ACore_FaultList_Start + i,
                          (double)bms_get_a_bank_idx_byte_value(i));
    }
}




static uint16_t power_off_fault_arr[MAX_CLUSTER_NUM] = {0};  // 存储每个RBMS的下电故障状态，0表示无下电故障，1表示有下电故障

/*****************************************************************************
 * @brief       获取rbms是否存在下电故障
 * @param[in]   rbms_id: rbms序号，从0开始
 *****************************************************************************/
uint8_t get_rbms_power_off_fault(uint8_t rbms_id)
{
    //待追加实现
    return 1;

}