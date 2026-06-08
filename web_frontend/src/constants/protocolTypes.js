/**
 * 与 C 端 protocol_type_master_e（kit_data.h）一致。
 * 0..7 可写入库；8=kProto_Master_End 仅作枚举边界，不可写入库。
 */
export const ProtocolType = {
  LOCAL_INFO: 0,
  BBMS_PRIVATE: 1,
  MODBUS_TCP: 2,
  MODBUS_RTU: 3,
  RBMS_TCP: 4,
  RBMS_CAN: 5,
  MODBUS_MULTI: 6,
  TMS_CAN: 7,
  MASTER_END: 8,
};

export const protocolTypes = [
  { label: "本机信息协议", en_label: "Local Device Protocol", value: 0, cName: "kProto_Master_Info_Master" },
  { label: "BBMS 私有协议", en_label: "BBMS Private Protocol", value: 1, cName: "kProto_BbmsPrivate_Master" },
  { label: "Modbus TCP 主站", en_label: "Modbus TCP Master", value: 2, cName: "kProto_ModbusTCP_Master" },
  { label: "Modbus RTU 主站", en_label: "Modbus RTU Master", value: 3, cName: "kProto_ModbusRTU_Master" },
  { label: "RBMS TCP 私有协议", en_label: "RBMS Private TCP Protocol", value: 4, cName: "kProto_RbmsPrivate_TCP_Master" },
  { label: "RBMS CAN 私有协议", en_label: "RBMS Private CAN Protocol", value: 5, cName: "kProto_RbmsPrivate_Can_Master" },
  { label: "ModbusMultiId TCP 协议", en_label: "ModbusMultiId TCP Protocol", value: 6, cName: "kProto_ModbusTCPMulti_Master" },
  { label: "TMS CAN 私有协议", en_label: "TMS Private CAN Protocol", value: 7, cName: "kProto_TMSPrivateCan_Master" },
];

export function isRtuProtocol(raw) {
  return Number(raw) === ProtocolType.MODBUS_RTU;
}

export function isCanProtocol(raw) {
  const v = Number(raw);
  return v === ProtocolType.RBMS_CAN || v === ProtocolType.TMS_CAN;
}

export function isTcpProtocol(raw) {
  const v = Number(raw);
  return (
    v === ProtocolType.MODBUS_TCP ||
    v === ProtocolType.RBMS_TCP ||
    v === ProtocolType.MODBUS_MULTI
  );
}

/** 需要填写 IP/串口/CAN 等连接参数（不含本机信息、BBMS 私有） */
export function needsProtocolConfig(raw) {
  const v = Number(raw);
  return v >= ProtocolType.MODBUS_TCP && v < ProtocolType.MASTER_END;
}

export default protocolTypes;
