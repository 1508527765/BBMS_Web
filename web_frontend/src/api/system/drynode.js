import request from '@/utils/request'

// 查询端口列表
export function listDrynode(query) {
  return request({
    url: '/drynode/list',
    method: 'get',
    params: query
  })
}

// 查询逻辑列表
export function listLogicList(query) {
  return request({
    url: '/drynode/logicList',
    method: 'get',
    params: query
  })
}

// 新增
export function addDrynode(data) {
  return request({
    url: '/drynode',
    method: 'post',
    data: data
  })
}

// 修改
export function updateDrynode(data) {
  return request({
    url: '/drynode',
    method: 'put',
    data: data
  })
}

// 删除
export function deleteDrynode(drynodeId) {
  return request({
    url: '/drynode/' + drynodeId,
    method: 'delete'
  })
}