import request from '@/utils/request'
import { parseStrEmpty } from "@/utils/ruoyi";

// 查询端口列表
export function listPort(query) {
  return request({
    url: '/system/port/list',
    method: 'get',
    params: query
  })
}

// 新增端口
export function addPort(data) {
  return request({
    url: '/system/port',
    method: 'post',
    data: data
  })
}

// 修改端口
export function updatePort(data) {
  return request({
    url: '/system/port',
    method: 'put',
    data: data
  })
}

// 删除端口
export function delPort(portId) {
  return request({
    url: '/system/port/' + portId,
    method: 'delete'
  })
}