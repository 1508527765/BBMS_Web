import request from '@/utils/request'
import { parseStrEmpty } from "@/utils/ruoyi";

// 查询设备列表
export function listDevice(query) {
  return request({
    url: '/device/list',
    method: 'get',
    params: query
  })
}

// 查询设备详细
export function getDevice(deviceId) {
  return request({
    url: '/device/' + parseStrEmpty(deviceId),
    method: 'get'
  })
}

// 新增设备
export function addDevice(data) {
  return request({
    url: '/device',
    method: 'post',
    data: data
  })
}

// 修改设备
export function updateDevice(data) {
  return request({
    url: '/device',
    method: 'put',
    data: data
  })
}

// 删除设备
export function delDevice(deviceId) {
  return request({
    url: '/device/' + deviceId,
    method: 'delete'
  })
}

// 获取设备的具体数据点
export function getDevicePoints(deviceId, query) {
  return request({
    url: '/device/points/' + parseStrEmpty(deviceId),
    method: 'get',
    params: query
  })
}

// 获取设备的具体数据点
export function getDevicePointsWithStatus(deviceId, query) {
  return request({
    url: '/device/points-with-status/' + parseStrEmpty(deviceId),
    method: 'get',
    params: query
  })
}

// 查询全部设备
export function listAllDevice(query) {
  return request({
    url: '/device/listAll',
    method: 'get',
    params: query
  })
}

// 新增测点
export function addDevicePoint(data) {
  return request({
    url: '/device/point',
    method: 'post',
    data: data
  })
}

// 修改测点
export function updateDevicePoint(data) {
  return request({
    url: '/device/point',
    method: 'put',
    data: data
  })
}

// 删除测点
export function delDevicePoint(pointId) {
  return request({
    url: '/device/point/' + pointId,
    method: 'delete'
  })
}

// 批量新增设备
export function batchAddSystemDevices(devices) {
  return request({
    url: '/device/batch',
    method: 'post',
    data: devices
  })
}

export function updateDeviceValue(query) {
  return request({
    url: '/rtdb/set_point_value',
    method: 'post',
    data: query
  })
}


// 根据设备类型查询设备列表
export function getDeviceListByType(deviceType) {
  return request({
    url: '/device/devices-by-type/' + deviceType,
    method: 'get'
  })
}