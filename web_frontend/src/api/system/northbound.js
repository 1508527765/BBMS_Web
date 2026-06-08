import request from '@/utils/request'
import { parseStrEmpty } from "@/utils/ruoyi";

// 查询设备列表
export function listNorthBound(query) {
  return request({
    url: '/northbound/list',
    method: 'get',
    params: query
  })
}

// 查询设备详细
export function getNorthBound(northBoundId) {
  return request({
    url: '/northbound/' + parseStrEmpty(northBoundId),
    method: 'get'
  })
}

// 新增设备
export function addNorthBound(data) {
  return request({
    url: '/northbound',
    method: 'post',
    data: data
  })
}

// 修改设备
export function updateNorthBound(data) {
  return request({
    url: '/northbound',
    method: 'put',
    data: data
  })
}

// 删除设备
export function delNorthBound(northBoundId) {
  return request({
    url: '/northbound/' + northBoundId,
    method: 'delete'
  })
}
