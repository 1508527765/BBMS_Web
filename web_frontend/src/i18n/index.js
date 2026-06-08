import Vue from 'vue';
import VueI18n from 'vue-i18n';
// 引入自定义语言配置
import zh from './langs/zh';
import en from './langs/en';
// 引入UI框架语言配置---element-ui
import ElementLocale from 'element-ui/lib/locale';
import enLocale from 'element-ui/lib/locale/lang/en'; // 引入饿了么的英文包
import zhLocale from 'element-ui/lib/locale/lang/zh-CN'; // 引入饿了么的中文包

Vue.use(VueI18n);

const i18n = new VueI18n({
  locale: localStorage.getItem('lang') || 'en',
  messages: {
    // 中文语言包
    zh: {
      ...zh,
      ...zhLocale,
    },
    // 英文语言包
    en: {
      ...en,
      ...enLocale,
    },
  },
  fallbackLocale: 'en',
});

// 设置 ElementUI 的语言切换
ElementLocale.i18n((key, value) => i18n.t(key, value));

export default i18n;
