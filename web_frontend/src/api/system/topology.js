import request from '@/utils/request'
import { parseStrEmpty } from "@/utils/ruoyi";

// 获取拓扑结构列表
export function listTopology(query) {
  return request({
    url: '/topology/list',
    method: 'get',
    params: query
  })
}

// 新增拓扑结构
export function addTopology(data) {
  return request({
    url: '/topology',
    method: 'post',
    data: data
  })
}

// 修改拓扑结构
export function updateTopology(data) {
  return request({
    url: '/topology',
    method: 'put',
    data: data
  })
}

// 删除拓扑结构
export function deleteTopology(topologyIds) {
  return request({
    url: '/topology/' + parseStrEmpty(topologyIds),
    method: 'delete'
  })
}
