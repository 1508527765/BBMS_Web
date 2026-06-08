import request from '@/utils/request'
import { parseStrEmpty } from "@/utils/ruoyi";

// 查询模板列表
export function listTemplate(query) {
  return request({
    url: '/template/list',
    method: 'get',
    params: query
  })
}

// 查询所有模板列表
export function listAllTemplate(query) {
  return request({
    url: '/template/all',
    method: 'get',
    params: query
  })
}

// 查询模板详细
export function getTemplate(templateId) {
  return request({
    url: '/template/' + parseStrEmpty(templateId),
    method: 'get'
  })
}

// 新增模板
export function addTemplate(data) {
  return request({
    url: '/template',
    method: 'post',
    data: data
  })
}

// 修改模板
export function updateTemplate(data) {
  return request({
    url: '/template',
    method: 'put',
    data: data
  })
}

// 删除模板
export function delTemplate(templateId) {
  return request({
    url: '/template/' + templateId,
    method: 'delete'
  })
}

// 获取模板的具体数据点
export function getTemplatePoints(templateId, query) {
  return request({
    url: '/template/points/' + parseStrEmpty(templateId),
    method: 'get',
    params: query
  })
}

// 新增测点
export function addPoint(data) {
  return request({
    url: '/template/point',
    method: 'post',
    data: data
  })
}

// 修改测点
export function updatePoint(data) {
  return request({
    url: '/template/point',
    method: 'put',
    data: data
  })
}

// 删除测点
export function delPoint(pointId) {
  return request({
    url: '/template/point/' + pointId,
    method: 'delete'
  })
}
