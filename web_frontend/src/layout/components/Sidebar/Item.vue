<script>
import i18n from '@/i18n'

export default {
  name: 'MenuItem',
  functional: true,
  props: {
    icon: {
      type: String,
      default: ''
    },
    title: {
      type: String,
      default: ''
    }
  },
  render(h, context) {
    const { icon, title } = context.props
    const vnodes = []

    // 优先使用多语言翻译：menu.中文标题
    const translateTitle = (text) => {
      if (!text) return ''
      const key = `menu.${text}`
      const translated = i18n.t(key)
      // 如果没配置多语言，则直接返回原始标题
      return translated === key ? text : translated
    }

    const displayTitle = translateTitle(title)

    if (icon) {
      vnodes.push(<svg-icon icon-class={icon}/>)
    }

    if (displayTitle) {
      if (displayTitle.length > 5) {
        vnodes.push(<span slot='title' title={displayTitle}>{displayTitle}</span>)
      } else {
        vnodes.push(<span slot='title'>{displayTitle}</span>)
      }
    }
    return vnodes
  }
}
</script>
