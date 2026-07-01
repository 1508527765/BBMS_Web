/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            app_cool_ctrl.c
 * @brief           海辰TMS液冷热管理策略（热管理策略-260528）
 * @author          gary
 * @date            2026-06-09
 * @remark          switch-case 状态机；输出写 tms_target，CAN 层自动下发
 *****************************************************************************/

#include "app_cool_crtl.h"
#include "app_bms_statistics.h"
#include "kit_log.h"

#include <math.h>
#include <unistd.h>

TMS_Status_t tms_status;
BMS_To_TMS_RTC_t tms_rtc;
BMS_To_TMS_Cmd_t tms_cmd;
TMS_Target_t tms_target = {
    .req_mode = BMS_REQ_SHUTDOWN,
    .ctrl_mode = CTRL_MODE_BMS,
    .target_temp = 22,
    .weather_mode = WEATHER_CLOSE,
};

/* 热管理策略-260528 默认阈值（RTDB CcTHMC 无效时使用） */
#define TMS_COOL_ENTER_TMAX_LO   29.0f
#define TMS_COOL_ENTER_TMAX_HI   35.0f
#define TMS_COOL_ENTER_TAVG      27.0f
#define TMS_COOL_EXIT_TMAX       28.0f
#define TMS_COOL_EXIT_TAVG       26.0f
#define TMS_HEAT_ENTER_TMIN      12.0f
#define TMS_HEAT_ENTER_TAVG      18.0f
#define TMS_HEAT_EXIT_TMIN       18.0f
#define TMS_HEAT_EXIT_TAVG       20.0f
#define TMS_COOL_TEMP_CHARGE     20.0f
#define TMS_COOL_TEMP_DISCHARGE  26.0f
#define TMS_COOL_TEMP_STANDBY    22.0f
#define TMS_HEAT_TEMP            30.0f
#define TMS_CIRC_DELTA_T         4.0f

typedef enum
{
    TMS_STATE_SHUTDOWN = 0,
    TMS_STATE_COOLING  = 1,
    TMS_STATE_HEATING  = 2,
    TMS_STATE_CIRCULATE = 3,
} tms_state_e;

static tms_state_e s_tms_state = TMS_STATE_SHUTDOWN;

static float tms_thr_val(double rtdb, float def)
{
    return isfinite(rtdb) ? (float)rtdb : def;
}

static void tms_strategy_step(void)
{
    float tmax;
    float tmin;
    float tavg;
    float delta_t;
    float cool_lo;
    float cool_avg;
    float cool_exit_max;
    float cool_exit_avg;
    float heat_min;
    float heat_avg;
    float heat_exit_min;
    float heat_exit_avg;
    int cool_want;
    int heat_want;
    int cool_done;
    int heat_done;
    int bank_chg_dischg;
    bms_charge_status_e ch_st;
    double hmi_en;

    /* HMI 强控优先 */
    hmi_en = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_Hmi_TMSManCtrlEnaFlg);
    if (hmi_en >= 0.5)
    {
        tms_target.req_mode = (BMS_RequestMode_e)getRtdbPointValue(
            Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_Hmi_TMSManCtrlMode);
        tms_target.target_temp = (int16_t)getRtdbPointValue(
            Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_Hmi_TMSManCtrlTempDegC);
        tms_target.ctrl_mode = CTRL_MODE_BMS;
        return;
    }

    /* 读温度与阈值 */
    tmax = (float)getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_MaxCellT);
    tmin = (float)getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_MinCellT);
    tavg = (float)getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_AvgCellT);
    delta_t = tmax - tmin;

    cool_lo = tms_thr_val(getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_CcTHMC_EntCoolMaxTemLowLimtA),
                          TMS_COOL_ENTER_TMAX_LO);
    cool_avg = tms_thr_val(getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_CcTHMC_EntCoolAvgTemLowLimtA),
                           TMS_COOL_ENTER_TAVG);
    cool_exit_max = tms_thr_val(getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_CcTHMC_ExtCoolMaxTemUpLimtA),
                                TMS_COOL_EXIT_TMAX);
    cool_exit_avg = tms_thr_val(getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_CcTHMC_ExtCoolAvgTemUpLimtA),
                                TMS_COOL_EXIT_TAVG);
    heat_min = tms_thr_val(getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_CcTHMC_EntHeatMinTemLpLmt),
                           TMS_HEAT_ENTER_TMIN);
    heat_avg = tms_thr_val(getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_CcTHMC_EntHeatAvgTemLpLmt),
                           TMS_HEAT_ENTER_TAVG);
    heat_exit_min = tms_thr_val(getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_CcTHMC_ExtHeatMinTemLowLmt),
                                TMS_HEAT_EXIT_TMIN);
    heat_exit_avg = tms_thr_val(getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_CcTHMC_ExtHeatAvgTemLowLmt),
                                TMS_HEAT_EXIT_TAVG);

    /* 进入/退出条件（文档 260528） */
    cool_want = (tmax >= TMS_COOL_ENTER_TMAX_HI)
                || (tmax >= cool_lo && tmax < TMS_COOL_ENTER_TMAX_HI && tavg >= cool_avg);
    heat_want = (tmin <= heat_min && tavg <= heat_avg);
    cool_done = (tmax <= cool_exit_max && tavg <= cool_exit_avg);
    heat_done = (tmin >= heat_exit_min || tavg >= heat_exit_avg);

    /* 状态迁移：制冷/制热优先，冲突来自循环，否则关机 */
    switch (s_tms_state)
    {
    case TMS_STATE_COOLING:
        if (!cool_done)
        {
            break;
        }
        s_tms_state = TMS_STATE_SHUTDOWN;
        /* fall through */

    case TMS_STATE_SHUTDOWN:
    case TMS_STATE_CIRCULATE:
        if (cool_want && heat_want)
        {
            s_tms_state = TMS_STATE_CIRCULATE;
        }
        else if (cool_want)
        {
            s_tms_state = TMS_STATE_COOLING;
        }
        else if (heat_want)
        {
            s_tms_state = TMS_STATE_HEATING;
        }
        else if (delta_t <= TMS_CIRC_DELTA_T)
        {
            s_tms_state = TMS_STATE_CIRCULATE;
        }
        else
        {
            s_tms_state = TMS_STATE_SHUTDOWN;
        }
        break;

    case TMS_STATE_HEATING:
        if (!heat_done)
        {
            break;
        }
        s_tms_state = TMS_STATE_SHUTDOWN;
        if (cool_want && heat_want)
        {
            s_tms_state = TMS_STATE_CIRCULATE;
        }
        else if (cool_want)
        {
            s_tms_state = TMS_STATE_COOLING;
        }
        else if (heat_want)
        {
            s_tms_state = TMS_STATE_HEATING;
        }
        else if (delta_t <= TMS_CIRC_DELTA_T)
        {
            s_tms_state = TMS_STATE_CIRCULATE;
        }
        else
        {
            s_tms_state = TMS_STATE_SHUTDOWN;
        }
        break;

    default:
        s_tms_state = TMS_STATE_SHUTDOWN;
        break;
    }

    /* 按当前状态写 tms_target（CAN 层读取后组帧下发） */
    tms_target.ctrl_mode = CTRL_MODE_BMS;

    switch (s_tms_state)
    {
    case TMS_STATE_COOLING:
        tms_target.req_mode = BMS_REQ_COOLING;

        bank_chg_dischg = 0;
        {
            uint16_t rbms_num = (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_Con, 0, kCon_RBMSNum);
            uint16_t i;
            uint8_t has_chg = 0u;
            uint8_t has_dis = 0u;

            for (i = 0u; i < rbms_num; i++)
            {
                bms_charge_status_e rbms_st;

                if (!get_rbms_online(i))
                {
                    continue;
                }
                rbms_st = (bms_charge_status_e)getRtdbPointValue(Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_ChaSt);
                if (rbms_st == kBms_ChargeStatus_Charge || rbms_st == kBms_ChargeStatus_FullCharge)
                {
                    has_chg = 1u;
                }
                if (rbms_st == kBms_ChargeStatus_Discharge || rbms_st == kBms_ChargeStatus_FullDischarge)
                {
                    has_dis = 1u;
                }
            }
            bank_chg_dischg = (has_chg && has_dis) ? 1 : 0;
        }

        if (bank_chg_dischg)
        {
            tms_target.target_temp = (int16_t)TMS_COOL_TEMP_CHARGE;
        }
        else
        {
            ch_st = (bms_charge_status_e)getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_ChSt);
            if (ch_st == kBms_ChargeStatus_Charge || ch_st == kBms_ChargeStatus_FullCharge)
            {
                tms_target.target_temp = (int16_t)TMS_COOL_TEMP_CHARGE;
            }
            else if (ch_st == kBms_ChargeStatus_Discharge || ch_st == kBms_ChargeStatus_FullDischarge)
            {
                tms_target.target_temp = (int16_t)TMS_COOL_TEMP_DISCHARGE;
            }
            else
            {
                tms_target.target_temp = (int16_t)TMS_COOL_TEMP_STANDBY;
            }
        }
        break;

    case TMS_STATE_HEATING:
        tms_target.req_mode = BMS_REQ_HEATING;
        tms_target.target_temp = (int16_t)TMS_HEAT_TEMP;
        break;

    case TMS_STATE_CIRCULATE:
        tms_target.req_mode = BMS_REQ_CIRCULATE;
        tms_target.target_temp = (int16_t)TMS_COOL_TEMP_STANDBY;
        break;

    case TMS_STATE_SHUTDOWN:
    default:
        tms_target.req_mode = BMS_REQ_SHUTDOWN;
        tms_target.target_temp = (int16_t)TMS_COOL_TEMP_STANDBY;
        break;
    }

    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_BMS_TMSReqMode, (double)tms_target.req_mode);
    setRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, 0, kBbms_BBMS_WtrTmpCmd, (double)tms_target.target_temp);
}

static void *tms_temp_ctrl_thread(void *arg)
{
    (void)arg;

    KITLOG(LOG_APP_EN, INFO_EN, "TMS热管理策略任务启动");

    while (1)
    {
        tms_strategy_step();
        sleep(1);
    }

    return NULL;
}

/*****************************************************************************
 * @brief       创建TMS温度控制任务
 * @param[in]   TmsNum：TMS数量
 * @return      void
 *****************************************************************************/
void creatTmsTempCtrlTaskEntry(uint16_t TmsNum)
{
    pthread_t tid;
    int ret;

    (void)TmsNum;

    ret = pthread_create(&tid, NULL, tms_temp_ctrl_thread, NULL);
    if (ret != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "TMS热管理策略任务创建失败");
        return;
    }

    pthread_detach(tid);
    KITLOG(LOG_APP_EN, INFO_EN, "TMS热管理策略任务创建成功");
}
