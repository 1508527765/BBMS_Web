/*****************************************************************************
 * @copyright       2000-2050,Copyright (c) 2025, gary. All rights reserved.
 * @file            bsp_rbms_com.c
 * @brief           RBMS通信功能实现
 * @author          gary
 * @date            2026-03-19
 * @remark
 *****************************************************************************/

#include "bsp_bms_com.h"
#include "kit_math.h"

/* 接收队列和发送队列 */
static kit_queue_t bbms_info_queue;
static kit_queue_t recv_queue;
static kit_queue_t send_queue; //总发送队列（上位机发送RBMS指令队列+bbms发送至RBMS指令队列）

/* 全局RBMS连接数组及管理器 */
static rbms_tcp_master_t *g_rbms_array = NULL;
static uint16_t g_rbms_count = 0;
static pthread_mutex_t g_rbms_array_mutex = PTHREAD_MUTEX_INITIALIZER;

/* 发送分发线程标志 */
static pthread_t g_send_dispatch_thread = 0;
static volatile uint8_t g_send_dispatch_thread_exit = 0;

/* 静态缓冲区（减少堆分配） 后续用于透传上送上位机*/
static uint8_t recv_queue_buf[RBMS_RECV_QUEUE_ITEM_SIZE * sizeof(bms_queue_msg_t)];

/* ==================== 与上位机通信相关 ==================== */
/* 控制回复队列（高优先级） */
static kit_queue_t ctrl_reply_queue;
static uint8_t ctrl_reply_queue_buf[RBMS_CTRL_QUEUE_ITEM_SIZE * sizeof(bms_queue_msg_t)];
static volatile uint8_t g_uplink_send_thread_exit = 0;  // 发送线程退出标志
static volatile uint8_t g_uplink_recv_thread_exit = 0;  // 接收线程退出标志


//bbms本体信息队列
static uint8_t bbms_info_queue_buf[BBMS_INFO_QUEUE_ITEM_SIZE * sizeof(bms_queue_msg_t)] = {0};

static uplink_context_t g_uplink_ctx = {0};

// 定义解析rbms上报数据解析的回调函数
rbmsTcpMasterParseRecvFun rbmsTcpMasterRecvCall = NULL;

void registerRbmsTcpMasterRecvCall(rbmsTcpMasterParseRecvFun func) { rbmsTcpMasterRecvCall = func; }

//已知data创建发送帧，依据RBMS通信协议定义
/* 解析RBMS数据
 * RBMS通信协议定义
| Byte1 | Byte2-3          | Byte4-5 | Byte6 | Byte7  | Byte8 | Byte9   | Byte10        | Byte11  | Byte12   | Byte13 | Byte14-n |
| ----- | ---------------- | ------- | ----- | ------ | ----- | ------- | ------------- | ------- | -------- | ------ | -------- |
| Head  | Version&datalen  | CRC16   | src   | srcSub | dest  | destSub | transportType | frameId | cmdGroup | cmdId  | data     |
| 0xA5  | version=2        |         |       |        |       |         |               |         |          |        |          |
*/
/*
 * @brief 创建RBMS发送帧
 * @param linkMsg   协议头
 * @param networkMsg 网络层报文
 * @param transportMsg 传输层报文
 * @param cmdGroup 命令组
 * @param cmdId 命令号
 * @param data 有效负载数据
 * @param len 有效负载数据长度
 * @param frame 输出的RBMS发送帧缓冲区
*/
void createSendFrame(SrcDestMsgV2 *networkMsg, TransportMsgV1 *transportMsg, uint8_t cmdGroup,
                     uint8_t cmdId, uint8_t *data, int len, uint8_t *frame)
{
    LinkMsg linkMsgTemp = {0};
    LinkMsg *linkMsg = &linkMsgTemp;
    uint8_t *pFrame = frame;
    uint8_t dataBodyLen =
        8 + len; // SrcDestMsgV2(4) + TransportMsgV1(2) + ExpressionMsgV1(2) + data(len)

    if (len < 0 || len > (int)RBMS_MAX_PAYLOAD_LEN)
    {
        return;
    }

    // 1. 组装协议头 LinkMsg (5 bytes)
    linkMsg->head = RBMS_PROTOCOL_HEAD;
    linkMsg->version = RBMS_PROTOCOL_VERSION;
    linkMsg->len = dataBodyLen; // data总长度（byte6-n的长度）

    // 先计算CRC16（暂时为0，后面需要实际计算）
    linkMsg->check = 0;
    memcpy(pFrame, linkMsg, sizeof(LinkMsg));
    pFrame += sizeof(LinkMsg);

    // 2. 组装网络层 SrcDestMsgV2 (4 bytes)
    memcpy(pFrame, networkMsg, sizeof(SrcDestMsgV2));
    pFrame += sizeof(SrcDestMsgV2);

    // 3. 组装传输层 TransportMsgV1 (2 bytes)
    memcpy(pFrame, transportMsg, sizeof(TransportMsgV1));
    pFrame += sizeof(TransportMsgV1);

    // 4. 组装表达层 ExpressionMsgV1 (2 bytes + data)
    *pFrame++ = cmdGroup;
    *pFrame++ = cmdId;
    if (data != NULL && len > 0)
    {
        memcpy(pFrame, data, len);
        pFrame += len;
    }

    // 5. 计算并填充CRC16（对byte6-n进行CRC16-Modbus校验）
    // byte6-n的起始位置: frame + sizeof(LinkMsg)
    uint8_t *crcDataStart = frame + sizeof(LinkMsg);
    linkMsg->check = kit_check_crc16(crcDataStart, dataBodyLen);
    memcpy(frame + 3, &linkMsg->check, sizeof(uint16_t));
}

//bms_queue_msg_t
bms_queue_msg_t createSendFrameQueueItem(SrcDestMsgV2 *networkMsg, TransportMsgV1 *transportMsg,
                                         uint8_t cmdGroup, uint8_t cmdId, uint8_t *data, int len)
{
    bms_queue_msg_t msgTemp = {0};
    bms_queue_msg_t *msg = &msgTemp;
    LinkMsg linkMsgTemp = {0};
    LinkMsg *linkMsg = &linkMsgTemp;
    uint8_t *pFrame = msg->data;
    uint8_t dataBodyLen = 8 + len; // SrcDestMsgV2(4) + TransportMsgV1(2) + ExpressionMsgV1(2) + data(len)

    if (len < 0 || len > (int)RBMS_MAX_PAYLOAD_LEN)
    {
        return msgTemp;
    }

    // 1. 组装协议头 LinkMsg (5 bytes)
    linkMsg->head = RBMS_PROTOCOL_HEAD;
    linkMsg->version = RBMS_PROTOCOL_VERSION;
    linkMsg->len = dataBodyLen; // data总长度（byte6-n的长度）

    // 先计算CRC16（暂时为0，后面需要实际计算）
    linkMsg->check = 0;
    memcpy(pFrame, linkMsg, sizeof(LinkMsg));
    pFrame += sizeof(LinkMsg);

    // 2. 组装网络层 SrcDestMsgV2 (4 bytes)
    memcpy(pFrame, networkMsg, sizeof(SrcDestMsgV2));
    pFrame += sizeof(SrcDestMsgV2);

    // 3. 组装传输层 TransportMsgV1 (2 bytes)
    memcpy(pFrame, transportMsg, sizeof(TransportMsgV1));
    pFrame += sizeof(TransportMsgV1);

    // 4. 组装表达层 ExpressionMsgV1 (2 bytes + data)
    *pFrame++ = cmdGroup;
    *pFrame++ = cmdId;
    if (data != NULL && len > 0)
    {
        memcpy(pFrame, data, len);
        pFrame += len;
    }

    // 5. 计算并填充CRC16（对byte6-n进行CRC16-Modbus校验）
    // byte6-n的起始位置: frame + sizeof(LinkMsg)
    uint8_t *crcDataStart = msg->data + sizeof(LinkMsg);
    linkMsg->check = kit_check_crc16(crcDataStart, dataBodyLen);
    memcpy(msg->data + 3, &linkMsg->check, sizeof(uint16_t));
    msg->len = (int)sizeof(LinkMsg) + dataBodyLen;

    return msgTemp;
}

//对外开放send_queue入队列函数
void send_queue_enqueue(bms_queue_msg_t *msg)
{
    if (msg == NULL)
    {
        return;
    }
    kit_queue_enqueue(&send_queue, msg);
}

//对外开放 将bbms数据添加至bbms_info_queue队列，等待上位机线程处理
void bbms_info_enqueue(bms_queue_msg_t *msg)
{
    if (msg == NULL)
    {
        return;
    }
    if (kit_queue_enqueue(&bbms_info_queue, msg) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "bbms_info_queue full, uplink frame dropped");
    }
}

/**
 * @brief 向上位机发送控制回复（高优先级）
 */
void uplink_bbms_ctrl_reply_enqueue(bms_queue_msg_t *msg)
{
    if (msg == NULL)
    {
        return;
    }
    if (kit_queue_enqueue(&ctrl_reply_queue, msg) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "ctrl_reply_queue full, ctrl reply dropped");
    }
}

/**
 * @brief 根据目的地址获取对应的RBMS连接
 * @param dest    目的器件地址 (协议中的dest字段)
 * @param destSub 目的器件子地址 (协议中的destSub字段)
 * @return 返回对应的rbms_tcp_master_t指针，未找到返回NULL
 */
rbms_tcp_master_t *getRbmsByDest(uint8_t dest, uint8_t destSub)
{
    if (g_rbms_array == NULL || g_rbms_count == 0)
    {
        return NULL;
    }

    pthread_mutex_lock(&g_rbms_array_mutex);
    for (uint16_t i = 0; i < g_rbms_count; i++)
    {
        // 根据协议定义：RBMS的dest=0x04，destSub对应设备ID（索引+1）
        if (g_rbms_array[i].isUsed && dest == 0x04)
        {
            // destSub是RBMS的子节点号，需要与devId匹配
            if (destSub == g_rbms_array[i].devId)
            {
                pthread_mutex_unlock(&g_rbms_array_mutex);
                return &g_rbms_array[i];
            }
        }
    }
    pthread_mutex_unlock(&g_rbms_array_mutex);
    return NULL;
}

/**
 * @brief 发送队列分发线程 - 从全局send_queue分发到各RBMS的send_queue
 */
static void *send_queue_dequeue_thread_func(void *arg)
{
    bms_queue_msg_t msg = {0};

    KITLOG(LOG_APP_EN, INFO_EN, "send_queue_dequeue_thread started");

    while (!g_send_dispatch_thread_exit)
    {
        // 从全局send_queue出队
        if (kit_queue_dequeue(&send_queue, &msg) == 0)
        {
            // 解析消息获取目的地址
            // 协议格式: | Head(1) | Version&len(3) | CRC16(2) | src(1) | srcSub(1) | dest(1) | destSub(1) | ...
            // 0-based: src[5] srcSub[6] dest[7] destSub[8]（与 protocol_bms_hmi.c 一致）
            if (msg.len >= 9)
            {
                uint8_t dest = msg.data[7];
                uint8_t destSub = msg.data[8];

                // 查找对应的RBMS连接
                rbms_tcp_master_t *rbms = getRbmsByDest(dest, destSub);

            if (rbms != NULL)
            {
               // 分发到对应RBMS的send_queue（队列内部有互斥锁保护）
               if (kit_queue_enqueue(&rbms->send_queue, &msg) != 0)
               {
                  KITLOG(LOG_APP_EN, INFO_EN, "RBMS send_queue full, devId=%d", rbms->devId);
               }
            }
            else
            {
               KITLOG(LOG_APP_EN, ERROR_EN, "No RBMS found for dest=0x%02X, destSub=%d", dest, destSub);
            }
         }
         else
         {
            KITLOG(LOG_APP_EN, ERROR_EN, "Invalid message length: %d", msg.len);
         }
      }
      else
      {
         // 队列为空，短暂休眠
         usleep(10000); // 10ms
      }
   }

    KITLOG(LOG_APP_EN, INFO_EN, "send_queue_dequeue_thread exiting");
    return NULL;
}

/**
 * @brief 启动从全局send_queue分发到各RBMS的线程
 */
void startSendQueueDequeueThread(void)
{
   if (g_send_dispatch_thread != 0)
   {
      KITLOG(LOG_APP_EN, INFO_EN, "send_queue_dequeue_thread already running");
      return;
   }

    g_send_dispatch_thread_exit = 0;
    if (pthread_create(&g_send_dispatch_thread, NULL, send_queue_dequeue_thread_func, NULL) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Failed to create send_queue_dequeue_thread");
    }
}

/* 解析RBMS数据
 * RBMS通信协议定义
| Byte1 | Byte2-3          | Byte4-5 | Byte6 | Byte7  | Byte8 | Byte9   | Byte10        | Byte11  | Byte12   | Byte13 | Byte14-n |
| ----- | ---------------- | ------- | ----- | ------ | ----- | ------- | ------------- | ------- | -------- | ------ | -------- |
| Head  | Version&datalen  | CRC16   | src   | srcSub | dest  | destSub | transportType | frameId | cmdGroup | cmdId  | data     |
| 0xA5  | version=2        |         |       |        |       |         |               |         |          |        |          |
*/

/*****************************************************************************
 * @brief  从 LinkMsg 原始字节解析 version / body_len / crc（小端，避免 bitfield 别名问题）
 * @return 0-成功  1-失败
 *****************************************************************************/
static uint8_t rbmsDecodeLinkHeader(const uint8_t *data, uint16_t *body_len, uint16_t *check_crc)
{
    if (data == NULL || body_len == NULL || check_crc == NULL)
    {
        return 1;
    }

    if (data[0] != RBMS_PROTOCOL_HEAD)
    {
        return 1;
    }

    uint16_t ver_len = (uint16_t)data[1] | ((uint16_t)data[2] << 8);
    uint8_t version = (uint8_t)(ver_len & 0x1FU);
    if (version != RBMS_PROTOCOL_VERSION)
    {
        return 1;
    }

    *body_len = (uint16_t)(ver_len >> 5);
    *check_crc = (uint16_t)data[3] | ((uint16_t)data[4] << 8);
    return 0;
}

uint8_t parseCheckRbmsData(uint8_t *data, int len)
{
    uint16_t body_len = 0;
    uint16_t check_crc = 0;
    const int link_sz = (int)sizeof(LinkMsg);

    if (data == NULL || len < link_sz + 8)
    {
        return 1; /* frame too short */
    }

    if (rbmsDecodeLinkHeader(data, &body_len, &check_crc) != 0)
    {
        return 2; /* bad head/version */
    }

    if (body_len > RBMS_MAX_DATA_LEN)
    {
        return 3; /* len out of range */
    }

    if (body_len != len - link_sz)
    {
        return 3; /* len mismatch (常见：drvTcpRecv 粘包/截断) */
    }

    uint16_t calc_crc = kit_check_crc16(data + link_sz, body_len);
    if (check_crc != calc_crc)
    {
        return 4; /* crc mismatch */
    }

    return 0;
}

/*****************************************************************************
 * @brief       按 LinkMsg.len 收满整帧（Volt/Temp 等大包超过单次 1024 recv）
 * @return      0-收满一帧  -128-超时无数据  -1-错误/断开
 *****************************************************************************/
static int rbmsTcpRecvFullFrame(tcp_client_lib_t *p_tcp)
{
    if (p_tcp == NULL)
    {
        return -1;
    }

    if (p_tcp->timeout == 0)
    {
        p_tcp->timeout = 1000;
    }

    if (p_tcp->timeout != 0)
    {
        fd_set set;
        FD_ZERO(&set);
        FD_SET(p_tcp->socket_fd, &set);
        if (p_tcp->timeout / 1000 > 30)
        {
            p_tcp->timeout = 30 * 1000;
        }
        struct timeval time_out = {1, 0};
        int selRet = select(p_tcp->socket_fd + 1, &set, NULL, NULL, &time_out);
        if (selRet == 0)
        {
            return -128;
        }
        if (selRet < 0)
        {
            return -1;
        }
    }

    size_t got = 0;
    const size_t hdr_sz = sizeof(LinkMsg);

    while (got < hdr_sz)
    {
        ssize_t n = recv(p_tcp->socket_fd, p_tcp->recv_buf + got, hdr_sz - got, 0);
        if (n < 0)
        {
            p_tcp->recv_len = 0;
            return -1;
        }
        if (n == 0)
        {
            p_tcp->recv_len = 0;
            return -128;
        }
        got += (size_t)n;
    }

    LinkMsg *hdr = (LinkMsg *)p_tcp->recv_buf;
    size_t frame_len = hdr_sz + hdr->len;
    if (frame_len > MAX_TCP_LEN)
    {
        p_tcp->recv_len = 0;
        KITLOG(LOG_APP_EN, ERROR_EN, "RBMS frame too large: %zu", frame_len);
        return -1;
    }

    while (got < frame_len)
    {
        ssize_t n = recv(p_tcp->socket_fd, p_tcp->recv_buf + got, frame_len - got, 0);
        if (n < 0)
        {
            p_tcp->recv_len = 0;
            return -1;
        }
        if (n == 0)
        {
            p_tcp->recv_len = 0;
            return -128;
        }
        got += (size_t)n;
    }

    p_tcp->recv_len = (uint32_t)got;
    p_tcp->last_time = get_epoch_time_ms();
    return 0;
}

static uint16_t rbmsRtdbDevSlot(const rbms_tcp_master_t *item)
{
    if (item == NULL)
    {
        return 0;
    }
    return (item->devId > 0) ? (uint16_t)(item->devId - 1) : 0;
}

/*****************************************************************************
 * @brief       开始rbms tcp通道链接
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
tcp_client_state_e rbmsTcpMasterStartCall(void *arg)
{
    rbms_tcp_master_t *item = arg;

   setRtdbPointValue(rtdbType, item->devType, item->devId - 1, DEV_IS_ONLINE, 0.0);

   item->isOnline = 0;

    if (drvTcpOpen(&item->itemTcp) == 0)
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
 * @brief       rbms tcp通道链接成功后，开始数据上报
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
tcp_client_state_e rbmsTcpMasterConnected(void *arg)
{
    rbms_tcp_master_t *item = arg;
    bms_queue_msg_t recv_msg = {0};

   // 设置在线状态
   setRtdbPointValue(rtdbType, item->devType, item->devId - 1, DEV_IS_ONLINE, 1.0);
   item->isOnline = 1;
   item->isUsed = 1;

    KITLOG(LOG_APP_EN, INFO_EN, "RBMS connected: devId=%d, ip=%s, port=%d", item->devId,
           item->itemTcp.ip, item->itemTcp.port);

    while (1)
    {
        // 先判断发送队列中有没有数据需要发送，如果有则优先发送数据
        if (kit_queue_is_empty(&item->send_queue) == 0)
        {
            bms_queue_msg_t send_msg = {0};
            if (kit_queue_dequeue(&item->send_queue, &send_msg) == 0)
            {
                item->itemTcp.send_len = send_msg.len;
                memcpy(item->itemTcp.send_buf, send_msg.data, send_msg.len);

                // 发送数据
                if (drvTcpSend(&item->itemTcp) != 0)
                {
                    KITLOG(LOG_APP_EN, ERROR_EN, "Failed to send data to RBMS devId=%d",
                           item->devId);
                }
            }
        }

        /* 按 LinkMsg.len 组满整帧后再校验（避免 drvTcpRecv 1024 截断 / TCP 粘包） */
        for (;;)
        {
            int revRet = rbmsTcpRecvFullFrame(&item->itemTcp);
            if (revRet == -128)
            {
                break; /* 当前无更多数据 */
            }
            if (revRet < 0)
            {
                item->isOnline = 0;
                setRtdbPointValue(rtdbType, item->devType, item->devId - 1, DEV_IS_ONLINE, 0.0);
                KITLOG(LOG_APP_EN, ERROR_EN, "RBMS recv error, devId=%d", item->devId);
                return TCP_CLIENT_START;
            }

            uint8_t parseRet =
                parseCheckRbmsData(item->itemTcp.recv_buf, (int)item->itemTcp.recv_len);
            if (parseRet == 0)
            {
                item->itemTcp.count = 0;
                recv_msg.len = item->itemTcp.recv_len;
                memcpy(recv_msg.data, item->itemTcp.recv_buf, recv_msg.len);

                /* Link(5) + src[5] srcSub[6] dest[7] destSub[8] transportType[9] */
                uint8_t transportType = item->itemTcp.recv_buf[9];

                if (transportType == 0x03)
                {
                    uplink_ctrl_reply_enqueue(item->itemTcp.recv_buf, item->itemTcp.recv_len);
                }
                else
                {
                    if (kit_queue_enqueue(&recv_queue, &recv_msg) != 0)
                    {
                        KITLOG(LOG_APP_EN, ERROR_EN, "kit_queue_enqueue error for recv_queue");
                    }
                }

                uint8_t *data = item->itemTcp.recv_buf + sizeof(LinkMsg);
                uint16_t dataLen = (uint16_t)(item->itemTcp.recv_len - sizeof(LinkMsg));

                if (item->recvCall != NULL)
                {
                    item->recvCall(item->devType, item->devId - 1, data, (int)dataLen);
                }
            }
            else
            {
                item->itemTcp.count++;
                uint16_t body_len = 0;
                uint16_t check_crc = 0;
                uint16_t calc_crc = 0;
                (void)rbmsDecodeLinkHeader(item->itemTcp.recv_buf, &body_len, &check_crc);
                if (item->itemTcp.recv_len > (uint32_t)sizeof(LinkMsg))
                {
                    calc_crc = kit_check_crc16(item->itemTcp.recv_buf + sizeof(LinkMsg), body_len);
                }
                KITLOG(LOG_APP_EN, ERROR_EN,
                       "parseCheckRbmsData err=%u devId=%d recv_len=%u body_len=%u "
                       "expect_len=%u crc=0x%04X calc=0x%04X cmd=0x%02X/0x%02X",
                       parseRet, item->devId, item->itemTcp.recv_len, body_len,
                       (unsigned)(body_len + sizeof(LinkMsg)), check_crc, calc_crc,
                       item->itemTcp.recv_len > 11 ? item->itemTcp.recv_buf[11] : 0,
                       item->itemTcp.recv_len > 12 ? item->itemTcp.recv_buf[12] : 0);
            }
        }

      // 检查在线状态
      if (item->isOnline == 0)
      {
         KITLOG(LOG_APP_EN, INFO_EN, "RBMS offline, reconnecting...");
         return TCP_CLIENT_START;
      }

        usleep(100000); // 100ms
    }

    return TCP_CLIENT_ONLINE;
}

/*****************************************************************************
 * @brief       rbms tcp通道链接异常后，重新连接
 * @param[in]   item: 指针
 * @return      NONE
 *****************************************************************************/
tcp_client_state_e rbmsTcpMasterConnectClosed(void *arg)
{
    rbms_tcp_master_t *item = arg;

    if (item->isOnline == 0)
    {
        return TCP_CLIENT_START;
    }

    item->itemTcp.count = 0;
    sleep(5);

    return TCP_CLIENT_START;
}

// 定义tcp状态机函数
rbmsTcpMasterTaskFun threadRbmsTcpClientFunc[TCP_CLIENT_END] = {
    rbmsTcpMasterStartCall, rbmsTcpMasterConnected, rbmsTcpMasterConnectClosed};

// 初始化rbms tcp通道
static void initChanRbmsTcp(dev_info_t *infoItem, rbms_tcp_master_t *rbmsItem)
{
    if (infoItem->protocolType == kProto_RbmsPrivate_TCP_Master)
    {
        // 初始化TCP协议相关配置
        rbmsItem->itemTcp.netId = infoItem->devProtocol.tcpRbmsItem.netId;
        rbmsItem->itemTcp.uId = infoItem->devProtocol.tcpRbmsItem.uId;
        strncpy((char *)rbmsItem->itemTcp.ip, (char *)infoItem->devProtocol.tcpRbmsItem.ip,
                MAX_IP_LEN);
        rbmsItem->itemTcp.port = infoItem->devProtocol.tcpRbmsItem.port;
        rbmsItem->itemTcp.timeout = infoItem->devProtocol.tcpRbmsItem.timeout;

        // 设备信息
        rbmsItem->devId = infoItem->devId;
        rbmsItem->devType = infoItem->devType;
    }
}

void *creatNetRbmsThread(void *arg)
{

    if (arg == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "creatNetRbmsThread: arg is NULL");
        return NULL;
    }

    rbms_tcp_master_t *item = arg;

    tcp_client_state_e state = TCP_CLIENT_START;
    while (1)
    {
        state = threadRbmsTcpClientFunc[state](item);
        usleep(100000);
    }
}

// 创建rbms tcp私有协议采集任务
void creatNetRbmsTaskEntry(proto_dev_point_map_t *proto)
{
    if (proto == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "creatNetRbmsTaskEntry: proto is NULL");
        return;
    }

    // 保存全局RBMS数组（用于后续查找映射）
    pthread_mutex_lock(&g_rbms_array_mutex);

    // 先检查是否已经初始化全局队列
    static uint8_t global_queues_inited = 0;
    if (!global_queues_inited)
    {
        // 初始化全局接收队列 - 使用静态缓冲区
        if (kit_queue_init(&recv_queue, recv_queue_buf, RBMS_RECV_QUEUE_ITEM_SIZE,
                           sizeof(bms_queue_msg_t)) != 0)
        {
            KITLOG(LOG_APP_EN, ERROR_EN, "Failed to init recv_queue");
            pthread_mutex_unlock(&g_rbms_array_mutex);
            return;
        }

        // 初始化全局发送队列 - 动态分配
        if (kit_queue_init(&send_queue, NULL, RBMS_CTRL_QUEUE_TOTAL_SIZE,
                           sizeof(bms_queue_msg_t)) != 0)
        {
            KITLOG(LOG_APP_EN, ERROR_EN, "Failed to init send_queue");
            kit_queue_destroy(&recv_queue);
            pthread_mutex_unlock(&g_rbms_array_mutex);
            return;
        }

        // 启动发送队列分发线程
        startSendQueueDequeueThread();

        global_queues_inited = 1;
        KITLOG(LOG_APP_EN, INFO_EN, "Global RBMS queues initialized");
    }

    // 分配RBMS数组
    if (proto->devNum > 0)
    {
        g_rbms_array = calloc(proto->devNum, sizeof(rbms_tcp_master_t));
        if (g_rbms_array == NULL)
        {
            KITLOG(LOG_APP_EN, ERROR_EN, "creatNetRbmsTaskEntry: memory allocation failed");
            pthread_mutex_unlock(&g_rbms_array_mutex);
            return;
        }
        g_rbms_count = proto->devNum;
    }

    pthread_mutex_unlock(&g_rbms_array_mutex);

    // 初始化每个RBMS连接
    for (uint16_t i = 0; i < proto->devNum; i++)
    {
        initChanRbmsTcp(&proto->devPointMapArr[i], &g_rbms_array[i]);

        // 注册数据上报回调函数
        g_rbms_array[i].recvCall = rbmsTcpMasterRecvCall;

        // 初始化互斥锁
        pthread_mutex_init(&g_rbms_array[i].send_queue_mutex, NULL);

        // 初始化每个RBMS的send_queue - 动态分配
        if (kit_queue_init(&g_rbms_array[i].send_queue, NULL, RBMS_CTRL_QUEUE_ITEM_SIZE,
                           sizeof(bms_queue_msg_t)) != 0)
        {
            KITLOG(LOG_APP_EN, ERROR_EN, "Failed to init rbms send_queue for device %d",
                   g_rbms_array[i].devId);
            continue;
        }

        // 创建线程
        if (pthread_create(&g_rbms_array[i].tfd, NULL, creatNetRbmsThread, &g_rbms_array[i]) != 0)
        {
            KITLOG(LOG_APP_EN, ERROR_EN,
                   "creatNetRbmsTaskEntry: failed to create thread for device %d",
                   g_rbms_array[i].devId);
            kit_queue_destroy(&g_rbms_array[i].send_queue);
        }
        else
        {
            KITLOG(LOG_APP_EN, INFO_EN, "RBMS thread created for device %d", g_rbms_array[i].devId);
        }
    }

    KITLOG(LOG_APP_EN, INFO_EN, "creatNetRbmsTaskEntry completed, created %d RBMS threads",
           proto->devNum);
}

/* =================================================== 与上位机通信实现 ============================================ */

// 注册周期性数据上报回调函数
void registerUplinkCycleTimeCallback(uplink_cycle_time_data callback)
{
    if (callback == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "registerUplinkCycleTimeCallback: callback is NULL");
        return;
    }
    g_uplink_ctx.uplink_cycle_callback = callback;
}

void registerUplinkFaultCycleTimeCallback(uplink_cycle_time_data callback)
{
    if (callback == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "registerUplinkFaultCycleTimeCallback: callback is NULL");
        return;
    }
    g_uplink_ctx.uplink_fault_cycle_callback = callback;
}

/**
 * @brief 向上位机发送控制回复（高优先级）
 */
void uplink_ctrl_reply_enqueue(uint8_t *data, int len)
{
    if (data == NULL || len <= 0 || len > RBMS_MAX_DATA_LEN)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Invalid ctrl reply data, len=%d", len);
        return;
    }

    uplink_msg_t msg;
    //msg.msg_type = UPLINK_MSG_TYPE_CTRL_REPLY;
    msg.len = len;
    memcpy(msg.data, data, len);

    if (kit_queue_enqueue(&ctrl_reply_queue, &msg) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "ctrl_reply_queue full, ctrl reply dropped");
    }
}

/**
 * @brief 上位机数据接收回调
 * 处理从上位机接收到的控制命令
 */
// static int uplink_recv_callback(void *arg, int fd)
// {
//    tcp_server_lib_t *p_tcp = (tcp_server_lib_t *)arg;

//    if (p_tcp->recv_len <= 0)
//    {
//       return -1;
//    }

//    // 校验数据
//    if (parseCheckRbmsData(p_tcp->recv_buf, p_tcp->recv_len) == 0)
//    {
//       // 解析目的地址
//       uint8_t dest = p_tcp->recv_buf[6];
//       uint8_t destSub = p_tcp->recv_buf[7];

//       // 如果是发给RBMS的命令，入队到全局send_queue
//       if (dest == RBMS_DEV_TYPE_SIGN && destSub >= 1 && destSub <= g_rbms_count)
//       {
//          bms_queue_msg_t msg;
//          msg.len = p_tcp->recv_len;
//          memcpy(msg.data, p_tcp->recv_buf, p_tcp->recv_len);
//          send_queue_enqueue(&msg);
//          KITLOG(LOG_APP_EN, INFO_EN, "Received cmd from uplink, dest=0x%02X, destSub=%d, fd=%d", dest, destSub, fd);
//       }
//       else if(dest == HMI_DEV_TYPE_SIGN && destSub == HMI_SUB_TYPE_SIGN)
//       {
//          KITLOG(LOG_APP_EN, INFO_EN, "Received invalid dest from uplink: 0x%02X, fd=%d", dest, fd);
//       }
//    }
//    else
//    {
//       KITLOG(LOG_APP_EN, ERROR_EN, "parseCheckRbmsData error for uplink data, fd=%d", fd);
//    }

//    return 0;
// }

/**
 * @brief 短超时接收上位机数据
 * @param p_server TCP服务器指针
 * @return 0-无数据, 1-有数据并处理
 */
static int uplink_recv_short(tcp_server_lib_t *p_server)
{
    struct epoll_event events[MAX_EVENTS];
    int nfds = epoll_wait(p_server->epoll_fd, events, MAX_EVENTS, 10); // 10ms超时

    if (nfds <= 0)
    {
        return 0; // 无数据
    }

    // 处理epoll事件
    time_t current_time = time(NULL);
    for (int i = 0; i < nfds; i++)
    {
        int fd = events[i].data.fd;

        if (fd == p_server->socket_fd)
        {
            // 新连接
            while (p_server->nClient < MAX_CLIENTS)
            {
                struct sockaddr_in client_addr;
                socklen_t addr_len = sizeof(client_addr);
                int client_fd =
                    accept(p_server->socket_fd, (struct sockaddr *)&client_addr, &addr_len);
                if (client_fd == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        break;
                    }
                    KITLOG(LOG_APP_EN, ERROR_EN, "accept failed: errno=%d", errno);
                    break;
                }

                // 设置非阻塞
                int flags = fcntl(client_fd, F_GETFL, 0);
                fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                // 添加到epoll
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                epoll_ctl(p_server->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

                // 添加到客户端列表
                p_server->clients[p_server->nClient].fd = client_fd;
                p_server->clients[p_server->nClient].client = client_addr;
                p_server->clients[p_server->nClient].last_active_time = current_time;
                p_server->nClient++;
            }
        }
        else
        {
            // 客户端数据
            ssize_t recv_len = recv(fd, p_server->recv_buf, MAX_FRAME_LEN, 0);

         if (recv_len < 0)
         {
            if (errno != EWOULDBLOCK && errno != EAGAIN)
            {
               // 关闭连接
               epoll_ctl(p_server->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
               close(fd);
               for (int j = 0; j < p_server->nClient; j++)
               {
                  if (p_server->clients[j].fd == fd)
                  {
                     for (int k = j; k < p_server->nClient - 1; k++)
                     {
                        p_server->clients[k] = p_server->clients[k + 1];
                     }
                     p_server->nClient--;
                     KITLOG(LOG_APP_EN, INFO_EN, "uplink_recv_thread: client fd=%d disconnected, nClient=%d\n", fd, p_server->nClient);
                     break;
                  }
               }
            }
         }
         else if (recv_len > 0)
         {
            p_server->recv_len = recv_len;
            p_server->status = 1;
            // 更新活动时间
            for (int j = 0; j < p_server->nClient; j++)
            {
               if (p_server->clients[j].fd == fd)
               {
                  p_server->clients[j].last_active_time = current_time;
                  break;
               }
            }
            // 调用回调处理数据
            if (p_server->revCall != NULL)
            {
               p_server->revCall(p_server, fd);
            }
            return 1; // 有数据并处理
         }
      }
   }

   // 检查超时连接
   for (int i = 0; i < p_server->nClient; i++)
   {
      if (current_time - p_server->clients[i].last_active_time > CLIENT_TIMEOUT)
      {
         epoll_ctl(p_server->epoll_fd, EPOLL_CTL_DEL, p_server->clients[i].fd, NULL);
         close(p_server->clients[i].fd);
         for (int j = i; j < p_server->nClient - 1; j++)
         {
            p_server->clients[j] = p_server->clients[j + 1];
         }
         p_server->nClient--;
         KITLOG(LOG_APP_EN, INFO_EN, "uplink_recv_thread: client fd=%d disconnected, nClient=%d\n", p_server->clients[i].fd, p_server->nClient);
         i--;
      }
   }

    if (p_server->nClient == 0)
    {
        p_server->status = CONN_OFFLINE;
    }

    return 0; // 无数据
}

static void close_client_connection(tcp_server_lib_t *p_server, int fd)
{
    epoll_ctl(p_server->epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    close(fd);
    
    for (int j = 0; j < p_server->nClient; j++)
    {
        if (p_server->clients[j].fd == fd)
        {
            for (int k = j; k < p_server->nClient - 1; k++)
            {
                p_server->clients[k] = p_server->clients[k + 1];
            }
            p_server->nClient--;
            KITLOG(LOG_APP_EN, INFO_EN, "Client fd=%d removed, nClient=%d", fd, p_server->nClient);
            break;
        }
    }
}

/**
 * @brief 上位机TCP接收线程 - 专门负责接收上位机控制命令
 * 持续监听TCP连接，保持连接活跃
 */
static void *uplink_recv_thread_func(void *arg)
{
    uplink_context_t *ctx = (uplink_context_t *)arg;
    tcp_server_lib_t *p_server = &ctx->server;
    struct epoll_event events[MAX_EVENTS];

    KITLOG(LOG_APP_EN, INFO_EN, "uplink_recv_thread started");

   while (!g_uplink_recv_thread_exit)
   {

      // 持续监听TCP连接，无阻塞或短超时
      int nfds = epoll_wait(p_server->epoll_fd, events, MAX_EVENTS, 100); // 100ms超时

        if (nfds <= 0)
        {
            // 无事件，继续监听
            continue;
        }

        // 处理epoll事件
        time_t current_time = time(NULL);
        for (int i = 0; i < nfds; i++)
        {
            int fd = events[i].data.fd;

         if (fd == p_server->socket_fd)
         {
            // 新连接
            while (p_server->nClient < MAX_CLIENTS)
            {
               struct sockaddr_in client_addr;
               socklen_t addr_len = sizeof(client_addr);
               int client_fd = accept(p_server->socket_fd, (struct sockaddr *)&client_addr, &addr_len);
               if (client_fd == -1)
               {
                  if (errno == EAGAIN || errno == EWOULDBLOCK)
                  {
                     break;
                  }
                  KITLOG(LOG_APP_EN, ERROR_EN, "accept failed: errno=%d", errno);
                  KITPTF(LOG_APP_EN, ERROR_EN, "accept failed: errno=%d", errno);
                  break;
               }

                    // 设置非阻塞
                    int flags = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                    // 添加到epoll
                    struct epoll_event ev;
                    ev.events = EPOLLIN | EPOLLET;
                    ev.data.fd = client_fd;
                    epoll_ctl(p_server->epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);

                    // 添加到客户端列表
                    p_server->clients[p_server->nClient].fd = client_fd;
                    p_server->clients[p_server->nClient].client = client_addr;
                    p_server->clients[p_server->nClient].last_active_time = current_time;
                    p_server->nClient++;

               KITLOG(LOG_APP_EN, INFO_EN, "New uplink client connected, fd=%d, total=%d", client_fd, p_server->nClient);
               KITPTF(LOG_APP_EN, INFO_EN, "New uplink client connected, fd=%d, total=%d", client_fd, p_server->nClient);
            }
         }
         else
         {
            // 客户端数据
            //printf("uplink_recv_thread: receiving data from fd=%d\n", fd);
            ssize_t recv_len = recv(fd, p_server->recv_buf, MAX_FRAME_LEN, 0);
            //printf("uplink_recv_thread: received %d bytes from fd=%d\n", recv_len, fd);

            if (recv_len > 0)
            {
               p_server->recv_len = recv_len;
               p_server->status = 1;
               // 更新活动时间
               for (int j = 0; j < p_server->nClient; j++)
               {
                  if (p_server->clients[j].fd == fd)
                  {
                     p_server->clients[j].last_active_time = current_time;
                     break;
                  }
               }
               // 调用回调处理数据
               if (p_server->revCall != NULL)
               {
                  // 校验数据
                  if (parseCheckRbmsData(p_server->recv_buf, p_server->recv_len) == 0)
                  {
                     p_server->revCall(p_server, fd);
                  }
               }
            }
            else if (recv_len == 0)
            {
               // recv_len == 0, 客户端正常关闭连接
               // 对方正常关闭连接
               KITLOG(LOG_APP_EN, INFO_EN, "Client fd=%d closed connection", fd);
               close_client_connection(p_server, fd);
            }
            else
            {
               // 错误处理
               if (errno == EAGAIN || errno == EWOULDBLOCK)
               {
                  // 没有数据可读，这不是错误，继续
                  KITLOG(LOG_APP_EN, INFO_EN, "No data available for fd=%d, EAGAIN", fd);
                  continue;
               }
               else if (errno == ECONNRESET || errno == EPIPE)
               {
                  // 连接被重置
                  KITLOG(LOG_APP_EN, INFO_EN, "Connection reset for fd=%d", fd);
                  close_client_connection(p_server, fd);
               }
               else
               {
                  // 其他错误
                  KITLOG(LOG_APP_EN, ERROR_EN, "recv error for fd=%d, errno=%d", fd, errno);
                  close_client_connection(p_server, fd);
               }
            }
         }
      }

      #if 0 //先不使能超时检查,确认上位机是否有心跳后在考虑是否需要超时检查
      // 检查超时连接
      for (int i = 0; i < p_server->nClient; i++)
      {
         if (current_time - p_server->clients[i].last_active_time > CLIENT_TIMEOUT )
         {
            KITLOG(LOG_APP_EN, INFO_EN, "Client fd=%d timeout, closing", p_server->clients[i].fd);
            epoll_ctl(p_server->epoll_fd, EPOLL_CTL_DEL, p_server->clients[i].fd, NULL);
            close(p_server->clients[i].fd);
            for (int j = i; j < p_server->nClient - 1; j++)
            {
               p_server->clients[j] = p_server->clients[j + 1];
            }
            p_server->nClient--;
            KITLOG(LOG_APP_EN, INFO_EN, "uplink_recv_thread: client fd=%d disconnected, nClient=%d\n", p_server->clients[i].fd, p_server->nClient);
            KITPTF(LOG_APP_EN, INFO_EN, "uplink_recv_thread: client fd=%d disconnected, nClient=%d\n", p_server->clients[i].fd, p_server->nClient);
            i--;
         }
      }
      #endif
      

        if (p_server->nClient == 0)
        {
            p_server->status = CONN_OFFLINE;
        }
    }

    KITLOG(LOG_APP_EN, INFO_EN, "uplink_recv_thread exiting");
    return NULL;
}

/**
 * @brief 上位机TCP发送线程 - 专门负责发送数据到上位机
 * 优先级: 控制回复 > 周期性数据 > BBMS信息
 */
static void *uplink_send_thread_func(void *arg)
{

   uplink_context_t *ctx = (uplink_context_t *)arg;
   bms_queue_msg_t msg = {0};
   tcp_server_lib_t *p_server = &ctx->server;
   static uint8_t fault_uplink_tick = 0;

    KITLOG(LOG_APP_EN, INFO_EN, "uplink_send_thread started");

   while (!g_uplink_send_thread_exit)
   {
      //printf("uplink_send_thread: nClient=%d\n", p_server->nClient);
      // 检查是否有客户端连接
      if (p_server->nClient == 0)
      {
         usleep(100000); // 100ms，无客户端时休眠
         //printf("uplink_send_thread: no client connected, sleep 100ms\n");
         continue;
      }
      else
      {
         printf("uplink_send_thread: nClient=%d\n", p_server->nClient);
      }

      
      // 优先级1: 发送控制回复（最高优先级）
      while (!kit_queue_is_empty(&ctrl_reply_queue))
      {
         if (kit_queue_dequeue(&ctrl_reply_queue, &msg) == 0)
         {
            p_server->send_len = msg.len;
            memcpy(p_server->send_buf, msg.data, msg.len);
            if (drvTcpServerSend(p_server) < 0)
            {
               KITLOG(LOG_APP_EN, ERROR_EN, "Failed to send ctrl reply to uplink");
            }
         }
         else
         {
            printf("uplink_send_thread: ctrl_reply_queue is empty, break\n");
            break;
         }
         usleep(10000); // 10ms
      }

      // 优先级2: 发送周期性数据
      if (kit_queue_dequeue(&recv_queue, &msg) == 0)
      {
         p_server->send_len = msg.len;
         memcpy(p_server->send_buf, msg.data, msg.len);
         if (drvTcpServerSend(p_server) < 0)
         {
            KITLOG(LOG_APP_EN, ERROR_EN, "Failed to send data to uplink");
         }
      }
      

      // 优先级3: 发送BBMS信息
      while (!kit_queue_is_empty(&bbms_info_queue))
      {
         if (kit_queue_dequeue(&bbms_info_queue, &msg) == 0)
         {
            p_server->send_len = msg.len;
            memcpy(p_server->send_buf, msg.data, msg.len);
            if (drvTcpServerSend(p_server) < 0)
            {
               KITLOG(LOG_APP_EN, ERROR_EN, "Failed to send bbms info to uplink");
               printf("uplink_send_thread: bbms_info_queue is empty, break\n");
            }
         }
         else
         {
            printf("uplink_send_thread: bbms_info_queue is empty, break\n");
            break;
         }
         usleep(10000); // 10ms
      }
      

      // 调用周期性数据上报回调函数，这里入队列bbms_info_queue，等待线程发送处理
      if (ctx->uplink_cycle_callback != NULL)
      {
         ctx->uplink_cycle_callback(0);
      }

      fault_uplink_tick++;
      // 故障类周期上报（Matrix 1000ms），入队列 bbms_info_queue，等待线程发送处理
      if ((fault_uplink_tick >= 10U) && (ctx->uplink_fault_cycle_callback != NULL))
      {
         fault_uplink_tick = 0U;
         ctx->uplink_fault_cycle_callback(0);
      }

      // 短暂休眠，避免CPU空转
      usleep(100000); // 100ms
   }

    KITLOG(LOG_APP_EN, INFO_EN, "uplink_send_thread exiting");
    return NULL;
}

//注册接收回调函数drvInitServerCall(p_server, uplink_recv_callback);
void registerUplinkRecvCallback(respondCall recv_callback)
{
    if (recv_callback == NULL)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "registerUplinkRecvCallback: recv_callback is NULL");
        return;
    }
    g_uplink_ctx.server.revCall = recv_callback;
}

/**
 * @brief 启动与上位机通信的任务
 * @param ip 监听IP地址（NULL表示监听所有接口）
 * @param port 监听端口
 */
void startUplinkTcpTask(const char *ip, uint16_t port)
{
    tcp_server_lib_t *p_server = &g_uplink_ctx.server;

    if (port == 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Invalid uplink parameters");
        return;
    }

   // 初始化控制回复队列
   if (kit_queue_init(&ctrl_reply_queue, ctrl_reply_queue_buf, RBMS_CTRL_QUEUE_ITEM_SIZE, sizeof(bms_queue_msg_t)) != 0)
   {
      KITLOG(LOG_APP_EN, ERROR_EN, "Failed to init ctrl_reply_queue");
      return;
   }

   // 初始化BBMS信息队列
   if (kit_queue_init(&bbms_info_queue, bbms_info_queue_buf, RBMS_RECV_QUEUE_ITEM_SIZE, sizeof(bms_queue_msg_t)) != 0)
   {
      KITLOG(LOG_APP_EN, ERROR_EN, "Failed to init bbms_info_queue");
      return;
   }

    // 配置TCP服务器
    if (ip != NULL)
    {
        strncpy(p_server->ip, ip, MAX_SERVER_IP_LEN);
    }
    else
    {
        p_server->ip[0] = '\0'; // 监听所有接口
    }
    p_server->port = port;
    p_server->timeout = 5000; // 5秒超时

    // 初始化接收回调
    //drvInitServerCall(p_server, uplink_recv_callback);

    // 打开TCP服务器
    if (drvTcpServerOpen(p_server) != 0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Failed to open uplink TCP server");
        kit_queue_destroy(&ctrl_reply_queue);
        return;
    }

    // 启动接收线程 - 专门负责接收上位机控制命令
    g_uplink_recv_thread_exit = 0;
    if (pthread_create(&g_uplink_ctx.recv_thread, NULL, uplink_recv_thread_func, &g_uplink_ctx) !=
        0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Failed to create uplink_recv_thread");
        kit_queue_destroy(&ctrl_reply_queue);
        kit_queue_destroy(&bbms_info_queue);
        drvTcpServerClose(p_server);
        return;
    }

    // 启动发送线程 - 专门负责发送数据到上位机
    g_uplink_send_thread_exit = 0;
    if (pthread_create(&g_uplink_ctx.send_thread, NULL, uplink_send_thread_func, &g_uplink_ctx) !=
        0)
    {
        KITLOG(LOG_APP_EN, ERROR_EN, "Failed to create uplink_send_thread");
        g_uplink_recv_thread_exit = 1;
        pthread_join(g_uplink_ctx.recv_thread, NULL);
        kit_queue_destroy(&ctrl_reply_queue);
        kit_queue_destroy(&bbms_info_queue);
        drvTcpServerClose(p_server);
        return;
    }

    KITLOG(LOG_APP_EN, INFO_EN, "Uplink TCP server started: %s:%d (recv_thread+send_thread)",
           ip ? ip : "0.0.0.0", port);
}