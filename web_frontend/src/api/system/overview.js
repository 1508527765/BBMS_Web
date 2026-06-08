import request from '@/utils/request'

// // 查询dsp设备信息
// export function getTopologyInfo() {
//   return request({
//     url: '/overview/topology-info',
//     method: 'get'
//   })
// }

// // 查询电站信息
// export function getStationInfo() {
//     return request({
//       url: '/overview/station-info',
//       method: 'get'
//     })
//   }

  
// // 查询储能信息
// export function getEnergyStorageInfo() {
//     return request({
//       url: '/overview/energy-storage-info',
//       method: 'get'
//     })
//   }

//获取首页展示的数据
export function getCombinedInfo() {
  return request({
    url: '/overview/combined-info',
    method: 'get'
  })
}
  

// 下发单个dsp信息
export function setDspState(data) {
  return request({
    url: '/overview/dsp-state',
    method: 'post',
    data: data
  })
}
