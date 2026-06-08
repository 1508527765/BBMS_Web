import request from '@/utils/request'

// 查询数据列表
export function listTimedData(query) {
  return request({
    url: '/history/timed-data',
    method: 'post',
    data: query
  })
}


// 查询事件/故障列表
export function listEventData(query) {
  return request({
    url: '/history/event-data',
    method: 'post',
    data: query
  })
}


