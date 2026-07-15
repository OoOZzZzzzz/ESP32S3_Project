import { defineConfig } from 'vite'
import vue from '@vitejs/plugin-vue'
import { resolve } from 'path'

export default defineConfig({
  plugins: [vue()],

  // server: {
  //   proxy: {
  //     '/api': {
  //       target: 'http://192.168.1.100', // 改成你的 ESP32 IP
  //       changeOrigin: true,
  //       rewrite: (path) => path.replace(/^\/api/, '')
  //     }
  //   }
  // }
  resolve: {
    alias: {
      '@': resolve(__dirname, 'src') // 映射@到src目录
    }
  }

})