/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            bsp_modbus.c
 * @brief           modbus协议栈
 * @author          Gary
 * @remark
 *****************************************************************************/
#include "bsp_modbus.h"

// 功能码对应关系
uint8_t func_code[kModbus_End] =
    {
        kModbus_Func_Start,
        kModbus_F01_Read_Coil,           // 读取线圈寄存器
        kModbus_F02_Read_Discrete_Input, // 读取离散输入寄存器
        kModbus_F03_Read_Hold_Reg,       // 读取保持寄存器
        kModbus_F04_Read_Input_Reg,      // 读取输入寄存器
        kModbus_F05_Write_S_Coil,        // 写入单个线圈寄存器
        kModbus_F06_Write_S_Hold_Reg,    // 写单个保持寄存器
        kModbus_F0F_Write_M_Coil,        // 写多个线圈寄存器
        kModbus_F10_Write_M_Hold_Reg,    // 写多个寄存器
};

static void cmdPrintLog(const char *str, uint8_t *data, uint16_t len)
{
    char sdata[512 * 2] = {0};
    int pos = 0;
    if (len > 0)
    {
        for (int i = 0; i < len; i++)
        {
            pos += snprintf(sdata + pos, sizeof(sdata) - pos, "%02x ", data[i]);
            // 检查snprintf返回值，确保写入成功，并处理潜在错误
            if (pos < 0 || pos >= sizeof(sdata))
            {
                KITPTF(LOG_MODBUS_EN, ERROR_EN, "tcpPrintLog: snprintf failed!");
                break; // 停止继续处理
            }
        }
        // 使用snprintf，避免缓冲区溢出
        pos += snprintf(sdata + pos, sizeof(sdata) - pos, "\n");
        // 检查snprintf返回值，确保写入成功，并处理潜在错误
        if (pos < 0 || pos >= sizeof(sdata))
        {
            KITPTF(LOG_MODBUS_EN, ERROR_EN, "tcpPrintLog: snprintf failed!");
        }
    }
    KITPTF(LOG_MODBUS_EN, INFO_EN, "%s:%s", str, sdata);
    KITLOG(LOG_MODBUS_EN, INFO_EN, "%s:%s", str, sdata);
}

// 计算 Modbus TCP 和 UART 的数据长度
uint16_t getModbusDataLength(uint8_t functionCode, uint16_t numRegistersOrCoils, uint8_t isTcp)
{
    uint16_t length = 0;

    switch (functionCode)
    {
    case kModbus_F01_Read_Coil:
    case kModbus_F02_Read_Discrete_Input:
        length = ((numRegistersOrCoils + 7) / 8); // 每8个线圈返回1字节
        break;

    case kModbus_F03_Read_Hold_Reg:
    case kModbus_F04_Read_Input_Reg:
        length = numRegistersOrCoils * 2; // 每个寄存器返回2字节
        break;

    case kModbus_F05_Write_S_Coil:
    case kModbus_F06_Write_S_Hold_Reg:
        length = 2 + 2 - 1; // 起始地址（2byte)+控制数据（2字节）-1（return中多加了len）
        break;

    case kModbus_F0F_Write_M_Coil:
    case kModbus_F10_Write_M_Hold_Reg:
        length = 2 + 2 - 1; // 起始地址（2byte)+寄存器数量（2字节）-1（return中多加了len）
        break;

    default:
        return 0; // 未知功能码返回0
    }

    // 考虑 Modbus TCP 的额外头部长度
    if (isTcp)
    {
        return length + 6 + 1 + 1 + 1; // TCP 头部长度（6字节） + id + 功能码（1字节） + 长度（1字节）
    }
    else
    {
        return length + 1 + 1 + 1 + 2; // UART: id + fun + len（适用于03,04,01，02） + crc
    }
}

/*****************************************************************************
 * @brief       把分组按寄存器地址从小到大排序
 * @param[in]   pointInfo: 测点指针
 * @return      NONE
 *****************************************************************************/
void modbusSort(mb_point_t *pointInfo)
{
    if (NULL == pointInfo || 0 == pointInfo->pointNum)
    {
        return;
    }

    point_t tmp;
    // 按地址从小到大排序
    for (int i = 0; i < pointInfo->pointNum - 1; i++)
    {
        for (int j = 0; j < pointInfo->pointNum - i - 1; j++)
        {
            if ((pointInfo->pointItem[j].devId == pointInfo->pointItem[j + 1].devId) &&
                (pointInfo->pointItem[j].devType == pointInfo->pointItem[j + 1].devType) &&
                (pointInfo->pointItem[j].regAddr > pointInfo->pointItem[j + 1].regAddr))
            {
                tmp = pointInfo->pointItem[j];
                pointInfo->pointItem[j] = pointInfo->pointItem[j + 1];
                pointInfo->pointItem[j + 1] = tmp;
            }
        }
    }
}

uint8_t calcRegister(data_type_e type)
{
    uint8_t ret = 0;
    switch (type)
    {
    case Uint8:
    case Int8:   //=1   有符号8位整形
    case Uint16: //=2   无符号16位整形
    case Int16:  //=3   有符号16位整形
        ret = 1;
        break;
    case Uint32:  //=4   无符号32位整形
    case Int32:   //=5   有符号32位整形
    case Float32: //=6  32位单精度浮点数
        ret = 2;
        break;
    case Uint64:  //=7   无符号64位整形
    case Int64:   //=8   有符号64位整形
    case Float64: //=9  64位双精度浮点数
        ret = 4;
        break;
    default:
        break;
    }
    return ret;
}

/*****************************************************************************
 * @brief       合并地址
 * @param[in]   pointInfo: 测点指针
 * @param[in]   group: 写入的数组
 * @param[in]   maxRegNum: 分组最大寄存器连续数量
 * @return      false - 分组出错  true -- 分组成功  mbItem->mbFuncReqInfo[i].groupNum;
 *****************************************************************************/
bool modbusMergeAddr(mb_group_ret_t *group, mb_point_t pointInfo, int maxRegNum)
{
    if (NULL == group)
    {
        return false;
    }
    if (0 == pointInfo.pointNum)
    {
        group->groupNum = 0;
        return true;
    }

    int group_num = 0;
    int newLen = 0;
    point_t leftPoint = {0};
    point_t rightPoint = {0};

    if (pointInfo.pointNum == 1)
    {
        group_num = 1;
        group->groupNum = 1;
    }
    else
    {
        // 初始值为1
        group_num = 1;
        // 计算分组组数
        for (int i = 0; i < pointInfo.pointNum - 1; i++)
        {
            leftPoint = pointInfo.pointItem[i];
            rightPoint = pointInfo.pointItem[i + 1];
            // 查看是否连续
            if ((rightPoint.regAddr - leftPoint.regAddr == 1 || leftPoint.regAddr + calcRegister(leftPoint.dataType) == rightPoint.regAddr) && (newLen < maxRegNum) && leftPoint.devType == rightPoint.devType && leftPoint.devId == rightPoint.devId) // devType和devId是为了串口接不同设备时不能合并
            {
                if (leftPoint.dataType < Uint16)
                {
                    newLen++;
                }
                else
                {
                    newLen = newLen + calcRegister(leftPoint.dataType);
                }
            }
            else
            {
                group_num++;
                newLen = 0;
            }
        }
    }

    group->groupNum = group_num;
    group->groupItem = calloc(group->groupNum, sizeof(mb_req_group_t));
    if (NULL == group->groupItem)
    {
        return false;
    }
    newLen = 0;
    group_num = 0;

    // 给一个起始的地址
    group->groupItem[0].startAddr = pointInfo.pointItem[0].regAddr;

    if (pointInfo.pointNum == 1)
    {
        group->groupItem[group_num].endAddr = pointInfo.pointItem[0].regAddr + calcRegister(pointInfo.pointItem[0].dataType) - 1;
    }

    // 遍历所有点，分组(-1: 最后一个单独判断，最后一个点可能是单独一个组的)
    for (int i = 0; i < pointInfo.pointNum - 1; i++)
    {
        leftPoint = pointInfo.pointItem[i];
        rightPoint = pointInfo.pointItem[i + 1];
        // 查看是否连续
        if ((rightPoint.regAddr - leftPoint.regAddr == 1 || leftPoint.regAddr + calcRegister(leftPoint.dataType) == rightPoint.regAddr) && newLen < maxRegNum && leftPoint.devType == rightPoint.devType && leftPoint.devId == rightPoint.devId) // devType和devId是为了串口接不同设备时不能合并
        {
            if (leftPoint.dataType < Uint16)
            {
                newLen++;
            }
            else
            {
                newLen = newLen + calcRegister(leftPoint.dataType);
            }
        }
        else
        {
            // 赋值上一个匹配组的最后一个地址
            group->groupItem[group_num].endAddr = leftPoint.regAddr + calcRegister(leftPoint.dataType) - 1;
            group_num++;
            newLen = 0;
            // 紧接着写入下一个位置的开始地址
            group->groupItem[group_num].startAddr = rightPoint.regAddr;
        }

        // 放置最后一个分组的最后一个结束地址
        if (i == pointInfo.pointNum - 2)
        {
            group->groupItem[group_num].endAddr = rightPoint.regAddr + calcRegister(rightPoint.dataType) - 1;
        }
    }
    return true;
}

// 设备和点位结构体
void initPointModbus(dev_info_t *infoItem, modbus_master_t *mbItem)
{
    uint16_t idx = 0;
    if (infoItem != NULL && mbItem != NULL)
    {
        // 首先统计功能码对应的测点数量
        if (infoItem->pointNum > 0)
        {
            for (idx = 0; idx < infoItem->pointNum; idx++)
            {
                // 将设备的ID号赋值到每个测点中，从0开始
                infoItem->pointArr[idx].devId = (infoItem->devId > 0 ? (infoItem->devId - 1) : 0);
                // 统计实际的点数，这些点是非解析点
                if (infoItem->pointArr[idx].bitNum == 0 && infoItem->pointArr[idx].bitPos == 0)
                {
                    switch (infoItem->pointArr[idx].func)
                    {
                    case kModbus_F01_Read_Coil:
                        mbItem->mbFuncNum[kModbus_F01]++;
                        break;
                    case kModbus_F02_Read_Discrete_Input:
                        mbItem->mbFuncNum[kModbus_F02]++;
                        break;
                    case kModbus_F03_Read_Hold_Reg:
                        mbItem->mbFuncNum[kModbus_F03]++;
                        break;
                    case kModbus_F04_Read_Input_Reg:
                        mbItem->mbFuncNum[kModbus_F04]++;
                        break;
                    case kModbus_F05_Write_S_Coil:
                        mbItem->mbFuncNum[kModbus_F05]++;
                        break;
                    case kModbus_F06_Write_S_Hold_Reg:
                        mbItem->mbFuncNum[kModbus_F06]++;
                        break;
                    case kModbus_F0F_Write_M_Coil:
                        mbItem->mbFuncNum[kModbus_F0F]++;
                        break;
                    case kModbus_F10_Write_M_Hold_Reg:
                        mbItem->mbFuncNum[kModbus_F10]++;
                        break;
                    default:
                        break;
                    }
                }
                else if (infoItem->pointArr[idx].bitNum != 0 && infoItem->pointArr[idx].bitPos != 0)
                {
                    switch (infoItem->pointArr[idx].func)
                    {
                    case kModbus_F01_Read_Coil:
                        mbItem->mbParseAlarmNum[kModbus_F01]++;
                        break;
                    case kModbus_F02_Read_Discrete_Input:
                        mbItem->mbParseAlarmNum[kModbus_F02]++;
                        break;
                    case kModbus_F03_Read_Hold_Reg:
                        mbItem->mbParseAlarmNum[kModbus_F03]++;
                        break;
                    case kModbus_F04_Read_Input_Reg:
                        mbItem->mbParseAlarmNum[kModbus_F04]++;
                        break;
                    default:
                        break;
                    }
                }
            }

            for (idx = kModbus_Start; idx < kModbus_End; idx++)
            {
                if (mbItem->mbFuncNum[idx] > 0)
                {
                    mbItem->mbFuncInfo[idx].pointNum = mbItem->mbFuncNum[idx];
                    mbItem->mbFuncInfo[idx].pointItem = calloc(mbItem->mbFuncInfo[idx].pointNum, sizeof(point_t));
                }

                if (mbItem->mbParseAlarmNum[idx] > 0)
                {
                    mbItem->mbParseAlarm[idx].pointNum = mbItem->mbParseAlarmNum[idx];
                    mbItem->mbParseAlarm[idx].pointItem = calloc(mbItem->mbParseAlarm[idx].pointNum, sizeof(point_t));
                }
            }

            /*将测点信息按照功能码进行分组*/
            uint16_t mbFuncIdx[kModbus_End] = {0};
            uint16_t mbFuncIndex[kModbus_End] = {0};
            for (idx = 0; idx < infoItem->pointNum; idx++)
            {
                // 统计实际的点数，这些点是非解析点
                if (infoItem->pointArr[idx].bitNum == 0 && infoItem->pointArr[idx].bitPos == 0)
                {
                    switch (infoItem->pointArr[idx].func)
                    {
                    case kModbus_F01_Read_Coil:
                        mbItem->mbFuncInfo[kModbus_F01].pointItem[mbFuncIdx[kModbus_F01]++] = infoItem->pointArr[idx];
                        break;
                    case kModbus_F02_Read_Discrete_Input:
                        mbItem->mbFuncInfo[kModbus_F02].pointItem[mbFuncIdx[kModbus_F02]++] = infoItem->pointArr[idx];
                        break;
                    case kModbus_F03_Read_Hold_Reg:
                        mbItem->mbFuncInfo[kModbus_F03].pointItem[mbFuncIdx[kModbus_F03]++] = infoItem->pointArr[idx];
                        break;
                    case kModbus_F04_Read_Input_Reg:
                        mbItem->mbFuncInfo[kModbus_F04].pointItem[mbFuncIdx[kModbus_F04]++] = infoItem->pointArr[idx];
                        break;
                    case kModbus_F05_Write_S_Coil:
                        mbItem->mbFuncInfo[kModbus_F05].pointItem[mbFuncIdx[kModbus_F05]++] = infoItem->pointArr[idx];
                        break;
                    case kModbus_F06_Write_S_Hold_Reg:
                        mbItem->mbFuncInfo[kModbus_F06].pointItem[mbFuncIdx[kModbus_F06]++] = infoItem->pointArr[idx];
                        break;
                    case kModbus_F0F_Write_M_Coil:
                        mbItem->mbFuncInfo[kModbus_F0F].pointItem[mbFuncIdx[kModbus_F0F]++] = infoItem->pointArr[idx];
                        break;
                    case kModbus_F10_Write_M_Hold_Reg:
                        mbItem->mbFuncInfo[kModbus_F10].pointItem[mbFuncIdx[kModbus_F10]++] = infoItem->pointArr[idx];
                        break;
                    default:
                        break;
                    }
                }
                else if (infoItem->pointArr[idx].bitNum != 0 && infoItem->pointArr[idx].bitPos != 0)
                {
                    switch (infoItem->pointArr[idx].func)
                    {
                    case kModbus_F01_Read_Coil:
                        mbItem->mbParseAlarm[kModbus_F01].pointItem[mbFuncIndex[kModbus_F01]++] = infoItem->pointArr[idx];
                        break;
                    case kModbus_F02_Read_Discrete_Input:
                        mbItem->mbParseAlarm[kModbus_F02].pointItem[mbFuncIndex[kModbus_F02]++] = infoItem->pointArr[idx];
                        break;
                    case kModbus_F03_Read_Hold_Reg:
                        mbItem->mbParseAlarm[kModbus_F03].pointItem[mbFuncIndex[kModbus_F03]++] = infoItem->pointArr[idx];
                        break;
                    case kModbus_F04_Read_Input_Reg:
                        mbItem->mbParseAlarm[kModbus_F04].pointItem[mbFuncIndex[kModbus_F04]++] = infoItem->pointArr[idx];
                        break;
                    default:
                        break;
                    }
                }
            }

            // 对modbus按照功能码分组进行排序,为了请求帧生成
            for (idx = kModbus_Start; idx <= kModbus_F04; idx++)
            {
                if (mbItem->mbFuncNum[idx] > 0)
                {
                    // 排序
                    modbusSort(&mbItem->mbFuncInfo[idx]);
                    // 排序后进行组帧拆分//mbItem->mbFuncReqInfo[i].groupNum;
                    modbusMergeAddr(&mbItem->mbFuncReqInfo[idx], mbItem->mbFuncInfo[idx], 30);
                }
            }
        }
    }
}

// 组帧入队列
uint8_t initReqModbus(modbus_master_t *mbItem)
{
    uint16_t repCount = 0;
    if (mbItem == NULL)
    {
        return 1;
    }

    for (uint8_t i = kModbus_Start; i <= kModbus_F04; i++)
    {
        repCount += mbItem->mbFuncReqInfo[i].groupNum;
    }

    if (repCount <= 0)
    {
        return 1;
    }

    mbItem->mbReqList.listArray = calloc(repCount, sizeof(list_item_t));
    if (mbItem->mbReqList.listArray == NULL)
    {
        return 1;
    }

    // memset(mbItem->mbReqList.listArray, 0x00, repCount * sizeof(list_item_t));
    uint16_t listIdx = 0;
    mbItem->mbReqList.size = repCount;
    mbItem->mbReqList.listNum = repCount;
    if (mbItem->mdType == E_MODBUS_TCP)
    {
        for (uint8_t i = kModbus_Start; i <= kModbus_F04; i++)
        {
            if (mbItem->mbFuncReqInfo[i].groupNum > 0)
            {
                for (uint16_t k = 0; k < mbItem->mbFuncReqInfo[i].groupNum; k++)
                {
                    uint8_t idx = 0;
                    uint8_t *frameItem = &mbItem->mbReqList.listArray[listIdx].byte[6];
                    frameItem[idx++] = mbItem->itemTcp.uId;
                    frameItem[idx++] = func_code[i];
                    frameItem[idx++] = mbItem->mbFuncReqInfo[i].groupItem[k].startAddr >> 8;
                    frameItem[idx++] = mbItem->mbFuncReqInfo[i].groupItem[k].startAddr;
                    uint8_t count = mbItem->mbFuncReqInfo[i].groupItem[k].endAddr - mbItem->mbFuncReqInfo[i].groupItem[k].startAddr + 1;

                    // printf("功能码：%d，开始地址：%d，结束地址：%d\n", func_code[i], mbItem->mbFuncReqInfo[i].groupItem[k].startAddr, mbItem->mbFuncReqInfo[i].groupItem[k].endAddr);
                    frameItem[idx++] = count >> 8;
                    frameItem[idx++] = count;
                    mbItem->mbReqList.listArray[listIdx].frameLen = 6 + idx;
                    // 插入head的长度
                    mbItem->mbReqList.listArray[listIdx].byte[4] = idx >> 8;
                    mbItem->mbReqList.listArray[listIdx].byte[5] = idx;
                    listIdx++;
                    // mbItem->mbReqList.size++;
                }
            }
        }
    }
    else if (mbItem->mdType == E_MODBUS_RTU)
    {
        for (uint8_t i = kModbus_Start; i <= kModbus_F04; i++)
        {
            if (mbItem->mbFuncReqInfo[i].groupNum > 0)
            {
                for (uint16_t k = 0; k < mbItem->mbFuncReqInfo[i].groupNum; k++)
                {
                    uint8_t idx = 0;
                    uint8_t *frameItem = mbItem->mbReqList.listArray[listIdx].byte;
                    frameItem[idx++] = mbItem->itemUart.uId;
                    frameItem[idx++] = func_code[i];
                    frameItem[idx++] = mbItem->mbFuncReqInfo[i].groupItem[k].startAddr >> 8;
                    frameItem[idx++] = mbItem->mbFuncReqInfo[i].groupItem[k].startAddr;
                    uint8_t count = mbItem->mbFuncReqInfo[i].groupItem[k].endAddr - mbItem->mbFuncReqInfo[i].groupItem[k].startAddr + 1;
                    frameItem[idx++] = count >> 8;
                    frameItem[idx++] = count;
                    kit_append_crc16(frameItem, idx);
                    mbItem->mbReqList.listArray[listIdx].frameLen = idx + 2;
                    listIdx++;
                    // mbItem->mbReqList.size++;
                }
            }
        }
    }
    return 0;
}

/*****************************************************************************
 * @brief       队列是否为空
 * @return      false - 不为空  true -- 为空
 *****************************************************************************/
bool listIsEmpty(list_t *list)
{
    if (list->listNum > 0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void initChanModbus(dev_info_t *infoItem, modbus_master_t *mbItem)
{
    if (infoItem->protocolType == kProto_ModbusTCP_Master || infoItem->protocolType == kProto_ModbusTCPMulti_Master)
    {
        mbItem->itemTcp.netId = infoItem->devProtocol.tcpClientLib.netId;
        mbItem->itemTcp.uId = infoItem->devProtocol.tcpClientLib.uId;
        strncpy((char *)mbItem->itemTcp.ip, (char *)infoItem->devProtocol.tcpClientLib.ip, MAX_IP_LEN);
        mbItem->itemTcp.port = infoItem->devProtocol.tcpClientLib.port;
        mbItem->itemTcp.timeout = infoItem->devProtocol.tcpClientLib.timeout;
        mbItem->mdType = E_MODBUS_TCP;
    }
    else
    {
        mbItem->itemUart.uartId = infoItem->devProtocol.uartLib.uartId;
        strncpy((char *)mbItem->itemUart.address, (char *)infoItem->devProtocol.uartLib.address, MAX_ADDR_LEN);
        mbItem->itemUart.uId = infoItem->devProtocol.uartLib.uId;
        mbItem->itemUart.data = (uart_data_e)infoItem->devProtocol.uartLib.data;
        mbItem->itemUart.stop = (uart_stop_e)infoItem->devProtocol.uartLib.stop;
        mbItem->itemUart.baud = (uart_baud_e)infoItem->devProtocol.uartLib.baud;
        mbItem->itemUart.parity = (uart_parity_e)infoItem->devProtocol.uartLib.parity;
        mbItem->itemUart.flow = (uart_flow_e)infoItem->devProtocol.uartLib.flow;
        mbItem->itemUart.timeout = infoItem->devProtocol.uartLib.timeout;
    }
    mbItem->devType = infoItem->devType;
    mbItem->devId = (infoItem->devId > 0) ? (infoItem->devId - 1) : 0;
}

/*****************************************************************************
 * @brief       创建modbus任务解析告警初始化
 * @param[in]   item: 指针
 * @return      0:sucessful  1:fail
 *****************************************************************************/
int8_t initParseAlarmThread(void *arg)
{
    modbus_master_t *item = arg;

    for (uint8_t idx = kModbus_Start; idx < kModbus_End; idx++)
    {
        if (item->mbParseAlarmNum[idx] == 0)
        {
            continue;
        }
        item->mbAlarmParent[idx] = malloc(item->mbParseAlarmNum[idx] * sizeof(uint16_t));

        if (item->mbAlarmParent[idx] == NULL)
        {
            KITLOG(LOG_MODBUS_EN, ERROR_EN, "功能码：%d，故障告警的父测点数组mbAlarmParent，内存申请失败", idx);
            KITPTF(LOG_MODBUS_EN, ERROR_EN, "功能码：%d，故障告警的父测点数组mbAlarmParent，内存申请失败", idx);
            return 1;
        }

        for (uint16_t i = 0; i < item->mbParseAlarmNum[idx]; i++)
        {
            bool found = false;
            for (uint16_t j = 0; j < item->mbFuncNum[idx]; j++)
            {
                if (item->mbParseAlarm[idx].pointItem[i].regAddr == item->mbFuncInfo[idx].pointItem[j].regAddr)
                {
                    item->mbAlarmParent[idx][i] = j;
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                KITLOG(LOG_MODBUS_EN, ERROR_EN, "功能码%d: 告警测点[%d] (pointId=%d, regAddr=%d, bitPos=%d, bitNum=%d) 未找到父测点！", 
                       idx, i, item->mbParseAlarm[idx].pointItem[i].pointId, 
                       item->mbParseAlarm[idx].pointItem[i].regAddr,
                       item->mbParseAlarm[idx].pointItem[i].bitPos,
                       item->mbParseAlarm[idx].pointItem[i].bitNum);
                KITPTF(LOG_MODBUS_EN, ERROR_EN, "功能码%d: 告警测点[%d] (pointId=%d, regAddr=%d, bitPos=%d, bitNum=%d) 未找到父测点！", 
                       idx, i, item->mbParseAlarm[idx].pointItem[i].pointId, 
                       item->mbParseAlarm[idx].pointItem[i].regAddr,
                       item->mbParseAlarm[idx].pointItem[i].bitPos,
                       item->mbParseAlarm[idx].pointItem[i].bitNum);
            }
        }
    }
    return 0;
}

/*****************************************************************************
 * @brief       解析告警
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
void calcAlarmCall(modbus_master_t *item)
{
    uint32_t word = 0;
    static uint8_t prntf_count = 0;
    if (item->isOnline)
    {
        for (uint8_t idx = kModbus_Start; idx < kModbus_End; idx++)
        {
            prntf_count++;
            for (uint16_t i = 0; i < item->mbParseAlarmNum[idx]; i++)
            {
                point_t *point = &item->mbParseAlarm[idx].pointItem[i];
                uint16_t index = item->mbAlarmParent[idx][i];
                point_t *parentPoint = &item->mbFuncInfo[idx].pointItem[index];
                uint32_t parentPointId = parentPoint->pointId;
                
                // 从实时库获取浮点数值（已经是 寄存器值 × ratio）
                double floatValue = getRtdbPointValue(rtdbType, item->devType, item->devId, parentPointId);
                
                // 除以ratio还原为原始寄存器值，然后取整
                double rawRegisterValue = 0.0;
                if (parentPoint->ratio != 0.0)
                {
                    rawRegisterValue = floatValue / parentPoint->ratio;
                }
                else
                {
                    // ratio为0时，直接使用原值（避免除零）
                    rawRegisterValue = floatValue;
                }
                
                // 取整并转换为uint32_t进行位提取
                // 修复：使用标准库函数lround进行四舍五入，避免值超过int32_t范围（~2.1亿）时的未定义行为
                // lround返回long类型，然后安全转换为uint32_t，适用于Modbus 32位寄存器（最大2^32-1）
                word = (uint32_t)lround(rawRegisterValue); // 使用标准库函数四舍五入取整
                uint8_t bitValue = get_bit_word(point->bitPos, point->bitNum, word);
                setRtdbPointValue(rtdbType, item->devType, item->devId, point->pointId, (double)bitValue);
#ifdef DBEUG_MODBUS
                // 添加详细调试信息
                if (prntf_count > 100 && (uint8_t)getRtdbPointValue(rtdbType, kDev_Type_Con, 0, kCon_ModBusLog_Enable) == 1)
                {
                    KITLOG(LOG_MODBUS_EN, INFO_EN, "位提取: 设备类型=%d, 设备号=%d, 测点=%s (pointId=%d), 父测点pointId=%d, regAddr=%d, word=0x%04X, bitPos=%d, bitNum=%d, bitValue=%d", 
                           point->devType, item->devId + 1, point->pointName, point->pointId,
                           item->mbFuncInfo[idx].pointItem[index].pointId,
                           point->regAddr, word, point->bitPos, point->bitNum, bitValue);
                    KITPTF(LOG_MODBUS_EN, INFO_EN, "位提取: 设备类型=%d, 设备号=%d, 测点=%s (pointId=%d), 父测点pointId=%d, regAddr=%d, word=0x%04X, bitPos=%d, bitNum=%d, bitValue=%d", 
                           point->devType, item->devId + 1, point->pointName, point->pointId,
                           item->mbFuncInfo[idx].pointItem[index].pointId,
                           point->regAddr, word, point->bitPos, point->bitNum, bitValue);
                }
#endif
            }

            if (prntf_count > 100)
            {
                prntf_count = 0;
            }
        }
    }
}

/*****************************************************************************
 * @brief       开始modbus通道链接
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
tcp_client_state_e mbMasterStartCall(void *arg)
{
    modbus_master_t *item = arg;

    setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 0.0);
    item->isOnline = 0;

    if (drvTcpOpen(&item->itemTcp) == TCP_OPEN_SUCESS)
    {
        return TCP_CLIENT_ONLINE;
    }
    else
    {
        sleep(1);
        return TCP_CLIENT_START;
    }
}

/*****************************************************************************
 * @brief       开始modbus通道链接
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
tcp_client_state_e mbMasterMultStartCall(void *arg)
{
    modbus_tcp_multid_master_t *item = arg;

    for (int i = 0; i < item->devNum; i++)
    {
        modbus_master_t *items = &item->mbMasterItem[i];
        setRtdbPointValue(rtdbType, items->devType, items->devId, DEV_IS_ONLINE, 0.0);
        items->isOnline = 0;
    }

    if (drvTcpOpen(&item->itemTcp) == TCP_OPEN_SUCESS)
    {
        return TCP_CLIENT_ONLINE;
    }
    else
    {
        sleep(1);
        return TCP_CLIENT_START;
    }
}

/*****************************************************************************
 * @brief       解析相关信息
 * @param[in]   uint8_t *data:接受数据,TcpReVData_T *revdata：解析后数据
 * @return      -1：数据有误 0：接受正确
 *****************************************************************************/
uint8_t modbusTcpCmdAnalize(modbus_master_t *item, uint8_t *data, modbus_rev_frame_t *revdata)
{
    int ret = 0;
    revdata->tranction = ((uint16_t)data[0]) << 8 | data[1]; // 事物号，用来判断是否为发送命令的返回帧
#if 1
    if (item->transaction != revdata->tranction) // 事物号不等
    {
        ret = 1;
        return ret;
    }
#endif
    revdata->protocolType = data[2] << 8 | data[3]; // 协议号
    if (0x00 != revdata->protocolType)              // 不是modbusTCP协议，丢弃报文
    {
        ret = 1;
        KITLOG(LOG_MODBUS_EN, WARN_EN, "不是modbusTCP协议，丢弃报文\n");
        return ret;
    }
    uint16_t allLength = 0;
    allLength = data[4] << 8 | data[5]; // 数据长度
    revdata->devAddr = data[6];
    if (revdata->devAddr != item->itemTcp.uId)
    {
        ret = 1;
        KITLOG(LOG_MODBUS_EN, WARN_EN, "modbus tcp地址错误，丢弃报文\n");
        return ret;
    }

    revdata->function = data[7]; // 功能码
    switch (revdata->function)
    {
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    {
        revdata->len = data[8];

        if ((allLength - 3) != revdata->len) // 数据长度不一致
        {
            KITLOG(LOG_MODBUS_EN, WARN_EN, "数据长度不一致\n");
            // 取消长度校验
            // ret = 1;
            // return ret;
        }

        revdata->data = &data[9];
        break;
    }

    case 0x05:
    case 0x06:
    {
        revdata->startAddress = data[8] << 8 | data[9];
        revdata->len = 1;
        revdata->data = &data[10];
        int64_t value = data[10] << 8 | data[11];
        // 重要日志，需要记录
        KITLOG(LOG_MODBUS_EN, INFO_EN, "控制指令回复：设备类型：%d，设备号：%d，寄存器地址：%d，值：%ld，Hex：%x", item->devType, item->devId + 1, revdata->startAddress, value, value);
        KITPTF(LOG_MODBUS_EN, INFO_EN, "控制指令回复：设备类型：%d，设备号：%d，寄存器地址：%d，值：%ld，Hex：%x", item->devType, item->devId + 1, revdata->startAddress, value, value);
        cmdPrintLog("控制指令回复报文：", item->itemTcp.recv_buf, item->itemTcp.recv_len);
        // 设置的和收到的不是一个东西仍然继续发送
        if (revdata->data[0] != item->sendFrame.data[0] || revdata->data[1] != item->sendFrame.data[1])
        {
            ret = 1;
        }
        break;
    }

    case 0x0f:
    case 0x10:
    {
        revdata->startAddress = data[8] << 8 | data[9];
        revdata->len = data[10] << 8 | data[11]; // 寄存器数目;
        // 重要日志，需要记录
        KITLOG(LOG_MODBUS_EN, INFO_EN, "控制指令回复：设备类型：%d，设备号：%d，寄存器起始地址：%d", item->devType, item->devId + 1, revdata->startAddress);
        KITPTF(LOG_MODBUS_EN, INFO_EN, "控制指令回复：设备类型：%d，设备号：%d，寄存器起始地址：%d", item->devType, item->devId + 1, revdata->startAddress);
        cmdPrintLog("控制指令回复报文：", item->itemTcp.recv_buf, item->itemTcp.recv_len);
        break;
    }
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x90:
    case 0x97:
    {
        ret = 1;
        KITLOG(LOG_MODBUS_EN, WARN_EN, "不存在的功能码\n");
        break;
    }
    }
    return ret;
}

/*****************************************************************************
 * @brief       解析相关信息
 * @param[in]   uint8_t *data:接受数据,TcpReVData_T *revdata：解析后数据
 * @return      -1：crc数据有误  -3:功能码错误  0：接受正确
 *****************************************************************************/
uint8_t modbusRtuCmdAnalize(modbus_master_t *item, uint8_t *data, modbus_rev_frame_t *revdata)
{
    int ret = 0;
    // crc错误
    if (kit_check_crc16(data, item->itemUart.recv_len) != 0)
    {
        return -1;
    }
    revdata->devAddr = data[0];

    // 地址错误
    if (revdata->devAddr != item->itemUart.uId)
    {
        return -1;
    }

    revdata->function = data[1]; // 功能码

    switch (revdata->function)
    {
    case 0x01:
    case 0x02:
    case 0x03:
    case 0x04:
    {
        revdata->len = data[2];
        revdata->data = &data[3];
        break;
    }

    case 0x05:
    case 0x06:
    {
        revdata->startAddress = data[2] << 8 | data[3];
        revdata->len = 1;
        revdata->data = &data[4];
        break;
    }

    case 0x0f:
    case 0x10:
    {
        revdata->startAddress = data[2] << 8 | data[3];
        revdata->len = data[4] << 8 | data[5]; // 寄存器数目;
        break;
    }
    case 0x81:
    case 0x82:
    case 0x83:
    case 0x84:
    case 0x85:
    case 0x86:
    case 0x90:
    case 0x97:
    {
        ret = -3;
        KITLOG(LOG_MODBUS_EN, WARN_EN, "不存在的功能码\n");
        break;
    }
    }
    return ret;
}

/*****************************************************************************
 * @brief       解析03,04功能码信息并入实时库
 * @param[in]   item:
 * @return      -1：数据有误 0：接受正确
 *****************************************************************************/
void mbParseFunc03Fun04Respond(modbus_master_t *item, modbus_rev_frame_t *revData)
{
    if (item == NULL || revData == NULL)
    {
        return;
    }

    uint8_t mbFuncIdx = 0;

    // 先寻找功能码索引
    for (uint8_t funIdx = kModbus_Start; funIdx < kModbus_End; funIdx++)
    {
        if (revData->function == func_code[funIdx])
        {
            mbFuncIdx = funIdx;
            break;
        }
    }

    // 先功能码中的点索引
    volatile uint16_t pointIdx = 0;
    uint16_t i = 0;
    for (i = 0; i < item->mbFuncInfo[mbFuncIdx].pointNum; i++)
    {
        if (item->mbFuncInfo[mbFuncIdx].pointItem[i].regAddr == item->sendFrame.startAddr)
        {
            pointIdx = i;
            break;
        }
    }

    if (i == item->mbFuncInfo[mbFuncIdx].pointNum)
    {
        return;
    }

    uint8_t parsePos = 0;
    double rtdbValue = 0;
    uint8_t u8_value = 0;
    int8_t s8_value = 0;
    int16_t s16_value = 0;
    uint16_t u16_value = 0;

    int32_t s32_value = 0;
    uint32_t u32_value = 0;
    int64_t s64_value = 0;
    uint64_t u64_value = 0;

    float f32_value = 0;
    double f64_value = 0;

    point_t *point = NULL;
    while (parsePos < revData->len)
    {
        data_type_e dataType = item->mbFuncInfo[mbFuncIdx].pointItem[pointIdx].dataType;
        point = &item->mbFuncInfo[mbFuncIdx].pointItem[pointIdx];
        if(pointIdx >= item->mbFuncInfo[mbFuncIdx].pointNum)
        {
            return;
        }
        switch (dataType)
        {
        case Uint8: //=0   无符号8位整形
            u8_value = revData->data[parsePos];
            rtdbValue = (u8_value * point->ratio) + point->offset;
            parsePos += 1;
            break;
        case Int8: //=1   有符号8位整形
            s8_value = revData->data[parsePos];
            rtdbValue = (s8_value * point->ratio) + point->offset;
            parsePos += 1;
            break;
        case Uint16: //=2   无符号16位整形
            u16_value = kit_get_u16(&revData->data[parsePos], point->endian);
            rtdbValue = (u16_value * point->ratio) + point->offset;
            parsePos += 2;
            break;
        case Int16: //=3   有符号16位整形
            s16_value = kit_get_s16(&revData->data[parsePos], point->endian);
            rtdbValue = (s16_value * point->ratio) + point->offset;
            parsePos += 2;
            break;
        case Uint32: //=4   无符号32位整形
            u32_value = kit_get_u32(&revData->data[parsePos], point->endian);
            rtdbValue = (u32_value * point->ratio) + point->offset;
            parsePos += 4;
            break;
        case Int32: //=5   有符号32位整形
            s32_value = kit_get_s32(&revData->data[parsePos], point->endian);
            rtdbValue = (s32_value * point->ratio) + point->offset;
            parsePos += 4;
            break;
        case Uint64: //=6   无符号64位整形
            u64_value = kit_get_u64(&revData->data[parsePos], point->endian);
            rtdbValue = (u64_value * point->ratio) + point->offset;
            parsePos += 8;
            break;
        case Int64: //=7   有符号64位整形
            s64_value = kit_get_s64(&revData->data[parsePos], point->endian);
            rtdbValue = (s64_value * point->ratio) + point->offset;
            parsePos += 8;
            break;
        case Float32: //=8  32位单精度浮点数
            f32_value = kit_get_float(&revData->data[parsePos], point->endian);
            rtdbValue = (f32_value * point->ratio) + point->offset;
            parsePos += 4;
            break;
        case Float64: //=9  64位双精度浮点数
            f64_value = kit_get_double(&revData->data[parsePos], point->endian);
            rtdbValue = (f64_value * point->ratio) + point->offset;
            parsePos += 8;
            break;
        default:
            break;
        }

        // 入实时库数据
        setRtdbPointValue(Rtdb_ShMem, point->devType, item->devId, point->pointId, rtdbValue);

#ifdef DBEUG_MODBUS
        uint8_t a = (uint8_t)getRtdbPointValue(rtdbType, kDev_Type_Con, 0, kCon_ModBusLog_Enable);
        if (a == 1)
        {
            KITLOG(LOG_MODBUS_EN, INFO_EN, "设备类型：%d，设备号：%d，%s 值：%.*f", point->devType, item->devId + 1, point->pointName, GET_PRECISION(point->ratio), rtdbValue);
            KITPTF(LOG_MODBUS_EN, INFO_EN, "设备类型：%d，设备号：%d，%s 值：%.*f", point->devType, item->devId + 1, point->pointName, GET_PRECISION(point->ratio), rtdbValue);
        }
#endif

        pointIdx += 1;
    }
}

/*****************************************************************************
 * @brief       解析01,02功能码信息并入实时库
 * @param[in]   item:
 * @return      -1：数据有误 0：接受正确
 *****************************************************************************/
void mbParseFunc01Fun02Respond(modbus_master_t *item, modbus_rev_frame_t *revData)
{
    if (item == NULL || revData == NULL)
    {
        return;
    }

    uint8_t mbFuncIdx = 0;
    // 先寻找功能码索引
    for (uint8_t funIdx = kModbus_Start; funIdx < kModbus_End; funIdx++)
    {
        if (revData->function == func_code[funIdx])
        {
            mbFuncIdx = funIdx;
            break;
        }
    }

    // 先功能码中的点索引
    uint16_t pointIdx = 0;
    uint16_t i = 0;
    for (i = 0; i < item->mbFuncInfo[mbFuncIdx].pointNum; i++)
    {
        if (item->mbFuncInfo[mbFuncIdx].pointItem[i].regAddr == item->sendFrame.startAddr)
        {
            pointIdx = i;
            break;
        }
    }

    double rtdbValue = 0;

    // 计算点位多少，01和02功能码不能通过字节去索引
    point_t *point = NULL;

    if (revData->len * 8 >= item->sendFrame.reqNum)
    {
        for (uint16_t i = 0; i < item->sendFrame.reqNum; i++)
        {
            point = &item->mbFuncInfo[mbFuncIdx].pointItem[pointIdx + i];
            uint8_t value = kit_byte_pos_bit(revData->data, i, revData->len);
            rtdbValue = value * 1.0;

            // 入实时库数据
            setRtdbPointValue(Rtdb_ShMem, point->devType, item->devId, point->pointId, rtdbValue);
#ifdef DBEUG_MODBUS
            if ((uint8_t)getRtdbPointValue(rtdbType, kDev_Type_Con, 0, kCon_ModBusLog_Enable) == 1)
            {
                KITLOG(LOG_MODBUS_EN, INFO_EN, "设备类型：%d，设备号：%d，%s 值：%.*f", point->devType, item->devId + 1, point->pointName, GET_PRECISION(point->ratio), rtdbValue);
                KITPTF(LOG_MODBUS_EN, INFO_EN, "设备类型：%d，设备号：%d，%s 值：%.*f", point->devType, item->devId + 1, point->pointName, GET_PRECISION(point->ratio), rtdbValue);
            }
#endif
        }
    }
}

/*****************************************************************************
 * @brief       解析05,05功能码的响应
 * @param[in]   item:
 * @return      -1：数据有误 0：接受正确
 *****************************************************************************/
void mbParseFunc05Fun06Respond(modbus_master_t *item, modbus_rev_frame_t *revData)
{
    if (item == NULL || revData == NULL)
    {
        return;
    }

    if (item->sendFrame.func != revData->function)
    {
        KITLOG(LOG_MODBUS_EN, WARN_EN, "错误的功能码，发送控制帧控制码%d，响应的功能码%d", item->sendFrame.func, revData->function);
        KITPTF(LOG_MODBUS_EN, WARN_EN, "错误的功能码，发送控制帧控制码%d，响应的功能码%d", item->sendFrame.func, revData->function);
        return;
    }

    // 清空发送帧
    memset(item->mbCmdList.listArray[item->mbCmdList.cur].byte, 0x00, item->mbCmdList.listArray[item->mbCmdList.cur].frameLen);
    item->mbCmdList.listArray[item->mbCmdList.cur].frameLen = 0;

    // 清空发送帧缓存
    memset(&item->sendFrame, 0x00, sizeof(item->sendFrame));

    // 控制命令发送成功，队列元素向后移位
    item->mbCmdList.cur++;
    item->mbCmdList.cur %= item->mbCmdList.size;
    if (item->mbCmdList.listNum != 0)
    {
        item->mbCmdList.listNum--;
    }
}

/*****************************************************************************
 * @brief       解析相关信息
 * @param[in]   item:
 * @return      1 ：数据有误 0：接受正确
 *****************************************************************************/
uint8_t mbRespondParseData(modbus_master_t *item)
{
    modbus_rev_frame_t revdata = {0};
    uint8_t ret = 1;
    if (item->mdType == E_MODBUS_TCP)
    {
        ret = modbusTcpCmdAnalize(item, item->itemTcp.recv_buf, &revdata);
    }
    else
    {
        ret = modbusRtuCmdAnalize(item, item->itemUart.recv_buf, &revdata);
    }

    if (!ret)
    {
        switch (revdata.function)
        {
        case kModbus_F01_Read_Coil:
        case kModbus_F02_Read_Discrete_Input:
            mbParseFunc01Fun02Respond(item, &revdata);
            break;

        case kModbus_F03_Read_Hold_Reg:
        case kModbus_F04_Read_Input_Reg:
            mbParseFunc03Fun04Respond(item, &revdata);
            break;

        case kModbus_F05_Write_S_Coil:
        case kModbus_F06_Write_S_Hold_Reg:
            mbParseFunc05Fun06Respond(item, &revdata);
            break;
        case kModbus_F0F_Write_M_Coil:
            break;
        case kModbus_F10_Write_M_Hold_Reg:
            break;
        }
        return 0;
    }
    else
    {
        return 1;
    }
}

/*****************************************************************************
 * @brief      发送请求数据
 * @param[in]   item:
 * @return      1：数据有误 0：接受正确
 *****************************************************************************/
int8_t mbTcpSendFrame(modbus_master_t *item, list_item_t *listItem)
{
    //
    uint8_t *data = listItem->byte;
    // uint8_t *data = item->mbReqList.listArray[item->mbReqIdx].byte;
    item->sendFrame.protocol = ((uint16_t)data[2]) << 8 | data[3];
    item->sendFrame.bufLen = ((uint16_t)data[4]) << 8 | data[5];
    item->sendFrame.devAddr = data[6];
    item->sendFrame.func = data[7];
    item->sendFrame.startAddr = ((uint16_t)data[8]) << 8 | data[9];
    if (item->sendFrame.func == kModbus_F01_Read_Coil || item->sendFrame.func == kModbus_F02_Read_Discrete_Input || item->sendFrame.func == kModbus_F03_Read_Hold_Reg || item->sendFrame.func == kModbus_F04_Read_Input_Reg ||
        item->sendFrame.func == kModbus_F0F_Write_M_Coil || item->sendFrame.func == kModbus_F10_Write_M_Hold_Reg)
    {
        item->sendFrame.reqNum = ((uint16_t)data[10]) << 8 | data[11];
    }

    memcpy(item->itemTcp.send_buf, data, listItem->frameLen);

    // 用于校验控制控制数据是否回复正确
    if (item->sendFrame.func == kModbus_F05_Write_S_Coil || item->sendFrame.func == kModbus_F06_Write_S_Hold_Reg)
    {
        memcpy(item->sendFrame.data, &data[10], listItem->frameLen - 10);
    }
    else if (item->sendFrame.func == kModbus_F0F_Write_M_Coil || item->sendFrame.func == kModbus_F10_Write_M_Hold_Reg)
    {
        memcpy(item->sendFrame.data, &data[13], listItem->frameLen - 13);
    }

    item->itemTcp.send_buf[0] = item->transaction >> 8;
    item->itemTcp.send_buf[1] = (uint8_t)item->transaction;
    item->itemTcp.send_len = listItem->frameLen;

    if (drvTcpSend(&item->itemTcp) == 0)
    {
        // 重要控制日志需要记录
        if (item->sendFrame.func >= kModbus_F05_Write_S_Coil)
        {
            KITLOG(LOG_MODBUS_EN, INFO_EN, "控制指令下发：设备类型：%d，设备号：%d，寄存器地址：%d", item->devType, item->devId + 1, item->sendFrame.startAddr);
            KITPTF(LOG_MODBUS_EN, INFO_EN, "控制指令下发：设备类型：%d，设备号：%d，寄存器地址：%d", item->devType, item->devId + 1, item->sendFrame.startAddr);
            cmdPrintLog("控制指令报文：", item->itemTcp.send_buf, item->itemTcp.send_len);
        }

        return 0;
    }
    else
    {
        return -1;
    }
}

/*****************************************************************************
 * @brief      发送请求数据
 * @param[in]   item:
 * @return      1：数据有误 0：接受正确
 *****************************************************************************/
int8_t mbTcpMultSendFrame(modbus_master_t *item, list_item_t *listItem, modbus_tcp_multid_master_t *tcpItem)
{
    //
    uint8_t *data = listItem->byte;
    // uint8_t *data = item->mbReqList.listArray[item->mbReqIdx].byte;
    item->sendFrame.protocol = ((uint16_t)data[2]) << 8 | data[3];
    item->sendFrame.bufLen = ((uint16_t)data[4]) << 8 | data[5];
    item->sendFrame.devAddr = data[6];
    item->sendFrame.func = data[7];
    item->sendFrame.startAddr = ((uint16_t)data[8]) << 8 | data[9];
    if (item->sendFrame.func == kModbus_F01_Read_Coil || item->sendFrame.func == kModbus_F02_Read_Discrete_Input || item->sendFrame.func == kModbus_F03_Read_Hold_Reg || item->sendFrame.func == kModbus_F04_Read_Input_Reg ||
        item->sendFrame.func == kModbus_F0F_Write_M_Coil || item->sendFrame.func == kModbus_F10_Write_M_Hold_Reg)
    {
        item->sendFrame.reqNum = ((uint16_t)data[10]) << 8 | data[11];
    }

    // memcpy(item->itemTcp.send_buf, data, listItem->frameLen);
    memcpy(tcpItem->itemTcp.send_buf, data, listItem->frameLen);

    // 用于校验控制控制数据是否回复正确
    if (item->sendFrame.func == kModbus_F05_Write_S_Coil || item->sendFrame.func == kModbus_F06_Write_S_Hold_Reg)
    {
        memcpy(item->sendFrame.data, &data[10], listItem->frameLen - 10);
    }
    else if (item->sendFrame.func == kModbus_F0F_Write_M_Coil || item->sendFrame.func == kModbus_F10_Write_M_Hold_Reg)
    {
        memcpy(item->sendFrame.data, &data[13], listItem->frameLen - 13);
    }

    // item->itemTcp.send_buf[0] = item->transaction >> 8;
    // item->itemTcp.send_buf[1] = (uint8_t)item->transaction;
    // item->itemTcp.send_len = listItem->frameLen;

    tcpItem->itemTcp.send_buf[0] = tcpItem->itemTcp.transaction >> 8;
    tcpItem->itemTcp.send_buf[1] = tcpItem->itemTcp.transaction &0xFF;
    tcpItem->itemTcp.send_len = listItem->frameLen;

    if (drvTcpSend(&tcpItem->itemTcp) == 0)
    {
        // 重要控制日志需要记录
        if (item->sendFrame.func >= kModbus_F05_Write_S_Coil)
        {
            KITLOG(LOG_MODBUS_EN, INFO_EN, "控制指令下发：设备类型：%d，设备号：%d，寄存器地址：%d", item->devType, item->devId + 1, item->sendFrame.startAddr);
            KITPTF(LOG_MODBUS_EN, INFO_EN, "控制指令下发：设备类型：%d，设备号：%d，寄存器地址：%d", item->devType, item->devId + 1, item->sendFrame.startAddr);
            cmdPrintLog("控制指令报文：", item->itemTcp.send_buf, item->itemTcp.send_len);
        }

        return 0;
    }
    else
    {
        return -1;
    }
}

uint8_t initMbList(list_t *list, uint16_t size)
{
    list->size = size;
    list->listArray = malloc(sizeof(list_item_t) * size);
    if (list->listArray == NULL)
    {
        KITLOG(LOG_MODBUS_EN, INFO_EN, "初始化分配队列内存失败");
        KITPTF(LOG_MODBUS_EN, INFO_EN, "初始化分配队列内存失败");
        return 1;
    }
    return 0;
}

/*****************************************************************************
 * @brief       向队列中添加元素
 * @param[in]   list: 指针
 * @return      NONE
 *****************************************************************************/
void addItemToList(list_t *list, list_item_t *item)
{
    if (list->size > 0)
    {
        list->listNum++;
        list->listArray[list->rear].frameLen = item->frameLen;
        memcpy(list->listArray[list->rear].byte, item->byte, item->frameLen);
        list->rear++;
        list->rear %= list->size;
    }
}

/*****************************************************************************
 * @brief       获取当前的队列元素，队列光标索引加一
 * @param[in]   list: 指针
 * @return      NONE
 *****************************************************************************/
list_item_t getItemByList(list_t *list)
{
    list_item_t item = {0};
    if (list->size > 0)
    {
        if (list->listArray[list->cur].frameLen > 0)
        {
            item.frameLen = list->listArray[list->cur].frameLen;
            memcpy(item.byte, list->listArray[list->cur].byte, list->listArray[list->cur].frameLen);

#if 0 // 控制成功后再移动标，移动到解析地方处理
            list->cur++;
            list->cur %= list->size;
#endif
            return item;
        }
    }
    return item;
}

/*****************************************************************************
 * @brief
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
list_item_t genCmdListItem(modbus_type_e mdType, uint8_t uId, uint16_t startAddr, uint16_t endAddr, uint8_t func,
                           data_type_e dataType, data_endian_e endian, double value)
{
    list_item_t item = {0};
    uint8_t idx = 0;
    if (mdType == E_MODBUS_RTU)
    {
        item.byte[idx++] = uId;
        item.byte[idx++] = func;
        item.byte[idx++] = startAddr >> 8;
        item.byte[idx++] = startAddr;

        if (func == kModbus_F05_Write_S_Coil || func == kModbus_F06_Write_S_Hold_Reg)
        {
        }
        else if (func == kModbus_F0F_Write_M_Coil || func == kModbus_F10_Write_M_Hold_Reg)
        {
            uint8_t count = endAddr - startAddr + 1;
            item.byte[idx++] = count >> 8;
            item.byte[idx++] = count;
        }
        idx += kit_fill_byte(&item.byte[idx], dataType, endian, value);
        kit_append_crc16(item.byte, idx);
        item.frameLen = idx + 2;
    }
    else if (mdType == E_MODBUS_TCP)
    {
        idx = 6;
        item.byte[idx++] = uId;
        item.byte[idx++] = func;
        item.byte[idx++] = startAddr >> 8;
        item.byte[idx++] = startAddr;
        if (func == kModbus_F05_Write_S_Coil || func == kModbus_F06_Write_S_Hold_Reg)
        {
        }
        else if (func == kModbus_F0F_Write_M_Coil || func == kModbus_F10_Write_M_Hold_Reg)
        {
            uint8_t count = endAddr - startAddr + 1;
            item.byte[idx++] = count >> 8;
            item.byte[idx++] = count;
        }
        idx += kit_fill_byte(&item.byte[idx], dataType, endian, value);
        item.frameLen = idx;

        // 补充长度
        item.byte[4] = (item.frameLen - 6) >> 8;
        item.byte[5] = (item.frameLen - 6);
    }
    return item;
}

// 扫描下发数据生成队列,
void mbScanCtrlCmdInputList(modbus_master_t *item)
{
    double curValue = 0;
    double cmdValue = 0;
    uint16_t i = 0;
    point_t *pointer;
    if (item->mbFuncInfo[kModbus_F05].pointNum > 0)
    {
        for (i = 0; i < item->mbFuncNum[kModbus_F05]; i++)
        {
            pointer = &item->mbFuncInfo[kModbus_F05].pointItem[i];
            curValue = getRtdbPointValue(rtdbType, pointer->devType, pointer->devId, pointer->pointId);
            // 查看是否变位
            if (kit_diff_change(pointer->value, curValue, pointer->mutateBound) == 1)
            {
                pointer->value = curValue;

                // 系数偏移转换
                cmdValue = (curValue - pointer->offset) / pointer->ratio;

                // 写单个线圈值强制转换
                if (double_equal(cmdValue, 1))
                {
                    cmdValue = 0xFF00;
                }
                else
                {
                    cmdValue = 0x0000;
                }
                // 生成队列
                if (item->mdType == E_MODBUS_RTU)
                {
                    list_item_t listItem = genCmdListItem(item->mdType, item->itemUart.uId, pointer->regAddr, pointer->regAddr, pointer->func,
                                                          pointer->dataType, pointer->endian, cmdValue);
                    // 变位入队列
                    addItemToList(&item->mbCmdList, &listItem);
                }
                else
                {
                    list_item_t listItem = genCmdListItem(item->mdType, item->itemTcp.uId, pointer->regAddr, pointer->regAddr, pointer->func,
                                                          pointer->dataType, pointer->endian, cmdValue);
                    // 变位入队列
                    addItemToList(&item->mbCmdList, &listItem);
                }
            }
        }
    }

    if (item->mbFuncNum[kModbus_F06] > 0)
    {
        for (uint16_t i = 0; i < item->mbFuncNum[kModbus_F06]; i++)
        {
            pointer = &item->mbFuncInfo[kModbus_F06].pointItem[i];
            curValue = getRtdbPointValue(rtdbType, pointer->devType, pointer->devId, pointer->pointId);
            // 查看是否变位
            if (kit_diff_change(pointer->value, curValue, pointer->mutateBound) == 1)
            {
                pointer->value = curValue;

                // 系数偏移转换
                cmdValue = (curValue - pointer->offset) / pointer->ratio;

                // 生成队列
                if (item->mdType == E_MODBUS_RTU)
                {
                    list_item_t listItem = genCmdListItem(item->mdType, item->itemUart.uId, pointer->regAddr, pointer->regAddr, pointer->func,
                                                          pointer->dataType, pointer->endian, cmdValue);
                    // 变位入队列
                    addItemToList(&item->mbCmdList, &listItem);
                }
                else
                {
                    list_item_t listItem = genCmdListItem(item->mdType, item->itemTcp.uId, pointer->regAddr, pointer->regAddr, pointer->func,
                                                          pointer->dataType, pointer->endian, cmdValue);
                    // 变位入队列
                    addItemToList(&item->mbCmdList, &listItem);
                }
            }
        }
    }
}

/*****************************************************************************
 * @brief       modbus在线处理
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
tcp_client_state_e mbMasterOnlineCall(void *arg)
{
    modbus_master_t *item = arg;

    tcp_client_state_e state = TCP_CLIENT_ONLINE;
    while (1)
    {
        int32_t ret = -1;
        // 在次数加入扫描05,06等功能码对应的测点值
        // 后期如果需要过快控制任务，在入控制队列这一块用独立线程入队做
        mbScanCtrlCmdInputList(item);
        if (listIsEmpty(&item->mbCmdList) == false)
        {
            list_item_t currItem = getItemByList(&item->mbCmdList);
            // 在此处不停地进行写队列元素的下发
            ret = mbTcpSendFrame(item, &currItem);
        }
        else if (listIsEmpty(&item->mbReqList) == false)
        {
            list_item_t *reqItem = &item->mbReqList.listArray[item->mbReqIdx];
            ret = mbTcpSendFrame(item, reqItem);
            item->mbReqIdx++;
            item->mbReqIdx %= item->mbReqList.size;
        }

        if (ret == 0)
        {
            int32_t rett = drvTcpRecv(&item->itemTcp);
            if (rett < 0)
            {
                item->transaction++;
                item->transaction %= 65535;
                // 超时次数或者接受失败测试
                item->itemTcp.count++;
                if (item->itemTcp.count > 30)
                {
                    state = TCP_CLIENT_OFFLINE;
                    goto OffLineDeal;
                }
            }
            else
            {
                if (mbRespondParseData(item) != 0)
                {
                    // 超时次数或者接受失败测试
                    item->itemTcp.count++;

#ifdef DBEUG_MODBUS
                    if ((uint8_t)getRtdbPointValue(rtdbType, kDev_Type_Con, 0, kCon_ModBusLog_Enable) == 1)
                    {
                        KITLOG(LOG_MODBUS_EN, INFO_EN, "丢帧个数：%d", item->itemTcp.count);
                        KITPTF(LOG_MODBUS_EN, INFO_EN, "丢帧个数：%d", item->itemTcp.count);
                    }
#endif

                    if (item->itemTcp.count > 30)
                    {
                        state = TCP_CLIENT_OFFLINE;
                        goto OffLineDeal;
                    }
                }
                else
                {
                    setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 1.0);
                    item->itemTcp.count = 0;
                    item->isOnline = 1;
                }
                // 解析后事务加1
                item->transaction++;
                item->transaction %= 65535;
            }
        }
        else
        {
            state = TCP_CLIENT_OFFLINE;
            goto OffLineDeal;
        }
        calcAlarmCall(item);
        usleep(30000);
    }

OffLineDeal: // 连接异常状态跳出
    setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 0.0);
    item->isOnline = 0;
    return state;
}

/*****************************************************************************
 * @brief       modbus在线处理
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
tcp_client_state_e mbMasterMultOnlineCall(void *arg)
{
    modbus_tcp_multid_master_t *itemAll = arg;

    tcp_client_state_e state = TCP_CLIENT_ONLINE;
    uint8_t i = 0;
    while (1)
    {
        for (int i = 0; i < itemAll->devNum; i++)
        {
            modbus_master_t *item = &itemAll->mbMasterItem[i];
            int32_t ret = -1;
            // 在次数加入扫描05,06等功能码对应的测点值
            // 后期如果需要过快控制任务，在入控制队列这一块用独立线程入队做
            mbScanCtrlCmdInputList(item);
            if (listIsEmpty(&item->mbCmdList) == false)
            {
                list_item_t currItem = getItemByList(&item->mbCmdList);
                // 在此处不停地进行写队列元素的下发
                ret = mbTcpMultSendFrame(item, &currItem, itemAll);
            }
            else if (listIsEmpty(&item->mbReqList) == false)
            {
                list_item_t *reqItem = &item->mbReqList.listArray[item->mbReqIdx];
                ret = mbTcpMultSendFrame(item, reqItem, itemAll);
                item->mbReqIdx++;
                item->mbReqIdx %= item->mbReqList.size;
            }

            if (ret == 0)
            {
                int32_t rett = drvTcpRecv(&itemAll->itemTcp);
                if (rett < 0)
                {
                    itemAll->itemTcp.transaction++;
                    itemAll->itemTcp.transaction %= 65535;
                    // 超时次数或者接受失败测试
                    itemAll->itemTcp.count++;
                    item->itemTcp.count++;
                    #if 1
                    if (item->itemTcp.count > 2)
                    {
                        setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 0.0);
                        item->isOnline = 0;
                        //state = TCP_CLIENT_OFFLINE;
                        //goto OffLineDeal;
                    }
                    #endif
                }
                else
                {
                    if(itemAll->itemTcp.recv_len > 0 && itemAll->itemTcp.recv_len < MAX_TCP_LEN)
                    {
                        memcpy(item->itemTcp.recv_buf, itemAll->itemTcp.recv_buf, itemAll->itemTcp.recv_len);
                        item->itemTcp.recv_len = itemAll->itemTcp.recv_len;
                        item->transaction = itemAll->itemTcp.transaction;
                    }
                    else
                    {
                        itemAll->itemTcp.count++;
                        item->itemTcp.count++;
                        #if 1
                        if (item->itemTcp.count > 2)
                        {
                            setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 0.0);
                            item->isOnline = 0;
                            //state = TCP_CLIENT_OFFLINE;
                            //goto OffLineDeal;
                        }
                        #endif
                    }

                    if (mbRespondParseData(item) != 0)
                    {
                        // 超时次数或者接受失败测试
                        item->itemTcp.count++;

                        #ifdef DBEUG_MODBUS
                        if ((uint8_t)getRtdbPointValue(rtdbType, kDev_Type_Con, 0, kCon_ModBusLog_Enable) == 1)
                        {
                            KITLOG(LOG_MODBUS_EN, INFO_EN, "丢帧个数：%d", item->itemTcp.count);
                            KITPTF(LOG_MODBUS_EN, INFO_EN, "丢帧个数：%d", item->itemTcp.count);
                        }
                        #endif

                        #if 1
                        if (item->itemTcp.count > 2)
                        {
                            setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 0.0);
                            item->isOnline = 0;
                            //state = TCP_CLIENT_OFFLINE;
                            //goto OffLineDeal;
                        }
                        #endif
                    }
                    else
                    {
                        item->itemTcp.last_time = get_epoch_time_ms();
                        itemAll->itemTcp.last_time = get_epoch_time_ms();
                        setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 1.0);
                        itemAll->itemTcp.count = 0;
                        item->itemTcp.count = 0;
                        item->isOnline = 1;
                    }

                    if(itemAll->itemTcp.recv_len > 0 && itemAll->itemTcp.recv_len < MAX_TCP_LEN)
                    {
                        memset(item->itemTcp.recv_buf, 0x00, itemAll->itemTcp.recv_len);
                        itemAll->itemTcp.recv_len = 0;
                        item->itemTcp.recv_len = 0;
                    }
                    // 解析后事务加1
                    itemAll->itemTcp.transaction++;
                    itemAll->itemTcp.transaction %= 65535;
                }

                // 离线设备在线超时处理
                for (int i = 0; i < itemAll->devNum; i++)
                {
                    modbus_master_t *item = &itemAll->mbMasterItem[i];
                    uint64_t curTime = get_epoch_time_ms();
                    //如果不配置，或者超时60s，则默认为3s超时判断
                    if(item->itemTcp.timeout == 0 || item->itemTcp.timeout > 60000)
                    {
                        item->itemTcp.timeout = 3000;
                    }
                    if (item->isOnline == 1 && (curTime - item->itemTcp.last_time) > item->itemTcp.timeout)
                    {
                        setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 0.0);
                        item->isOnline = 0;
                    }
                }
            }
            else
            {
                state = TCP_CLIENT_OFFLINE;
                goto OffLineDeal;
            }

            uint16_t onlind_num = 0;
            for (int i = 0; i < itemAll->devNum; i++)
            {
                modbus_master_t *item = &itemAll->mbMasterItem[i];
                if(item->isOnline == 0)
                {
                    onlind_num++;
                }
            }
            uint64_t curTime = get_epoch_time_ms();

            //如果不配置，或者超时60s，则默认为3s超时判断
            if(itemAll->itemTcp.timeout == 0 || itemAll->itemTcp.timeout > 60000)
            {
                itemAll->itemTcp.timeout = 3000;
            }

            if((onlind_num == itemAll->devNum) || (curTime - itemAll->itemTcp.last_time > itemAll->itemTcp.timeout))
            {
                state = TCP_CLIENT_OFFLINE;
                goto OffLineDeal;
            }

            calcAlarmCall(item);
        }
        usleep(30000);
    }

OffLineDeal: // 连接异常状态跳出
    for (int i = 0; i < itemAll->devNum; i++)
    {
        modbus_master_t *item = &itemAll->mbMasterItem[i];
        setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 0.0);
        item->isOnline = 0;
        return state;
    }
}

/*****************************************************************************
 * @brief       modbus离线处理
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
tcp_client_state_e mbMasterOfflineCall(void *arg)
{
    modbus_master_t *item = arg;
    // tcp_client_state_e state = TCP_CLIENT_ONLINE;

    setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 0.0);
    item->isOnline = 0;

    drvTcpClose(&item->itemTcp);

    KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus tcp客户端关闭IP：%s，Port：%d的TCP连接", item->itemTcp.ip, item->itemTcp.port);
    KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus tcp客户端关闭IP：%s，Port：%d的TCP连接", item->itemTcp.ip, item->itemTcp.port);
    item->itemTcp.count = 0;
    sleep(5);

    return TCP_CLIENT_START;
}

/*****************************************************************************
 * @brief       modbus离线处理
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
tcp_client_state_e mbMasterMultOfflineCall(void *arg)
{
    modbus_tcp_multid_master_t *item = arg;
    // tcp_client_state_e state = TCP_CLIENT_ONLINE;
    for (int i = 0; i < item->devNum; i++)
    {
        modbus_master_t *items = &item->mbMasterItem[i];
        setRtdbPointValue(rtdbType, items->devType, items->devId, DEV_IS_ONLINE, 0.0);
        items->isOnline = 0;
    }
        
    drvTcpClose(&item->itemTcp);

    KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus tcp客户端关闭IP：%s，Port：%d的TCP连接", item->itemTcp.ip, item->itemTcp.port);
    KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus tcp客户端关闭IP：%s，Port：%d的TCP连接", item->itemTcp.ip, item->itemTcp.port);
    item->itemTcp.count = 0;
    sleep(5);

    return TCP_CLIENT_START;
}

tcpStateMacFun mbClientStateCall[TCP_CLIENT_END] =
    {
        mbMasterStartCall,
        mbMasterOnlineCall,
        mbMasterOfflineCall};

/*****************************************************************************
 * @brief       初始化单个modbus基础信息
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
uint8_t initModbusSigle(modbus_master_t *item)
{
    item->exitTaskFlag = 0;
    item->mbCmdIdx = 0;
    item->mbReqIdx = 0;
    item->transaction = 0;
    // 初始化写队列
    return initMbList(&item->mbCmdList, 1000);
}

/*****************************************************************************
 * @brief       创建modbus任务
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
void *creatNetModbusThread(void *arg)
{
    if (arg == NULL)
    {
        KITPTF(LOG_MODBUS_EN, INFO_EN, "item指针为空");
        KITLOG(LOG_MODBUS_EN, INFO_EN, "item指针为空");
    }
    modbus_master_t *item = arg;
    tcp_client_state_e state = TCP_CLIENT_START;

    if (initModbusSigle(item) != 0)
    {
        KITPTF(LOG_MODBUS_EN, INFO_EN, "modbus tcp线程 (ip：%s,port:%d) initModbusSigle 初始化失败", item->itemTcp.ip, item->itemTcp.port);
        KITLOG(LOG_MODBUS_EN, INFO_EN, "modbus tcp线程 (ip：%s,port:%d) initModbusSigle 初始化失败", item->itemTcp.ip, item->itemTcp.port);
        return (void *)0;
    }

    if (initParseAlarmThread(item) != 0)
    {
        return (void *)0;
    }

    while (1)
    {
        state = mbClientStateCall[state](item);

        // 一般是重试次数超过会退出线程置为1，当前不做限制
        if ((item->exitTaskFlag == 1))
        {
            item->exitTaskFlag = 0;
            pthread_detach(item->tfd); // 要将id分配出去。
            pthread_exit((void *)1);
        }

        printf("进入主线程\n");
        usleep(50000);
    }
}

// 初始化modbus tcp 任务入口
void creatNetModbusTaskEntry(proto_dev_point_map_t *inputItem)
{
    // NULL 保护
    if (inputItem == NULL)
    {
        KITLOG(LOG_MODBUS_EN, ERROR_EN, "输入项为NULL,无法创建Modbus任务条目");
        return;
    }
    uint8_t i = 0;
    modbus_master_t *mbItem = NULL;
    if (inputItem->devNum > 0)
    {
        mbItem = calloc(inputItem->devNum, sizeof(modbus_master_t));
    }
    else
    {
        return;
    }

    if (mbItem == NULL)
    {
        KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus tcp客户端：creatNetModbusTaskEntry函数内存分配错误");
        KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus tcp客户端：creatNetModbusTaskEntry函数内存分配错误");
        return;
    }

    for (i = 0; i < inputItem->devNum; i++)
    {
        // 初始化通道信息
        initChanModbus(&inputItem->devPointMapArr[i], &mbItem[i]);

        // 初始化测点信息
        initPointModbus(&inputItem->devPointMapArr[i], &mbItem[i]);

        // 初始化发送队列
        if (initReqModbus(&mbItem[i]) == 0)
        {
            if (pthread_create(&mbItem[i].tfd, NULL, creatNetModbusThread, (void *)&mbItem[i]) == 0)
            {
                KITPTF(LOG_MODBUS_EN, INFO_EN, "modbus tcp线程 (ip：%s,port:%d) 创建成功", mbItem[i].itemTcp.ip, mbItem[i].itemTcp.port);
                KITLOG(LOG_MODBUS_EN, INFO_EN, "modbus tcp线程 (ip：%s,port:%d) 创建成功", mbItem[i].itemTcp.ip, mbItem[i].itemTcp.port);
            }
        }
    }
}

/*****************************************************************************
 * @brief       初始化配置，以串口号进行分类用于创建线程
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
// uint8_t initManagerParseCfg(proto_dev_point_map_t *inputItem, proto_dev_point_map_t *outputItem)
uint8_t initManagerParseCfg(proto_dev_point_map_t *inputItem, uart_chan_proto_map_t *uartItem)
{
    bool arr[MAX_COM_NUM] = {false};
    uint8_t uart_array[MAX_COM_NUM] = {0}, i = 0, uartNum = 0;
    // uint16_t pointArrayIdx[MAX_COM_NUM] = {0};
    for (i = 0; i < inputItem->devNum; i++)
    {
        if ((!arr[inputItem->devPointMapArr[i].devProtocol.uartLib.uartId]) && (inputItem->devPointMapArr[i].devProtocol.uartLib.uartId != 0) && (inputItem->devPointMapArr[i].protocolType == kProto_ModbusRTU_Master))
        {
            arr[inputItem->devPointMapArr[i].devProtocol.uartLib.uartId] = true;
            uart_array[uartNum] = inputItem->devPointMapArr[i].devProtocol.uartLib.uartId;
            uartNum++;
        }
    }

    if (uartNum > 0)
    {
        uartItem->uartCount = uartNum;
        uartItem->uartProtoItem = calloc(uartItem->uartCount, sizeof(proto_dev_point_map_t));

        if (uartItem->uartProtoItem == NULL)
        {
            KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus rtu 初始化内存分配错误");
            KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus rtu 初始化内存分配错误");
            return 1;
        }
    }

    for (uint8_t idx = 0; idx < uartItem->uartCount; idx++)
    {
        for (i = 0; i < inputItem->devNum; i++)
        {
            if (inputItem->devPointMapArr[i].devProtocol.uartLib.uartId == uart_array[idx])
            {
                uartItem->uartProtoItem[idx].devNum++;
            }
        }
    }

    // 对串口下挂设备台数进行内存分配
    for (uint8_t idx = 0; idx < uartItem->uartCount; idx++)
    {
        uartItem->uartProtoItem[idx].devPointMapArr = calloc(uartItem->uartProtoItem[idx].devNum, sizeof(dev_info_t));
        if (uartItem->uartProtoItem[idx].devPointMapArr == NULL)
        {
            KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus rtu 初始化内存分配错误");
            KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus rtu 初始化内存分配错误");
            return 1;
        }
    }

    uint8_t devIdx[100] = {0};
    // 对串口下挂设备台数进行内存分配
    for (uint8_t idx = 0; idx < uartItem->uartCount; idx++)
    {
        for (i = 0; i < inputItem->devNum; i++)
        {
            dev_info_t *src = &uartItem->uartProtoItem[idx].devPointMapArr[devIdx[idx]];
            if (inputItem->devPointMapArr[i].devProtocol.uartLib.uartId == uart_array[idx])
            {
                src->devProtocol = inputItem->devPointMapArr[i].devProtocol;
                src->devId = inputItem->devPointMapArr[i].devId;
                src->protocolType = inputItem->devPointMapArr[i].protocolType;
                src->devType = inputItem->devPointMapArr[i].devType;
                src->pointNum = inputItem->devPointMapArr[i].pointNum;
                src->pointArr = calloc(src->pointNum, sizeof(point_t));

                if (src->pointArr == NULL)
                {
                    KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus rtu pointArr 内存分配错误");
                    KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus rtu pointArr 初始化内存分配错误");
                    return 1;
                }

                for (uint16_t index = 0; index < src->pointNum; index++)
                {
                    src->pointArr[index] = inputItem->devPointMapArr[i].pointArr[index];
                }

                devIdx[idx]++;
            }
        }
    }
    return 0;
}

/*****************************************************************************
 * @brief      发送请求数据
 * @param[in]   item:
 * @return      -1：数据有误 0：接受正确
 *****************************************************************************/
int8_t mbRtuSendFrame(modbus_master_t *item, list_item_t *listItem)
{
    //
    uint8_t *data = listItem->byte;
    item->sendFrame.devAddr = data[0];
    item->sendFrame.func = data[1];
    item->sendFrame.startAddr = ((uint16_t)data[2]) << 8 | data[3];
    item->sendFrame.reqNum = ((uint16_t)data[4]) << 8 | data[5];

    memcpy(item->itemUart.send_buf, data, listItem->frameLen);
    item->itemUart.send_len = listItem->frameLen;

    // add @GQ 2024-11-05
    item->itemUart.recv_len = getModbusDataLength(item->sendFrame.func, item->sendFrame.reqNum, 0);

    if (drvUartSend(&item->itemUart) == 0)
    {
        return 0;
    }
    else
    {
        return -1;
    }
}

/*****************************************************************************
 * @brief       创建modbus任务
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
void *creatUartModbusThread(void *arg)
{
    modbus_rtu_master_t *uartItem = (modbus_rtu_master_t *)arg;
    uint8_t i = 0;

    if (drvUartOpen(&uartItem->itemUart) != 0)
    {
        return (void *)0;
    }
    else
    {
        for (i = 0; i < uartItem->devNum; i++)
        {
            uartItem->mbRtuMasterItem[i].itemUart.fd = uartItem->itemUart.fd;
            uartItem->mbRtuMasterItem[i].itemUart.status = uartItem->itemUart.status;
            initMbList(&uartItem->mbRtuMasterItem[i].mbCmdList, 1000);
        }
    }

    while (1)
    {
        for (i = 0; i < uartItem->devNum; i++)
        {
            int32_t ret = 0;
            modbus_master_t *item = &uartItem->mbRtuMasterItem[i];
            mbScanCtrlCmdInputList(item);
            if (listIsEmpty(&item->mbCmdList) == false)
            {
                list_item_t currItem = getItemByList(&item->mbCmdList);
                // 在此处不停地进行写队列元素的下发
                ret = mbRtuSendFrame(item, &currItem);
            }
            else if (listIsEmpty(&item->mbReqList) == false)
            {
                list_item_t *reqItem = &item->mbReqList.listArray[item->mbReqIdx];
                ret = mbRtuSendFrame(item, reqItem);
                item->mbReqIdx++;
                item->mbReqIdx %= item->mbReqList.size;
            }

            if (ret == 0)
            {
                int32_t rett = drvUartRecv(&item->itemUart);
                if (rett != 0)
                {
                    // 超时次数或者接受失败测试
                    item->itemUart.error_cnt++;
                    // 子设备设置为离线
                    if (item->itemUart.error_cnt > uartItem->itemUart.timeout / 100)
                    {
                        item->itemUart.status = CONN_OFFLINE;
                        item->isOnline = 0;
                        setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 0.0);
                    }
                }
                else
                {
                    if (mbRespondParseData(item) != 0)
                    {
                        // 超时次数或者接受失败测试
                        item->itemUart.error_cnt++;
                    }
                    else
                    {
                        item->itemUart.status = CONN_ONLINE;
                        item->isOnline = 1;
                        setRtdbPointValue(rtdbType, item->devType, item->devId, DEV_IS_ONLINE, 1.0);
                        item->itemUart.error_cnt = 0;
                    }
                }
            }

            // 解析告警测点
            if (item->itemUart.status == CONN_ONLINE)
            {
                calcAlarmCall(item);
            }
            usleep(20000);
        }

        for (i = 0; i < uartItem->devNum; i++)
        {
            uartItem->itemUart.error_cnt += uartItem->mbRtuMasterItem[i].itemUart.error_cnt;
        }

        if (uartItem->itemUart.error_cnt > uartItem->itemUart.timeout / 100)
        {
            uartItem->itemUart.status = CONN_OFFLINE;
        }

        usleep(20000);
    }
}

/*****************************************************************************
 * @brief       静态函数
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
static void uartInfoMemcpy(uart_lib_t *dest, kit_uart_lib_t *src)
{
    dest->uartId = src->uartId;
    strncpy((char *)dest->address, (char *)src->address, MAX_ADDR_LEN);
    dest->uId = src->uId;
    dest->data = (uart_data_e)src->data;
    dest->stop = (uart_stop_e)src->stop;
    dest->baud = (uart_baud_e)src->baud;
    dest->parity = (uart_parity_e)src->parity;
    dest->flow = (uart_flow_e)src->flow;
    dest->timeout = src->timeout;
}
/*****************************************************************************
 * @brief       创建modbus rtu 任务入口
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
void creatUartModbusTaskEntry(proto_dev_point_map_t *CfgItem)
{
    // NULL 保护
    if (CfgItem == NULL)
    {
        KITLOG(LOG_MODBUS_EN, ERROR_EN, "输入项为NULL,无法创建Modbus任务条目");
        return;
    }
    // 先统计串口的个数，根据串口个数创建线程
    static uart_chan_proto_map_t item = {0};
    static pthread_t tfd[MAX_COM_NUM] = {0};
    if (initManagerParseCfg(CfgItem, &item) != 0)
    {
        return;
    }

    uart_chan_proto_map_t *inputItem = &item;
    modbus_rtu_master_t *mbRtuItem = NULL;
    if (inputItem->uartCount > 0)
    {
        mbRtuItem = calloc(inputItem->uartCount, sizeof(modbus_rtu_master_t));
        if (mbRtuItem == NULL)
        {
            KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus RTU客户端初始化函数内存分配错误");
            KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus RTU客户端初始化函数内存分配错误");
            return;
        }
    }

    for (uint8_t i = 0; i < inputItem->uartCount; i++)
    // for (uint8_t i = 0; i < 1; i++)
    {
        mbRtuItem[i].devNum = inputItem->uartProtoItem[i].devNum;
        uartInfoMemcpy(&mbRtuItem[i].itemUart, &inputItem->uartProtoItem[i].devPointMapArr[0].devProtocol.uartLib);
        mbRtuItem[i].mbRtuMasterItem = calloc(mbRtuItem[i].devNum, sizeof(modbus_master_t));

        for (uint16_t k = 0; k < inputItem->uartProtoItem[i].devNum; k++)
        {
            initChanModbus(&inputItem->uartProtoItem[i].devPointMapArr[k], &mbRtuItem[i].mbRtuMasterItem[k]);
            initPointModbus(&inputItem->uartProtoItem[i].devPointMapArr[k], &mbRtuItem[i].mbRtuMasterItem[k]);

            if (initParseAlarmThread(&mbRtuItem[i].mbRtuMasterItem[k]) != 0)
            {
                return;
            }

            if (initReqModbus(&mbRtuItem[i].mbRtuMasterItem[k]) == 0)
            {
                KITLOG(LOG_MODBUS_EN, INFO_EN, "初始化串口%d，设备类型%d，设备号：%d，初始化队列成功", inputItem->uartProtoItem[i].devPointMapArr[k].devProtocol.uartLib.uartId,
                       inputItem->uartProtoItem[i].devPointMapArr[k].devType, inputItem->uartProtoItem[i].devPointMapArr[k].devId);
                KITPTF(LOG_MODBUS_EN, INFO_EN, "初始化串口%d，设备类型%d，设备号：%d，初始化队列成功", inputItem->uartProtoItem[i].devPointMapArr[k].devProtocol.uartLib.uartId,
                       inputItem->uartProtoItem[i].devPointMapArr[k].devType, inputItem->uartProtoItem[i].devPointMapArr[k].devId);
            }
        }

        if (pthread_create(&tfd[i], NULL, creatUartModbusThread, (void *)&mbRtuItem[i]) == 0)
        {
            KITPTF(LOG_MODBUS_EN, INFO_EN, "modbus rtu线程 (uart：%d,baud rate:%d,parity:%d) 创建成功", mbRtuItem[i].itemUart.uartId, mbRtuItem[i].itemUart.baud, mbRtuItem[i].itemUart.stop);
            KITLOG(LOG_MODBUS_EN, INFO_EN, "modbus rtu线程 (uart：%d,baud rate::%d,parity:%d) 创建成功", mbRtuItem[i].itemUart.uartId, mbRtuItem[i].itemUart.baud, mbRtuItem[i].itemUart.stop);
        }
    }
}

int is_same_tcp_port(const char *ip1, int port1, const char *ip2, int port2)
{
    return (strcmp(ip1, ip2) == 0) && (port1 == port2);
}

int uid_exists(int *list, int count, int uid)
{
    for (int i = 0; i < count; i++)
    {
        if (list[i] == uid)
            return 1;
    }
    return 0;
}

/*****************************************************************************
 * @brief       初始化配置，以串口号进行分类用于创建线程
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
#define MAX_NET_DEVICES 16
uint8_t initnetUartModeParseCfg(proto_dev_point_map_t *inputItem, net_chan_proto_map_t *netItem)
{
    // bool arr[MAX_COM_NUM] = {false};
    uint8_t i = 0, thread_count = 0;
    kit_tcp_client_lib_t threads[MAX_NET_DEVICES] = {0};
    // uint16_t pointArrayIdx[MAX_COM_NUM] = {0};

    // 统计ip和port的个数，及线程的个数
    for (i = 0; i < inputItem->devNum; i++)
    {
        kit_tcp_client_lib_t current = {0};
        strcpy(current.ip, inputItem->devPointMapArr[i].devProtocol.tcpClientLib.ip);
        current.port = inputItem->devPointMapArr[i].devProtocol.tcpClientLib.port;
        int found = 0;
        kit_tcp_client_lib_t *d = &inputItem->devPointMapArr[i].devProtocol.tcpClientLib;
        for (int j = 0; j < thread_count; ++j)
        {
            if (is_same_tcp_port(d->ip, d->port, threads[j].ip, threads[j].port))
            {
                found = 1;
                break;
            }
        }
        if (!found)
        {
            threads[thread_count++] = current;
        }
    }

    if (thread_count > 0)
    {
        netItem->taskCount = thread_count;
        netItem->ProtoItem = calloc(netItem->taskCount, sizeof(proto_dev_point_map_t));

        if (netItem->ProtoItem == NULL)
        {
            KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus rtu 初始化内存分配错误");
            KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus rtu 初始化内存分配错误");
            return 1;
        }
    }
    else
    {
        return 0;
    }

    for (uint8_t idx = 0; idx < netItem->taskCount; idx++)
    {
        for (i = 0; i < inputItem->devNum; i++)
        {
            if (is_same_tcp_port(inputItem->devPointMapArr[i].devProtocol.tcpClientLib.ip, inputItem->devPointMapArr[i].devProtocol.tcpClientLib.port, threads[idx].ip, threads[idx].port))
            {
                netItem->ProtoItem[idx].devNum++;
            }
        }
    }

    // 对IP - port下挂设备台数进行内存分配
    for (uint8_t idx = 0; idx < netItem->taskCount; idx++)
    {
        netItem->ProtoItem[idx].devPointMapArr = calloc(netItem->ProtoItem[idx].devNum, sizeof(dev_info_t));
        if (netItem->ProtoItem[idx].devPointMapArr == NULL)
        {
            KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus rtu 初始化内存分配错误");
            KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus rtu 初始化内存分配错误");
            return 1;
        }
    }

    uint8_t devIdx[100] = {0};
    // 对网口下挂设备台数进行内存分配
    for (uint8_t idx = 0; idx < netItem->taskCount; idx++)
    {
        for (i = 0; i < inputItem->devNum; i++)
        {
            dev_info_t *src = &netItem->ProtoItem[idx].devPointMapArr[devIdx[idx]];
            if (is_same_tcp_port(inputItem->devPointMapArr[i].devProtocol.tcpClientLib.ip, inputItem->devPointMapArr[i].devProtocol.tcpClientLib.port,
                                 threads[idx].ip, threads[idx].port))
            {
                src->devProtocol = inputItem->devPointMapArr[i].devProtocol;
                memcpy(src->devProtocol.tcpClientLib.ip, inputItem->devPointMapArr[i].devProtocol.tcpClientLib.ip, 32);
                src->devProtocol.tcpClientLib.port = inputItem->devPointMapArr[i].devProtocol.tcpClientLib.port;
                src->devProtocol.tcpClientLib.uId = inputItem->devPointMapArr[i].devProtocol.tcpClientLib.uId;
                src->devProtocol.tcpClientLib.timeout = inputItem->devPointMapArr[i].devProtocol.tcpClientLib.timeout;
                src->devProtocol.tcpClientLib.netId = inputItem->devPointMapArr[i].devProtocol.tcpClientLib.netId;
                src->devId = inputItem->devPointMapArr[i].devId;
                src->protocolType = inputItem->devPointMapArr[i].protocolType;
                src->devType = inputItem->devPointMapArr[i].devType;
                src->pointNum = inputItem->devPointMapArr[i].pointNum;
                src->pointArr = calloc(src->pointNum, sizeof(point_t));

                if (src->pointArr == NULL)
                {
                    KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus rtu pointArr 内存分配错误");
                    KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus rtu pointArr 初始化内存分配错误");
                    return 1;
                }

                for (uint16_t index = 0; index < src->pointNum; index++)
                {
                    src->pointArr[index] = inputItem->devPointMapArr[i].pointArr[index];
                }

                devIdx[idx]++;
            }
        }
    }
    return 0;
}

/*****************************************************************************
 * @brief       静态函数
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
static void netInfoMemcpy(tcp_client_lib_t *dest, kit_tcp_client_lib_t *src)
{
    dest->port = src->port;
    strncpy((char *)dest->ip, (char *)src->ip, MAX_IP_LEN);
    dest->uId = src->uId;
    dest->timeout = src->timeout;
    dest->transaction = 0;
}

tcpStateMacFun mbClientMultStateCall[TCP_CLIENT_END] =
    {
        mbMasterMultStartCall,
        mbMasterMultOnlineCall,
        mbMasterMultOfflineCall};

/*****************************************************************************
 * @brief       创建modbus任务
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
void *creatNetModbusMultThread(void *arg)
{
    if (arg == NULL)
    {
        KITPTF(LOG_MODBUS_EN, INFO_EN, "item指针为空");
        KITLOG(LOG_MODBUS_EN, INFO_EN, "item指针为空");
    }
    modbus_tcp_multid_master_t *item = arg;
    tcp_client_state_e state = TCP_CLIENT_START;

    while (1)
    {
        state = mbClientMultStateCall[state](item);

        printf("进入主线程\n");
        usleep(50000);
    }
}

// 初始化modbus tcp 串口模式任务入口
void creatNetModbusMultTaskEntry(proto_dev_point_map_t *CfgItem)
{
    // NULL 保护
    if (CfgItem == NULL)
    {
        KITLOG(LOG_MODBUS_EN, ERROR_EN, "输入项为NULL,无法创建Modbus任务条目");
        return;
    }
    // 先统计IP的个数，根据IP和端口个数创建线程
    net_chan_proto_map_t item = {0};
    pthread_t tfd[MAX_COM_NUM] = {0};
    if (initnetUartModeParseCfg(CfgItem, &item) != 0)
    {
        return;
    }

    net_chan_proto_map_t *inputItem = &item;
    modbus_tcp_multid_master_t *mbTcpMultIdItem = NULL;
    if (inputItem->taskCount > 0)
    {
        mbTcpMultIdItem = calloc(inputItem->taskCount, sizeof(modbus_tcp_multid_master_t));
        if (mbTcpMultIdItem == NULL)
        {
            KITLOG(LOG_MODBUS_EN, ERROR_EN, "modbus Mult Tcp客户端初始化函数内存分配错误");
            KITPTF(LOG_MODBUS_EN, ERROR_EN, "modbus Mult Tcp客户端初始化函数内存分配错误");
            return;
        }
    }
    else
    {
        return;
    }

    for (uint8_t i = 0; i < inputItem->taskCount; i++)
    {
        mbTcpMultIdItem[i].devNum = inputItem->ProtoItem[i].devNum;
        netInfoMemcpy(&mbTcpMultIdItem[i].itemTcp, &inputItem->ProtoItem[i].devPointMapArr[0].devProtocol.tcpClientLib);
        mbTcpMultIdItem[i].mbMasterItem = calloc(mbTcpMultIdItem[i].devNum, sizeof(modbus_master_t));

        for (uint16_t k = 0; k < inputItem->ProtoItem[i].devNum; k++)
        {
            initChanModbus(&inputItem->ProtoItem[i].devPointMapArr[k], &mbTcpMultIdItem[i].mbMasterItem[k]);
            initPointModbus(&inputItem->ProtoItem[i].devPointMapArr[k], &mbTcpMultIdItem[i].mbMasterItem[k]);

            if (initParseAlarmThread(&mbTcpMultIdItem[i].mbMasterItem[k]) != 0)
            {
                return;
            }

            if (initReqModbus(&mbTcpMultIdItem[i].mbMasterItem[k]) == 0)
            {
                KITLOG(LOG_MODBUS_EN, INFO_EN, "TcpMult:初始化ip:%s，端口号：%d，ud：%d，设备类型%d，设备号：%d，初始化队列成功", inputItem->ProtoItem[i].devPointMapArr[k].devProtocol.tcpClientLib.ip,
                       inputItem->ProtoItem[i].devPointMapArr[k].devProtocol.tcpClientLib.port, inputItem->ProtoItem[i].devPointMapArr[k].devProtocol.tcpClientLib.uId,
                       inputItem->ProtoItem[i].devPointMapArr[k].devType, inputItem->ProtoItem[i].devPointMapArr[k].devId);
                KITPTF(LOG_MODBUS_EN, INFO_EN, "TcpMult:初始化ip:%s，端口号：%d，ud：%d，设备类型%d，设备号：%d，初始化队列成功", inputItem->ProtoItem[i].devPointMapArr[k].devProtocol.tcpClientLib.ip,
                    inputItem->ProtoItem[i].devPointMapArr[k].devProtocol.tcpClientLib.port, inputItem->ProtoItem[i].devPointMapArr[k].devProtocol.tcpClientLib.uId,
                    inputItem->ProtoItem[i].devPointMapArr[k].devType, inputItem->ProtoItem[i].devPointMapArr[k].devId);
            }
        }

        if (pthread_create(&tfd[i], NULL, creatNetModbusMultThread, (void *)&mbTcpMultIdItem[i]) == 0)
        {
            KITPTF(LOG_MODBUS_EN, INFO_EN, "modbus tcp multi 线程 (ip:%s,port:%d) 创建成功", mbTcpMultIdItem[i].itemTcp.ip, mbTcpMultIdItem[i].itemTcp.port);
            KITLOG(LOG_MODBUS_EN, INFO_EN, "modbus tcp multi 线程 (ip:%s,port:%d) 创建成功", mbTcpMultIdItem[i].itemTcp.ip, mbTcpMultIdItem[i].itemTcp.port);
        }
    }
}