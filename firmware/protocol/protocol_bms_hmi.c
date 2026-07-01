/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, Gary. All rights reserved.
 * @file            protocol_bbms.c
 * @brief           BBMS通信协议点表配置实现,主要为了上送至HMI显示
 * @author          gary
 * @date            2026-03-27
 * @remark
 *****************************************************************************/
#include <stdio.h>
#include "protocol_bms.h"
#include "bsp_bms_com.h"
#include "kit_model.h"
#include "kit_log.h"
#include "bsp_rtdb.h"

static TransportMsgV1 bbmsmCtrlTansportMsg = {0};

void bbms_ctrl_prepare_for_send(uint16_t bbms_idx)
{
    bbms_ctrl.bbms_no = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_BBMSNo) & 0x0FU;
    bbms_ctrl.bank_hb = (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_Con, 0, kCon_RunHeartCount);
}

void bbms_safe_signal_prepare_for_send(uint16_t bbms_idx)
{
    bbms_safe_signal_ctrl.bbms_no =
        (uint8_t)getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, bbms_idx, kBbms_BBMSNo) & 0x0FU;
}


bbms_hmi_crtl_t bbmsHmiCtrlWord = {0};
bbms_hmi_tms_ctrl_t bbmsHmiTmsCtrlWord = {0};
bbms_hmi_do_light_ctrl_t bbmsHmiDoLightCtrlWord = {0};
bbms_para_thr_tms_t bbmsHmiParaThrTmsCtrlWord = {0};
bbms_hmi_bbms_do_ctrl_t bbmsHmiBbmsDoCtrlWord = {0};
/*============================================================
 * BBMS_SumInfo - BBMS概要数据 (130 Bytes)
 * 对应消息ID: BBMS_SumInfo (cmdGroup: 0x04, cmdid: 0x01)
 *============================================================*/
const bmsPointAttr_t bbmsCmd02_01_BBMS_SumInfo_PointAttr[] =
{
    /* pointId,                dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,          minVal,   repeatCnt */
    {kBbms_BatSt,                0,       8,           0,             Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_ChSt,                 1,       8,           8,             Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_SysOperSt,            2,       16,          16,            Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_PmtRkFltNum,          4,       16,          32,            Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_SetInverterSt,        6,       8,           48,            Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_RackFltLvl1Nbr,       7,       16,          56,            Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_CntrSt,               9,       16,          72,            Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_SOC,                  11,      8,           88,            Uint8,     0.5f,     0.0f,      127.5f,          0.0f,          0},
    {kBbms_SOH,                  12,      8,           96,            Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MaxChrgE,             13,      16,          104,           Uint16,    0.125f,   0.0f,      8191.875f,       0.0f,          0},
    {kBbms_MaxDisChrgE,          15,      16,          120,           Uint16,    0.125f,   0.0f,      8191.875f,       0.0f,          0},
    {kBbms_MaxContChrgPower,     17,      16,          136,           Uint16,    0.125f,   0.0f,      8191.875f,       0.0f,          0},
    {kBbms_Max60sChrgPower,      19,      16,          152,           Uint16,    0.125f,   0.0f,      8191.875f,       0.0f,          0},
    {kBbms_MaxContDisChrgPower,  21,      16,          168,           Uint16,    0.125f,   0.0f,      8191.875f,       0.0f,          0},
    {kBbms_Max60sDisChrgPower,   23,      16,          184,           Uint16,    0.125f,   0.0f,      8191.875f,       0.0f,          0},
    {kBbms_MaxContChrgCur,       25,      16,          200,           Uint16,    0.125f,   0.0f,      8191.875f,       0.0f,          0},
    {kBbms_Max60sChrgCur,        27,      16,          216,           Uint16,    0.125f,   0.0f,      8191.875f,       0.0f,          0},
    {kBbms_MaxContDisChrgCur,    29,      16,          232,           Uint16,    0.125f,   0.0f,      8191.875f,       0.0f,          0},
    {kBbms_Max60sDisChrgCur,     31,      16,          248,           Uint16,    0.125f,   0.0f,      8191.875f,       0.0f,          0},
    {kBbms_HumSensorRH,          33,     16,          264,           Uint16,    0.1f,     0.0f,      6553.5f,         0.0f,          0},
    {kBbms_TmpSensorRH,          35,     16,          280,           Int16,    0.1f,    -40.0f,     125.1f,         -40.0f,         0},
    {kBbms_WtrSt,                37,      8,           296,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_StrCtrlHb,            38,      16,          304,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_DCBusV,               40,      16,          320,           Uint16,    0.5f,     0.0f,      32767.5f,        0.0f,          0},
    {kBbms_TotalDCCur,           42,      16,          336,           Int16,    0.125f, -4000.0f,   4191.875f,     -4000.0f,        0},
    {kBbms_TotalPower,           44,      16,          352,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_MaxRackCur,           46,      16,          368,           Int16,    0.125f, -2000.0f,   6191.875f,     -2000.0f,        0},
    {kBbms_MaxCurRackNr,         48,      8,           384,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinRackCur,           49,      16,          392,           Int16,    0.125f, -2000.0f,   6191.875f,     -2000.0f,        0},
    {kBbms_MinCurRackNr,         51,      8,           408,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MaxCellV,             52,      16,          416,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_MaxVCellRackNr,       54,      8,           432,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MaxVCellTrayNr,       55,      8,           440,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MaxVCellNr,           56,      8,           448,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinCellV,             57,      16,          456,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_MinVCellRackNr,       59,      8,           472,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinVCellTrayNr,       60,      8,           480,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinVCellNr,           61,      8,           488,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_AvgCellV,             62,      16,          496,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_MaxHvBoxTRackNr,      64,      8,           512,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MaxTCellRackNr,       65,      8,           520,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MaxTCellTrayNr,       66,      8,           528,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MaxTCellNr,           67,      8,           536,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinHvBoxTRackNr,      68,      8,           544,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinTCellRackNr,       69,      8,           552,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinTCellTrayNr,       70,      8,           560,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinTCellNr,           71,      8,           568,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_ProhUpgradeFlg,       72,      8,           576,           Uint8,     1.0f,     0.0f,      1.0f,            0.0f,          0},
    {kBbms_BMSMaxFltLevel,       73,      8,           584,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_IDLearnSt,            74,      8,           592,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_IDLearnFaiLocationNbr,75,     8,           600,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_IsoRPos,              76,      16,          608,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_IsoRNeg,              78,      16,          624,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_AccuChrgC,            80,      32,          640,           Uint32,    1.0f,     0.0f,      4294967295.0f,   0.0f,          0},
    {kBbms_AccuChrgE,            84,      32,          672,           Uint32,    1.0f,     0.0f,      4294967295.0f,   0.0f,          0},
    {kBbms_AccuDisChrgC,         88,      32,          704,           Uint32,    1.0f,     0.0f,      4294967295.0f,   0.0f,          0},
    {kBbms_AccuDisChrgE,         92,      32,          736,           Uint32,    1.0f,     0.0f,      4294967295.0f,   0.0f,          0},
    {kBbms_LstChEkWh,            96,      16,          768,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_LstDchEkWh,           98,      16,          784,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_RackEnaNbr,           100,     8,           800,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_RackOnlineNbr,        101,     8,           808,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_ConFail,              102,     8,           816,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MaxChrgCap,           103,    16,          824,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_MaxDisChrgCap,        105,    16,          840,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
    {kBbms_MaxHvBoxTDegC,        107,    16,          856,           Int16,    0.1f,    -40.0f,     6513.5f,        -40.0f,         0},
    {kBbms_MinHvBoxTDegC,        109,    16,          872,           Int16,    0.1f,    -40.0f,     6513.5f,        -40.0f,         0},
    {kBbms_MaxCellT,             111,    16,          888,           Int16,    0.1f,    -40.0f,     6513.5f,        -40.0f,         0},
    {kBbms_MinCellT,             113,    16,          904,           Int16,    0.1f,    -40.0f,     6513.5f,        -40.0f,         0},
    {kBbms_AvgCellT,             115,    16,          920,           Int16,    0.1f,    -40.0f,     6513.5f,        -40.0f,         0},
    {kBbms_MaxPoleT,             117,    16,          936,           Int16,    0.1f,    -40.0f,     6513.5f,        -40.0f,         0},
    {kBbms_MinPoleT,             119,    16,          952,           Int16,    0.1f,    -40.0f,     6513.5f,        -40.0f,         0},
    {kBbms_MaxPoleTRackNr,       121,     8,           968,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MaxPoleTMdulNr,       122,     8,           976,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MaxPoleTMdulInnerNr,  123,     8,           984,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinPoleTRackNr,       124,     8,           992,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinPoleTMdulNr,       125,     8,          1000,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_MinPoleTMdulInnerNr,  126,     8,          1008,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          0},
    {kBbms_BBMSNo,               127,     4,          1016,           Uint8,     1.0f,     0.0f,      15.0f,           0.0f,          0},
    {kBbms_StartReConDectFlg,    127,     1,          1020,           Uint8,     1.0f,     0.0f,      1.0f,            0.0f,          0},
    {kBbms_SumInfo_Byte128Reserved, 127,  3,          1021,           Uint8,     1.0f,     0.0f,      7.0f,            0.0f,          0},
    {kBbms_ReverConRackFailNbr,  128,     16,         1024,           Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,          0},
};

/*
 * BBMS_Fault - BBMS故障列表 (26 Bytes) cmdGroup: 0x02, cmdid: 0x13
 * 每个故障位占用1个字节，共计200个故障位，web解析详细的数据
 * 故障数组 25 字节 (200 bit) + byte26: BBMSNo (4 bit) + 预留 (4 bit)
 */
const bmsPointAttr_t bbmsCmd02_13_BBMS_Fault_PointAttr[] =
{
    /* pointId,                dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,          minVal,   repeatCnt */
    {kBbms_FaultList_Start,       0,       8,           0,             Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          25},
    {kBbms_BBMSNo,               25,       4,          200,            Uint8,     1.0f,     0.0f,      15.0f,           0.0f,          0},
};


/*
 * BBMS_A_Fault - BBMS故障列表 (25 Bytes) cmdGroup: 0x01, cmdid: 0x09
 * 每个故障位占用1个字节，共计200个故障位，web解析详细的数据
*/
const bmsPointAttr_t bbmsCmd01_09_BBMS_A_Fault_PointAttr[] =
{
    /* pointId,                dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,          minVal,   repeatCnt */
    {kBbms_ACore_FaultList_Start,       0,       8,           0,             Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          25},
};

/*
 * HMI_FltEna - 故障使能数组 (25 Bytes) 上位机读取：cmdGroup: 0x04, cmdid: 0x08
 * 每个故障位占用1个字节，共计200个故障位，web解析详细的数据
*/
const bmsPointAttr_t bbmsCmd04_08_HMI_FltEna_PointAttr[] =
{
    /* pointId,                dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,          minVal,   repeatCnt */
    {kBbms_HMI_FaultEnaFlg_1,       0,       8,           0,             Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          25},
};


/*
 * HMI_FltEna - 故障使能数组 (25 Bytes) 上位机设置：cmdGroup: 0x04, cmdid: 0x09
 * 每个故障位占用1个字节，共计200个故障位，web解析详细的数据
*/
const bmsPointAttr_t bbmsCmd04_09_HMI_FltEna_PointAttr[] =
{
    /* pointId,                dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,          minVal,   repeatCnt */
    {kBbms_HMI_FaultEnaFlg_1,       0,       8,           0,             Uint8,     1.0f,     0.0f,      255.0f,          0.0f,          25},
};



/*============================================================
 * HMI_CtlWord - HMI控制指令 (7 Bytes)
 * 对应消息ID: HMI_CtlWord (cmdGroup: 0x04, cmdid: 0x04)
 *============================================================*/
const bmsPointAttr_t bbmsCmd04_04_Hmi_Control_PointAttr[] =
{
    /* pointId,                      dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,    minVal,   repeatCnt */
    {kBbms_Hmi_AlmRst,                0,       2,           0,           Uint8,     1.0f,     0.0f,      3.0f,      0.0f,     0},
    {kBbms_Hmi_SetEnalso,             0,       2,           2,           Uint8,     1.0f,     0.0f,      3.0f,      0.0f,     0},
    {kBbms_Hmi_Reserved_4_5,          0,       2,           4,           Uint8,     1.0f,     0.0f,      3.0f,      0.0f,     0},
    {kBbms_Hmi_SetEna,                0,       2,           6,           Uint8,     1.0f,     0.0f,      3.0f,      0.0f,     0},

    {kBbms_Hmi_EPORst,                1,       2,           8,           Uint8,     1.0f,     0.0f,      3.0f,      0.0f,     0},
    {kBbms_Hmi_EnalsoManualCtrl,      1,       2,           10,          Uint8,     1.0f,     0.0f,      3.0f,      0.0f,     0},
    {kBbms_Hmi_SetCon,                1,       4,           12,          Uint8,     1.0f,     0.0f,      15.0f,     0.0f,     0},

    {kBbms_Hmi_IDLearnManualCtrl,     2,       8,           16,          Uint8,     1.0f,     0.0f,      255.0f,    0.0f,     0},
    {kBbms_Hmi_AutoHvUpConf,          3,       8,           24,          Uint8,     1.0f,     0.0f,      255.0f,    0.0f,     0},
    {kBbms_Hmi_IDLearnDoCtrl,         4,       8,           32,          Uint8,     1.0f,     0.0f,      255.0f,    0.0f,     0},

    {kBbms_Hmi_PmtRkFltNum,           5,       6,           40,          Uint8,     1.0f,     0.0f,      63.0f,    0.0f,     0},
    {kBbms_Hmi_PCSComDiagConf,        5,       2,           46,          Uint8,     1.0f,     0.0f,      3.0f,     0.0f,     0},

    {kBbms_Hmi_LocRemCtrl,            6,       3,           48,          Uint8,     1.0f,     0.0f,      7.0f,     0.0f,     0},
    {kBbms_Hmi_Reserved_3_7,          6,       5,           51,          Uint8,     1.0f,     0.0f,      31.0f,    0.0f,     0},
};


/*============================================================
 * HMI_TMSCtrlWord -HMI TMS控制指令(BBMS) (4 Bytes)
 * 对应消息ID: HMI_TMSCtrlWord (cmdGroup: 0x02, cmdid: 0x03)
 *============================================================*/
const bmsPointAttr_t bbmsCmd02_03_Hmi_Tms_Control_PointAttr[] =
{
    /* pointId,                      dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,    minVal,   repeatCnt */
    {kBbms_Hmi_TMSManCtrlMode,          0,       8,           0,           Uint8,     1.0f,     0.0f,      255.0f,    0.0f,     0},
    {kBbms_Hmi_TMSManCtrlTempDegC,      1,       8,           8,           Uint8,     1.0f,     0.0f,      255.0f,    0.0f,     0},
    {kBbms_Hmi_TMSManCtrlEnaFlg,        2,       8,           16,          Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_Hmi_TMSNo,                    3,       8,           24,          Uint8,     1.0f,     0.0f,      255.0f,    0.0f,     0},
};

/*============================================================
 * HMI_BankDOCtrl - HMI三级板DO控制指令（灯）
 * 对应消息ID: HMI_DOCtrlWord (cmdGroup: 0x02, cmdid: 0x04)
 *============================================================*/
const bmsPointAttr_t bbmsCmd02_04_Hmi_DO_Control_PointAttr[] =
{
    /* pointId,                      dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,    minVal,   repeatCnt */
    {kBbms_Hmi_DOManCtrlMode,          0,       8,           0,           Uint8,     1.0f,     0.0f,      255.0f,    0.0f,     0},
    {kBbms_Hmi_DOManCtrlReserved,      0,       5,           3,           Uint8,     1.0f,     0.0f,      31.0f,      0.0f,     0},
};

/*============================================================
 * TMS_SumInfo - 热管理信息(RBMS/BBMS) (12 Bytes)
 * 对应消息ID: TMS_SumInfo (cmdGroup: 0x02, cmdid: 0x02)
 *============================================================*/
const bmsPointAttr_t bbmsCmd02_02_TmsSumInfo_PointAttr[] =
{
    /* pointId,                      dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,    minVal,   repeatCnt */
    {kBbms_BBMS_WtrTmpCmd,            0,       8,           0,           Int8,     1.0f,     -40.0f,    215.0f,    -40.0f,   0},
    {kBbms_BBMS_cellTmpMax,           1,       8,           8,           Int8,     1.0f,     -40.0f,    215.0f,    -40.0f,   0},
    {kBbms_BBMS_cellTmpMin,           2,       8,           16,          Int8,     1.0f,     -40.0f,    215.0f,    -40.0f,   0},
    {kBbms_BBMS_cellTmpAvg,           3,       8,           24,          Int8,     1.0f,     -40.0f,    215.0f,    -40.0f,   0},
    {kBbms_TMS_OutWtrTmp,             4,       8,           32,          Int8,     1.0f,     -40.0f,    215.0f,    -40.0f,   0},
    {kBbms_TMS_BackWtrTmp,            5,       8,           40,          Int8,     1.0f,     -40.0f,    215.0f,    -40.0f,   0},
    {kBbms_TMS_EnvTmp,                6,       8,           48,          Int8,     1.0f,     -40.0f,    215.0f,    -40.0f,   0},
    {kBbms_TMS_InWtrP,                7,       8,           56,          Uint8,     0.1f,     0.0f,      25.5f,     0.0f,     0},
    {kBbms_TMS_OutWtrP,               8,       8,           64,          Uint8,     0.1f,     0.0f,      25.5f,     0.0f,     0},
    {kBbms_TMS_BBMSFltLevel,          9,       2,           72,          Uint8,     1.0f,     0.0f,      3.0f,      0.0f,     0},
    {kBbms_TMS_BBMSFltCode,           9,       6,           74,          Uint8,     1.0f,     0.0f,      63.0f,     0.0f,     0},
    {kBbms_BMS_TMSReqMode,            10,      2,           80,          Uint8,     1.0f,     0.0f,      3.0f,      0.0f,     0},
    {kBbms_TMS_BMSMode,               10,      2,           82,          Uint8,     1.0f,     0.0f,      3.0f,      0.0f,     0},
    {kBbms_TMSNo,                     10,      4,           84,          Uint8,     1.0f,     0.0f,      15.0f,     0.0f,     0},
    {kBbms_TMSCoolState,              11,      8,           88,          Uint8,     1.0f,     0.0f,      255.0f,    0.0f,     0},
};

/*============================================================
 * ParaThr_TMS - TMS阈值参数 (8 Bytes) 
 * 上位机获取TMS参数 CmdGroup: 0x02, CmdId: 0x06)
 * 上位机设置TMS参数 CmdGroup: 0x02, CmdId: 0x07)
 * 对应消息ID: ParaThr_TMS 
 *============================================================*/
const bmsPointAttr_t bbms_ParaThr_TMS_PointAttr[] =
{
    /* pointId,                          dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,  offset,  maxVal,   minVal,  repeatCnt */
    {kBbms_CcTHMC_EntHeatMinTemLpLmt,      0,       8,           0,             Int8,     1.0f,   -40.0f,  215.0f,  -40.0f,  0},
    {kBbms_CcTHMC_EntHeatAvgTemLpLmt,      1,       8,           8,             Int8,     1.0f,   -40.0f,  215.0f,  -40.0f,  0},
    {kBbms_CcTHMC_ExtHeatMinTemLowLmt,     2,       8,           16,            Int8,     1.0f,   -40.0f,  215.0f,  -40.0f,  0},
    {kBbms_CcTHMC_ExtHeatAvgTemLowLmt,     3,       8,           24,            Int8,     1.0f,   -40.0f,  215.0f,  -40.0f,  0},
    {kBbms_CcTHMC_EntCoolMaxTemLowLimtA,   4,       8,           32,            Int8,     1.0f,   -40.0f,  215.0f,  -40.0f,  0},
    {kBbms_CcTHMC_EntCoolAvgTemLowLimtA,   5,       8,           40,            Int8,     1.0f,   -40.0f,  215.0f,  -40.0f,  0},
    {kBbms_CcTHMC_ExtCoolMaxTemUpLimtA,    7,       8,           48,            Int8,     1.0f,   -40.0f,  215.0f,  -40.0f,  0},
    {kBbms_CcTHMC_ExtCoolAvgTemUpLimtA,    7,       8,           56,            Int8,     1.0f,   -40.0f,  215.0f,  -40.0f,  0},
};


/*============================================================
 * HMI_BBMSDOCtrl - HMI三级板DO控制命令 (2 Bytes) cmdGroup: 0x02, CmdId: 10
 * 对应消息ID: HMI_BBMSDOCtrl
 *============================================================*/
const bmsPointAttr_t bbmsCmd02_10_Hmi_BBMSDO_Ctrl_PointAttr[] =
{
    /* pointId,                      dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,    minVal,   repeatCnt */
    {kBbms_HMI_BBMSDO0CtrlFlg,        0,       1,           0,           Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO1CtrlFlg,        0,       1,           1,           Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO2CtrlFlg,        0,       1,           2,           Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO3CtrlFlg,        0,       1,           3,           Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO4CtrlFlg,        0,       1,           4,           Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO5CtrlFlg,        0,       1,           5,           Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO6CtrlFlg,        0,       1,           6,           Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO7CtrlFlg,        0,       1,           7,           Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO8CtrlFlg,        1,       1,           8,           Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO9CtrlFlg,        1,       1,           9,           Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO10CtrlFlg,       1,       1,           10,          Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
    {kBbms_HMI_BBMSDO11CtrlFlg,       1,       1,           11,          Uint8,     1.0f,     0.0f,      1.0f,      0.0f,     0},
};

/*============================================================
 * HMI_RackFaultCali - Rack故障累积次数标定 (4001 Bytes) cmdGroup: 0x02, CmdId: 11,需要回复
 * 对应消息ID: HMI_RackFaultCali
 *============================================================*/
const bmsPointAttr_t bbmsCmd02_11_Hmi_RackFaultCali_PointAttr[] =
{
    /* pointId,                                  dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,    minVal,   repeatCnt */
    {kBbms_HMI_RackFltHistInfoIndicator,            0,       8,           0,           Uint8,     1.0f,     0.0f,      255.0f,    0.0f,     0},
    // Rack故障累积次数标定数组[200]，每个元素占2字节(16bit)，从Byte=2开始
    {kBbms_HMI_RackAllFltHistAccuCaliVal_0,         1,       16,          8,           Uint16,    1.0f,     0.0f,      65535.0f,  0.0f,     200},
};


/*============================================================
 * HMI_FltOvTiNbr - 读取-RBMS/BBMS故障越限次数 cmdGroup: 0x04, CmdId: 5
 * 对应消息ID: HMI_FltOvTiNbr
 *============================================================*/
const bmsPointAttr_t bbmsCmd04_5_Hmi_FltOvTiNbr_PointAttr[] =
{
    /* pointId,                                  dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,    minVal,   repeatCnt */
    {kBbms_BBMS_FltOvTiNbr_Start,                  0,       16,           0,           Uint16,     1.0f,     0.0f,      65525.0f,    0.0f,     200},
};


/*============================================================
 * BBMS_A_SOCInfo - 单芯计算的SOC以及各簇SOC结果 (120 Bytes) CmdGroup: 0x01, CmdId: 07 周期性上送
 *============================================================*/
const bmsPointAttr_t bbms_A_SOCInfo_PointAttr[] =
{
    /* pointId,                       dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,    minVal,   repeatCnt */
    /* ========== 1. 等间隔位置电芯SOC数组[16] (Byte 1-32, 共32字节) ========== */
    {kBbms_SaSOCB_BankCellSOCPctxT_0,     0,   16,  0,    Uint16, 0.01f, 0.0f, 100.01f, 0.0f, 16},

    /* ========== 2. 等间隔位置电芯SOC状态数组[16] (Byte 33-48, 共32字节) ========== */
    {kBbms_SaSOCB_BankCellSOCStatexT_0,  32,  8,   256,  Uint8,  1.0f,  0.0f, 10.0f,    0.0f, 16},

    /* ========== 3. Rack实时SOC数组[12] (Byte 49-72, 共24字节) ========== */
    {kBbms_SaSOCB_RackRealSOCPct_0,      48,  16,  384,  Uint16, 0.01f, 0.0f, 100.01f, 0.0f, 12},

    /* ========== 4. Rack最大SOC数组[12] (Byte 73-96, 共24字节) ========== */
    {kBbms_SaSOCB_RackMaxSOCPct_0,       72,  16,  576,  Uint16, 0.01f, 0.0f, 100.01f, 0.0f, 12},

    /* ========== 5. Rack最小SOC数组[12] (Byte 97-120, 共24字节) ========== */
    {kBbms_SaSOCB_RackMinSOCPct_0,       96,  16,  768,  Uint16, 0.01f, 0.0f, 100.01f, 0.0f, 12},
};


/*============================================================
 * BBMS_A_SOHInfo - 单芯计算的SOH以及各簇SOH结果 (168 Bytes)  CmdGroup: 0x01, CmdId: 08 周期性上送
 *============================================================*/
const bmsPointAttr_t bbms_A_SOHInfo_PointAttr[] =
{
    /* pointId,                      dataIdx  dataBitLen  dataStartBit  dataType  coeff   offset  maxVal    minVal  repeatCnt */
    // 等间隔电芯容量 16个 * 16bit
    {kBbms_SaSOHB_BankCellCapAhxT_0,      0,      16,        0,           Uint16,   0.1f,   0.0f,   1500.0f,  0.0f,   16 },
    // 等间隔电芯DFCL容量 16个 * 16bit
    {kBbms_SaSOHB_DFCLCapResultAhxT_0,   32,     16,        256,         Uint16,   0.1f,   0.0f,   1500.0f,  0.0f,   16 },
    // 等间隔电芯MFCL容量 16个 * 16bit
    {kBbms_SaSOHB_MFCLCapResultAhxT_0,   64,     16,        512,         Uint16,   0.1f,   0.0f,   1500.0f,  0.0f,   16 },
    // Rack实时容量 12个 * 16bit
    {kBbms_SaSOHB_RackRealCapAh_0,       96,     16,        768,         Uint16,   0.1f,   0.0f,   1500.0f,  0.0f,   12 },
    // Rack最大容量 12个 * 16bit
    {kBbms_SaSOHB_RackMaxCapAh_0,       120,     16,        960,         Uint16,   0.1f,   0.0f,   1500.0f,  0.0f,   12 },
    // Rack最小容量 12个 * 16bit
    {kBbms_SaSOHB_RackMinCapAh_0,       144,     16,       1152,         Uint16,   0.1f,   0.0f,   1500.0f,  0.0f,   12 },
};


/*============================================================
 * BBMS_A_Selfdr - 读取-自放电率演变信息 (13 Bytes) 上位机设置信息：cmdGroup: 0x01, CmdId: 10 周期性上送
 *============================================================*/
const bmsPointAttr_t bbmsCmd02_10_BBMS_A_Selfdr_PointAttr[] =
{
    /* pointId,                          dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,    minVal,   repeatCnt */
    {kBbms_SbEMCR_RTCnCMTimeVldFlg,            0,       8,           0,           Uint8,     1.0f,     0.0f,      255.0f,    0.0f,     0},
    {kBbms_ScEMCR_CellUsedMonth,               1,       16,          8,           Uint16,    0.1f,     0.0f,      6553.5f,   0.0f,     0},
    {kBbms_ScEMCR_CellDischargeRatePct,       3,       16,          24,          Uint16,    0.01f,    0.0f,      655.35f,   0.0f,     0},
};


static void bbms_hmi_pack_raw_bits(uint8_t *data, uint16_t startBit, uint16_t bitLen, uint64_t rawValue)
{
    for (uint16_t b = 0; b < bitLen; b++)
    {
        uint16_t bitPos  = startBit + b;
        uint16_t byteIdx = bitPos / 8U;
        uint16_t bitIdx  = bitPos % 8U;

        if ((rawValue >> b) & 0x01U)
        {
            data[byteIdx] |= (uint8_t)(1U << bitIdx);
        }
        else
        {
            data[byteIdx] &= (uint8_t)~(1U << bitIdx);
        }
    }
}

static uint64_t bbms_hmi_value_to_raw(uint8_t dataType, uint16_t bitLen, double value)
{
    switch (dataType)
    {
        case Uint8:
        case Uint16:
        case Uint32:
        case Uint64:
            return (uint64_t)value;

        case Int8:
        case Int16:
        case Int32:
        case Int64:
        {
            int64_t signedValue = (int64_t)value;
            if (bitLen < 64)
            {
                int64_t signBit = 1LL << (bitLen - 1U);
                int64_t mask    = (~0LL) << bitLen;
                if (signedValue & signBit)
                {
                    signedValue |= mask;
                }
            }
            return (uint64_t)signedValue;
        }

        default:
            return (uint64_t)value;
    }
}

static uint16_t bbms_hmi_append_raw_value(uint8_t *data, uint16_t dataIdx, uint8_t dataType, double value)
{
    switch (dataType)
    {
        case Uint8:
            data[dataIdx++] = (uint8_t)value;
            break;

        case Int8:
            data[dataIdx++] = (int8_t)value;
            break;

        case Uint16:
            data[dataIdx++] = (uint16_t)value & 0xFF;
            data[dataIdx++] = ((uint16_t)value >> 8) & 0xFF;
            break;

        case Int16:
            data[dataIdx++] = (int16_t)value & 0xFF;
            data[dataIdx++] = ((int16_t)value >> 8) & 0xFF;
            break;

        case Uint32:
            data[dataIdx++] = (uint32_t)value & 0xFF;
            data[dataIdx++] = ((uint32_t)value >> 8) & 0xFF;
            data[dataIdx++] = ((uint32_t)value >> 16) & 0xFF;
            data[dataIdx++] = ((uint32_t)value >> 24) & 0xFF;
            break;

        case Int32:
            data[dataIdx++] = (int32_t)value & 0xFF;
            data[dataIdx++] = ((int32_t)value >> 8) & 0xFF;
            data[dataIdx++] = ((int32_t)value >> 16) & 0xFF;
            data[dataIdx++] = ((int32_t)value >> 24) & 0xFF;
            break;

        default:
            KITLOG(LOG_APP_EN, ERROR_EN, "Unsupported dataType: %d", dataType);
            break;
    }

    return dataIdx;
}


static void generate_bbms_info_report_with_route(uint16_t devId, const bmsPointAttr_t *pointAttr,
                                                 uint16_t pointCount, uint16_t cmdGroup,
                                                 uint16_t cmdId, uint8_t src, uint8_t srcSub,
                                                 uint8_t dest, uint8_t destSub)
{
    uint8_t  data[RBMS_MAX_DATA_LEN] = {0};
    uint16_t payloadLen = 0;
    uint16_t maxEndBit  = 0;
    uint16_t i, j;

    for (i = 0; i < pointCount; i++)
    {
        bmsPointAttr_t *attr = &pointAttr[i];
        uint16_t repeatTimes = (attr->repeatCnt == 0) ? 1 : attr->repeatCnt;

        for (j = 0; j < repeatTimes; j++)
        {
            uint32_t currentPointId = attr->pointId + j;

            double value = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, devId, currentPointId);

            if (attr->coeff < 0.000001f)
            {
                attr->coeff = 1.0f;
            }
            value = (value - attr->offset) / attr->coeff;

            uint16_t startBit = attr->dataStartBit + j * attr->dataBitLen;
            uint64_t rawValue = bbms_hmi_value_to_raw(attr->dataType, attr->dataBitLen, value);
            bbms_hmi_pack_raw_bits(data, startBit, attr->dataBitLen, rawValue);

            uint16_t endBit = startBit + attr->dataBitLen;
            if (endBit > maxEndBit)
            {
                maxEndBit = endBit;
            }
        }
    }

    payloadLen = (maxEndBit + 7U) / 8U;

    if (payloadLen > RBMS_MAX_PAYLOAD_LEN)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "BBMS info payload overflow cmdGroup=0x%02X cmdId=0x%02X len=%u",
               cmdGroup, cmdId, payloadLen);
        payloadLen = RBMS_MAX_PAYLOAD_LEN;
    }

    SrcDestMsgV2 srcDestMsg = {0};
    srcDestMsg.src = src;
    srcDestMsg.srcSub = srcSub;
    srcDestMsg.dest = dest;
    srcDestMsg.destSub = destSub;

    bbmsmCtrlTansportMsg.transportType = kTransportType_0x01;
    bbmsmCtrlTansportMsg.frameId++;

    bms_queue_msg_t msg =
        createSendFrameQueueItem(&srcDestMsg, &bbmsmCtrlTansportMsg, cmdGroup, cmdId, data, payloadLen);

    bbms_info_enqueue(&msg);
}

// 生成上送报文，上送至 HMI（默认 BBMS_M -> HMI）
void generate_bbms_info_report(uint16_t devId, const bmsPointAttr_t *pointAttr, uint16_t pointCount,
                               uint16_t cmdGroup, uint16_t cmdId)
{
    generate_bbms_info_report_with_route(devId, pointAttr, pointCount, cmdGroup, cmdId,
                                         BBMSM_DEV_TYPE_SIGN, BBMSM_SUB_TYPE_SIGN,
                                         HMI_DEV_TYPE_SIGN, HMI_SUB_TYPE_SIGN);
}



//存储所有Rack的BBMS控制命令
void generate_bbms_crtl_info_report(uint16_t devId)
{
    SrcDestMsgV2 srcDestMsg = {0};
    srcDestMsg.src = BBMSM_DEV_TYPE_SIGN;
    srcDestMsg.srcSub = BBMSM_SUB_TYPE_SIGN;
    srcDestMsg.dest = HMI_BBMSA_DEV_TYPE_SIGN;
    srcDestMsg.destSub = HMI_BBMSA_SUB_TYPE_SIGN;
    
    bbmsmCtrlTansportMsg.transportType = kTransportType_0x02;
    bbmsmCtrlTansportMsg.frameId++;

    bbms_ctrl_prepare_for_send(devId);
    bms_queue_msg_t msg = createSendFrameQueueItem( &srcDestMsg, &bbmsmCtrlTansportMsg, 0x03, 0x07, (uint8_t *)&bbms_ctrl, sizeof(bbms_ctrl));
    bbms_info_enqueue(&msg);
}


//解析pointAttr表，解析HMI发送的数据，入内存数据库
void parse_bbms_rev_data(uint16_t devId,const bmsPointAttr_t *pointAttr,uint16_t pointCount,uint16_t cmdGroup,uint16_t cmdId,uint8_t *recvData,uint16_t recvDataLen)
{
    devId = devId - 1;
    for(uint16_t i = 0; i < pointCount; i++)
    {
        bmsPointAttr_t *attr = &pointAttr[i];
        uint16_t repeat = (attr->repeatCnt == 0) ? 1 : attr->repeatCnt;
        for(uint16_t j = 0; j < repeat; j++)
        {
            //计算当前元素的起始bit
            uint16_t startBit = attr->dataStartBit + j * attr->dataBitLen;
            uint16_t bitLen   = attr->dataBitLen;
            uint64_t rawValue = 0;
            //bit提取
            for(uint16_t b = 0; b < bitLen; b++)
            {
                uint16_t bitPos   = startBit + b;
                uint16_t byteIdx  = bitPos / 8;
                uint16_t bitIdx   = bitPos % 8;

                if(byteIdx >= recvDataLen)
                    break;

                uint8_t bitVal = (recvData[byteIdx] >> bitIdx) & 0x01;
                rawValue |= ((uint64_t)bitVal << b);
            }

            double value = 0.0;

            //类型转换
            switch(attr->dataType)
            {
                case Uint8:
                case Uint16:
                case Uint32:
                case Uint64:
                {
                    value = (double)rawValue;
                    break;
                }

                case Int8:
                case Int16:
                case Int32:
                case Int64:
                {
                    uint64_t signBit = 1ULL << (bitLen - 1);
                    if(rawValue & signBit)
                    {
                        uint64_t mask = (~0ULL) << bitLen;
                        rawValue |= mask;
                    }
                    value = (double)((int64_t)rawValue);
                    break;
                }

                case Float32:
                {
                    float f;
                    uint32_t tmp = (uint32_t)rawValue;
                    memcpy(&f, &tmp, sizeof(float));
                    value = (double)f;
                    break;
                }

                case Float64:
                {
                    double d;
                    uint64_t tmp = rawValue;
                    memcpy(&d, &tmp, sizeof(double));
                    value = d;
                    break;
                }

                default:
                    continue;
            }

            //系数 + 偏移
            value = value * attr->coeff + attr->offset;

            //限幅
            if(value > attr->maxVal)
            {
                value = attr->maxVal;
            }
            if(value < attr->minVal)
            {
                value = attr->minVal;
            }

            setRtdbPointValue(Rtdb_ShMem,kDev_Type_BBMS,devId,attr->pointId, value);
        }
    }
}


/*
 * 生成需要回复的报文，并如入控制队列，等待上位机发送线程上送
*/
void generate_bbms_ctrl_and_read_reply(uint16_t devId, const bmsPointAttr_t *pointAttr, uint16_t pointCount, uint16_t cmdGroup, uint16_t cmdId)
{
    uint8_t  data[RBMS_MAX_DATA_LEN] = {0};
    uint16_t dataIdx   = 0;  // 数据字节索引
    uint16_t i, j;

    // 遍历所有配置点
    for (i = 0; i < pointCount; i++)
    {
        bmsPointAttr_t *attr = &pointAttr[i];
        uint16_t repeatTimes = (attr->repeatCnt == 0) ? 1 : attr->repeatCnt;

        // 循环读取连续数据（支持 repeatCnt）
        for (j = 0; j < repeatTimes; j++)
        {
            // 读取实时值（支持连续ID：pointId + j）
            uint32_t currentPointId = attr->pointId + j;
            double value = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, devId, currentPointId);

            // 协议计算：物理值 → 原始值
            value = (value - attr->offset) / attr->coeff;

            /* 小端序填入 data 缓冲区 */
            dataIdx = bbms_hmi_append_raw_value(data, dataIdx, attr->dataType, value);
        }
    }

    if (dataIdx > RBMS_MAX_PAYLOAD_LEN)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "BBMS ctrl reply payload overflow cmdGroup=0x%02X cmdId=0x%02X len=%u",
               cmdGroup, cmdId, dataIdx);
        dataIdx = RBMS_MAX_PAYLOAD_LEN;
    }

    SrcDestMsgV2 srcDestMsg = {0};
    srcDestMsg.src = BBMSM_DEV_TYPE_SIGN;
    srcDestMsg.srcSub = BBMSM_SUB_TYPE_SIGN;
    srcDestMsg.dest = HMI_BBMSA_DEV_TYPE_SIGN;
    srcDestMsg.destSub = HMI_BBMSA_SUB_TYPE_SIGN;

    bbmsmCtrlTansportMsg.transportType = kTransportType_0x02;
    bbmsmCtrlTansportMsg.frameId++;

    bms_queue_msg_t msg = createSendFrameQueueItem(&srcDestMsg, &bbmsmCtrlTansportMsg, cmdGroup, cmdId, data, dataIdx);

    uplink_bbms_ctrl_reply_enqueue(&msg);
}


/*
 * 生成需要回复的报文，并如入控制队列，等待上位机发送线程上送
*/
void generate_bbms_ctrl_and_read_replyV2(uint16_t devId, const bmsPointAttr_t *pointAttr, uint16_t pointCount, uint16_t cmdGroup, uint16_t cmdId,uint8_t src,uint8_t src_sub,uint8_t dest,uint8_t dest_sub)
{
    devId = devId - 1;
    uint8_t  data[RBMS_MAX_DATA_LEN] = {0};
    uint16_t dataIdx   = 0;  // 数据字节索引
    uint16_t i, j;

    // 遍历所有配置点
    for (i = 0; i < pointCount; i++)
    {
        bmsPointAttr_t *attr = &pointAttr[i];
        uint16_t repeatTimes = (attr->repeatCnt == 0) ? 1 : attr->repeatCnt;

        // 循环读取连续数据（支持 repeatCnt）
        for (j = 0; j < repeatTimes; j++)
        {
            // 读取实时值（支持连续ID：pointId + j）
            uint32_t currentPointId = attr->pointId + j;
            double value = getRtdbPointValue(Rtdb_ShMem, kDev_Type_BBMS, devId, currentPointId);

            // 协议计算：物理值 → 原始值
            value = (value - attr->offset) / attr->coeff;

            /* 小端序填入 data 缓冲区 */
            dataIdx = bbms_hmi_append_raw_value(data, dataIdx, attr->dataType, value);
        }
    }

    if (dataIdx > RBMS_MAX_PAYLOAD_LEN)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "BBMS ctrl replyV2 payload overflow cmdGroup=0x%02X cmdId=0x%02X len=%u",
               cmdGroup, cmdId, dataIdx);
        dataIdx = RBMS_MAX_PAYLOAD_LEN;
    }

    SrcDestMsgV2 srcDestMsg = {0};
    srcDestMsg.src = src;
    srcDestMsg.srcSub = src_sub;
    srcDestMsg.dest = dest;
    srcDestMsg.destSub = dest_sub;

    bbmsmCtrlTansportMsg.transportType = kTransportType_0x02;
    bbmsmCtrlTansportMsg.frameId++;

    bms_queue_msg_t msg = createSendFrameQueueItem(&srcDestMsg, &bbmsmCtrlTansportMsg, cmdGroup, cmdId, data, dataIdx);

    uplink_bbms_ctrl_reply_enqueue(&msg);
}



/* 解析RBMS数据
 * RBMS通信协议定义,从1字节开始解析，实际索引需要减1
| Byte1 | Byte2-3          | Byte4-5 | Byte6 | Byte7  | Byte8 | Byte9   | Byte10        | Byte11  | Byte12   | Byte13 | Byte14-n |
| ----- | ---------------- | ------- | ----- | ------ | ----- | ------- | ------------- | ------- | -------- | ------ | -------- |
| Head  | Version&datalen  | CRC16   | src   | srcSub | dest  | destSub | transportType | frameId | cmdGroup | cmdId  | data     |
| 0xA5  | version=2        |         |       |        |       |         |               |         |          |        |          |
*/
int8_t bbms_uplink_recv_callback(void *arg, int fd)
{
   tcp_server_lib_t *p_tcp = (tcp_server_lib_t *)arg;

   if (p_tcp->recv_len <= 0)
   {
      return -1;
   }
   // 解析源地址
   uint8_t src = p_tcp->recv_buf[5];
   uint8_t srcSub = p_tcp->recv_buf[6];

    // 解析目的地址
    uint8_t dest = p_tcp->recv_buf[7];
    uint8_t destSub = p_tcp->recv_buf[8];

    uint8_t cmdGroup = p_tcp->recv_buf[11];
    uint8_t cmdId = p_tcp->recv_buf[12];

    // 如果是发给RBMS的命令，入队到全局send_queue
    if (dest == RBMS_DEV_TYPE_SIGN && destSub < 20) //当子节点为0是，表示组播，需要广播给所有RBMS设备，在bbms与rbms通讯现场中处理发给所有rbms
    {
        bms_queue_msg_t msg;
        msg.len = p_tcp->recv_len;
        memcpy(msg.data, p_tcp->recv_buf, p_tcp->recv_len);
        send_queue_enqueue(&msg);
        KITLOG(LOG_APP_EN, INFO_EN, "Received cmd from uplink, dest=0x%02X, destSub=%d, fd=%d", dest, destSub, fd);
    }
    //这种应该不存在的情况
    else if(dest == HMI_BBMSA_DEV_TYPE_SIGN && destSub == HMI_BBMSA_SUB_TYPE_SIGN)
    {
        // 解析上位机广播的命令
        bms_queue_msg_t msg;
        msg.len = p_tcp->recv_len;
        memcpy(msg.data, p_tcp->recv_buf, p_tcp->recv_len);
        send_queue_enqueue(&msg);
        KITLOG(LOG_APP_EN, INFO_EN, "Received cmd from uplink, dest=0x%02X, destSub=%d, fd=%d", dest, destSub, fd);
    }
    else if((dest == BBMSM_DEV_TYPE_SIGN && destSub == BBMSM_SUB_TYPE_SIGN) || (dest == BBMSA_DEV_TYPE_SIGN && destSub == BBMSA_SUB_TYPE_SIGN))
    {
        switch(cmdGroup)
        {
            case 0x01:
                if(cmdId == 0x10) //读取-自放电率演变信息
                {
                    generate_bbms_ctrl_and_read_replyV2(src, bbmsCmd02_10_BBMS_A_Selfdr_PointAttr, sizeof(bbmsCmd02_10_BBMS_A_Selfdr_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }
                break;
            case 0x02:
                if(cmdId == 0x03) ////HMI TMS控制指令(BBMS)
                {
                    uint16_t pointCount = sizeof(bbmsCmd02_03_Hmi_Tms_Control_PointAttr) / sizeof(bmsPointAttr_t);
                    parse_bbms_rev_data(src,bbmsCmd02_03_Hmi_Tms_Control_PointAttr,pointCount,cmdGroup,cmdId, &p_tcp->recv_buf[13],p_tcp->recv_len-13);
                    memcpy(&bbmsHmiTmsCtrlWord, &p_tcp->recv_buf[13], 4); // HMI_Tms_Control指令前7字节为控制字，直接存储到全局变量

                    //生成回复
                    generate_bbms_ctrl_and_read_replyV2(src, bbmsCmd02_03_Hmi_Tms_Control_PointAttr, sizeof(bbmsCmd02_03_Hmi_Tms_Control_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }
                else if(cmdId == 0x04) ////HMI_BankDOCtrl HMI三级板DO控制指令（灯）
                {
                    uint16_t pointCount = sizeof(bbmsCmd02_04_Hmi_DO_Control_PointAttr) / sizeof(bmsPointAttr_t);
                    parse_bbms_rev_data(src,bbmsCmd02_04_Hmi_DO_Control_PointAttr,pointCount,cmdGroup,cmdId, &p_tcp->recv_buf[13],p_tcp->recv_len-13);
                    memcpy(&bbmsHmiDoLightCtrlWord, &p_tcp->recv_buf[13], 1); // HMI_BankDOCtrl指令前2字节为控制字，直接存储到全局变量

                    //生成回复
                    generate_bbms_ctrl_and_read_replyV2(src, bbmsCmd02_04_Hmi_DO_Control_PointAttr, sizeof(bbmsCmd02_04_Hmi_DO_Control_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }
                else if(cmdId == 0x06) //读取-TMS阈值参数(BBMS)
                {
                    //generate_bbms_ctrl_and_read_reply(src, bbms_ParaThr_TMS_PointAttr, sizeof(bbms_ParaThr_TMS_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId);
                    generate_bbms_ctrl_and_read_replyV2(src, bbms_ParaThr_TMS_PointAttr, sizeof(bbms_ParaThr_TMS_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }
                else if(cmdId == 0x07) //写入-TMS阈值参数(BBMS)
                {
                    uint16_t pointCount = sizeof(bbms_ParaThr_TMS_PointAttr) / sizeof(bmsPointAttr_t);
                    parse_bbms_rev_data(src,bbms_ParaThr_TMS_PointAttr,pointCount,cmdGroup,cmdId, &p_tcp->recv_buf[13],p_tcp->recv_len-13);
                    memcpy(&bbmsHmiParaThrTmsCtrlWord, &p_tcp->recv_buf[13], 8); // HMI_BBMSDOCtrl指令前2字节为控制字，直接存储到全局变量

                    //生成回复
                    generate_bbms_ctrl_and_read_replyV2(src, bbms_ParaThr_TMS_PointAttr, sizeof(bbms_ParaThr_TMS_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }
                else if(cmdId == 10) //HMI Control HMI 控制指令
                {
                    uint16_t pointCount = sizeof(bbmsCmd02_10_Hmi_BBMSDO_Ctrl_PointAttr) / sizeof(bmsPointAttr_t);
                    parse_bbms_rev_data(src,bbmsCmd02_10_Hmi_BBMSDO_Ctrl_PointAttr,pointCount,cmdGroup,cmdId, &p_tcp->recv_buf[13],p_tcp->recv_len-13);
                    memcpy(&bbmsHmiBbmsDoCtrlWord, &p_tcp->recv_buf[13], 2); // HMI_BBMSDOCtrl指令前2字节为控制字，直接存储到全局变量

                    //生成回复
                    generate_bbms_ctrl_and_read_replyV2(src, bbmsCmd02_10_Hmi_BBMSDO_Ctrl_PointAttr, sizeof(bbmsCmd02_10_Hmi_BBMSDO_Ctrl_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }
                else if(cmdId == 11) //HMI RackFaultCali Rack故障累积次数标定
                {
                    uint16_t pointCount = sizeof(bbmsCmd02_11_Hmi_RackFaultCali_PointAttr) / sizeof(bmsPointAttr_t);
                    parse_bbms_rev_data(src,bbmsCmd02_11_Hmi_RackFaultCali_PointAttr,pointCount,cmdGroup,cmdId, &p_tcp->recv_buf[13],p_tcp->recv_len-13);
                   
                    //生成回复
                    generate_bbms_ctrl_and_read_replyV2(src, bbmsCmd02_11_Hmi_RackFaultCali_PointAttr, sizeof(bbmsCmd02_11_Hmi_RackFaultCali_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }
                break;
            case 0x04:
                //HMI Control HMI 控制指令
                if(cmdId == 0x04)
                {
                    uint16_t pointCount = sizeof(bbmsCmd04_04_Hmi_Control_PointAttr) / sizeof(bmsPointAttr_t);
                    parse_bbms_rev_data(src,bbmsCmd04_04_Hmi_Control_PointAttr,pointCount,cmdGroup,cmdId, &p_tcp->recv_buf[13],p_tcp->recv_len-13);
                    memcpy(&bbmsHmiCtrlWord, &p_tcp->recv_buf[13], 7); // HMI_Control指令前7字节为控制字，直接存储到全局变量

                    //生成回复
                    generate_bbms_ctrl_and_read_replyV2(src, bbmsCmd04_04_Hmi_Control_PointAttr, sizeof(bbmsCmd04_04_Hmi_Control_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }   
                else if(cmdId == 5) //HMI FltOvTiNbr 读取-RBMS/BBMS故障越限次数
                {
                    generate_bbms_ctrl_and_read_replyV2(src, bbmsCmd04_5_Hmi_FltOvTiNbr_PointAttr, sizeof(bbmsCmd04_5_Hmi_FltOvTiNbr_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }
                else if(cmdId == 8) //读取-故障使能数组
                {
                    generate_bbms_ctrl_and_read_replyV2(src, bbmsCmd04_08_HMI_FltEna_PointAttr, sizeof(bbmsCmd04_08_HMI_FltEna_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }
                else if(cmdId == 9) //写入-故障使能数组
                {
                    uint16_t pointCount = sizeof(bbmsCmd04_08_HMI_FltEna_PointAttr) / sizeof(bmsPointAttr_t);
                    parse_bbms_rev_data(src,bbmsCmd04_08_HMI_FltEna_PointAttr,pointCount,cmdGroup,cmdId, &p_tcp->recv_buf[13],p_tcp->recv_len-13);

                    //生成回复
                    generate_bbms_ctrl_and_read_replyV2(src, bbmsCmd04_08_HMI_FltEna_PointAttr, sizeof(bbmsCmd04_08_HMI_FltEna_PointAttr) / sizeof(bmsPointAttr_t), cmdGroup, cmdId,dest,destSub,src,srcSub);
                }
                break;

            default:
                KITLOG(LOG_APP_EN, ERROR_EN, "Unsupported cmdGroup: %d", cmdGroup);
                break;
        }


    }

   return 0;
}


/**
 * @brief 上传BBMS总信息和故障列表（非故障类约 100ms；故障类见 generate_bbms_fault_uplink_info）
 *
 * @param devId 设备ID
 */
void generate_bbms_uplink_info(uint16_t devId)
{
    //上传BBMS总信息
    generate_bbms_info_report(devId, bbmsCmd02_01_BBMS_SumInfo_PointAttr, sizeof(bbmsCmd02_01_BBMS_SumInfo_PointAttr) / sizeof(bmsPointAttr_t), 0x02, 0x01);

    // BBMS_A 算法模块周期性上送 (cmdGroup 0x01, Matrix 1000ms, src=BBMS_A 0x01/0x02)
    generate_bbms_info_report_with_route(devId, bbms_A_SOCInfo_PointAttr,
                                         sizeof(bbms_A_SOCInfo_PointAttr) / sizeof(bmsPointAttr_t), 0x01,
                                         0x07, BBMSA_DEV_TYPE_SIGN, BBMSA_SUB_TYPE_SIGN, HMI_DEV_TYPE_SIGN,
                                         HMI_SUB_TYPE_SIGN);
    generate_bbms_info_report_with_route(devId, bbms_A_SOHInfo_PointAttr,
                                         sizeof(bbms_A_SOHInfo_PointAttr) / sizeof(bmsPointAttr_t), 0x01,
                                         0x08, BBMSA_DEV_TYPE_SIGN, BBMSA_SUB_TYPE_SIGN, HMI_DEV_TYPE_SIGN,
                                         HMI_SUB_TYPE_SIGN);

    
    //上送存储所有Rack的BBMS控制命令 bbms_ctrl_t bbms_ctrl 0x02 12
    //上送 BBMS_CtlWord (8 Bytes, Matrix V1.0.50) bbms_ctrl_t
    SrcDestMsgV2 srcDestMsg = {0};
    srcDestMsg.src = BBMSM_DEV_TYPE_SIGN;
    srcDestMsg.srcSub = BBMSM_SUB_TYPE_SIGN;
    srcDestMsg.dest = RBMS_DEV_TYPE_SIGN;
    srcDestMsg.destSub = RBMS_SUB_TYPE_SIGN;
    
    bbmsmCtrlTansportMsg.transportType = kTransportType_0x02;
    bbmsmCtrlTansportMsg.frameId++;

    bbms_ctrl_prepare_for_send(devId);
        
    bms_queue_msg_t msg = createSendFrameQueueItem( &srcDestMsg, &bbmsmCtrlTansportMsg, 0x02, 0x0C, (uint8_t *)&bbms_ctrl, sizeof(bbms_ctrl));
    bbms_info_enqueue(&msg);
    
    //上传热管理信息
    // generate_bbms_info_report(devId, bbmsCmd02_02_TmsSumInfo_PointAttr, sizeof(bbmsCmd02_02_TmsSumInfo_PointAttr) / sizeof(bmsPointAttr_t), 0x02, 0x02);

}

/**
 * @brief 上传 BBMS 故障类周期信息（Matrix 1000ms）
 *
 * @param devId 设备ID
 */
void generate_bbms_fault_uplink_info(uint16_t devId)
{
    //上传BBMS故障列表 (Matrix V1.0.50: cmdGroup 0x02 / cmdId 0x13)
    generate_bbms_info_report(devId, bbmsCmd02_13_BBMS_Fault_PointAttr,
                            sizeof(bbmsCmd02_13_BBMS_Fault_PointAttr) / sizeof(bmsPointAttr_t), 0x02,
                            0x13);

    // BBMS_A 算法模块周期性上送 (cmdGroup 0x01, Matrix 1000ms, src=BBMS_A 0x01/0x02)
    generate_bbms_info_report_with_route(devId, bbmsCmd01_09_BBMS_A_Fault_PointAttr,
                                         sizeof(bbmsCmd01_09_BBMS_A_Fault_PointAttr) /
                                             sizeof(bmsPointAttr_t),
                                         0x01, 0x09, BBMSA_DEV_TYPE_SIGN, BBMSA_SUB_TYPE_SIGN,
                                         HMI_DEV_TYPE_SIGN, HMI_SUB_TYPE_SIGN);
}





















