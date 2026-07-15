<template>
  <div id="app">
    <div class="container">
      <h1>🌡️ ESP32-S3 传感器监控</h1>

      <!-- 新增IP输入区域 -->
      <div class="ip-setting">
        <label>ESP32 地址：</label>
        <input
          v-model="esp32IP"
          placeholder="http://10.218.102.xxx"
          @input="resetData"
        />
        <div class="chick">
          <button @click="fetchData" class="btn">切换IP并重试</button>
        </div>
      </div>

      <div class="status" :class="statusClass">
        {{ statusMessage }}
      </div>

      <div v-if="data" class="sensor-data">
        <div class="data-item">
          <span class="label">温度:</span>
          <span class="value">{{ data.temperature }}°C</span>
        </div>
        <div class="data-item">
          <span class="label">湿度:</span>
          <span class="value">{{ data.humidity }}%</span>
        </div>
      </div>

      <div class="controls">
        <button @click="fetchData" :disabled="loading" class="btn">
          {{ loading ? '获取中...' : '获取数据' }}
        </button>
      </div>

      <div v-if="error" class="error">
        <h4>错误详情:</h4>
        <p>{{ error }}</p>
        <div class="solutions">
          <h5>解决方案:</h5>
          <ol>
            <li>确保 ESP32 代码已添加 CORS 支持</li>
            <li>尝试使用 Chrome 禁用 CORS 模式</li>
            <li>或在浏览器中直接访问: <a :href="apiUrl" target="_blank">{{ apiUrl }}</a></li>
          </ol>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { ESP32_DEFAULT_IP } from "@/config/env.js";

export default {
  name: 'App',
  data() {
    return {
      loading: false,
      error: '',
      data: null,
      testResult: '',
      lastResponse: '',
      showDebug: false,
      // 默认填你自己的静态IP，也可以使用页面修改输入框即可
      esp32IP: ESP32_DEFAULT_IP
    }
  },
  computed: {
    apiUrl() {
      return `${this.esp32IP}/api/data`
    },
    statusMessage() {
      if (this.loading) return '🔄 连接中...'
      if (this.error) return '❌ 连接失败'
      if (this.data) return '✅ 已连接'
      return '⏳ 等待连接'
    },
    statusClass() {
      if (this.loading) return 'loading'
      if (this.error) return 'error-status'
      if (this.data) return 'connected'
      return 'disconnected'
    }
  },
  mounted() {
    this.fetchData()
  },
  methods: {
    // 新增：修改IP后清空旧数据和错误
    resetData() {
      this.data = null
      this.error = ''
    },

    async fetchData() {
      try {
        this.loading = true
        this.error = ''
        this.lastResponse = '开始请求...'

        const response = await fetch(this.apiUrl, {
          method: 'GET',
          mode: 'cors',
          headers: {
            'Accept': 'application/json'
          }
        })

        this.lastResponse = `状态: ${response.status} ${response.statusText}`

        if (!response.ok) {
          throw new Error(`HTTP ${response.status}: ${response.statusText}`)
        }

        const result = await response.json()
        this.lastResponse = `成功: ${JSON.stringify(result)}`
        
        if (result && typeof result.temperature === 'number' && typeof result.humidity === 'number') {
          this.data = result
        } else {
          throw new Error('无效的数据格式')
        }

      } catch (err) {
        console.error('请求失败:', err)
        this.error = err.message
        this.lastResponse = `错误: ${err.message}`
        
        if (err.message.includes('Failed to fetch') || err.message.includes('CORS')) {
          this.error += ' (可能是 CORS 问题)'
        }
      } finally {
        this.loading = false
      }
    },

    async testConnection() {
      try {
        this.testResult = '测试中...'
        const response = await fetch(this.esp32IP, {
          method: 'GET',
          mode: 'no-cors'
        })
        this.testResult = '✅ 基本连接正常'
      } catch (err) {
        this.testResult = '❌ 连接测试失败'
      }
    },

    updateIP() {
      this.data = null
      this.error = ''
      this.fetchData()
    },

    clearDebug() {
      this.lastResponse = ''
    }
  }
}
</script>


<style>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}

body {
  font-family: Arial, sans-serif;
  background: #f5f5f5;
  padding: 20px;
}

#app {
  max-width: 600px;
  margin: 0 auto;
}

.container {
  background: white;
  padding: 30px;
  border-radius: 10px;
  box-shadow: 0 2px 10px rgba(0,0,0,0.1);
}

h1 {
  text-align: center;
  margin-bottom: 20px;
  color: #333;
}

.config {
  background: #f8f9fa;
  padding: 15px;
  border-radius: 5px;
  margin-bottom: 20px;
}

/* .ip-input, .connection-test {
  display: flex;
  align-items: center;
  gap: 10px;
  margin-bottom: 10px;
}

.ip-input input {
  padding: 8px;
  border: 1px solid #ddd;
  border-radius: 4px;
  flex: 1;
} */

/* .btn-small {
  padding: 6px 12px;
  background: #6c757d;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
} */


.ip-setting {
  text-align: center;
  margin: 6px 0;
}

.ip-setting input {
  width: 220px;
  padding: 6px 8px;
  margin: 0 8px;
}

.chick {
  text-align: center;
  margin: 12px 0;
}

.btn {
  padding: 10px 20px;
  background: #007bff;
  color: white;
  border: none;
  border-radius: 4px;
  cursor: pointer;
  font-size: 16px;
}

.btn:disabled {
  background: #ccc;
  cursor: not-allowed;
}

.status {
  padding: 10px;
  border-radius: 5px;
  text-align: center;
  margin: 20px 0;
  font-weight: bold;
}

.loading { background: #fff3cd; color: #856404; }
.connected { background: #d4edda; color: #155724; }
.error-status { background: #f8d7da; color: #721c24; }
.disconnected { background: #e2e3e5; color: #383d41; }

.sensor-data {
  background: #f8f9fa;
  padding: 20px;
  border-radius: 5px;
  margin: 20px 0;
}

.data-item {
  display: flex;
  justify-content: space-between;
  margin: 10px 0;
  padding: 10px;
  background: white;
  border-radius: 4px;
}

.label {
  font-weight: bold;
}

.value {
  color: #007bff;
  font-weight: bold;
  font-size: 1.2em;
}

.controls {
  text-align: center;
  margin: 20px 0;
}

.error {
  background: #f8d7da;
  border: 1px solid #f5c6cb;
  border-radius: 5px;
  padding: 15px;
  margin: 20px 0;
}

.error h4 {
  color: #721c24;
  margin-bottom: 10px;
}

.solutions {
  margin-top: 10px;
}

.solutions h5 {
  color: #721c24;
  margin-bottom: 5px;
}

.solutions ol {
  margin-left: 20px;
}

/* .debug {
  background: #e9ecef;
  padding: 15px;
  border-radius: 5px;
  margin-top: 20px;
} */

.test-result {
  margin-left: 10px;
  font-weight: bold;
}

a {
  color: #007bff;
  text-decoration: none;
}

a:hover {
  text-decoration: underline;
}
</style>