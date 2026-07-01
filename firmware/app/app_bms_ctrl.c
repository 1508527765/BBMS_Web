/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, Gary. All rights reserved.
 * @file            app_bms.c
 * @brief           bms功能实现
 * @author          gary
 * @date            2026-03-31
 * @remark          
 *****************************************************************************/
#include "app_bms_ctrl.h"
#include "logic_di.h"

bms_config_t g_bms_config = {0};
bbms_ctrl_t bbms_ctrl = {0, 0, 0, 0, 0, 0};



bbms_safe_signal_ctrl_t bbms_safe_signal_ctrl = {0, 0, 0};   
TransportMsgV1 bbmsmCtrlTansportMsg = {0};

//stack_config_t  bbms_sys_cfg;
bms_sys_config_t bbms_sys_cfg = {0};



static stack_state_e g_state = kStackState_Init;


static stack_cluster_t g_clusters[MAX_CLUSTER_NUM];


static uint32_t state_timer = 0;
static int8_t master_idx = -1;

static uint8_t g_valid_list[MAX_CLUSTER_NUM];
static uint8_t g_valid_cnt = 0;
static uint8_t g_grid_index = 0;


//从rtdb中将实时数据同步至g_clusters数组中
/*    uint8_t online;
    uint8_t ready;
    uint8_t fault;
    float voltage;
    uint8_t contactor_on;
    uint8_t selected;*/
void syncClustersFromRtdb(void)
{

    uint16_t cluster_count = get_rbms_num(); // 获取簇控数量

    for (uint16_t i = 0; i < cluster_count; i++)
    {
        stack_cluster_t *cluster = &g_clusters[i];
        //获取在线状态
        cluster->online = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_Online_State);
        //获取是否存在下电故障
        cluster->fault = get_rbms_power_off_fault(i);
        //获取当前高压
        cluster->voltage = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_DCBusV);
        //获取是否闭合接触器
        uint16_t sys_oper_st = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SysOperSt);
        cluster->contactor_on = ((sys_oper_st & 0x03) == 0x03 ? 1 : 0);
    }



    // 从rtdb中读取实时数据，并同步到g_clusters数组中
    // 这里假设rtdb中的数据结构为cluster_data_t
    // 并且假设rtdb中的数据已经按照cluster_id排序
    // cluster_data_t *cluster_data = (cluster_data_t *)rtdb_get_data("cluster_data");
}



//从数据库中读取bbms_ctrl_t
void readBmsCtrlFromDb(void)
{
    // 从数据库中读取bbms_ctrl_t 使用临时变量代理
     if (kit_get_bms_config(0xFF, &g_bms_config) != 0) // 1-控制配置
     {
         // 获取配置失败，记录日志并返回
         KITLOG(LOG_DB_EN, ERROR_EN, "Failed to get BMS control config from database");
         return;
     }

    bbms_ctrl_t tempCtrl = {0};
    // 从数据库中读取bbms_ctrl_t 存储到tempCtrl
    memcpy(&tempCtrl, &g_bms_config.ctrl_config, sizeof(bbms_ctrl_t));

    // 从数据库中读取bbms_sys_config_t 存储到bbms_sys_cfg
    memcpy(&bbms_sys_cfg, &g_bms_config.sys_config, sizeof(bms_sys_config_t));

    uint8_t isChangeBbmsCtrl = 0;

    //判断数据中那些数据有变化，给出总标记位，同时通知其他线程发送通讯指令
    if (tempCtrl.sys_arch_type != bbms_ctrl.sys_arch_type)
    {
        bbms_ctrl.sys_arch_type = tempCtrl.sys_arch_type;
        isChangeBbmsCtrl = 1;
    }
    if (tempCtrl.bank_err_lvl != bbms_ctrl.bank_err_lvl)
    {
        bbms_ctrl.bank_err_lvl = tempCtrl.bank_err_lvl;
        isChangeBbmsCtrl = 1;
    }
    if (tempCtrl.disch_pwr_lim != bbms_ctrl.disch_pwr_lim)
    {
        bbms_ctrl.disch_pwr_lim = tempCtrl.disch_pwr_lim;
        isChangeBbmsCtrl = 1;
    }
    if (tempCtrl.chg_pwr_lim != bbms_ctrl.chg_pwr_lim)
    {
        bbms_ctrl.chg_pwr_lim = tempCtrl.chg_pwr_lim;
        isChangeBbmsCtrl = 1;
    }

    //如果有变化，生成发送队列元素入全局队列
    if(isChangeBbmsCtrl)
    {
        SrcDestMsgV2 srcDestMsg = {0};
        srcDestMsg.src = BBMSM_DEV_TYPE_SIGN;
        srcDestMsg.srcSub = BBMSM_SUB_TYPE_SIGN;
        srcDestMsg.dest = RBMS_DEV_TYPE_SIGN;
        srcDestMsg.destSub = RBMS_SUB_TYPE_SIGN;
        
        bbmsmCtrlTansportMsg.transportType = kTransportType_0x02;
        bbmsmCtrlTansportMsg.frameId++;

        bbms_ctrl_prepare_for_send(0);
        bms_queue_msg_t msg = createSendFrameQueueItem( &srcDestMsg, &bbmsmCtrlTansportMsg, 0x03, 0x07, (uint8_t *)&bbms_ctrl, sizeof(bbms_ctrl));
        send_queue_enqueue(&msg);
    }
}

/**
 * @brief 生成并ms上电指令
 * @param id 簇控ID
 * 0: NO REQ 1: NORMAL CONNECT 2: MANUAL CONNECT 3: DISCONNECT
 */
void generateRbmsPowerOnCmd(uint8_t id)   //指定簇控发送“并网/上电请求”
{
    bbms_ctrl.bat_conn = 1;
    SrcDestMsgV2 srcDestMsg = {0};
    srcDestMsg.src = BBMSM_DEV_TYPE_SIGN;
    srcDestMsg.srcSub = BBMSM_SUB_TYPE_SIGN;
    srcDestMsg.dest = RBMS_DEV_TYPE_SIGN;
    srcDestMsg.destSub = RBMS_SUB_TYPE_SIGN;
    
    bbmsmCtrlTansportMsg.transportType = kTransportType_0x02;
    bbmsmCtrlTansportMsg.frameId++;

    bbms_ctrl_prepare_for_send(0);
    bms_queue_msg_t msg = createSendFrameQueueItem( &srcDestMsg, &bbmsmCtrlTansportMsg, 0x03, 0x07, (uint8_t *)&bbms_ctrl, sizeof(bbms_ctrl));
    send_queue_enqueue(&msg);
}
/**
 * @brief 生成并ms下电指令
 * @param id 簇控ID
 * 0: NO REQ 1: NORMAL CONNECT 2: MANUAL CONNECT 3: DISCONNECT
 */
void generateRbmsPowerOffCmd(uint8_t id)  //指定簇控发送“断网/下电请求”
{
    bbms_ctrl.bat_conn = 3;
    SrcDestMsgV2 srcDestMsg = {0};
    srcDestMsg.src = BBMSM_DEV_TYPE_SIGN;
    srcDestMsg.srcSub = BBMSM_SUB_TYPE_SIGN;
    srcDestMsg.dest = RBMS_DEV_TYPE_SIGN;
    srcDestMsg.destSub = RBMS_SUB_TYPE_SIGN;
    
    bbmsmCtrlTansportMsg.transportType = kTransportType_0x02;
    bbmsmCtrlTansportMsg.frameId++;

    bbms_ctrl_prepare_for_send(0);
    bms_queue_msg_t msg = createSendFrameQueueItem( &srcDestMsg, &bbmsmCtrlTansportMsg, 0x03, 0x07, (uint8_t *)&bbms_ctrl, sizeof(bbms_ctrl));
    send_queue_enqueue(&msg);
}

/**
 * @brief 周期下发 BBMS_SafetySignal 至 RBMS (cmdGroup 0x02 / cmdId 0x14, 4 Bytes；原 cmdId 0x0E)
 */
void bbms_send_safety_signal_to_rbms(uint16_t bbms_idx)
{
    bbms_safe_signal_ctrl.container_epo_flg =
        get_di_signal_value(kDi_Signal_Container_EStopI) ? 1U : 0U;

    bbms_safe_signal_prepare_for_send(bbms_idx);
    bbms_safe_signal_ctrl.rolling_counter++;
    bbms_safe_signal_ctrl.checksum = (uint8_t)(bbms_safe_signal_ctrl.container_epo_flg +
                                               bbms_safe_signal_ctrl.rolling_counter);

    SrcDestMsgV2 srcDestMsg = {0};
    srcDestMsg.src = BBMSM_DEV_TYPE_SIGN;
    srcDestMsg.srcSub = BBMSM_SUB_TYPE_SIGN;
    srcDestMsg.dest = RBMS_DEV_TYPE_SIGN;
    srcDestMsg.destSub = RBMS_SUB_TYPE_SIGN;

    bbmsmCtrlTansportMsg.transportType = kTransportType_0x01;
    bbmsmCtrlTansportMsg.frameId++;

    bms_queue_msg_t msg = createSendFrameQueueItem(&srcDestMsg, &bbmsmCtrlTansportMsg, 0x02, 0x14,
                                                   (uint8_t *)&bbms_safe_signal_ctrl,
                                                   sizeof(bbms_safe_signal_ctrl));
    send_queue_enqueue(&msg);
}

/**************************************************************************
 * @brief            簇控绝缘检测管理，每60s轮训一次检测
    @param[in]       
    @details:
                    
*************************************************************************/
void app_bms_insulation_detection(void)
{
    // 发送指令到rbms检测绝缘
    uint8_t isInsulation = 0;


}


// 获取候选簇数量
uint8_t getBmsCandidateClusters(uint8_t *list)
{
    uint8_t cnt = 0;

    uint16_t cluster_count = get_rbms_num(); // 获取簇控数量

    //获取簇数量有效值，防止数组越界
    uint8_t valid_clusters = (cluster_count > MAX_CLUSTER_NUM) ? MAX_CLUSTER_NUM : cluster_count;

    for (uint8_t i = 0; i < valid_clusters; i++)
    {
        if (g_clusters[i].online && g_clusters[i].ready && !g_clusters[i].fault)
        {
            list[cnt++] = i;
        }
    }

    return cnt;
}


/*********************************************************************************
 * @brief 构建在网簇集合选出一个最大的子集，使得该子集内任意两簇的压差都小于allow_grid_volt_diff
 * @param out_list 输出的在网簇集合
 * @param out_cnt 输出的在网簇集合数量
 * @return uint8_t 1: 成功 0: 失败
/*********************************************************************************/
uint8_t buildValidClusterSet(uint8_t *out_list, uint8_t *out_cnt)
{
    uint8_t candidate[MAX_CLUSTER_NUM];
    uint8_t cnt = getBmsCandidateClusters(candidate);

    // 检查候选簇数量是否为0
    if (cnt == 0)
    {
        return 0;
    }

    // 按电压排序（冒泡排序，小规模数据够用）
    uint8_t sorted[MAX_CLUSTER_NUM];
    float   sorted_v[MAX_CLUSTER_NUM];
  
    // 集中式式
    if(bbms_sys_cfg.cluster_crtl_mode == 0x00)
    {
        for (uint8_t i = 0; i < cnt; i++)
        {
            sorted[i] = candidate[i];
            sorted_v[i] = g_clusters[candidate[i]].voltage;
        }

        // 简单冒泡排序：按电压从小到大
        for (uint8_t i = 0; i < cnt - 1; i++)
        {
            for (uint8_t j = 0; j < cnt - 1 - i; j++)
            {
                if (sorted_v[j] > sorted_v[j + 1])
                {
                    // 交换电压
                    float tmp_v = sorted_v[j];
                    sorted_v[j] = sorted_v[j + 1];
                    sorted_v[j + 1] = tmp_v;
                    // 交换簇ID
                    uint8_t tmp_id = sorted[j];
                    sorted[j] = sorted[j + 1];
                    sorted[j + 1] = tmp_id;
                }
            }
        }

        // 滑动窗口找最大合格子集
        uint8_t best_start = 0;
        uint8_t best_len   = 0;
        uint8_t head = 0;

        for (uint8_t tail = 0; tail < cnt; tail++)
        {
            // 移动 head，直到窗口内压差 ≤ allow_grid_volt_diff
            while ((sorted_v[tail] - sorted_v[head]) > bbms_sys_cfg.allow_grid_volt_diff)
            {
                head++;
            }

            uint8_t cur_len = tail - head + 1;
            if (cur_len > best_len)
            {
                best_len = cur_len;
                best_start = head;
            }
        }

        // 最大合格簇数量不足 → 不允许上电
        if (best_len < bbms_sys_cfg.min_online_cluster_num)
            return 0;

        // 输出该最大子集
        for (uint8_t i = 0; i < best_len; i++)
        {
            out_list[i] = sorted[best_start + i];
        }
        *out_cnt = best_len;
    }
    else if(bbms_sys_cfg.cluster_crtl_mode == 0x01)
    {
        // 全部候选簇作为在网集合
        for (uint8_t i = 0; i < cnt; i++)
        {
            out_list[i] = candidate[i];
        }
        *out_cnt = cnt;
    }
    else
    {
        return 0;
    }

    return 1;
}


/**********************************************************************
 * @brief 从候选簇集合中选主簇, 选电压最接近平均电压的簇作为主簇
 * @param list : 候选簇集合
 * @param cnt : 候选簇集合数量
 * @return int8_t 主簇ID, -1: 无主簇选
 **********************************************************************/
int8_t getBmsMasterCluster(uint8_t *list, uint8_t cnt)
{
    float v_avg = 0;

    for (uint8_t i = 0; i < cnt; i++)
    {
        v_avg += g_clusters[list[i]].voltage;
    }

    v_avg /= cnt;

    int8_t idx = -1;
    float min_diff = 10000;

    for (uint8_t i = 0; i < cnt; i++)
    {
        float v = g_clusters[list[i]].voltage;
        float diff = fabs(v - v_avg);

        if (diff < min_diff)
        {
            min_diff = diff;
            idx = list[i];
        }
    }

    return idx;
}



uint32_t get_tick_ms(void)
{
    // 获取时间戳，单位：毫秒
    // 从kernel/kit_time.h中获取
    return get_epoch_time_ms();
}


/**
 * @brief 判断自检是否通过
 * @return uint8_t 1: 自检通过 0: 自检未通过
 */
uint8_t bbmsSelfCheckOk(void)       //判断自检是否通过
{
    uint16_t cluster_count = get_rbms_num(); // 获取簇控数量
    for (uint16_t i = 0; i < cluster_count; i++)
    {
        if (!g_clusters[i].online || g_clusters[i].fault)
        {
            return 0; // 自检未通过
        }
    }

    // 检查bbms自身故障等级
    fault_level_e max_flt_level = bms_get_bank_max_flt_level(0); // 获取bbms自身的最大故障等级

    if(max_flt_level == kFaultLevel_1)
    {
        return 0; // 自检未通过
    }

    //检查rbms故障等级
    for (uint16_t i = 0; i < cluster_count; i++)
    {
        if (g_clusters[i].fault)
        {
            return 0; // 自检未通过
        }
    }

    return 1;
}



/*************************************************************************
 * @brief        簇控并网上下电管理
 * @param[in]    无       
 * @details:    簇控并网管理，包括自检、待机、预充、并网、断网等状态的切换
 * @Note:       控制架构：BBMS为堆控、下挂多个电池簇（簇控为rbms）
 *              每个电池簇有多个电池pack，每个电池pack有多个电芯     
 *                  
*************************************************************************/
static uint8_t pending_cluster = 0xFF;
static uint8_t poweroff_cmd_sent = 0;
 static uint32_t last_poweroff_send_time = 0;

void bmsStackRunControl_Old(void)
{
    uint32_t now = get_tick_ms();

    switch (g_state)
    {
    case kStackState_Init:
        g_state = kStackState_SelfCheck;
        break;

    case kStackState_SelfCheck:
        if (bbmsSelfCheckOk())
            g_state = kStackState_Standby;
        else
            g_state = kStackState_Fault;
        break;

    case kStackState_Standby:
        if (buildValidClusterSet(g_valid_list, &g_valid_cnt))
        {
            g_state = kStackState_Prepare;
        }
        else
        {
            g_state = kStackState_SelfCheck;
        }
        break;

    case kStackState_Prepare:

        // 清空selected
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            g_clusters[i].selected = 0;
        }

        // 标记selected
        for (uint8_t i = 0; i < g_valid_cnt; i++)
        {
            g_clusters[g_valid_list[i]].selected = 1;
        }

        //如果是集中式架构，选主簇
        if(bbms_sys_cfg.cluster_crtl_mode == kClusterCtrlMode_Central)
        {
            master_idx = getBmsMasterCluster(g_valid_list, g_valid_cnt);

            // 无主簇选, 则返回待机状态
            if (master_idx < 0)
            {
                g_state = kStackState_Init;
                break;
            }

            //发送并网命令给主簇，上电
            generateRbmsPowerOnCmd(master_idx);

            state_timer = now;
            g_grid_index = 0;
            pending_cluster = 0xFF;
            // 主簇并网状态
            g_state = kStackState_MasterGridOn;
            break;
        }
        else if (bbms_sys_cfg.cluster_crtl_mode == kClusterCtrlMode_Distributed)
        {
            //发布市可以直接所有簇同时并网
            for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
            {
                if (g_clusters[i].selected)
                {
                    generateRbmsPowerOnCmd(g_valid_list[i]);
                }
            }
            state_timer = now;
            // 进入所有簇并网状态
            g_state = kStackState_GridOn;
            break;
        }

    case kStackState_MasterGridOn:
        if (!g_clusters[master_idx].online || g_clusters[master_idx].fault)
        {
            g_state = kStackState_PowerOff;
            break;
        }

        if (g_clusters[master_idx].contactor_on)
        {
            state_timer = now;
            g_state = kStackState_GridOn;
        }
        else if ((now - state_timer) > bbms_sys_cfg.poweron_timeout_ms)
        {
            g_state = kStackState_PowerOff;
        }
        break;

    case kStackState_GridOn:
    {
        //如果是集中式架构，簇依次并网，等待所有簇并网完成
        if (bbms_sys_cfg.cluster_crtl_mode == kClusterCtrlMode_Central)
        {
            if (!g_clusters[master_idx].online || g_clusters[master_idx].fault || !g_clusters[master_idx].contactor_on)
            {
                g_state = kStackState_PowerOff;
                break;
            }

            // 正在并网
            if (pending_cluster != 0xFF)
            {
                if (g_clusters[pending_cluster].contactor_on)
                {
                    pending_cluster = 0xFF;
                }

                else if ((now - state_timer) > bbms_sys_cfg.poweron_timeout_ms)
                {
                    // 跳过该簇
                    if(g_clusters[pending_cluster].selected == 1)
                    {
                        g_valid_cnt--; // 无效簇数减1
                        g_clusters[pending_cluster].selected = 0;
                    }
                    
                    //如果有效簇小于最小在网簇数，直接下电
                    if(g_valid_cnt < bbms_sys_cfg.min_online_cluster_num)
                    {
                        g_state = kStackState_PowerOff;
                        break;
                    }
                }
                break;
            }

            // 找下一个selected簇
            for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
            {
                if (!g_clusters[i].selected)
                    continue;

                if (i == master_idx)
                    continue;

                if (g_clusters[i].fault)
                    continue;

                if (!g_clusters[i].contactor_on)
                {
                    float dv = fabs(g_clusters[i].voltage - g_clusters[master_idx].voltage);

                    if (dv < bbms_sys_cfg.allow_grid_volt_diff)
                    {
                        generateRbmsPowerOnCmd(i);
                        pending_cluster = i;
                        state_timer = now;
                        break;
                    }
                }
            }

            // 判断是否全部完成
            uint8_t all_on = 1;
            for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
            {
                if (g_clusters[i].selected && !g_clusters[i].contactor_on)
                {
                    all_on = 0;
                    break;
                }
            }

            if (all_on && pending_cluster == 0xFF)
            {
                g_state = kStackState_Run;
            }

            break;
        }
        else if (bbms_sys_cfg.cluster_crtl_mode == kClusterCtrlMode_Distributed)
        {
            uint8_t poweron_cnt = 0;
            //分布式可以直接所有簇同时并网，检测是否所有簇并网完成
            for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
            {
                if (g_clusters[i].selected)
                {
                    if (g_clusters[i].contactor_on)
                    {
                        poweron_cnt++;
                    }
                }

            }

            if (poweron_cnt == g_valid_cnt)
            {
                g_state = kStackState_Run;
                break;
            }

            //如果超时还未闭合，进入run状态，但是对应簇发下电
            if ((now - state_timer) > bbms_sys_cfg.poweron_timeout_ms)
            {
                //分布式可以直接所有簇同时并网，检测是否所有簇并网完成
                for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
                {
                    if (g_clusters[i].selected)
                    {
                        if (g_clusters[i].contactor_on == 0)
                        {
                            // 发送下电请求
                            generateRbmsPowerOffCmd(i);
                        }
                    }

                }
                g_state = kStackState_Run;
            }

            break;
        }


    }

    case kStackState_Run:
    {
        //如果是集中式架构,运行中所有告警判断，这里区分是直接下电、还是降流等
        if (bbms_sys_cfg.cluster_crtl_mode == kClusterCtrlMode_Central)
        {
            if (!g_clusters[master_idx].contactor_on || g_clusters[master_idx].fault)
            {
                g_state = kStackState_PowerOff;
                break;
            }
            uint8_t abnormal_on = 0;

            for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
            {
                if (g_clusters[i].selected)
                {
                    if (g_clusters[i].contactor_on && g_clusters[i].fault)
                    {
                        g_state = kStackState_PowerOff;
                        break;
                    }
                }
                else
                {
                    if (g_clusters[i].contactor_on)
                    {
                        abnormal_on = 1;
                    }
                }
            }

            if (g_state == kStackState_PowerOff) break;

            if (abnormal_on)
            {
                for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
                {
                    if (!g_clusters[i].selected && g_clusters[i].contactor_on)
                        generateRbmsPowerOffCmd(i);
                }
            }
            

            //运行中电压差判断
            float v_max = -10000, v_min = 10000;
            for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
            {
                if (g_clusters[i].selected && g_clusters[i].contactor_on)
                {
                    if (g_clusters[i].voltage > v_max) v_max = g_clusters[i].voltage;
                    if (g_clusters[i].voltage < v_min) v_min = g_clusters[i].voltage;
                }
            }

            if ((v_max - v_min) > bbms_sys_cfg.allow_grid_volt_diff)
            {
                g_state = kStackState_PowerOff;
            }

            break;
        }
        else if (bbms_sys_cfg.cluster_crtl_mode == kClusterCtrlMode_Distributed)
        {
            //分布式架构,实时监控所有簇的故障，如果有故障，立即下电所有簇，无故障，保持运行状态
            for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
            {
                if (g_clusters[i].selected && g_clusters[i].fault && g_clusters[i].contactor_on)
                {
                    // 发送下电请求
                    generateRbmsPowerOffCmd(i);
                    break;
                }
                else if (g_clusters[i].selected && !g_clusters[i].fault && !g_clusters[i].contactor_on)
                {
                    //发送上电请求
                    generateRbmsPowerOnCmd(i);
                }

                //如果所有簇都下电则进入待机状态
                if (g_clusters[i].selected && !g_clusters[i].fault && !g_clusters[i].contactor_on)
                {
                    g_state = kStackState_Standby;
                }
            }
        }
        else
        {
            g_state = kStackState_PowerOff;
        }
    }
    case kStackState_PowerOff:
    {
        uint8_t on_cnt = 0;
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if (g_clusters[i].contactor_on)
            {
                on_cnt++;
            }
        }
        if (on_cnt > 0)
        {
            if ((now - last_poweroff_send_time) > bbms_sys_cfg.poweroff_timeout_ms)
            {
                for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
                {
                    if (g_clusters[i].contactor_on)
                    {
                        generateRbmsPowerOffCmd(i);
                    }
                }

                last_poweroff_send_time = now;
            }
        }
        if (on_cnt == 0)
        {
            g_state = kStackState_Standby;
        }
        else if ((now - state_timer) > bbms_sys_cfg.poweroff_timeout_ms)
        {
            g_state = kStackState_Fault;
        }

        break;
    }

    case kStackState_Fault:

        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            generateRbmsPowerOffCmd(i);
        }
        break;

    default:
        break;
    }
}

static void bmsCentralRunControl(void);
static void bmsDistributedRunControl(void);
/*************************************************************************
 * @brief        簇控并网上下电管理（入口）
 * @param[in]    无       
 * @details:    根据簇控模式调用对应的状态机
*************************************************************************/
void bmsStackRunControl(void)
{
    if (bbms_sys_cfg.cluster_crtl_mode == kClusterCtrlMode_Central)
    {
        bmsCentralRunControl();
    }
    else if (bbms_sys_cfg.cluster_crtl_mode == kClusterCtrlMode_Distributed)
    {
        bmsDistributedRunControl();
    }
    else
    {
        g_state = kStackState_Fault;
    }
}


/*************************************************************************
 * @brief        集中式模式上下电管理（完整状态机）
 * @details:     所有簇并联，需要选主簇，依次并网，压差保护
*************************************************************************/
static void bmsCentralRunControl(void)
{
    uint32_t now = get_tick_ms();

    switch (g_state)
    {
    case kStackState_Init:
        g_state = kStackState_SelfCheck;
        break;

    case kStackState_SelfCheck:
        if (bbmsSelfCheckOk())
            g_state = kStackState_Standby;
        else
            g_state = kStackState_Fault;
        break;

    case kStackState_Standby:
        if (buildValidClusterSet(g_valid_list, &g_valid_cnt))
        {
            g_state = kStackState_Prepare;
        }
        else
        {
            g_state = kStackState_SelfCheck;
        }
        break;

    case kStackState_Prepare:
        // 清空selected
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            g_clusters[i].selected = 0;
        }

        // 标记selected
        for (uint8_t i = 0; i < g_valid_cnt; i++)
        {
            g_clusters[g_valid_list[i]].selected = 1;
        }

        // 选主簇
        master_idx = getBmsMasterCluster(g_valid_list, g_valid_cnt);

        // 无主簇选，则返回待机状态
        if (master_idx < 0)
        {
            g_state = kStackState_Init;
            break;
        }

        // 发送并网命令给主簇，上电
        generateRbmsPowerOnCmd(master_idx);

        state_timer = now;
        g_grid_index = 0;
        pending_cluster = 0xFF;
        // 主簇并网状态
        g_state = kStackState_MasterGridOn;
        break;

    case kStackState_MasterGridOn:
        if (!g_clusters[master_idx].online || g_clusters[master_idx].fault)
        {
            g_state = kStackState_PowerOff;
            break;
        }

        if (g_clusters[master_idx].contactor_on)
        {
            state_timer = now;
            g_state = kStackState_GridOn;
        }
        else if ((now - state_timer) > bbms_sys_cfg.poweron_timeout_ms)
        {
            g_state = kStackState_PowerOff;
        }
        break;

    case kStackState_GridOn:

        // // 主簇异常则下电
        // if (!g_clusters[master_idx].online || g_clusters[master_idx].fault || !g_clusters[master_idx].contactor_on)
        // {
        //     g_state = kStackState_PowerOff;
        //     break;
        // }

        // 正在并网
        if (pending_cluster != 0xFF)
        {
            if (g_clusters[pending_cluster].contactor_on)
            {
                pending_cluster = 0xFF;
            }
            else if ((now - state_timer) > bbms_sys_cfg.poweron_timeout_ms)
            {
                // 跳过该簇
                if (g_clusters[pending_cluster].selected == 1)
                {
                    g_valid_cnt--; // 无效簇数减1
                    g_clusters[pending_cluster].selected = 0;
                }

                // 如果有效簇小于最小在网簇数，直接下电
                if (g_valid_cnt < bbms_sys_cfg.min_online_cluster_num)
                {
                    g_state = kStackState_PowerOff;
                    break;
                }
                pending_cluster = 0xFF;
            }
            break;
        }

        // 找下一个selected簇
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if (!g_clusters[i].selected)
                continue;

            if (i == master_idx)
                continue;

            if (g_clusters[i].fault)
                continue;

            if (!g_clusters[i].contactor_on)
            {
                float dv = fabs(g_clusters[i].voltage - g_clusters[master_idx].voltage);

                if (dv < bbms_sys_cfg.allow_grid_volt_diff)
                {
                    generateRbmsPowerOnCmd(i);
                    pending_cluster = i;
                    state_timer = now;
                    break;
                }
            }
        }

        // 判断是否全部完成
        uint8_t all_on = 1;
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if (g_clusters[i].selected && !g_clusters[i].contactor_on)
            {
                all_on = 0;
                break;
            }
        }

        if (all_on && pending_cluster == 0xFF)
        {
            g_state = kStackState_Run;
        }
        break;

    case kStackState_Run:
    {
        uint8_t abnormal_on = 0;
        uint16_t on_cnt = 0;
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if (g_clusters[i].selected)
            {
                if (g_clusters[i].fault)
                {
                    //有故障先下电
                    generateRbmsPowerOffCmd(i);
                }
                //临时统计在网簇数
                if(g_clusters[i].contactor_on)
                {
                    on_cnt++;
                }
            }
            else
            {
                if (g_clusters[i].contactor_on)
                {
                    abnormal_on = 1;
                }
            }
        }

        //如果在网簇数小于最小在网簇数，直接下电
        if(on_cnt < bbms_sys_cfg.min_online_cluster_num)
        {
            g_state = kStackState_PowerOff;
            break;
        }

        if (g_state == kStackState_PowerOff) 
        {
            break;
        }

        if (abnormal_on)
        {
            for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
            {
                if (!g_clusters[i].selected && g_clusters[i].contactor_on)
                {
                    generateRbmsPowerOffCmd(i);
                }
            }
        }

        // 运行中电压差判断
        float v_max = -10000, v_min = 10000;
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if (g_clusters[i].selected && g_clusters[i].contactor_on)
            {
                if (g_clusters[i].voltage > v_max) v_max = g_clusters[i].voltage;
                if (g_clusters[i].voltage < v_min) v_min = g_clusters[i].voltage;
            }
        }

        if ((v_max - v_min) > bbms_sys_cfg.allow_grid_volt_diff)
        {
            g_state = kStackState_PowerOff;
        }
        break;
    }
    case kStackState_PowerOff:
    {
        uint8_t on_cnt = 0;
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if (g_clusters[i].contactor_on)
            {
                on_cnt++;
            }
        }
        if (on_cnt > 0)
        {
            // if ((now - last_poweroff_send_time) > bbms_sys_cfg.poweroff_timeout_ms)
            // {
                for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
                {
                    if (g_clusters[i].contactor_on)
                    {
                        generateRbmsPowerOffCmd(i);
                        state_timer = now;
                    }
                }
            //      last_poweroff_send_time = now;
            //  }
        }
        if (on_cnt == 0)
        {
            g_state = kStackState_Standby;
        }
        else if ((now - state_timer) > bbms_sys_cfg.poweroff_timeout_ms)
        {
            g_state = kStackState_Fault;
        }
        break;
    }

    case kStackState_Fault:
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            generateRbmsPowerOffCmd(i);
        }
        //判断当前簇全部断开后进入到待机状态
        uint8_t all_off_cnt = 0;
        for(uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if(!g_clusters[i].contactor_on)
            {
                all_off_cnt++;
            }
        }
        if(all_off_cnt == MAX_CLUSTER_NUM)
        {
            g_state = kStackState_Standby;
        }
        break;

    default:
        g_state = kStackState_Fault;
        break;
    }
}

/*************************************************************************
 * @brief        分布式模式上下电管理（完整状态机）
 * @details:     各簇独立，可同时上电/下电，无需主簇和压差保护
*************************************************************************/
static void bmsDistributedRunControl(void)
{
    uint32_t now = get_tick_ms();

    switch (g_state)
    {
    case kStackState_Init:
        g_state = kStackState_SelfCheck;
        break;

    case kStackState_SelfCheck:
        if (bbmsSelfCheckOk())
            g_state = kStackState_Standby;
        else
            g_state = kStackState_Fault;
        break;

    case kStackState_Standby:
        if (buildValidClusterSet(g_valid_list, &g_valid_cnt))
        {
            g_state = kStackState_Prepare;
        }
        else
        {
            g_state = kStackState_SelfCheck;
        }
        break;

    case kStackState_Prepare:
        // 清空selected
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            g_clusters[i].selected = 0;
        }

        // 标记selected
        for (uint8_t i = 0; i < g_valid_cnt; i++)
        {
            g_clusters[g_valid_list[i]].selected = 1;
        }

        // 分布式：所有簇同时并网
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if (g_clusters[i].selected)
            {
                generateRbmsPowerOnCmd(g_valid_list[i]);
            }
        }
        state_timer = now;
        g_state = kStackState_GridOn;
        break;

    case kStackState_GridOn:
    {
        uint8_t poweron_cnt = 0;
        // 检测是否所有簇并网完成
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if (g_clusters[i].selected)
            {
                if (g_clusters[i].contactor_on)
                {
                    poweron_cnt++;
                }
            }
        }

        if (poweron_cnt == g_valid_cnt)
        {
            g_state = kStackState_Run;
            break;
        }

        // 如果超时还未闭合，进入run状态，未闭合的簇发下电
        if ((now - state_timer) > bbms_sys_cfg.poweron_timeout_ms)
        {
            for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
            {
                if (g_clusters[i].selected)
                {
                    if (g_clusters[i].contactor_on == 0)
                    {
                        generateRbmsPowerOffCmd(i);
                    }
                }
            }
            g_state = kStackState_Run;
        }

        break;
    }

    case kStackState_Run:
    {
        uint8_t all_off = 1;

        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if (!g_clusters[i].selected)
                continue;

            // 有故障则下电
            if (g_clusters[i].fault && g_clusters[i].contactor_on)
            {
                generateRbmsPowerOffCmd(i);
                continue;
            }

            // 无故障但未上电 → 重新上电
            if (!g_clusters[i].fault && !g_clusters[i].contactor_on)
            {
                generateRbmsPowerOnCmd(i);
                all_off = 0;
            }

            if (g_clusters[i].contactor_on)
            {
                all_off = 0;
            }
        }

        // 所有簇都下电则进入待机状态
        if (all_off)
        {
            g_state = kStackState_Standby;
        }
        break;
    }
    case kStackState_Fault:
        for (uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            generateRbmsPowerOffCmd(i);
        }
        uint8_t poweroff_cnt = 0;
        //所有被选择的簇都无故障则进入待机状态
        for(uint8_t i = 0; i < MAX_CLUSTER_NUM; i++)
        {
            if(!g_clusters[i].contactor_on || !g_clusters[i].online)
            {
                poweroff_cnt++;
            }
        }

        if(poweroff_cnt == MAX_CLUSTER_NUM)
        {
            g_state = kStackState_Standby;
        }

        break;

    default:
        g_state = kStackState_Init;
        break;
    }
}




//创建一个线程来运行bmsStackRunControl，周期100ms
void *bmsStackControlThread(void *arg)
{
    uint8_t safety_signal_tick = 0;

    //初始化参数
    readBmsCtrlFromDb();
    while (1)
    {
        //配置变化更新参数
        uint16_t bbms_update_cfg = readWebSign(kSign_ShMem, kSign_BBMSCtrlUpdate);
        if (bbms_update_cfg != 0) 
        {
            readBmsCtrlFromDb(); //每次循环都从数据库读取最新的控制配置，更新全局变量bbms_ctrl，并在有变化时发送指令到rbms
            bbms_update_cfg = 0; // 重置更新标志
            writeWebSign(kSign_ShMem, kSign_BBMSCtrlUpdate, bbms_update_cfg);
        }

        bmsStackRunControl();

        safety_signal_tick++;
        if (safety_signal_tick >= 10U)
        {
            safety_signal_tick = 0U;
            bbms_send_safety_signal_to_rbms(0);
        }

        usleep(100 * 1000); // 100ms
    }
    return NULL;
}



/*************************************************************************
 * @brief        启动簇控管理线程
 * @param[in]    无       
 * @details:    创建一个线程来运行bmsStackRunControl，周期100ms
 * @return       无
 *************************************************************************/
void startBmsStackControlThread(void)
{
    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, bmsStackControlThread, NULL) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "bmsStackControlThread create failed");
        return;
    }
    pthread_detach(thread_id);
}

