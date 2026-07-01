/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, Gary. All rights reserved.
 * @file            protocol_rbms.c
 * @brief           RBMS通信协议点表配置实现
 * @author          gary
 * @date            2026-03-27
 * @remark
 *****************************************************************************/
#include "protocol_bms.h"
#include <string.h>

/*
 * 点属性数组说明:
 *   {pointId, dataType, coeff, offset, maxVal, minVal}
 *   dataType: 0x00=无符号, 0x01=有符号(含负offset), 0x02=浮点/高精度
 * 
 * // 点位配置的数据类型枚举
typedef enum
{
    Data_Type_Start = 0,
    Uint8 = Data_Type_Start, //=0   无符号8位整形
    Int8,                    //=1   有符号8位整形
    Uint16,                  //=2   无符号16位整形
    Int16,                   //=3   有符号16位整形
    Uint32,                  //=4   无符号32位整形
    Int32,                   //=5   有符号32位整形
    Float32,                 //=6  32位单精度浮点数
    Uint64,                  //=7   无符号64位整形
    Int64,                   //=8   有符号64位整形
    Float64,                 //=9  64位双精度浮点数
    Data_Type_End
} data_type_e; //  数据传输类型
 *
 * coeff  : Resolution (分辨率/系数)
 * offset : offset (偏移量)
 * maxVal : Signal Max. Value (Valid)
 * minVal : Signal Min. Value (Valid)
 * repeatCnt: 重复点个数，针对数组类型 默认值为0表示单点数据，非零表示数组数据的个数
 */

/*============================================================
 * RBMS_SumInfo - Rack概要数据 (310 Bytes)
 * 对应消息ID: RBMS_SumInfo (cmdGroup: 0x03, cmdid: 0x01)
 * 依据 BMS2.0 LAN Matrix V1.0.50 Comm Matrix
 *============================================================*/
const bmsPointAttr_t rbmsCmd03_01_RBMS_SumInfo_PointAttr[] =
{
    /* pointId,                dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,          minVal,   repeatCnt */
    {kRbms_St,                 0,      8,          0,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_ChaSt,              1,      8,          8,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_SysOperSt,          2,      8,          16,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_ConSt,              3,      16,          24,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_ConFail,            5,      16,          40,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_V,                  7,      16,          56,           Uint16,    0.5f,   0.0f,  32767.5f,    0.0f,      0},
    {kRbms_DCBusV,             9,      16,          72,           Uint16,    0.5f,   0.0f,  32767.5f,    0.0f,      0},
    {kRbms_A_HighAccu,         11,      32,          88,           Int32,    0.001f,   -2000.0f,  4292967.295f,    -2000.0f,      0},
    {kRbms_IsoR,               15,      16,          120,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_HvBoxMaxTemp,       17,      16,          136,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_HvBoxMinTemp,       19,      16,          152,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_Reserved3,          21,      8,          168,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_LowPowSply,         22,      16,          176,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_RlyCtrlSeqStState,  24,      4,          192,           Uint8,    1.0f,   0.0f,  15.0f,    0.0f,      0},
    {kRbms_ReverConDetecOpenFlg, 24,    1,          196,           Uint8,    1.0f,   0.0f,  1.0f,    0.0f,      0},
    {kRbms_Reserved,           24,      3,          197,           Uint8,    1.0f,   0.0f,  7.0f,    0.0f,      0},
    {kRbms_Reserved1,          25,      8,          200,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_HisSleepTiWrMin,    26,      32,          208,           Uint32,    1.0f,   0.0f,  4294967295.0f,    0.0f,      0},
    {kRbms_HisRunTiWrMin,      30,      32,          240,           Uint32,    1.0f,   0.0f,  4294967295.0f,    0.0f,      0},
    {kRbms_LstSleepDurTiMin,   34,      32,          272,           Uint32,    1.0f,   0.0f,  4294967295.0f,    0.0f,      0},
    {kRbms_IDLearnCodeSt,      38,      8,          304,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_IsoRPos,            39,      16,          312,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_IsoRNeg,            41,      16,          328,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_CmdInhibitFlg,      43,      8,          344,           Uint8,    1.0f,   0.0f,  1.0f,    0.0f,      0},
    {kRbms_IslnHvVldNbr,       44,      8,          352,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_IslnInhibitNbr,     45,      8,          360,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_CellTMUXFaiIDNbr_Byte_1, 46,      8,          368,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      64},
    {kRbms_SoC,                110,      8,          880,           Uint8,    0.5f,   0.0f,  127.5f,    0.0f,      0},
    {kRbms_SoH,                111,      8,          888,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_SoEC,               112,      16,          896,           Uint16,    0.125f,   0.0f,  8191.875f,    0.0f,      0},
    {kRbms_SoED,               114,      16,          912,           Uint16,    0.125f,   0.0f,  8191.875f,    0.0f,      0},
    {kRbms_SoFPulseChrgCurr,   116,      16,          928,           Uint16,    0.125f,   0.0f,  8191.875f,    0.0f,      0},
    {kRbms_SoFPulseDisChrgCurr,  118,      16,          944,           Uint16,    0.125f,   0.0f,  8191.875f,    0.0f,      0},
    {kRbms_SoFPulseChrgPwr,    120,      16,          960,           Uint16,    0.125f,   0.0f,  8191.875f,    0.0f,      0},
    {kRbms_SoFPulseDisChrgPwr,  122,      16,          976,           Uint16,    0.125f,   0.0f,  8191.875f,    0.0f,      0},
    {kRbms_SoFContChrgCurr,    124,      16,          992,           Uint16,    0.125f,   0.0f,  8191.875f,    0.0f,      0},
    {kRbms_SoFContDisChrgCurr,  126,      16,          1008,           Uint16,    0.125f,   0.0f,  8191.875f,    0.0f,      0},
    {kRbms_SoFContChrgPwr,     128,      16,          1024,           Uint16,    0.125f,   0.0f,  8191.875f,    0.0f,      0},
    {kRbms_SoFContDisChrgPwr,  130,      16,          1040,           Uint16,    0.125f,   0.0f,  8191.875f,    0.0f,      0},
    {kRbms_CellVMax,           132,      14,          1056,           Uint16,    1.0f,   0.0f,  5000.0f,    0.0f,      0},
    {kRbms_CellVMaxPstn,       133,      11,          1070,           Uint16,    1.0f,   0.0f,  2047.0f,    0.0f,      0},
    {kRbms_CellVMin,           135,      14,          1081,           Uint16,    1.0f,   0.0f,  5000.0f,    0.0f,      0},
    {kRbms_CellVMinPstn,       136,      11,          1095,           Uint16,    1.0f,   0.0f,  2047.0f,    0.0f,      0},
    {kRbms_CellVAvg,           138,      14,          1106,           Uint16,    1.0f,   0.0f,  5000.0f,    0.0f,      0},
    {kRbms_ModTmpMax,          140,      16,          1120,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_Reserved4,          142,      8,          1136,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_ModTmpMin,          143,      16,          1144,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_MCURstRsnFltNbr,    145,      8,          1160,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_ModTmpAvg,          146,      16,          1168,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_MaxPoleTDegC,       148,      16,          1184,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_MaxPoleTIdxNbr,     150,      8,          1200,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_MinPoleTDegC,       151,      16,          1208,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_MinPoleTIdxNbr,     153,      8,          1224,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_MaxPoleTMdulIdxNbr,  154,      8,          1232,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_MinPoleTMdulIdxNbr,  155,      8,          1240,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_AvgPoleTDegC,       156,      16,          1248,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_CommType,           158,      8,          1264,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_StrCtrlHb,          159,      16,          1272,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_ResChAvlCapAh,      161,      16,          1288,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_ResDischAvlCapAh,   163,      16,          1304,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_LstFullChCapAh,     165,      16,          1320,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_LstFullDischCapAh,  167,      16,          1336,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_CycNbr,             169,      16,          1352,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_MaxPoleTMdulInnerIdxNbr,  171,      8,          1368,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_MinPoleTMdulInnerIdxNbr,  172,      8,          1376,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_AFEChipTmpMax,      173,      8,          1384,           Uint8,    1.0f,   -40.0f,  215.0f,    -40.0f,      0},
    {kRbms_AFEChipTmpMin,      174,      8,          1392,           Uint8,    1.0f,   -40.0f,  215.0f,    -40.0f,      0},
    {kRbms_MCURstRsnNbr,       175,      16,          1400,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_GenlVldFlg,         177,      16,          1416,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_IslnFinalBatV,      179,      16,          1432,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_IslnFinalPosV,      181,      16,          1448,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_IslnFinalNegV,      183,      16,          1464,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_IslnMeastStState,   185,      16,          1480,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_LstChEkWh,          187,      16,          1496,           Uint16,    0.1f,   0.0f,  6553.5f,    0.0f,      0},
    {kRbms_LstDchEkWh,         189,      16,          1512,           Uint16,    0.1f,   0.0f,  6553.5f,    0.0f,      0},
    {kRbms_BalBdTmpMax,        191,      16,          1528,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_RmngCellBalTiMax,   193,      16,          1544,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_RealSysSOHC,        195,      16,          1560,           Uint16,    0.01f,   0.0f,  655.35f,    0.0f,      0},
    {kRbms_RealSysCapAh,       197,      16,          1576,           Uint16,    0.1f,   0.0f,  6553.5f,    0.0f,      0},
    {kRbms_RealCellCapAh,      199,      16,          1592,           Uint16,    0.1f,   0.0f,  6553.5f,    0.0f,      0},
    {kRbms_SysMaxSOC,          201,      16,          1608,           Uint16,    0.01f,   0.0f,  655.35f,    0.0f,      0},
    {kRbms_SysMinSOC,          203,      16,          1624,           Uint16,    0.01f,   0.0f,  655.35f,    0.0f,      0},
    {kRbms_RealSysSOCPct,      205,      32,          1640,           Uint32,    0.01f,   0.0f,  100.0f,    0.0f,      0},
    {kRbms_AccuDisChrgAh2,     209,      32,          1672,           Uint32,    0.0625f,   0.0f,  268435455.9375f,    0.0f,      0},
    {kRbms_AccuDisChrgkWh2,    213,      32,          1704,           Uint32,    0.0625f,   0.0f,  268435455.9375f,    0.0f,      0},
    {kRbms_AccuChrgAh2,        217,      32,          1736,           Uint32,    0.0625f,   0.0f,  268435455.9375f,    0.0f,      0},
    {kRbms_AccuChrgkWh2,       221,      32,          1768,           Uint32,    0.0625f,   0.0f,  268435455.9375f,    0.0f,      0},
    {kRbms_BMSMaxFltLevel,     225,      8,          1800,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_MaxCellVMdulTDegC,  226,      16,          1808,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_MinCellVMdulTDegC,  228,      16,          1824,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_MaxCellVMdulTVldFlg,  230,      8,          1840,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_MinCellVMdulTVldFlg,  231,      8,          1848,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_A,                  232,      16,          1856,           Int16,    0.125f,   -2000.0f,  6191.875f,    -2000.0f,      0},
    {kRbms_A1,                 234,      16,          1872,           Int16,    0.125f,   -2000.0f,  6191.875f,    -2000.0f,      0},
    {kRbms_A2,                 236,      16,          1888,           Int16,    0.125f,   -2000.0f,  6191.875f,    -2000.0f,      0},
    {kRbms_BalBdTmpMin,        238,      16,          1904,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_PreChrgRT,          240,      8,          1920,           Uint8,    1.0f,   -40.0f,  215.0f,    -40.0f,      0},
    {kRbms_RlyCtrlCmd,         241,      8,          1928,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_FaultEvt,           242,      32,          1936,           Uint32,    1.0f,   0.0f,  4294967295.0f,    0.0f,      0},
    {kRbms_AttachState,        246,      8,          1968,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_Reserved2,          247,      8,          1976,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_MaxCellVMdulIdxNbr,  248,      16,          1984,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_MinCellVMdulIdxNbr,  250,      16,          2000,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_MaxTMdulIdxNbr,     252,      16,          2016,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_MinTMdulIdxNbr,     254,      16,          2032,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_MaxCellVMdulInnerIdxNbr,  256,      16,          2048,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_MinCellVMdulInnerIdxNbr,  258,      16,          2064,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_MaxTMdulInnerIdxNbr,  260,      16,          2080,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_MinTMdulInnerIdxNbr,  262,      16,          2096,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_DaisyChain1LastNode,  264,      8,          2112,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_DaisyChain2LastNode,  265,      8,          2120,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_ModTmpMaxPstn,      266,      16,          2128,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_ModTmpMinPstn,      268,      16,          2144,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_AccuDisChrgAh,      270,      32,          2160,           Uint32,    1.0f,   0.0f,  4294967295.0f,    0.0f,      0},
    {kRbms_AccuDisChrgkWh,     274,      32,          2192,           Uint32,    1.0f,   0.0f,  4294967295.0f,    0.0f,      0},
    {kRbms_AccuChrgAh,         278,      32,          2224,           Uint32,    1.0f,   0.0f,  4294967295.0f,    0.0f,      0},
    {kRbms_AccuChrgkWh,        282,      32,          2256,           Uint32,    1.0f,   0.0f,  4294967295.0f,    0.0f,      0},
    {kRbms_LstChCapAh,         286,      16,          2288,           Uint16,    0.1f,   0.0f,  6553.5f,    0.0f,      0},
    {kRbms_LstDchCapAh,        288,      16,          2304,           Uint16,    0.1f,   0.0f,  6553.5f,    0.0f,      0},
    {kRbms_FltSOPDischCoeffPct,  290,      8,          2320,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_FltSOPChCoeffPct,   291,      8,          2328,           Uint8,    1.0f,   0.0f,  255.0f,    0.0f,      0},
    {kRbms_CurrSnsrPowSplyVoltmV,  292,      16,          2336,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
    {kRbms_CtlBoxT0DegC,       294,      16,          2352,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_CtlBoxT1DegC,       296,      16,          2368,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_CtlBoxT2DegC,       298,      16,          2384,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_CtlBoxT3DegC,       300,      16,          2400,           Int16,    0.1f,   -40.0f,  6513.5f,    -40.0f,      0},
    {kRbms_PCSChVoltLim,       302,      16,          2416,           Uint16,    0.1f,   0.0f,  6553.5f,    0.0f,      0},
    {kRbms_PCSDchVoltLim,      304,      16,          2432,           Uint16,    0.1f,   0.0f,  6553.5f,    0.0f,      0},
    {kRbms_PCSSOP,             306,      16,          2448,           Uint16,    0.1f,   0.0f,  6553.5f,    0.0f,      0},
    {kRbms_PCSBMSSt,           308,      16,          2464,           Uint16,    1.0f,   0.0f,  65535.0f,    0.0f,      0},
};
/*============================================================
 * RBMS_Volt - 电芯电压及有效性、各AFE总压 (1012 Bytes)
 * 对应消息ID: RBMS_Volt (cmdGroup: 0x03, cmdid: 0x02)
 *============================================================*/
const bmsPointAttr_t rbmsCmd03_02_RBMS_Volt[] =
{
    /* pointId,                      dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,  offset,  maxVal,  minVal,  repeatCnt   */
    {kRbms_CellVolt_Validity_Start,  0,       8,           0,             Uint8,     1.0f,   0.0f,    255.0f, 0.0f,       52},
    {kRbms_CellVolt_Start,           52,      16,          416,           Uint16,    1.0f,   0.0f,    5000.0f, 0.0f,       416},
    {kRbms_AFEVolt_Start,            884,     32,          7072,          Uint32,    1.0f,   0.0f,    4294967295.0f,   0.0f,       32},
};

/*============================================================
 * RBMS_Temp - 电芯温度、极柱温度、Pack温度、均衡板温及有效性 (1188 Bytes)
 * 对应消息ID: RBMS_Temp (cmdGroup: 0x03, cmdid: 0x03)
 *============================================================*/
const bmsPointAttr_t rbmsCmd03_03_RBMS_Temp_CellT_PointAttr[] =
{
    /* pointId,                  dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,  offset,  maxVal,   minVal,  repeatCnt */
    {kRbms_TempInfo_Cell_Start,                 0,       16,          0,             Int16,     0.1f,   -40.0f,  6513.5f,  -40.0f,     416},
    {kRbms_TempInfo_PoleTemp_Start,             832,     16,          6656,          Int16,     0.1f,   -40.0f,  6513.5f,  -40.0f,     128},
    {kRbms_TempInfo_PackPosNegTemp_Start,       1088,    16,          8704,          Int16,     0.1f,   -40.0f,  6513.5f,  -40.0f,      16},
    {kRbms_TempInfo_BalBoardTempValidity_Start, 1120,    8,           8960,          Uint8,     1.0f,   0.0f,    255.0f, 0.0f,          4},
    {kRbms_TempInfo_BalBoardTemp_Start,         1124,    16,          8992,          Int16,     0.1f,   -40.0f,  6513.5f,  -40.0f,      32},
};

/*============================================================
 * RBMS_CellBalSt - 电芯均衡状态 (52 Bytes, 416 Bits)
 * 对应消息ID: RBMS_CellBalSt (cmdGroup: 0x03, cmdid: 0x04)
 *============================================================*/
const bmsPointAttr_t rbmsCmd03_04_RBMS_CellBalSt_PointAttr[] =
{
    /* pointId,                  dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,  maxVal,  minVal,  repeatCnt */
    {kRbms_CellBalStatus1_8,     0,       8,           0,             Uint8,      1.0f,     0.0f,    255.0f, 0.0f,      52},
};

/*============================================================
 * RBMS_CellSdr - 电芯自放电率 (416 Bytes)
 * 对应消息ID: RBMS_CellSdr (cmdGroup: 0x03, cmdid: 0x05)
 *============================================================*/
const bmsPointAttr_t rbmsCmd03_05_RBMS_CellSdr_PointAttr[] =
{
    /* pointId,              dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,  offset,  maxVal,   minVal,  repeatCnt  */
    {kRbms_CellSdrate_1,    0,       8,           0,             Uint8,      0.5f,   0.0f,    127.5f,   0.0f,       416},
};


/*============================================================
 * RBMS_Debug - 模型调试信息 (30 Bytes)
 * 对应消息ID: RBMS_Debug (cmdGroup: 0x03, cmdid: 23)
 * 依据 BMS2.0 LAN Matrix V1.0.50 Comm Matrix
 *============================================================*/
const bmsPointAttr_t rbmsCmd03_23_RBMS_Debug_PointAttr[] =
{
    /* pointId,                          dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,    offset,    maxVal,          minVal ,  repeatCnt*/
    {kRbms_SbCTSC_AllwPrechFlg,               0,       1,           0,             Uint8,     1.0f,     0.0f,      1.0f,            0.0f,      0},
    {kRbms_SbCTSC_BSWSMainNegRlyDryContactFlg,0,       1,           1,             Uint8,     1.0f,     0.0f,      1.0f,            0.0f,      0},
    {kRbms_SbCTSC_BSWSMainPosRlyDryContactFlg,0,       1,           2,             Uint8,     1.0f,     0.0f,      1.0f,            0.0f,      0},
    {kRbms_SbCTSC_SafetyState3TriggerFlg,     0,       1,           3,             Uint8,     1.0f,     0.0f,      1.0f,            0.0f,      0},
    {kRbms_ModeDebug_Reserved1,               0,       4,           4,             Uint8,     1.0f,     0.0f,      15.0f,           0.0f,      0},
    {kRbms_ScCTSC_HvPowupCmdNbr,              1,       8,           8,             Uint8,     1.0f,     0.0f,      255.0f,          0.0f,      0},
    {kRbms_ScCTSC_CtrlModeState,              2,       8,           16,            Uint8,     1.0f,     0.0f,      255.0f,          0.0f,      0},
    {kRbms_ScCTSC_MainPosRlyLsdDrvRawVoltV,   3,       16,          24,            Uint16,    0.1f,     0.0f,      6553.5f,         0.0f,      0},
    {kRbms_ScCTSC_MainNegRlyLsdDrvRawVoltV,   5,       16,          40,            Uint16,    0.1f,     0.0f,      6553.5f,         0.0f,      0},
    {kRbms_ScCTSC_GroundChannelRawVoltmV,     7,       16,          56,            Uint16,    1.0f,     0.0f,      65535.0f,        0.0f,      0},
    {kRbms_ScCTSC_MainPosRlyHsdDrvRawCurmA,   9,       16,          72,            Uint16,    0.1f,     0.0f,      6553.5f,         0.0f,      0},
    {kRbms_ScCTSC_PrechRlyHsdDrvRawCurmA,     11,      16,          88,            Uint16,    0.1f,     0.0f,      6553.5f,         0.0f,      0},
    {kRbms_ScCTSC_MainNegRlyHsdDrvRawCurmA,   13,      16,          104,           Uint16,    0.1f,     0.0f,      6553.5f,         0.0f,      0},
    {kRbms_ScCTSC_PrechOverTiCnt,             15,      8,           120,           Uint8,     1.0f,     0.0f,      255.0f,          0.0f,      0},
    {kRbms_ScBTCS_BSWSBatIDiagInfoNbr,        16,      2,           128,           Uint8,     1.0f,     0.0f,      3.0f,            0.0f,      0},
    {kRbms_ScBTCS_BSWSMsgAvlNbr,              16,      2,           130,           Uint8,     1.0f,     0.0f,      3.0f,            0.0f,      0},
    {kRbms_SbLVPH_CurrSnsrPowSplyNotOutRngFlg,16,     1,           132,           Uint8,     1.0f,     0.0f,      1.0f,            0.0f,      0},
    {kRbms_SbCTSC_RlyLoadBreakFlg,            16,      1,           133,           Uint8,     1.0f,     0.0f,      1.0f,            0.0f,      0},
    {kRbms_ModeDebug_Reserved2,               16,      2,           134,           Uint8,     1.0f,     0.0f,      3.0f,            0.0f,      0},
    {kRbms_ScCTSC_RlyLoadBreakCurrentA,       17,      32,          136,           Int32,     0.001f,   -2000.0f,  4292967.295f,    -2000.0f,      0},
    {kRbms_ScBTCS_BSWSMsgCtrResNbr,           21,      2,           168,           Uint8,     1.0f,     0.0f,      3.0f,            0.0f,      0},
    {kRbms_ScBTCS_BSWSMsgCRCResNbr,           21,      2,           170,           Uint8,     1.0f,     0.0f,      3.0f,            0.0f,      0},
    {kRbms_ScBTCS_BSWSCurValVldNbr,           21,      2,           172,           Uint8,     1.0f,     0.0f,      3.0f,            0.0f,      0},
    {kRbms_ScBTCS_CANBatIZeroDriftNbr,        21,      2,           174,           Uint8,     1.0f,     0.0f,      3.0f,            0.0f,      0},
    {kRbms_BSWSAFESampErrMonitor,             22,      32,          176,           Uint32,    1.0f,     0.0f,      4294967295.0f,   0.0f,      0},
    {kRbms_BSWEthPHYChipResetNbr,             26,      32,          208,           Uint32,    1.0f,     0.0f,      4294967295.0f,   0.0f,      0},
};


/*============================================================
 * RBMS_SOXdebugData1 - SOX算法调试信息 (60 Bytes)
 * 对应消息ID: RBMS_SOXdebugData1 (cmdGroup: 0x03, cmdid: 25)
 * SOX算法调试的输入数据，包含电流、电压、温度等
 * 依据 BMS2.0 LAN Matrix V1.0.50 Comm Matrix
 *============================================================*/
const bmsPointAttr_t rbmsCmd03_25_RBMS_SOXdebugData1_PointAttr[] =
{
    /* pointId,                             dataIdx,  dataBitLen,  dataStartBit,    dataType,       coeff,    offset,    maxVal,          minVal ,  repeatCnt*/
        
    {kRbms_ScSGPC_BatIA,                         0,       32,          0,             Int32,      0.001f,   -2000.0f,  2000.001f,    -2000.0f,      0},
    {kRbms_ScSGPC_MaxCellVmV,                     4,       16,          32,            Uint16,     1.0f,     0.0f,      5000.0f,        0.0f,      0},
    {kRbms_ScSGPC_MinCellVmV,                     6,       16,          48,            Uint16,     1.0f,     0.0f,      5000.0f,        0.0f,      0},
    {kRbms_ScSGPC_MaxCellTDegC,                   8,       16,          64,            Int16,      0.1f,     -40.0f,    125.1f,         -40.0f,      0},
    {kRbms_ScSGPC_MinCellTDegC,                   10,      16,          80,            Int16,      0.1f,     -40.0f,    125.1f,         -40.0f,      0},
    {kRbms_ScSGPC_AvgCellTDegC,                   12,      16,          96,            Int16,      0.1f,     -40.0f,    125.1f,         -40.0f,      0},
    {kRbms_ScCSPC_FullChEnaNbr,                   14,      8,           112,           Uint8,      1.0f,     0.0f,      255.0f,         0.0f,       0},
    {kRbms_ScCSPC_FullDischEnaNbr,                15,      8,           120,           Uint8,      1.0f,     0.0f,      255.0f,         0.0f,       0},
    {kRbms_ScHIST_HistAccuChCapAh,                16,      32,          128,           Uint32,     0.0625f,  0.0f,      268435455.9f,   0.0f,       0},
    {kRbms_ScHIST_HistAccuDischCapAh,             20,      32,          160,           Uint32,     0.0625f,  0.0f,      268435455.9f,   0.0f,       0},
    {kRbms_ScSGPC_BatIStNbr,                      24,      8,           192,           Uint8,      1.0f,     0.0f,      255.0f,         0.0f,       0},
    {kRbms_ScSGPC_LstSleepDurTiMin,               25,      32,          200,           Uint32,     1.0f,     0.0f,      4294967295.0f,  0.0f,       0},
    {kRbms_ScSGPC_HisSleepTiMin,                  29,      32,          232,           Uint32,     1.0f,     0.0f,      4294967295.0f,  0.0f,       0},
    {kRbms_ScSGPC_HisRunTiMin,                    33,      32,          264,           Uint32,     1.0f,     0.0f,      4294967295.0f,  0.0f,       0},
    {kRbms_ScHIST_CycNbr,                         37,      16,          296,           Uint16,     1.0f,     0.0f,      65535.0f,       0.0f,       0},
    {kRbms_BSWSAllSOHCalIndicator,                39,      8,           312,           Uint8,      1.0f,     0.0f,      255.0f,         0.0f,       0},
    {kRbms_BSWSAllSOHCalValPct,                   40,      8,           320,           Uint8,      1.0f,     0.0f,      255.0f,         0.0f,       0},
    {kRbms_ScBTCS_RackChrgCapmAh,                 41,      32,          328,           Uint32,     0.001f,   0.0f,      4294967295.0f,  0.0f,       0},
    {kRbms_ScBTCS_RackDsChrgCapmAh,               45,      32,          360,           Uint32,     0.001f,   0.0f,      4294967295.0f,  0.0f,       0},
    {kRbms_SoxInput_Reserver1,                    49,      8,           392,           Uint8,      1.0f,     0.0f,      255.0f,         0.0f,       0},
    {kRbms_SoxInput_Reserver2,                    50,      16,          400,           Uint16,     1.0f,     0.0f,      65535.0f,       0.0f,       0},
    {kRbms_BSWSRSOXHistInfoIndicator,             52,      8,           416,           Uint8,      1.0f,     0.0f,      255.0f,         0.0f,       0},
    {kRbms_BSWSAllSOCCalIndicator,                53,      8,           424,           Uint8,      1.0f,     0.0f,      255.0f,         0.0f,       0},
    {kRbms_BSWSAllSOCCalValPct,                   54,      8,           432,           Uint8,      1.0f,     0.0f,      255.0f,         0.0f,       0},
    {kRbms_SbSGPC_BatIVldFlg,                     55,      1,           440,           Uint8,      1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_SbSGPC_GenlCellVVldFlg,                55,      1,           441,           Uint8,      1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_SbSGPC_GenlCellTVldFlg,                55,      1,           442,           Uint8,      1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_SbSGPC_MaxCellVMdulTVldFlg,            55,      1,           443,           Uint8,      1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_SbSGPC_MinCellVMdulTVldFlg,            55,      1,           444,           Uint8,      1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_SoxInput_Reserver3,                    55,      3,           445,           Uint8,      1.0f,     0.0f,      7.0f,           0.0f,       0},
    {kRbms_ScSGPC_MaxCellVMdulTDegC,              56,      16,          448,           Int16,      0.1f,     -40.0f,    125.1f,         -40.0f,      0},
    {kRbms_ScSGPC_MinCellVMdulTDegC,              58,      16,          464,           Int16,      0.1f,     -40.0f,    125.1f,         -40.0f,      0},
};

/*============================================================
 * RBMS_SOXdebugData2 - SOX算法调试信息2 (63 Bytes)
 * 对应消息ID: RBMS_SOXdebugData2 (cmdGroup: 0x03, cmdid: 26)
 * SOX算法调试的输出数据，包含SOC、SOHC、容量等
 * 依据 BMS2.0 LAN Matrix V1.0.50 Comm Matrix
 *============================================================*/
const bmsPointAttr_t rbmsCmd03_26_RBMS_SOXdebugData2_PointAttr[] =
{
    /* pointId,                             dataIdx,  dataBitLen,  dataStartBit,    dataType,       coeff,    offset,    maxVal,          minVal ,  repeatCnt*/
    {kRbms_SaSGPC_CellVmVxT_Start,              0,       16,          0,             Uint16,      1.0f,     0.0f,      5000.0f,        0.0f,       10},
    {kRbms_ScSOHA_RealSysSOHCPct,               20,      16,          160,           Uint16,      0.01f,    0.0f,      100.0f,         0.0f,       0},
    {kRbms_ScSOHA_DispSysSOHCPct,               22,      16,          176,           Uint16,      0.01f,    0.0f,      100.0f,         0.0f,       0},
    {kRbms_ScSOHA_DFCLCapResultAh,              24,      16,          192,           Uint16,      0.1f,     0.0f,      1500.0f,        0.0f,       0},
    {kRbms_ScSOHA_DFCLPointSOCPct,              26,      16,          208,           Uint16,      0.01f,    0.0f,      100.0f,         0.0f,       0},
    {kRbms_ScSOHA_DFCLPointStats,               28,      8,           224,           Uint8,       1.0f,     0.0f,      2.0f,           0.0f,       0},
    {kRbms_ScSOHA_DFCLPointCapAh,               29,      16,          232,           Int16,       0.1f,     -1500.0f,   1500.0f,        -1500.0f,    0},
    {kRbms_ScSOHA_DFCLPointTimeDay,             31,      16,          248,           Uint16,      1.0f,     0.0f,      65535.0f,       0.0f,       0},
    {kRbms_ScSOHA_MFCLCapResultAh,              33,      16,          264,           Uint16,      0.1f,     0.0f,      1500.0f,        0.0f,       0},
    {kRbms_ScSOHA_MFCLTargtValueVmax,           35,      16,          280,           Uint16,      1.0f,     0.0f,      65535.0f,       0.0f,       0},
    {kRbms_ScSOHA_MFCLTargtValueVmin,           37,      16,          296,           Uint16,      1.0f,     0.0f,      65535.0f,       0.0f,       0},
    {kRbms_SaSOCA_MaxMinSOCPct1,                39,      16,          312,           Uint16,      0.01f,    0.0f,      100.0f,         0.0f,       0},
    {kRbms_SaSOCA_MaxMinSOCPct2,                41,      16,          328,           Uint16,      0.01f,    0.0f,      100.0f,         0.0f,       0},
    {kRbms_SaSOCA_MaxMinSOCState1,             43,      8,           344,           Uint8,       1.0f,     0.0f,      10.0f,          0.0f,       0},
    {kRbms_SaSOCA_MaxMinSOCState2,             44,      8,           352,           Uint8,       1.0f,     0.0f,      10.0f,          0.0f,       0},
    {kRbms_SaSOCA_SimCellVmV1,                 45,      16,          360,           Uint16,      1.0f,     0.0f,      5000.0f,        0.0f,       0},
    {kRbms_SaSOCA_SimCellVmV2,                 47,      16,          376,           Uint16,      1.0f,     0.0f,      5000.0f,        0.0f,       0},
    {kRbms_ScSOHA_RealSysCapAh_dbug,            49,      16,          392,           Uint16,      0.1f,     0.0f,      1500.0f,        0.0f,       0},
    {kRbms_ScSOCA_RealSysSOCPct_dbug,           51,      16,          408,           Uint16,      0.01f,    0.0f,      100.0f,         0.0f,       0},
    {kRbms_ScSOCA_SmthSysSOCPct,                53,      16,          424,           Uint16,      0.01f,    0.0f,      100.0f,         0.0f,       0},
    {kRbms_ScSOCA_DispSysSOCPct,                55,      16,          440,           Uint16,      0.01f,    0.0f,      100.0f,         0.0f,       0},
    {kRbms_ScSOHA_DFCLCalSuccFlg,               57,      1,           456,           Uint8,       1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_ScSOHA_MFCLCalSuccFlg,               57,      1,           457,           Uint8,       1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_ScSOHA_MFCLPoint1FlgVmax,            57,      1,           458,           Uint8,       1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_ScSOHA_MFCLPoint1FlgVmin,            57,      1,           459,           Uint8,       1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_ScSOHA_MFCLPoint2FlgVmax,            57,      1,           460,           Uint8,       1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_ScSOHA_MFCLPoint2FlgVmin,            57,      1,           461,           Uint8,       1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_ScSOHA_MFCLPoint3FlgVmax,            57,      1,           462,           Uint8,       1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_ScSOHA_MFCLPoint3FlgVmin,            57,      1,           463,           Uint8,       1.0f,     0.0f,      1.0f,           0.0f,       0},
    {kRbms_ScSOCA_DispSOCState,                 58,      8,           464,           Uint8,       1.0f,     0.0f,      255.0f,        0.0f,       0},
    {kRbms_ScSOCA_TgtSysSOCPct,                 59,      16,          472,           Uint16,      0.01f,    0.0f,      655.35f,        0.0f,       0},
    {kRbms_ScSOCA_AccuCapAh,                    61,      16,          488,           Uint16,      0.1f,     -3000.0f,  3553.5f,        0.0f,       0},
};

/*============================================================
 * TMS_SumInfo - 热管理信息 (12 Bytes)
 * 对应消息ID: TMS_SumInfo (cmdGroup: 0x03, cmdid: 38)
 * 依据 BMS2.0 LAN Matrix V1.0.50 Comm Matrix
 *============================================================*/
const bmsPointAttr_t rbmsCmd03_38_TMS_SumInfo_PointAttr[] =
{
    /* pointId,              dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,  offset,  maxVal,  minVal,  repeatCnt   */
    {kRbms_BBMS_WtrTmpCmd,  0,       8,           0,             Uint8,     1.0f,   -40.0f,  215.0f,  -40.0f,      0},
    {kRbms_BBMS_cellTmpMax, 1,       8,           8,             Uint8,     1.0f,   -40.0f,  215.0f,  -40.0f,      0},
    {kRbms_BBMS_cellTmpMin, 2,       8,           16,            Uint8,     1.0f,   -40.0f,  215.0f,  -40.0f,      0},
    {kRbms_BBMS_cellTmpAvg, 3,       8,           24,            Uint8,     1.0f,   -40.0f,  215.0f,  -40.0f,      0},
    {kRbms_TMS_OutWtrTmp,   4,       8,           32,            Uint8,     1.0f,   -40.0f,  215.0f,  -40.0f,      0},
    {kRbms_TMS_BackWtrTmp,  5,       8,           40,            Uint8,     1.0f,   -40.0f,  215.0f,  -40.0f,      0},
    {kRbms_TMS_EnvTmp,      6,       8,           48,            Uint8,     1.0f,   -40.0f,  215.0f,  -40.0f,      0},
    {kRbms_TMS_InWtrP,      7,       8,           56,            Uint8,     0.1f,   0.0f,    25.5f,   0.0f,      0},
    {kRbms_TMS_OutWtrP,     8,       8,           64,            Uint8,     0.1f,   0.0f,    25.5f,   0.0f,      0},
    {kRbms_TMS_BBMSFltLevel,9,       2,           72,            Uint8,     1.0f,   0.0f,    3.0f,    0.0f,      0},
    {kRbms_TMS_BBMSFltCode, 9,       6,           74,            Uint8,     1.0f,   0.0f,    63.0f,   0.0f,      0},
    {kRbms_BMS_TMSReqMode,  10,      2,           80,            Uint8,     1.0f,   0.0f,    3.0f,    0.0f,      0},
    {kRbms_TMS_BMSMode,     10,      2,           82,            Uint8,     1.0f,   0.0f,    3.0f,    0.0f,      0},
    {kRbms_TMSNo,           10,      4,           84,            Uint8,     1.0f,   0.0f,    15.0f,   0.0f,      0},
    {kRbms_TMSCoolState,    11,      8,           88,            Uint8,     1.0f,   0.0f,    255.0f,  0.0f,      0},
};

/*============================================================
 * RBMS_Fault - RBMS故障数组 (25 Bytes = 200 Bits)
 * 对应消息ID: RBMS_Fault (cmdGroup: 0x04, cmdid: 0x01)
 * 对应消息ID: RBMS_Fault (cmdGroup: 0x03, cmdid: 41 / 0x29)
 *============================================================*/
const bmsPointAttr_t rbmsCmd03_29_RBMS_Fault_PointAttr[] =
{
    /* pointId,            dataIdx,  dataBitLen,  dataStartBit,  dataType,  coeff,  offset,  maxVal,  minVal,  repeatCnt  */
    {kRbms_Fault_Start,  0,       8,           0,             Uint8,     1.0f,   0.0f,    255.0f, 0.0f,       25},
};



/**
 * @brief 从 payload 按 Intel LSB 顺序提取任意位宽字段（1~64 bit）
 */
static uint64_t rbmsExtractBitsLe(const uint8_t *data, uint32_t startBit, uint16_t bitLen)
{
    uint64_t result = 0;

    for (uint16_t b = 0; b < bitLen; b++)
    {
        uint32_t bitPos = startBit + b;
        uint32_t byteIdx = bitPos / 8U;
        uint8_t bitIdx = (uint8_t)(bitPos % 8U);
        uint8_t bitVal = (uint8_t)((data[byteIdx] >> bitIdx) & 0x01U);
        result |= ((uint64_t)bitVal << b);
    }

    return result;
}

static uint16_t rbmsReadU16Le(const uint8_t *p)
{
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

static int16_t rbmsReadS16Le(const uint8_t *p)
{
    return (int16_t)rbmsReadU16Le(p);
}

static uint32_t rbmsReadU32Le(const uint8_t *p)
{
    return (uint32_t)p[0]
        | ((uint32_t)p[1] << 8)
        | ((uint32_t)p[2] << 16)
        | ((uint32_t)p[3] << 24);
}

static int32_t rbmsReadS32Le(const uint8_t *p)
{
    return (int32_t)rbmsReadU32Le(p);
}

static float rbmsReadF32Le(const uint8_t *p)
{
    uint32_t raw = rbmsReadU32Le(p);
    float fval;

    memcpy(&fval, &raw, sizeof(fval));
    return fval;
}

static uint64_t rbmsReadU64Le(const uint8_t *p)
{
    return (uint64_t)p[0]
        | ((uint64_t)p[1] << 8)
        | ((uint64_t)p[2] << 16)
        | ((uint64_t)p[3] << 24)
        | ((uint64_t)p[4] << 32)
        | ((uint64_t)p[5] << 40)
        | ((uint64_t)p[6] << 48)
        | ((uint64_t)p[7] << 56);
}

static double rbmsReadF64Le(const uint8_t *p)
{
    uint64_t raw = rbmsReadU64Le(p);
    double dval;

    memcpy(&dval, &raw, sizeof(dval));
    return dval;
}

static double rbmsRawBitsToDouble(uint64_t rawValue, uint16_t bitLen, data_type_e dataType)
{
    switch (dataType)
    {
        case Int8:
        case Int16:
        case Int32:
        case Int64:
        {
            if (bitLen > 0U && bitLen < 64U && (rawValue & (1ULL << (bitLen - 1U))))
            {
                rawValue |= (~0ULL) << bitLen;
            }
            return (double)(int64_t)rawValue;
        }
        case Float32:
        {
            uint32_t tmp = (uint32_t)rawValue;
            float fval;

            memcpy(&fval, &tmp, sizeof(fval));
            return (double)fval;
        }
        case Float64:
        {
            double dval;
            uint64_t tmp = rawValue;

            memcpy(&dval, &tmp, sizeof(dval));
            return dval;
        }
        default:
            return (double)rawValue;
    }
}

/**
 * @brief 公共数据解析函数（适配全局 bit 定位规则，Intel Little Endian）
 */
void rbmsParseCommonFun(dev_type_e devType,uint16_t devId, uint8_t *data,const bmsPointAttr_t *pointAttr,uint16_t pointCnt)
{
    if (data == NULL || pointAttr == NULL || pointCnt == 0)
    {
        return;
    }

    for (uint16_t i = 0; i < pointCnt; i++)
    {
        uint16_t pointId       = pointAttr[i].pointId;
        uint16_t dataIdx       = pointAttr[i].dataIdx;
        uint16_t dataBitLen    = pointAttr[i].dataBitLen;
        uint32_t dataStartBit  = pointAttr[i].dataStartBit;
        data_type_e dataType   = pointAttr[i].dataType;
        float coeff            = pointAttr[i].coeff;
        float offset           = pointAttr[i].offset;
        double maxVal          = pointAttr[i].maxVal;
        float minVal           = pointAttr[i].minVal;
        uint16_t repeatCnt     = pointAttr[i].repeatCnt;

        uint16_t loopCnt = (repeatCnt == 0) ? 1 : repeatCnt;

        if (dataBitLen < 8U || (dataBitLen % 8U) != 0U)
        {
            uint32_t stepBits = dataBitLen;
            uint32_t currentStartBit = dataStartBit;

            for (uint16_t j = 0; j < loopCnt; j++)
            {
                uint64_t rawValue = rbmsExtractBitsLe(data, currentStartBit, dataBitLen);
                double rtdbVal = rbmsRawBitsToDouble(rawValue, dataBitLen, dataType) * coeff + offset;

                if (rtdbVal > maxVal)
                {
                    rtdbVal = maxVal;
                }
                if (rtdbVal < minVal)
                {
                    rtdbVal = minVal;
                }

                setRtdbPointValue(Rtdb_ShMem, devType, devId, (uint16_t)(pointId + j), rtdbVal);
                currentStartBit += stepBits;
            }
        }
        else
        {
            uint16_t bytesPerPoint = (uint16_t)(dataBitLen / 8U);
            uint16_t currentDataIdx = dataIdx;

            for (uint16_t j = 0; j < loopCnt; j++)
            {
                double rtdbVal = 0.0;

                switch (dataType)
                {
                    case Uint8:
                        rtdbVal = (double)data[currentDataIdx] * coeff + offset;
                        break;
                    case Int8:
                        rtdbVal = (double)(int8_t)data[currentDataIdx] * coeff + offset;
                        break;
                    case Uint16:
                        rtdbVal = (double)rbmsReadU16Le(&data[currentDataIdx]) * coeff + offset;
                        break;
                    case Int16:
                        rtdbVal = (double)rbmsReadS16Le(&data[currentDataIdx]) * coeff + offset;
                        break;
                    case Uint32:
                        rtdbVal = (double)rbmsReadU32Le(&data[currentDataIdx]) * coeff + offset;
                        break;
                    case Int32:
                        rtdbVal = (double)rbmsReadS32Le(&data[currentDataIdx]) * coeff + offset;
                        break;
                    case Float32:
                        rtdbVal = (double)rbmsReadF32Le(&data[currentDataIdx]) * coeff + offset;
                        break;
                    case Uint64:
                        rtdbVal = (double)rbmsReadU64Le(&data[currentDataIdx]) * coeff + offset;
                        break;
                    case Int64:
                        rtdbVal = (double)(int64_t)rbmsReadU64Le(&data[currentDataIdx]) * coeff + offset;
                        break;
                    case Float64:
                        rtdbVal = rbmsReadF64Le(&data[currentDataIdx]) * coeff + offset;
                        break;
                    default:
                        break;
                }

                if (rtdbVal > maxVal)
                {
                    rtdbVal = maxVal;
                }
                if (rtdbVal < minVal)
                {
                    rtdbVal = minVal;
                }

                setRtdbPointValue(Rtdb_ShMem, devType, devId, (uint16_t)(pointId + j), rtdbVal);
                currentDataIdx = (uint16_t)(currentDataIdx + bytesPerPoint);
            }
        }
    }
}

//解析cmdGroup：03，cmdId：01的功能码
void rbmsParseCmd03_01_RBMS_SumInfoFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len)
{
    if (len < 310)
    {
        return;
    }
    uint16_t cnt = sizeof(rbmsCmd03_01_RBMS_SumInfo_PointAttr) / sizeof(bmsPointAttr_t);
    rbmsParseCommonFun(devType, devId, data, rbmsCmd03_01_RBMS_SumInfo_PointAttr, cnt);
}


void rbmsParseCmd03_02_VoltageInfoFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len)
{
    if (len < 1012)
    {
        return;
    }
    uint16_t cnt = sizeof(rbmsCmd03_02_RBMS_Volt) / sizeof(bmsPointAttr_t);
    rbmsParseCommonFun(devType, devId, data, rbmsCmd03_02_RBMS_Volt, cnt);
}

void rbmsParseCmd03_03_FaultInfoFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len)
{
    if (len < 1188)
    {
        return;
    }
    uint16_t cnt = sizeof(rbmsCmd03_03_RBMS_Temp_CellT_PointAttr) / sizeof(bmsPointAttr_t);
    rbmsParseCommonFun(devType, devId, data, rbmsCmd03_03_RBMS_Temp_CellT_PointAttr, cnt);
}

void rbmsParseCmd03_04_TempInfoFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len)
{
    if (len < 52)
    {
        return;
    }
    uint16_t cnt = sizeof(rbmsCmd03_04_RBMS_CellBalSt_PointAttr) / sizeof(bmsPointAttr_t);
    rbmsParseCommonFun(devType, devId, data, rbmsCmd03_04_RBMS_CellBalSt_PointAttr, cnt);
}

void rbmsParseCmd03_05_CellBalStInfoFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len)
{
    if (len < 416)
    {
        return;
    }
    uint16_t cnt = sizeof(rbmsCmd03_05_RBMS_CellSdr_PointAttr) / sizeof(bmsPointAttr_t);
    rbmsParseCommonFun(devType, devId, data, rbmsCmd03_05_RBMS_CellSdr_PointAttr, cnt);
}

void rbmsParseCmd03_23_CellSdrInfoFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len)
{
    if (len < 30)
    {
        return;
    }
    uint16_t cnt = sizeof(rbmsCmd03_23_RBMS_Debug_PointAttr) / sizeof(bmsPointAttr_t);
    rbmsParseCommonFun(devType, devId, data, rbmsCmd03_23_RBMS_Debug_PointAttr, cnt);
}

void rbmsParseCmd03_25_DebugInfoFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len)
{
    if (len < 60)
    {
        return;
    }
    uint16_t cnt = sizeof(rbmsCmd03_25_RBMS_SOXdebugData1_PointAttr) / sizeof(bmsPointAttr_t);
    rbmsParseCommonFun(devType, devId, data, rbmsCmd03_25_RBMS_SOXdebugData1_PointAttr, cnt);
}

void rbmsParseCmd03_26_DebugInfoFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len)
{
    if (len < 63)
    {
        return;
    }
    uint16_t cnt = sizeof(rbmsCmd03_26_RBMS_SOXdebugData2_PointAttr) / sizeof(bmsPointAttr_t);
    rbmsParseCommonFun(devType, devId, data, rbmsCmd03_26_RBMS_SOXdebugData2_PointAttr, cnt);
}

void rbmsParseCmd03_38_TMS_SumInfoFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len)
{
    if (len < 12)
    {
        return;
    }
    uint16_t cnt = sizeof(rbmsCmd03_38_TMS_SumInfo_PointAttr) / sizeof(bmsPointAttr_t);
    rbmsParseCommonFun(devType, devId, data, rbmsCmd03_38_TMS_SumInfo_PointAttr, cnt);
}

void rbmsParseCmd03_29_FaultInfoFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len) // 原 rbmsParseCmd04_01_FaultInfoFun
{
    if (len < 25)
    {
        return;
    }
    uint16_t cnt = sizeof(rbmsCmd03_29_RBMS_Fault_PointAttr) / sizeof(bmsPointAttr_t);
    rbmsParseCommonFun(devType, devId, data, rbmsCmd03_29_RBMS_Fault_PointAttr, cnt);
}
/**
 * @brief 解析RBMS接收数据
 * 
 * @param devType 设备类型
 * @param devId 设备ID
 * @param data 接收数据指针,已经去除了协议头,byte6为data的第一个字节即data[0]
 * @param len 接收数据长度 (实际为byte6~byteN的数据长度，)
    // | Byte1 | Byte2-3          | Byte4-5 | Byte6 | Byte7  | Byte8 | Byte9   | Byte10        | Byte11  | Byte12   | Byte13 | Byte14-n |
    // | ----- | ---------------- | ------- | ----- | ------ | ----- | ------- | ------------- | ------- | -------- | ------ | -------- |
    // | Head  | Version&datalen  | CRC16   | src   | srcSub | dest  | destSub | transportType | frameId | cmdGroup | cmdId  | data     |
    // | 0xA5  | version=2        |         |       |        |       |         |               |         |          |        |          |
 */
void rbmsParseRecvDataFun(dev_type_e devType, uint16_t devId, uint8_t *data, int len)
{
    if (data == NULL || len < 8)
    {
        return;
    }

    uint16_t cmdGroup = data[6];
    uint16_t cmdId = data[7];
    uint8_t *payload = data + 8;
    uint16_t payloadLen = (uint16_t)(len - 8);

    if (cmdGroup == 0x03 && cmdId == 0x01)
    {
        rbmsParseCmd03_01_RBMS_SumInfoFun(devType, devId, payload, payloadLen);
    }
    else if (cmdGroup == 0x03 && cmdId == 0x02)
    {
        rbmsParseCmd03_02_VoltageInfoFun(devType, devId, payload, payloadLen);
    }
    else if (cmdGroup == 0x03 && cmdId == 0x03)
    {
        rbmsParseCmd03_03_FaultInfoFun(devType, devId, payload, payloadLen);
    }
    else if (cmdGroup == 0x03 && cmdId == 0x04)
    {
        rbmsParseCmd03_04_TempInfoFun(devType, devId, payload, payloadLen);
    }
    else if (cmdGroup == 0x03 && cmdId == 0x05)
    {
        rbmsParseCmd03_05_CellBalStInfoFun(devType, devId, payload, payloadLen);
    }
    else if (cmdGroup == 0x03 && cmdId == 23)
    {
        rbmsParseCmd03_23_CellSdrInfoFun(devType, devId, payload, payloadLen);
    }
    else if (cmdGroup == 0x03 && cmdId == 25)
    {
        rbmsParseCmd03_25_DebugInfoFun(devType, devId, payload, payloadLen);
    }
    else if (cmdGroup == 0x03 && cmdId == 26)
    {
        rbmsParseCmd03_26_DebugInfoFun(devType, devId, payload, payloadLen);
    }
    else if (cmdGroup == 0x03 && cmdId == 38)
    {
        rbmsParseCmd03_38_TMS_SumInfoFun(devType, devId, payload, payloadLen);
    }
    else if (cmdGroup == 0x03 && cmdId == 41) // RBMS_Fault (原 cmdGroup 0x04 / cmdId 0x01)
    {
        rbmsParseCmd03_29_FaultInfoFun(devType, devId, payload, payloadLen);
    }
}

