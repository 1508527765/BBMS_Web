// 元素尺寸监听插件
import ElementResizeDetectorMaker from 'element-resize-detector'
import debounce from 'lodash/debounce'

/**
 * 元素宽度监听
 * 使用：v-wResize="onWidthResize"
 * onWidthResize({width})
 */
export default {
  bind(el, binding) {
    let width = 0
    const erd = ElementResizeDetectorMaker()
		// 防抖
    const debounceHandler = element =>
      debounce(() => {
        let offsetWidth = element.offsetWidth
        if (width !== offsetWidth) {
          width = offsetWidth
          binding.value({ width })
        }
      }, 100)()
    erd.listenTo(el, debounceHandler)
    el.__vueErd__ = erd
  },

  // 销毁时移除detector
  unbind(el) {
    el.__vueErd__.uninstall(el)
  },
}