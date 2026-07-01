/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, Gary. All rights reserved.
 * @file            logic_dido.c
 * @brief           逻辑di do控制任务
 * @author          gary，li
 * @date            2026-05-25
 * @remark          待海辰确定
 *****************************************************************************/

#include "logic_do.h"
#include "logic_di.h"
#include "app_bms_statistics.h"
#include "kit_db.h"
#include "kit_data.h"
#include "utarray.h"

 
 /* ======================== 全局数据 ======================== */
 
 do_config_t do_config;
 uint8_t do_status[kDo_Ctrl_End] = {0};
 
 /* D5 PCS：与 m_fault_configs 中 Enabled 且 Level=1 的项保持一致 */
 static const bms_m_fault_type_e s_do_level1_faults[] = {
     kBmsMFault_AllRackCommLost,
     kBmsMFault_OneRackCommLost,
     kBmsMFault_CluVoltDiffExces,
     kBmsMFault_IDLearnFail,
     kBmsMFault_WaterCoolLvl1Fault,
     kBmsMFault_SPD2SurgeFault,
     kBmsMFault_WaterLeakAlarm,
     kBmsMFault_FireExtremeAlarm,
     kBmsMFault_ContainerEmergStop,
     kBmsMFault_MainsPowerAbnLvl1,
     kBmsMFault_FireAlarmActAbn,
     kBmsMFault_HVBoxConvReverse,
 };
 
 /* D4：海辰 RBMS「极限」类故障（簇级，经 get_rbms_fault_bit 读） */
 static const rbms_fault_type s_do_rbms_extreme_faults[] = {
     kRbmsFault_CellExtremeOvV,
     kRbmsFault_CellExtremeUvV,
     kRbmsFault_CellExtremeOt,
     kRbmsFault_CellExtremeUt,
     kRbmsFault_ChgExtremeOc,
     kRbmsFault_DsgExtremeOc,
 };
 
 /* ======================== DO 状态（给统计/HMI） ======================== */
 
 uint8_t get_do_output_status(do_ctrl_type_t do_ctrl_type)
 {
     if (do_ctrl_type >= kDo_Ctrl_End)
     {
         return 0;
     }
     return do_status[do_ctrl_type];
 }
 
 void set_do_output_status(do_ctrl_type_t do_ctrl_type, uint8_t status)
 {
     if (do_ctrl_type >= kDo_Ctrl_End)
     {
         return;
     }
     do_status[do_ctrl_type] = status;
 }
 
 /* ======================== 内部查询（问故障模块，不直接写 GPIO） ======================== */
 
 static uint16_t do_get_rbms_num(void)
 {
     return (uint16_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_Con, 0, kCon_RBMSNum);
 }
 
 static bool do_is_di_triggered(di_signal_type_t di_signal)
 {
     return (get_di_signal_value(di_signal) == kDiSignalValue_Trigger);
 }
 
 /* MIC D1：绿灯 = 低压上电（推断：堆未全通讯丢失即认为低压侧可用） */
 static bool do_is_low_voltage_on(void)
 {
     return !bms_get_m_fault_status(kBmsMFault_AllRackCommLost);
 }
 
 /* MIC D2：红灯 = 高压上电（任一簇非 NotConnected） */
 static bool do_is_high_voltage_on(void)
 {
     uint16_t n = do_get_rbms_num();
 
     for (uint16_t i = 0; i < n; i++)
     {
         bms_charge_status_e st = (bms_charge_status_e)getRtdbPointValue(
             Rtdb_ShMem, kDev_Type_RBMS, i, kRbms_SysOperSt);
         if (st != kBms_ChargeStatus_NotConnected)
         {
             return true;
         }
     }
     return false;
 }
 
 /* MIC D3：黄灯 = 触发故障（任意堆 M 核故障） */
 static bool do_bank_any_m_fault(void)
 {
     for (bms_m_fault_type_e ft = kBmsMFault_Start; ft < kBmsMFault_End; ft++)
     {
         if (bms_get_m_fault_status(ft))
         {
             return true;
         }
     }
     return false;
 }
 
 /* MIC D5：是否存在一级故障 */
 static bool do_bank_any_level1_fault(void)
 {
     for (size_t i = 0; i < sizeof(s_do_level1_faults) / sizeof(s_do_level1_faults[0]); i++)
     {
         if (bms_get_m_fault_status(s_do_level1_faults[i]))
         {
             return true;
         }
     }
     return false;
 }
 
 /* MIC D5：无一级故障才允许 PCS 常闭回路维持输出 */
 static bool do_pcs_output_allow(void)
 {
     return !do_bank_any_level1_fault();
 }
 
 /* MIC D4：极限故障（簇 RBMS 极限位；堆 M 核无单独「极限」枚举） */
 static bool do_any_rbms_extreme_fault(void)
 {
     uint16_t n = do_get_rbms_num();
 
     for (uint16_t c = 0; c < n; c++)
     {
         for (size_t i = 0; i < sizeof(s_do_rbms_extreme_faults) / sizeof(s_do_rbms_extreme_faults[0]); i++)
         {
             if (get_rbms_fault_bit(c, s_do_rbms_extreme_faults[i]) != 0)
             {
                 return true;
             }
         }
     }
     return false;
 }
 
 /*
  * MIC D4 美版：消防二级告警（不用国版 DI4H 消防三级作本条件）
  * DI3H + 堆侧消防相关故障 + 现网 Fire_AlarmI 采样
  */
 static bool do_bank_fire_us_level2_for_qf1(void)
 {
     if (do_is_di_triggered(kDi_Signal_Fire_AlarmII))
     {
         return true;
     }
     if (do_is_di_triggered(kDi_Signal_Fire_AlarmI))
     {
         return true;
     }
     if (bms_get_m_fault_status(kBmsMFault_FireModerateAlarm))
     {
         return true;
     }
     if (bms_get_m_fault_status(kBmsMFault_FireExtremeAlarm))
     {
         return true;
     }
     return false;
 }
 
 /* MIC D4：极限 OR 消防二级 → 使能 QF1 输出 */
 static bool do_qf1_output_enable(void)
 {
     return do_any_rbms_extreme_fault() || do_bank_fire_us_level2_for_qf1();
 }
 
 /* 从数据库 di_do_set 加载：do_idx + strategy_seq */
 static void get_do_config(void)
 {
     UT_array *dido_config = NULL;
 
     if (0 != kit_get_di_do_set_arr(&dido_config))
     {
         return;
     }
 
     do_config.do_config_num = 0;
 
     utarray_foreach(dido_config, di_do_logic_set_t *, p)
     {
         if (p->dIDOType != kDo)
         {
             continue;
         }
         if (do_config.do_config_num >= MAX_DO_CONFIG_NUM)
         {
             break;
         }
 
         uint16_t idx = do_config.do_config_num;
         do_config.do_logic_config[idx].do_idx = p->dIDOseq;
         do_config.do_logic_config[idx].logic_idx = (uint16_t)p->strategySeqDo;
         do_config.do_logic_config[idx].bms_idx = p->cabinetCode;
         do_config.do_logic_config[idx].do_output_mode = kDO_Ctrl_Normal;
         do_config.do_logic_config[idx].actual_value = 0;
         do_config.do_config_num++;
     }
 
     if (dido_config != NULL)
     {
         utarray_free(dido_config);
     }
 }
 
 /* ======================== MIC 策略回调（只算 0/1，不写 GPIO） ======================== */
 
 int do_ctrl_force_on(uint16_t bms_idx)
 {
     (void)bms_idx;
     set_do_output_status(kDo_Ctrl_Force_On, 1);
     return 1;
 }
 
 int do_ctrl_force_off(uint16_t bms_idx)
 {
     (void)bms_idx;
     set_do_output_status(kDo_Ctrl_Force_Off, 0);
     return 0;
 }
 
 int do_ctrl_green_light(uint16_t bms_idx)
 {
     (void)bms_idx;
     uint8_t v = do_is_low_voltage_on() ? 1U : 0U;
     set_do_output_status(kDo_Ctrl_Green_Light, v);
     return v;
 }
 
 int do_ctrl_red_light(uint16_t bms_idx)
 {
     (void)bms_idx;
     uint8_t v = do_is_high_voltage_on() ? 1U : 0U;
     set_do_output_status(kDo_Ctrl_Red_Light, v);
     return v;
 }
 
 int do_ctrl_yellow_light(uint16_t bms_idx)
 {
     (void)bms_idx;
     uint8_t v = do_bank_any_m_fault() ? 1U : 0U;
     set_do_output_status(kDo_Ctrl_Yellow_Light, v);
     return v;
 }
 
 int do_ctrl_qf1(uint16_t bms_idx)
 {
     (void)bms_idx;
     uint8_t v = do_qf1_output_enable() ? 1U : 0U;
     set_do_output_status(kDo_Ctrl_QF1, v);
     return v;
 }
 
 int do_ctrl_pcs_estop(uint16_t bms_idx)
 {
     (void)bms_idx;
     uint8_t v = do_pcs_output_allow() ? 1U : 0U;
     set_do_output_status(kDo_Ctrl_PCS_EStop, v);
     return v;
 }
 
 do_callback do_callbacks[kDo_Ctrl_End] = {
     NULL,
     do_ctrl_force_on,
     do_ctrl_force_off,
     do_ctrl_green_light,
     do_ctrl_red_light,
     do_ctrl_yellow_light,
     do_ctrl_qf1,
     do_ctrl_pcs_estop,
 };
 
 /* ======================== 任务：初始化 / 主循环 / 线程 ======================== */
 
 void logic_do_init(void)
 {
     kit_db_get_do_config(&do_config);
 }
 
 void *logic_do_ctrl(void *arg)
 {
     (void)arg;
 
     logic_do_init();
 
     while (1)
     {
         /* 更新故障（当前工程未在定时器里调用，DO 任务里必须调） */
         bms_fault_detect_loop();
 
         if (readWebSign(kSign_ShMem, kSign_DoLogicConfig) != 0)
         {
             writeWebSign(kSign_ShMem, kSign_DoLogicConfig, 0);
             kit_db_get_do_config(&do_config);
         }
 
         for (uint16_t i = 0; i < do_config.do_config_num; i++)
         {
             uint16_t do_idx = do_config.do_logic_config[i].do_idx;
             uint16_t logic_idx = do_config.do_logic_config[i].logic_idx;
             uint16_t bms_idx = do_config.do_logic_config[i].bms_idx;
             do_ctrl_mode_e mode = do_config.do_logic_config[i].do_output_mode;
             uint8_t actual = 0;
 
             if (mode == kDO_Ctrl_Disable)
             {
                 continue;
             }
 
             if (mode == kDO_Ctrl_Force_Close)
             {
                 actual = 1;
             }
             else if (mode == kDO_Ctrl_Force_Open)
             {
                 actual = 0;
             }
             else if (logic_idx < kDo_Ctrl_End && do_callbacks[logic_idx] != NULL)
             {
                 actual = (uint8_t)do_callbacks[logic_idx](bms_idx);
             }
 
             do_config.do_logic_config[i].actual_value = actual;
             drvGpioWrite(DO_Start + do_idx, actual);
         }
 
         usleep(20 * 1000);
     }
 }
 
 void creatLogicDoTask(void)
 {
     pthread_t th;
     pthread_create(&th, NULL, logic_do_ctrl, NULL);
 }