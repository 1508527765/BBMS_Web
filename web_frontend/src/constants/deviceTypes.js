/**
 * 与 C 端 dev_type_e（kit_data.h）一致。
 */
export function findDeviceTypeByValue(deviceTypesList, raw) {
  const v = raw === null || raw === undefined || raw === "" ? NaN : Number(raw);
  if (Number.isNaN(v)) return undefined;
  return deviceTypesList.find((item) => Number(item.value) === v);
}

export const deviceTypes = [
  { label: '本机系统', en_label: 'Local system', value: 0 },
  { label: '堆控', en_label: 'Stack BMS', value: 1 },
  { label: 'BMS簇控制器', en_label: 'BMS cluster controller', value: 2 },
  { label: 'PCS类型', en_label: 'PCS', value: 3 },
  { label: '变压器', en_label: 'Transformer', value: 4 },
  { label: '空调/液冷机组', en_label: 'Air conditioner / liquid cooling unit', value: 5 },
  { label: '电表设备', en_label: 'Meter device', value: 6 },
  { label: 'UPS设备', en_label: 'UPS device', value: 7 },
  { label: '消防设备', en_label: 'Firefighting device', value: 8 },
  { label: '除湿机', en_label: 'Dehumidifier', value: 9 },
  { label: '温度传感器', en_label: 'Temperature sensor', value: 10 },
  { label: '湿度传感器', en_label: 'Humidity sensor', value: 11 },
  { label: '水浸传感器', en_label: 'Water leak sensor', value: 12 },
  { label: '烟雾传感器', en_label: 'Smoke sensor', value: 13 },
  { label: '其他传感器1', en_label: 'Other sensor 1', value: 14 },
  { label: '其他传感器2', en_label: 'Other sensor 2', value: 15 },
  { label: '其他传感器3', en_label: 'Other sensor 3', value: 16 },
  { label: '预留设备类型17', en_label: 'Reserved device type 17', value: 17 },
  { label: '预留设备类型18', en_label: 'Reserved device type 18', value: 18 },
  { label: '预留设备类型19', en_label: 'Reserved device type 19', value: 19 },
  { label: '预留设备类型20', en_label: 'Reserved device type 20', value: 20 },
  { label: '预留设备类型21', en_label: 'Reserved device type 21', value: 21 },
  { label: '预留设备类型22', en_label: 'Reserved device type 22', value: 22 },
  { label: '预留设备类型23', en_label: 'Reserved device type 23', value: 23 },
  { label: '预留设备类型24', en_label: 'Reserved device type 24', value: 24 },
  { label: '预留设备类型25', en_label: 'Reserved device type 25', value: 25 },
  { label: '预留设备类型26', en_label: 'Reserved device type 26', value: 26 },
  { label: '预留设备类型27', en_label: 'Reserved device type 27', value: 27 },
  { label: '预留设备类型28', en_label: 'Reserved device type 28', value: 28 },
  { label: '预留设备类型29', en_label: 'Reserved device type 29', value: 29 },
]

export default deviceTypes
