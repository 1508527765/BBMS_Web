import request from "@/utils/request";

export function fetchBatterySysConfig(configType = 0) {
  return request({
    url: "/battery-config/sys-config",
    method: "get",
    params: { config_type: configType },
  });
}

export function saveBatterySysConfig(type, rows) {
  return request({
    url: "/battery-config/sys-config/save",
    method: "post",
    data: { type, rows },
  });
}

export function fetchBatteryAlertConfig() {
  return request({
    url: "/battery-config/alert-config",
    method: "get",
  });
}

export function saveBatteryAlertConfig(alertType, rows) {
  return request({
    url: "/battery-config/alert-config/save",
    method: "post",
    data: { alert_type: alertType, rows },
  });
}

export function fetchBatteryDidoConfig() {
  return request({
    url: "/battery-config/dido-config",
    method: "get",
  });
}

export function saveBatteryDidoConfig(diRows, doRows) {
  return request({
    url: "/battery-config/dido-config/save",
    method: "post",
    data: { diRows, doRows },
  });
}
