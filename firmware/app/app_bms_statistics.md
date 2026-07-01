# 记录bms统计疑问

## 问题列表

### 1. Bank工作状态极计算
 * 当前逻辑，需要确认是否正确？
   * 只要有一个RBMS未连接，Bank状态即为Not Connected
   * 只要有一个RBMS已连接，Bank状态即为Connected
   * 只要有一个RBMS处于SOC保护状态，Bank状态即为SOC Protection
   * 只要有一个RBMS处于故障状态，Bank状态即为Fault
   * 其他情况，Bank状态即为Allocation

### 2. Bank充放电状态
 * 当前逻辑，需要确认是否正确？
   * 只要有一个RBMS放电中，BBMS的充放电状态即为放电中
   * 只要有一个RBMS充电中，BBMS的充放电状态即为充电中
   * 其他情况，BBMS的充放电状态即为Not Connected


### 3. Bank系统运行状态
* 1.Bank系统运行状态中地址分配数据源？
* 2.Bank系统运行状态中故障清除数据源？是从ems获取？

### 4. 设置逆变器状态
* 数据源从哪里来，一般bms不获取pcs状态
* 枚举定义
  * 0X1: PCS Shut Down
  * 0X2: PCS Standby
  * 0X3: PCS Running

### 5. Bank接触器状态
* 需要确认防爆风机状态和故障状态区分，数据源在哪里，当前只有一个DI接入的

### 6. Bank湿度 (RH)、Bank温度 (℃)
* 数据源从哪里来？当前逻辑取自除湿机的温度和湿度，请确认？

### 7. Bank母线电压 (V)
* 当前逻辑取自rbms中最大的DC线电压作为母线电压，请确认？

### 8. Bank电池电流 (A)
* 当前逻辑取自rbms中DC线电流之和作为电池电流，请确认？
