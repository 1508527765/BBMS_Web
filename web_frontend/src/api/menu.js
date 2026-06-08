import request from '@/utils/request'

// 获取路由
export const getRouters = (lang) => {
  return request({
    url: '/getRouters',
    method: 'get',
    params: {
      lang: lang
    }
  })
}