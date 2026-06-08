'use strict'
const path = require('path')

function resolve(dir) {
  return path.join(__dirname, dir)
}

const CompressionPlugin = require('compression-webpack-plugin')

const name = process.env.VUE_APP_TITLE || 'BMS Management System' // ????

// 开发默认 9527，避免与生产 Nginx:80 冲突；可用环境变量 port 覆盖
const port = process.env.port || process.env.npm_config_port || 9527

// vue.config.js ????
//??vue.config.js ?????https://cli.vuejs.org/zh/config/#css-loaderoptions
// ????????????
module.exports = {
  //???????????URL??  // ??????Vue CLI ?????????????????????
  // ?? https://www.ruoyi.vip/?????????????????????????????????????????????? https://www.ruoyi.vip/admin/???? baseUrl ??/admin/??  publicPath: process.env.NODE_ENV === "production" ? "/" : "/",
  // ?npm run build ??yarn build ???????????????baseUrl?????????????dist??  outputDir: 'dist',
  // ????????????(js?css?img?fonts) ?????????????????????????  assetsDir: 'static',
  // 开发环境关闭「保存即 ESLint」，减轻 dev 卡顿；提交/CI 请执行 npm run lint
  lintOnSave: process.env.NODE_ENV === 'production',
  // ??????????? source map???????? false ???????????  productionSourceMap: false,
  transpileDependencies: ['quill'],
  // webpack-dev-server ????
  devServer: {
    host: '0.0.0.0',
    port: port,
    open: true,
    // 减少 inotify 占用，缓解 Linux 上 ENOSPC「文件监视数量到上限」刷屏
    watchOptions: {
      ignored: [
        path.resolve(__dirname, 'node_modules'),
        path.resolve(__dirname, 'dist'),
        path.resolve(__dirname, '.git'),
        '**/.git/**',
      ],
      aggregateTimeout: 600,
      poll: process.env.BBMS_DEV_POLL ? Number(process.env.BBMS_DEV_POLL) || 2000 : false,
    },
    proxy: {
      // detail: https://cli.vuejs.org/config/#devserver-proxy
      [process.env.VUE_APP_BASE_API]: {
      target: `http://127.0.0.1:9099`,
        changeOrigin: true,
        pathRewrite: {
          ['^' + process.env.VUE_APP_BASE_API]: ''
        }
      }
    },
    disableHostCheck: true
  },
  css: {
    loaderOptions: {
      sass: {
        sassOptions: { outputStyle: "expanded" }
      },
      less:{
        javascriptEnabled: true
      }
    }
  },
  configureWebpack: {
    name: name,
    resolve: {
      alias: {
        '@': resolve('src')
      }
    },
    // 开发环境不启用 gzip 插件，减少 dev 首次编译与 rebuild 开销
    plugins:
      process.env.NODE_ENV === 'production'
        ? [
            new CompressionPlugin({
              cache: false,
              test: /\.(js|css|html|jpe?g|png|gif|svg)?$/i,
              filename: '[path][base].gz[query]',
              algorithm: 'gzip',
              minRatio: 0.8,
              deleteOriginalAssets: false
            })
          ]
        : []
  },
  chainWebpack(config) {
    config.plugins.delete('preload') // TODO: need test
    config.plugins.delete('prefetch') // TODO: need test

    // set svg-sprite-loader
    config.module
      .rule('svg')
      .exclude.add(resolve('src/assets/icons'))
      .end()
    config.module
      .rule('icons')
      .test(/\.svg$/)
      .include.add(resolve('src/assets/icons'))
      .end()
      .exclude.add(resolve('src/assets/icons/overview_svg'))
      .end()
      .use('svg-sprite-loader')
      .loader('svg-sprite-loader')
      .options({
        symbolId: 'icon-[name]'
      })
      .end()
    
    // overview_svg ???? SVG ?? file-loader ???????    config.module
      .rule('overview-svg')
      .test(/\.svg$/)
      .include.add(resolve('src/assets/icons/overview_svg'))
      .end()
      .use('file-loader')
      .loader('file-loader')
      .options({
        name: 'static/icons/[name].[hash:8].[ext]'
      })
      .end()

    config.when(process.env.NODE_ENV !== 'development', config => {
          config
            .plugin('ScriptExtHtmlWebpackPlugin')
            .after('html')
            .use('script-ext-html-webpack-plugin', [{
            // `runtime` must same as runtimeChunk name. default is `runtime`
              inline: /runtime\..*\.js$/
            }])
            .end()

          config.optimization.splitChunks({
            chunks: 'all',
            cacheGroups: {
              libs: {
                name: 'chunk-libs',
                test: /[\\/]node_modules[\\/]/,
                priority: 10,
                chunks: 'initial' // only package third parties that are initially dependent
              },
              elementUI: {
                name: 'chunk-elementUI', // split elementUI into a single package
                test: /[\\/]node_modules[\\/]_?element-ui(.*)/, // in order to adapt to cnpm
                priority: 20 // the weight needs to be larger than libs and app or it will be packaged into libs or app
              },
              commons: {
                name: 'chunk-commons',
                test: resolve('src/components'), // can customize your rules
                minChunks: 3, //  minimum common number
                priority: 5,
                reuseExistingChunk: true
              }
            }
          })
          config.optimization.runtimeChunk('single')
    })
  }
}
