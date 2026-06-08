import request from '@/utils/request'

// 确认本机升级
export function confirmLocalUpgrade() {
  return request({
    url: "/localupdate/confirm/local",
    method: 'post',
  });
}

// 本机升级后重启
export function rebootAfterLocalUpgrade() {
  return request({
    url: "/localupdate/reboot/local",
    method: 'post',
  });
}

// 获取本机升级进度
export function getLocalUpgradeProgress() {
  return request({
    url: "/localupdate/progress/local",
    method: 'get',
  });
}

// 确认DSP升级
export function confirmDspUpgrade(params) {
  return request({
    url: "/localupdate/confirm/dsp",
    method: 'post',
    data: params,
  });
}

// 获取DSP升级进度
export function getDspUpgradeProgress(params) {
  return request({
    url: "/localupdate/progress/dsp",
    method: 'post',
    data: params,
  });
}

// 获取DSP编址进度
export function setDspAddressing(channel) {
  return request({
    url: "/localupdate/dsp/addressing",
    method: 'get',
    params: {
      channel: channel,
    },
  });
}

export function getDspAddressing(channel) {
  return request({
    url: "/localupdate/dsp/addressing/result",
    method: 'get',
    params: {
      channel: channel,
    },
  });
}