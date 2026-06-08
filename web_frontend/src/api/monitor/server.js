import request from '@/utils/request'

// 获取服务信息
export function getServer() {
  // 获取当前语言，中文为cn，英文为en
  const currentLang = localStorage.getItem('lang') || 'en'
  const lang = currentLang === 'zh' ? 'cn' : 'en'
  
  return request({
    url: '/monitor/server',
    method: 'get',
    params: {
      lang: lang
    }
  })
}

// 获取服务信息
export function getPcsSoftVersion() {
  return request({
    url: '/device/pcs-soft-version',
    method: 'get'
  })
}