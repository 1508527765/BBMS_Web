/*****************************************************************************
 * @copyright       1997-2010,AO DI LTD.
 * @file            kit_data.h
 * @brief           所有的数据类型定义以及
 * @author          Gary
 * @date            2024-12-03
 * @remark
 *****************************************************************************/
#ifndef KIT_DATA_H
#define KIT_DATA_H
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_CODE_BUF_LEN 50         // 编号最大长度
#define MAX_NAME_BUF_LEN 100        // 名称最大长度
#define MAX_ADDR_BUF_LEN 200        // 地址最大长度
#define MAX_CONFIG_CONTENT_LEN 1200 // 协议配置内容最大长度
#define MAX_DEV_POINT_NUM_LEN 3000  // 同一设备下点位数组的最大长度
#define MAX_POINT_ID_LEN 100        // 点位Id最大长度
#define MAX_POINT_NAME_LEN 100      // 点位名称最大长度
#define MAX_POINT_VALUE_LEN 255     // 点位初始化值最大长度
#define MAX_POINT_ADDR_LEN 255      // 点位地址最大长度
#define MAX_DEV_POINT_MAP_LEND 500  // 协议整合设备数组
#define MAX_IP_LEN 32               // ip地址最大长度
#define MAX_JSON_STR_LEN 10240      // JSON的Str最大长度
#define MAX_EXE_COUNT 86400         // 存储间隔最大个数
#define PATH_MAX_SIZE 128           // 地址最大长度
#define MAX_ADDR_LEN 255            // 串口地址最大长度

#define MAX_CLUSTER_NUM  20

// 模板类型类型枚举
typedef enum
{
    kTemplate_Type_Start = 0,                     // 起始值
    kTemplate_Type_Device = kTemplate_Type_Start, // 设备配置
    kTemplate_Type_North,                         // 北向配置
    kTemplate_Type_End                            // 结束值
} template_type_e;                                // 数据传输类型

// 控制器设备配置类型枚举
typedef enum
{
    kDev_Config_Start = 0,                // 起始值
    kDev_Config_Uart = kDev_Config_Start, // 控制器的串口配置
    kDev_Config_Net,                      // 控制器的网口配置
    kDev_Config_DI,                       // 控制器的DI配置
    kDev_Config_DO,                       // 控制器的DO配置
    kDev_Config_Wifi,                     // 控制器的Wifi配置
    kDev_Config_4G,                       // 控制器的4G配置
    kDev_Config_Rtc,                      // 控制器的对时配置
    kDev_Config_Can,                      // 控制器的can配置
    kDev_Config_End                       // 结束值
} con_dev_config_type_e;                  // 数据传输类型

// 点位配置的落库间隔
typedef enum
{
    kStorage_Interval_1s = 1000,   // 1秒
    kStorage_Interval_2s = 2000,   // 2秒
    kStorage_Interval_3s = 3000,   // 3秒
    kStorage_Interval_4s = 4000,   // 4秒
    kStorage_Interval_5s = 5000,   // 5秒
    kStorage_Interval_6s = 6000,   // 6秒
    kStorage_Interval_7s = 7000,   // 7秒
    kStorage_Interval_8s = 8000,   // 8秒
    kStorage_Interval_9s = 9000,   // 9秒
    kStorage_Interval_10s = 10000, // 10秒
    kStorage_Interval_15s = 15000, // 15秒
    kStorage_Interval_20s = 20000, // 20秒
    kStorage_Interval_30s = 30000, // 30秒
} storage_interval_type_e;

// 点位配置的数据类型枚举
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

// 点位配置的分组枚举
typedef enum
{
    kPoint_Group_Type_Start = 0,         // 开始值
    kPoint_YX = kPoint_Group_Type_Start, // 遥信
    kPoint_YC,                           // 遥测
    kPoint_YK,                           // 遥控
    kPoint_YT,                           // 遥调
    kPoint_LS,                           // 设置
    kPoint_YM,                           // 遥脉
    kPoint_SOE,                          // 故障
    kPoint_EVENT,                        // 事件
    kPoint_Group_Type_End                // 结束值
} point_group_type_e;                    // 数据传输类型

// 点位配置的读写特性枚举
typedef enum
{
    kPoint_Atrribute_Start = 0,                // 开始值
    kPoint_Read_Only = kPoint_Atrribute_Start, // 只读
    kPoint_Write_Only,                         // 只写
    kPoint_Read_Write,                         // 读写
    kPoint_Static,                             // 静态
    kPoint_Atrribute_End                       // 结束值
} point_attribute_e;                           // 数据传输类型

// 大小端类型枚举
typedef enum
{
    E_AB,       // 大端模式
    E_BA,       // 小端模式
    E_ABCD,     // 大端模式
    E_CDAB,     // 小端字、大端字节
    E_BADC,     // 大端字、小端字节
    E_DCBA,     // 小端模式
    E_ABCDEFGH, // 大端模式
    E_GHEFCDAB, // 小端字、大端字节
    E_BADCFEHG, // 大端字、小端字节
    E_HGFEDCBA  // 小端模式
} data_endian_e;

typedef enum
{
    kSign_Share_Start = 0,
    kSign_ShMem = kSign_Share_Start,
    kSign_Redis,
    kSign_MQ, // 消息队列
    kSign_Share_End
} sign_share_type_e; // 实时库类型

typedef enum
{
    E_STATUS_OK = 0,
    E_STATUS_ERROR = 1,
} status_e; // 返回状态类型

// 主站通讯协议类型枚举
typedef enum
{
    kProto_Master_Start = 0,                         // 起始值
    kProto_Master_Info_Master = kProto_Master_Start, // 本机信息协议
    kProto_BbmsPrivate_Master,                       // BBMS私有协议            // 协议配置为空，不需要配置，配置了也不生效
    kProto_ModbusTCP_Master,                         // Modbus TCP 标准主站
    kProto_ModbusRTU_Master,                         // Modbus RTU 主站
    kProto_RbmsPrivate_TCP_Master,                   // RBMS TCP私有协议
    kProto_RbmsPrivate_Can_Master,                   // RBMS CAN私有协议
    kProto_ModbusTCPMulti_Master,                    // ModbusMultiId TCP 协议
    kProto_TMSPrivateCan_Master,                    // TMS CAN私有协议
    kProto_Master_End                                // 结束值
} protocol_type_master_e;

// 从站通讯协议类型枚举
typedef enum
{
    kProto_Slave_Start = 0, // 起始值
    kProto_ModbusTCP_Slave, // Modbus TCP 从站
    kProto_ModbusRTU_Slave, // Modbus RTU 从站
    kProto_Slave_End        // 结束值
} protocol_type_slave_e;

// 上报下发点位配置类型枚举
typedef enum
{
    kUp_Dis_Cfg_Type_Start = 0,                   // 起始值
    kUp_Dis_Cfg_Type_Up = kUp_Dis_Cfg_Type_Start, // 上报点位配置
    kUp_Dis_Cfg_Type_Dis_Point,                   // 下发点位配置
    kUp_Dis_Cfg_Type_End
} up_dis_config_type_e;

// 拓扑节点类型枚举
typedef enum
{
    kMenu_Tree_Start = 0,                  // 起始值
    kMenu_Tree_Virtual = kMenu_Tree_Start, // 虚拟节点
    kMenu_Tree_Device,                     // 设备节点
    kMenu_Tree_End
} menu_tree_e;

// Modbus功能码枚举定义
typedef enum
{
    kModbus_Func_Start = 0,                 // 开始值
    kModbus_F01_Read_Coil = 0x01,           // 读取线圈寄存器
    kModbus_F02_Read_Discrete_Input = 0x02, // 读取离散输入寄存器
    kModbus_F03_Read_Hold_Reg = 0x03,       // 读取保持寄存器
    kModbus_F04_Read_Input_Reg = 0x04,      // 读取输入寄存器
    kModbus_F05_Write_S_Coil = 0x05,        // 写入单个线圈寄存器
    kModbus_F06_Write_S_Hold_Reg = 0x06,    // 写单个保持寄存器
    kModbus_F0F_Write_M_Coil = 0x0F,        // 写多个线圈寄存器
    kModbus_F10_Write_M_Hold_Reg = 0x10,    // 写多个寄存器
    kModbus_Func_end                        // 结束值
} modbus_func_e;                            // 数据传输类型

// 接入控制器的所有设备类型
typedef enum
{
    kDev_Type_Start = 0,             // 起始值
    kDev_Type_Con = kDev_Type_Start, // 本机信息
    kDev_Type_BBMS,                  // BMS堆控
    kDev_Type_RBMS,                  // BMS簇控制器
    kDev_Type_PCS,                   // PCS 类型
    kDev_Type_Transfer,              // 变压器
    kDev_Type_Lqd,                   // 空调/液冷机组
    kDev_Type_Meter,                 // 电表设备
    kDev_Type_UPS,                   // UPS设备
    kDev_Type_FireDevice,            // 消防设备
    kDev_Type_dehumidifier,          // 除湿机
    kDev_Type_TempSensor,            // 温度传感器
    kDev_Type_HumiditySensor,        // 湿度传感器
    kDev_Type_WaterSensor,           // 水浸传感器
    kDev_Type_SmokeSensor,           // 烟雾传感器
    kDev_Type_OtherSensor1,          // 其他传感器1
    kDev_Type_OtherSensor2,          // 其他传感器2
    kDev_Type_OtherSensor3,          // 其他传感器3

    // 预留设备类型
    kDev_Type_Resv17,
    kDev_Type_Resv18,
    kDev_Type_Resv19,
    kDev_Type_Resv20,
    kDev_Type_Resv21,
    kDev_Type_Resv22,
    kDev_Type_Resv23,
    kDev_Type_Resv24,
    kDev_Type_Resv25,
    kDev_Type_Resv26,
    kDev_Type_Resv27,
    kDev_Type_Resv28,
    kDev_Type_Resv29,
    kDev_Type_End
} dev_type_e;

// 控制器总的工作模式
typedef enum
{
    E_TACTIC_MODE_START, // 无
    E_TACTIC_MODE_DEBUG, // 调试模式
    E_TACTIC_MODE_END
} work_mode_e;

// DI  高电平(有效)1 低电平 0
typedef enum
{
    kDiFuc_Start = 0,                 // DI信号功能枚举开始
    kDiFuc_WaterLowOn = kDiFuc_Start, // R 水浸传感器低（常开)
    kDiFuc_WaterLowOff,               // R 水浸传感器低（常闭）
    kDiFuc_WaterHighOn,               // R 水浸传感器高（常开)
    kDiFuc_WaterHighOff,              // R 水浸传感器高（常闭）
    kDiFuc_EmergStopOn,               // R 急停（常开)
    kDiFuc_EmergStopOff,              // R 急停（常闭）
    kDiFuc_SmokeDetecOn,              // R 烟感（常开)
    kDiFuc_SmokeDetecOff,             // R 烟感（常闭）
    kDiFuc_TempSensorOn,              // R 温感（常开)
    kDiFuc_TempSensorOff,             // R 温感（常闭）
    kDiFuc_FireFeedbackOn,            // R 消防触发反馈（常开)
    kDiFuc_FireFeedbackOff,           // R 消防触发反馈（常闭）
    kDiFuc_AccessSensorOn,            // R 门禁传感器（常开)
    kDiFuc_AccessSensorOff,           // R 门禁传感器（常闭）
    kDiFuc_CircuitFeedOn,             // R 交流断路器反馈（常开)
    kDiFuc_CircuitFeedOff,            // R 交流断路器反馈（常闭）
    kDiFuc_AerosolGetOn,              // 气溶胶触发反馈常闭
    kDiFuc_FireAerosolGetOff,         // 气溶胶触发反馈常开
    kDiFuc_SurgeGetOn,                // 浪涌辅助触点常闭
    kDiFuc_SurgeGetOff,               // 浪涌辅助触点常开
    kDiFuc_FireFaultGetOn,            // 消防故障常闭
    kDiFuc_FireFaultGetOff,           // 消防故障常开
    kDiFuc_End                        // 结束
} diFuc_e;

// DO 高电平(有效)1 低电平 0
typedef enum
{
    kDoFuc_Start = 0,                  // DO信号功能枚举起始
    kDoFuc_NormalLight = kDoFuc_Start, // R/W 灯 常亮
    kDoFuc_NormalLightBli,             // R/W 灯 闪烁
    kDoFuc_FaultLight,                 // R/W 故障灯 常亮
    kDoFuc_FaultLightBli,              // R/W 故障灯 闪烁
    kDoFuc_CircuitTripping,            // R/W 交流断路器分励脱扣
    kDoFuc_CabinetFan,                 // R/W 机柜风扇
    kDoFuc_FireConfirm,                // R/W 消防触发确认
    kDoFuc_FireTrigger,                // R/W 消防触发 DO5 DO6同时置高电平才真的喷洒
    kDoFuc_PutHigh,                    // R/W 输出高电平
    kDoFuc_PutLow,                     // R/W 输出低电平
    kDoFuc_End,                        // 结束
} doFuc_e;

// 指示类型枚举
typedef enum
{
    kIndicate_Type_Start = 0, // 起始值
    kIndicate_Light_Close,    // 关闭
    kIndicate_Light_Open,     // 亮起
    kIndicate_Light_Flicker,  // 闪烁
    kIndicate_Type_End
} indicate_type_e;

// dido类型
typedef enum
{
    kDi_Do_Type_Start = 0,   // 起始值
    kDi = kDi_Do_Type_Start, // DI
    kDo,                     // DO
    kDi_Do_type_End
} di_do_type_e;

// 北向配置类型
typedef enum
{
    kNorth_Config_Type_Start = 0,                     // 起始值
    kNorth_Config_Default = kNorth_Config_Type_Start, // 默认全部上传点位
    kNorth_Config_Up,                                 // 配置上传
    kNorth_Config_Type_End
} north_config_type_e;

// 高级设置类型枚举
typedef enum
{
    kAdvanced_Setting_Type_Start = 0,         // 起始值
    kCon_Mode = kAdvanced_Setting_Type_Start, // 控制器工作模式
    kAdvanced_Setting_Type_End                // 结束值
} advanced_setting_type_e;

// 波特率
typedef enum
{
    kBaud_Start = 0,
    kBaud_115200 = kBaud_Start,
    kBaud_57600,
    kBaud_38400,
    kBaud_19200,
    kBaud_9600,
    kBaud_4800,
    kBaud_2400,
    kBaud_1800,
    kBaud_1200,
    kBaud_600,
    kBaud_300,
    kBaud_200,
    kBaud_150,
    kBaud_End
} baud_e;

// 校验位
typedef enum
{
    kParity_Start = 0,
    kParity_None = kParity_Start,
    kParity_Odd,
    kParity_Even,
    kParity_Mark,
    kParity_Space,
    kParity_End
} parity_e;

// 停止位
typedef enum
{
    kStop_Start = 0,
    kStop_1 = kStop_Start,
    kStop_2,
    kStop_End
} stop_e;

// 数据位
typedef enum
{
    kData_Start = 0,
    kData_5 = kData_Start,
    kData_6,
    kData_7,
    kData_8,
    kData_End
} data_e;

// 流控制
typedef enum
{
    kFlow_Start = 0,
    kFlow_Disable = kFlow_Start,
    kFlow_Hardware_Enable,
    kFlow_Software_Enable,
    kFlow_End
} flow_e;

// 温控模式
typedef enum
{
    kTemperature_ctrl_Start = 0,                    // 无
    kTemperature_ctrl_Ac = kTemperature_ctrl_Start, // 风冷模式（手动）
    kTemperature_ctrl_Lc,                           // 液冷模式（手动）
    kTemperature_auto_Ac,                           // 风冷模式（自动）
    kTemperature_auto_Lc,                           // 液冷模式（自动）
    kTemperature_ctrl_end
} temperature_ctrl_mod_e;

// 串口驱动结构体
typedef struct uart_lib
{
    uint8_t uartId;                // COM口对应序号
    uint8_t address[MAX_ADDR_LEN]; // 通道驱动地址
    uint16_t uId;                  // 设备序号
    data_e data;                   // 数据位
    stop_e stop;                   // 停止位
    baud_e baud;                   // 波特率
    parity_e parity;               // 校验位
    flow_e flow;                   // 流控制
    uint16_t timeout;              // 超时时间(milisecond)
} kit_uart_lib_t;

// tcp client驱动结构体
typedef struct
{
    uint8_t netId;       // NET口对应序号
    uint16_t uId;        // 设备序号
    char ip[MAX_IP_LEN]; // ip地址
    uint32_t port;       // 端口号
    uint16_t timeout;    // 超时时间(milisecond)
} kit_tcp_client_lib_t;

// can驱动结构体
typedef struct
{
    uint8_t canId;    // can口对应序号
    uint16_t uId;     // 设备序号
    uint32_t bitrate; // 波特率
    uint16_t timeout;
} kit_can_client_lib_t;

// 南向驱动协议的联合体
typedef union
{
    kit_uart_lib_t uartLib;            // uart串口协议
    kit_tcp_client_lib_t tcpClientLib; // tcp client协议
    kit_can_client_lib_t canRbmsItem;  // RBMS CAN私有协议
    kit_tcp_client_lib_t tcpRbmsItem;  // RBMS TCP私有协议
    kit_can_client_lib_t canItem;      // can2.0协议
} dev_protocol_u;

// 电站信息结构体
typedef struct
{
    uint8_t name[MAX_NAME_BUF_LEN];     // 电站名称
    uint8_t address[MAX_ADDR_BUF_LEN];  // 电站地址
    float_t lng;                        // 经度
    float_t lat;                        // 纬度
    uint8_t timeZone[MAX_CODE_BUF_LEN]; // 时区
} station_t;

// 控制器设备口配置结构体
typedef struct
{
    uint32_t dbId;                           // 配置数据库Id
    con_dev_config_type_e type;              // 配置类型
    uint8_t content[MAX_CONFIG_CONTENT_LEN]; // 配置内容
} con_dev_config_t;

// 控制器设备的网口配置结构体
typedef struct
{
    uint8_t netId;                 // 网口口对应序号
    uint8_t isDHCP;                // 是否动态分配
    uint8_t ip[MAX_IP_LEN];        // ip地址
    uint8_t mask[MAX_IP_LEN];      // 子网掩码
    uint8_t gateway[MAX_IP_LEN];   // 网关地址
    uint8_t address[MAX_ADDR_LEN]; // 通道驱动地址
} con_dev_eth_config_t;

// 控制器设备的网口配置结构体
typedef struct
{
    uint8_t canId; // 网口口对应序号
    int bitrate;   // 波特率
} con_dev_can_config_t;

// 控制器设备的DI配置结构体
typedef struct
{
    uint8_t diId;                  // DI口对应序号
    uint8_t validBit;              // 有效位
    uint8_t address[MAX_ADDR_LEN]; // 通道驱动地址
} con_dev_di_config_t;

// 控制器设备的DO配置结构体
typedef struct
{
    uint8_t doId;                  // DO口对应序号
    uint8_t validBit;              // 有效位
    uint8_t address[MAX_ADDR_LEN]; // 通道驱动地址
} con_dev_do_config_t;

// 控制器设备的系统时间配置结构体
typedef struct
{
    uint8_t isNtp;                 // 是否ntp自动对时
    uint16_t port;                 // 端口号
    uint8_t address[MAX_ADDR_LEN]; // 对时ntp地址
    uint8_t manualTime[22];        // "YYYY/DD/MM HH:MM:SS"
} con_dev_rtc_config_t;

// 点位配置的结构体
typedef struct
{
    uint32_t pointDbId;                      // 点位数据库Id
    uint32_t pointId;                        // 点ID
    storage_interval_type_e storageInterval; // 存储间隔
    uint32_t devDbId;                        // 设备数据库Id
    uint16_t devId;                          // 设备编号
    uint16_t regAddr;                        // 寄存器地址
    modbus_func_e func;                      // 功能码
    point_group_type_e groupType;            // 分组
    dev_type_e devType;                      // 设备类型
    data_type_e dataType;                    // 数据类型
    point_attribute_e attribute;             // 属性枚举
    data_endian_e endian;                    // 字节序
    uint8_t bitPos;                          // 位位置
    uint8_t bitNum;                          // 位数量
    uint8_t isPersisted;                     // 是否持久化（0否，1是）
    float_t precision;                       // 精度
    double ratio;                            // 比例，使用 float 以表示小数
    float_t offset;                          // 偏移量，使用 float 以表示小数
    float_t mutateBound;                     // 变更边界
    double_t value;                          // 采集值
    float_t minValue;                        // 最小值
    float_t maxValue;                        // 最大值
    float_t defaultValue;                    // 默认值
    uint8_t pointName[MAX_POINT_NAME_LEN];   // 点位名称
} point_t;

// 设备和点位结构体
typedef struct
{
    protocol_type_master_e protocolType; // 协议类型
    dev_type_e devType;                  // 设备类型
    dev_protocol_u devProtocol;          // 设备协议定义
    uint32_t devDbId;                    // 设备数据库Id
    uint16_t templateId;                 // 模板Id
    uint16_t devId;                      // 设备编号
    uint16_t pointNum;                   // 点位数量
    uint16_t sortOrder;                  // 序号
    point_t *pointArr;                   // 点位数组
    uint8_t devName[MAX_NAME_BUF_LEN];   // 设备名称
} dev_info_t;

// 协议和设备数组关联结构体
typedef struct
{
    // protocol_type_master_e protocolType; // 协议类型
    uint16_t devNum;            // 当前协议下设备数量
    dev_info_t *devPointMapArr; // 协议整合设备数组
} proto_dev_point_map_t;

typedef struct
{
    uint8_t uartCount;
    proto_dev_point_map_t *uartProtoItem;
} uart_chan_proto_map_t;

typedef struct
{
    uint8_t taskCount;
    proto_dev_point_map_t *ProtoItem;
} net_chan_proto_map_t;

// 实时数据定时存储的结构体
typedef struct
{
    dev_type_e devType;                      // 设备类型
    storage_interval_type_e storageInterval; // 存储间隔
    uint32_t devDbId;                        // 设备数据库Id
    uint32_t pointDbId;                      // 点位数据库Id
    uint16_t pointId;
    uint16_t devId;
    float_t timedValue; // 值
} timed_data_storage_t;

// 时序数据配置的结构体
typedef struct
{
    dev_type_e devType;                      // 设备类型
    storage_interval_type_e storageInterval; // 存储间隔
    uint8_t isPersisted;                     // 是否持久化（0否，1是）
    uint32_t devDbId;                        // 设备数据库Id
    uint16_t devId;                          // 设备编号
    uint32_t pointDbId;                      // 点位数据库Id
    uint32_t pointId;                        // 点位编号
    uint8_t devName[MAX_NAME_BUF_LEN];       // 设备名称
    uint8_t pointName[MAX_POINT_NAME_LEN];   // 点位名称
} timed_data_config_t;

// 故障点位定时存储的结构体
typedef struct
{
    dev_type_e devType;  // 设备类型
    uint32_t devDbId;    // 设备数据库Id
    uint16_t devId;      // 设备编号
    uint32_t pointDbId;  // 点位数据库Id
    uint32_t pointId;    // 点位编号
    float_t faultsValue; // 故障值
} faults_data_storage_t;

// 故障点位配置的结构体
typedef struct
{
    dev_type_e devType;                    // 设备类型
    uint32_t devDbId;                      // 设备数据库Id
    uint16_t devId;                        // 设备编号
    uint32_t pointDbId;                    // 点位数据库Id
    uint32_t pointId;                      // 点位编号
    float faultsValue;                     // 故障值
    uint8_t devName[MAX_NAME_BUF_LEN];     // 设备名称
    uint8_t pointName[MAX_POINT_NAME_LEN]; // 点位名称
} faults_data_config_t;

// 功率数据的结构体
typedef struct
{
    dev_type_e devType; // 设备类型
    uint16_t devId;     // 设备编号
    uint32_t pointId;   // 点ID
    float_t value;      // 值
} power_data_storage_t;

// DIDO逻辑设置的结构体
typedef struct
{
    uint8_t dIDOseq;       // DIDO序列号
    uint8_t cabinetCode;   // 柜子编号 0-9 最多十个柜子
    di_do_type_e dIDOType; // DIDO类型
    diFuc_e strategySeqDi; // DI策略序号
    doFuc_e strategySeqDo; // DO策略序号
} di_do_logic_set_t;

#define MAX_UPGRADE_VERSION_LEN 10

// 控制器的关于
typedef struct
{
    uint64_t dbId;                               // 数据库Id
    uint8_t webVersion[MAX_UPGRADE_VERSION_LEN]; // 控制器的Web管理端版本号
    uint8_t cVersion[MAX_UPGRADE_VERSION_LEN];   // 控制器核心控制端版本号
    uint8_t sn[MAX_CODE_BUF_LEN];                // 控制器的SN
    uint8_t firmware[MAX_NAME_BUF_LEN];          // 固件名称
    uint8_t os[MAX_NAME_BUF_LEN];                // 操作系统
} con_about_t;

#define MAX_KEY_LEN 40
#define MAX_VALUE_LEN 80
// 高级设置的结构体
typedef struct
{
    uint8_t key[MAX_KEY_LEN];     // 设置项
    uint8_t value[MAX_VALUE_LEN]; // 设置值
} advanced_setting_t;

/* 错误码定义 */
typedef enum
{
    SUCCESS = 0,                        // 成功
    ERR_FRAME_HEADER_ASSIGN_FAILED = 1, // 帧头配置失败
    ERR_DSP_NOT_INIT,                   // PCS未处于初始化状态
    ERR_HEX_NOT_FOUND,                  // HEX文件未找到
    ERR_HEX_TO_BIN,                     // HEX转换为二进制失败
    ERR_1807_CMD_FAILED,                // 1807命令失败
    ERR_1A06_RESP_FAILED,               // 1A06响应失败
    ERR_UPGRADE_CONDITION_TIMEOUT,      // 升级条件检查超时
    ERR_ENTER_BOOT_MODE_TIMEOUT,        // 进入引导模式超时
    ERR_FLASH_ERASE_TIMEOUT,            // 擦除Flash超时
    ERR_START_TRANSFER_TIMEOUT,         // 开始传输数据超时
    ERR_TRANSFER_DATA_TIMEOUT,          // 传输数据超时
    ERR_END_TRANSFER_TIMEOUT,           // 结束传输数据超时
    ERR_VERIFY_CRC_TIMEOUT,             // 文件校验超时
    ERR_REBOOT_TIMEOUT,                 // 重启超时
    ERR_CAN_SEND,                       // 发送CAN帧失败
    ERR_CAN_RECV,                       // 接收CAN帧失败
    ERR_MEMORY,                         // 内存分配失败
} UpgradeError;

typedef enum
{
    eMode_SocBalance, // soc均衡分配模式
    eMode_AvgPower    // 平均功率分配模式
} pcs_dist_mode_t;    // 功率分配模式（soc均衡，平均分配等）

typedef enum
{
    eWay_Precent,   // 按照比例方式
    eWay_DataValue, // 按照数值方式
} pcs_dist_way_t;   //

typedef struct
{
    /*储能控制目标部分*/
    float_t ess_active_power;   // 储能有功功率
    float_t ess_reactive_power; // 储能无功功率
    uint16_t ess_switch;        // 储能开关机
} pcs_target_t;

// PCS运行控制模式枚举
typedef enum
{
    eCtrl_Mode_Start = 0,                 // 控制模式开始标识
    eCtrl_Mode_Remote = eCtrl_Mode_Start, // 远程控制模式
    eCtrl_Mode_Local,                     // 本地控制模式
    eCtrl_Mode_End,                       // 控制模式结束标识
} pcs_ctrl_mode_e;

// // EMS本地远程控制模式枚举
// typedef enum
// {
//     eEms_Mode_Start = 0,                // 模式开始标识
//     eEms_Mode_Remote = eEms_Mode_Start, // 远程控制模式
//     eEms_Mode_Local,                    // 本地控制模式
//     eEms_Mode_End,                      // 模式结束标识
// } ems_local_remote_mode_e;

// PCS分布式集中式模式枚举
typedef enum
{
    ePcs_Mode_Start = 0,                     // PCS模式开始标识
    ePcs_Mode_Distributed = ePcs_Mode_Start, // 分布式模式
    ePcs_Mode_Centralized,                   // 集中式模式
    ePcs_Mode_End,                           // PCS模式结束标识
} pcs_arch_mode_e;

// 堆bms与pcs的匹配关系
typedef struct
{
    uint8_t bms_id; // bms id
    struct
    {
        uint8_t pcs_start_id; // pcs id
        uint8_t pcs_end_id;   // pcs id
    } pcs_range;
} BmsPcsMatch_t;

#define MAX_BMS_CLUSTER_NUM 8
/*调度控制参数-高级参数设置*/
typedef struct
{
    /*PCS运行控制模式*/
    pcs_ctrl_mode_e ctrl_mode; // 远程本地控制模式
    /*PCS充电方向为负*/
    int8_t charge_direction; // 1 充电方向为负，-1 充电方向为正
    /*分配方式*/
    pcs_dist_mode_t dist_type; // 功率分配模式（soc均衡，平均分配等）
    /*控制下发方式*/
    pcs_dist_way_t value_way; // 控制下发方式
    /*目标指令*/
    pcs_target_t pcs_target; // PCS目标功率

    pcs_arch_mode_e arch_mode; // PCS分布式集中式模式

    BmsPcsMatch_t bms_pcs_match[MAX_BMS_CLUSTER_NUM]; // bms与pcs的匹配关系

} pcs_advance_param_t;

/*TCP状态机 */
typedef enum
{
    TCP_CLIENT_START = 0, // 连接关闭
    TCP_CLIENT_ONLINE,    // 连接建立状态
    TCP_CLIENT_OFFLINE,   // 连接断开状态
    TCP_CLIENT_END        // 连接断开状态
} tcp_client_state_e;


typedef struct
{
    uint8_t min_cluster_num;  // 最小并网簇数
    float max_delta_v;         // 最大电压差（允许并网的最大压差）
    uint16_t step_delay_ms;     // 并网延迟（ms）
    uint16_t precharge_timeout_ms; // 预充超时时间（ms）
    uint16_t poweroff_send_period_ms; // 断网发送周期（ms）
    uint16_t poweroff_timeout_ms; // 断网超时时间（ms）
}stack_config_t;


// Enums
// Bit0: Insulation Unfinished
// Bit1: Debug Mode
// Bit2: Battery String Disabled
// Bit3: EMS Trip
// Bit4: Extreme Fault CRC Check Failed
// Bit5: Precharge Timeout
// Bit6: Hv Differential not Achieved During Precharge Phase
// Bit7: Precharge Fault Set
// Bit8: Relay Fault Set
// Bit9: RBMS has Level 1 Fault
// Bit10: Hv Control Mode Disconnected
// Bit11: Extreme Fault Set
// Bit12: Emergency Stop Signal Set
typedef enum
{
    kRbms_ConFail_Start = 0,                    // 开始
    kRbms_ConFail_InsulUnfin = kRbms_ConFail_Start,               // 绝缘未完成
    kRbms_ConFail_DebugMode = 1,                // 调试模式
    kRbms_ConFail_BatStrDis = 2,                // 电池串禁用
    kRbms_ConFail_EmsTrip = 3,                  // EMS跳闸
    kRbms_ConFail_ExtCrcFail = 4,               // 严重故障CRC失败
    kRbms_ConFail_PrechgTo = 5,                 // 预充超时
    kRbms_ConFail_PrechgNoDiff = 6,             // 预充未达压差
    kRbms_ConFail_PrechgFlt = 7,                // 预充故障
    kRbms_ConFail_RelayFlt = 8,                 // 继电器故障
    kRbms_ConFail_RbmsL1Flt = 9,                // RBMS一级故障
    kRbms_ConFail_HvCtrlDis = 10,               // 高压控制断开
    kRbms_ConFail_ExtFlt = 11,                  // 严重故障
    kRbms_ConFail_EmergStop = 12,               // 急停信号
    kRbms_ConFail_End,                          // 结束
} bms_con_fail_e;


/* BBMS控制RBMS结构体 (8 Bytes, Matrix V1.0.50 BBMS_CtlWord) */
typedef struct __attribute__((packed))
{
    uint8_t bat_conn:2;                 // BBMS_SetCon 连接/断开电池串
    uint8_t ins_meas_en:2;              // BBMS_SetEnaIso 使能/禁止绝缘检测
    uint8_t first_enable_rack_flg:1;    // BBMS_FirstEnableRackFlg 是否为使能第一簇标志位
    uint8_t rever_con_detec_cmp_flg:1;  // BBMS_ReverConDetecCmpFlg 防反接检测完成信号
    uint8_t bat_str_en:2;               // BBMS_SetEna 使能/禁止电池串
    uint8_t bank_hb:8;                  // BBMS_Heartbeat Bank 心跳
    uint8_t str_en_rack:8;            // BBMS_RackEnableFlg 退簇后使能/禁止电池串
    uint8_t rack_exit_flag:5;         // BBMS_AllowRackExitFlg 允许退簇系统标志位
    uint8_t ctrl_mode:3;              // BBMS_LocRemCtl 控制模式
    uint8_t sys_arch_type:4;          // BBMS_SystemAchitectureNbr 系统架构类型
    uint8_t bank_err_lvl:4;           // BBMS_BankFltLvl Bank故障等级
    uint8_t disch_pwr_lim:8;          // BBMS_FltSOPDischCoeffPct 放电功率限制系数(%)
    uint8_t chg_pwr_lim:8;            // BBMS_FltSOPChCoeffPct 充电功率限制系数(%)
    uint8_t bbms_no:4;                // BBMSNo 堆编号
    uint8_t byte8_reserved:4;         // 预留占位 (byte8 bit4-7)
} bbms_ctrl_t;


// 簇控制模式
typedef enum
{
    kClusterCtrlMode_Central = 0, // 0x00:集中式
    kClusterCtrlMode_Distributed = 1, // 0x01:组串式
} cluster_ctrl_mode_e;


// BMS系统配置结构体 (type=0)
typedef struct {
    uint8_t bat_type;                       // 电池类型
    char bat_model[32];                     // 电池型号
    uint8_t bat_group_num;                  // 电池组数量
    uint8_t cluster_crtl_mode;              // 簇控制模式 0x00:集中式 0x01:组串式
    uint8_t min_online_cluster_num;         // 最小在网簇数
    uint8_t sys_shutdown_word;              // 系统关机字
    uint8_t allow_grid_volt_diff;           // 允许电网电压差
    uint8_t detach_pulse_time;              // 分离脉冲时间
    uint8_t cut_delay_time;                 // 切断延时时间
    uint8_t alert_led_time_interval;        // 告警LED时间间隔
    uint8_t insulation_enable;              // 绝缘检测使能
    uint8_t insulation_monitor_interval;    // 绝缘监测间隔
    uint32_t poweron_timeout_ms;         // 并网超时时间（ms）
    uint32_t poweroff_timeout_ms;        // 断网超时时间（ms）
} bms_sys_config_t;

typedef struct {
    bms_sys_config_t sys_config;        // 类型0的系统配置
    bbms_ctrl_t ctrl_config;            // 类型1的控制配置
} bms_config_t;



// do信号输出枚举索引
typedef enum
{
    kDo_Ctrl_Start = 0,
    kDo_Ctrl_None = kDo_Ctrl_Start,//none,
    kDo_Ctrl_Force_On,                    //强控闭合
    kDo_Ctrl_Force_Off,                   //强控打开
    kDo_Ctrl_Green_Light,                 //绿灯亮
    kDo_Ctrl_Red_Light,                   //红灯
    kDo_Ctrl_Yellow_Light,                //黄灯
    kDo_Ctrl_QF1,                         //QF1断路器闭合控制
    kDo_Ctrl_PCS_EStop,                   //PCS急停控制
    kDo_Ctrl_Stop,
    kDo_Ctrl_End,
} do_ctrl_type_t;


//输出模式 （0:禁用 1： 正常 1:强控闭合 2:强控关闭）
typedef enum
{
    kDO_Ctrl_Disable = 0, // 禁用
    kDO_Ctrl_Normal,      // 正常
    kDO_Ctrl_Force_Close, // 强控闭合
    kDO_Ctrl_Force_Open   // 强控打开
} do_ctrl_mode_e;

typedef struct
{
    uint16_t do_idx;
    uint16_t logic_idx;
    uint16_t bms_idx;
    uint16_t actual_value; // 实际值
    do_ctrl_mode_e do_output_mode; // do控制模式
} do_logic_conf_t;



typedef struct
{
    uint16_t do_config_num;
    do_logic_conf_t do_logic_config[kDo_Ctrl_Stop];   // do策略配置数据
} do_config_t;

// di信号输入信号策略枚举索引
typedef enum
{
    kDi_Signal_Start,
    kDi_Signal_None = kDi_Signal_Start,     // 无信号输入
    kDi_Signal_Water_AlarmI,                //水浸告警一级告警；
    kDi_Signal_Fire_Fault,                  //消防故障二级告警；
    kDi_Signal_Fire_AlarmI,                 //消防一级告警二级告警；
    kDi_Signal_Fire_AlarmII,                //消防二级告警一级告警；
    kDi_Signal_Gas_AlarmI,                  //可燃气体告警二级告警；
    kDi_Signal_BMS_DCSPD_FaultI,            //中控柜直流SPD故障一级告警；
    kDi_Signal_Container_EStopI,            //集装箱急停一级告警；
    kDi_Signal_QF1_Feedback_AlarmII,        //QF1状态反馈二级告警；
    kDi_Signal_Door_AlarmIII,               //集装箱门禁三级告警；
    kDi_Signal_ACSPD1_AlarmI,               //交流SPD1状态一级告警；
    kDi_Signal_Fan_FaultII,                 //防爆风机故障二级告警；
    kDi_Signal_Fuse_FaultI,                 //汇流熔断器故障一级故障
    kDi_Signal_Crtl_Switch_AlarmIII,        //中控柜行程开关三级告警
    kDi_Signal_Low_Pressure_AlarmI,         //FM200低气压告警一级告警；
    kDi_Signal_PCS_EStopI,                  //PCS急停一级告警；
    kDi_Signal_Power_EStopI,                  //配电柜一级告警；
    kDi_Signal_BackupPowerSPD1Fault,        //备电SPD1故障一级告警；
    kDi_Signal_CustExtEmergStop,            // 客户外部急停一级告警；
    kDi_Signal_Stop,
} di_signal_type_t;



// DI信号有效边沿检测类型枚举
typedef enum
{
    kDI_Edge_None = 0, // 无边沿
    kDI_Edge_Rising,   // 上升沿（0->1->1）
    kDI_Edge_Falling   // 下降沿（1->0->0）
} di_edge_type_e;

typedef enum
{
	kDiSignalValue_None,
	kDiSignalValue_Trigger,
	kDiSignalValue_End,
}di_signal_value_t;



//DI输入有效模式 （0：禁用 1：常开 2 :常闭）
typedef enum
{
    kDI_Valid_Disable = 0, // 禁用
    kDI_Valid_Normally_Open, // 常开
    kDI_Valid_Normally_Closed // 常闭
} di_valid_mode_e;

typedef struct
{
    uint16_t di_idx;
    uint16_t logic_idx;
    uint16_t bms_idx;
    uint16_t actual_value; // 实际值
    di_signal_value_t signal_value; // 信号值
    di_valid_mode_e di_input_mode; // di输入有效模式
    uint16_t delay_time; // 防抖时间
} di_logic_conf_t;


typedef struct
{
    uint16_t di_config_num;
    di_logic_conf_t di_logic_config[kDi_Signal_Stop]; // di策略配置数据
} di_config_t;


#endif // KIT_DATA_H