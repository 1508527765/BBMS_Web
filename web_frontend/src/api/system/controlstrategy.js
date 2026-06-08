// src/api/config.js
import request from '@/utils/request'

// 获取配置项列表
export function fetchConfigList(type, device_code) {
  return request({
    url: '/control-strategy/list',
    method: 'get',
    params: {
      type,
      device_code,
    },
  })
}

// 批量更新配置项
export function updateConfigValues(updates) {
  return request({
    url: '/control-strategy/update',
    method: 'post',
    data:  updates 
  })
}

// 批量更新配置项
export function fetchDspDevices() {
  return request({
    url: '/device/dsp-devices',
    method: 'get'
  })
}


// 获取配置项列表
export function fetchAdvancedConfigs() {
  return request({
    url: '/control-strategy/advanced/list-configs',
    method: 'get',
  })
}

// 批量更新配置项
export function saveAdvancedConfigs(updates) {
  return request({
    url: '/control-strategy/advanced/save-configs',
    method: 'post',
    data:  updates 
  })
}

//获取pcs列表
export function getPcsList() {
  return request({
    url: '/control-strategy/pcs/list-with-status',
    method: 'get',
  })
}

//保存pcs配置
export function savePcsConfigList(data) {
  return request({
    url: '/control-strategy/pcs/save-configs',
    method: 'post',
    data: data,
  })
}

