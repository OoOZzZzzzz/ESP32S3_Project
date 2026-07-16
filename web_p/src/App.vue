<template>
  <!-- 页面根容器 -->
  <div id="app">
    <div class="container">
      <!-- 页面大标题 -->
      <h1>🌡️ ESP32-S3 温湿度实时监控系统</h1>

      <!-- IP输入配置卡片区域 -->
      <div class="card ip-card">
        <h3>设备连接配置</h3>
        <div class="ip-setting">
          <label>ESP32 服务地址：</label>
          <input
            v-model="esp32IP"
            placeholder="http://192.168.248.100"
            @input="resetAll"
          />
          <button @click="fetchData" class="btn primary">立即连接</button>
        </div>
      </div>

      <!-- 连接状态提示栏 -->
      <div class="status" :class="statusClass">
        {{ statusMessage }}
      </div>

      <!-- 波形图表卡片 -->
      <div class="card chart-card" v-show="dataList.length > 0">
        <h3>温湿度实时波形曲线</h3>
        <div class="chart-wrap">
          <canvas ref="chartRef"></canvas>
        </div>
      </div>

      <!-- 当前实时数值面板 -->
      <div class="card data-card" v-if="data">
        <h3>当前实时数据</h3>
        <div class="data-row">
          <div class="data-item temp">
            <div class="label">温度</div>
            <div class="value">{{ data.temperature }} ℃</div>
          </div>
          <div class="data-item hum">
            <div class="label">湿度</div>
            <div class="value">{{ data.humidity }} %RH</div>
          </div>
        </div>
      </div>

      <!-- ESP32 LED硬件控制卡片 -->
      <div class="card ctrl-card">
        <h3>硬件LED控制</h3>
        <div class="btn-group">
          <button @click="sendCmd('led_on')" class="btn success">点亮LED</button>
          <button @click="sendCmd('led_off')" class="btn danger">关闭LED</button>
        </div>
      </div>

      <!-- 操作按钮区域 -->
      <div class="refresh-box">
        <button @click="fetchData" :disabled="loading" class="btn primary">
          {{ loading ? "请求中..." : "手动刷新数据" }}
        </button>
        <button @click="resetAll" class="btn danger" style="margin-left:12px">清空曲线重置</button>
      </div>

      <!-- 网络/接口错误提示卡片 -->
      <div v-if="error && !data" class="card error-card">
        <h4>⚠️ 连接异常</h4>
        <p class="err-text">{{ error }}</p>
        <div class="solutions">
          <h5>排查方案</h5>
          <ol>
            <li>ESP32与电脑/手机连接同一局域网WiFi</li>
            <li>确认ESP32固件完整开启CORS跨域代码</li>
            <li>点击链接单独访问接口测试：<a :href="apiUrl" target="_blank">{{ apiUrl }}</a></li>
          </ol>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import { ESP32_DEFAULT_IP } from "@/config/env.js";
import { Chart, registerables } from "chart.js";
Chart.register(...registerables);

export default {
  name: "App",
  data() {
    return {
      loading: false,
      error: "",
      data: null,
      esp32IP: ESP32_DEFAULT_IP,

      chart: null,
      chartRef: null,
      dataList: [],
      timeLabelList: [],
      maxCache: 30,
      timer: null,
    };
  },
  computed: {
    apiUrl() {
      return `${this.esp32IP}/api/data`;
    },
    statusMessage() {
      if (this.loading) return "🔄 正在请求设备数据";
      if (this.data) return "✅ 设备正常通信中";
      if (this.error) return "❌ 设备连接失败";
      return "⏳ 等待首次连接";
    },
    statusClass() {
      if (this.loading) return "loading";
      if (this.data) return "connected";
      if (this.error) return "error-status";
      return "disconnected";
    },
  },
  mounted() {
    // 延时初始化图表，保证canvas DOM就绪
    setTimeout(() => this.initChart(), 200);
    // 页面打开先拉一次数据
    this.fetchData();
    // 4秒轮询稳定获取数据
    this.timer = setInterval(() => this.fetchData(), 4000);
  },
  beforeUnmount() {
    clearInterval(this.timer);
    if (this.chart) this.chart.destroy();
  },
  methods: {
    resetAll() {
      this.data = null;
      this.error = "";
      this.dataList = [];
      this.timeLabelList = [];
      if (this.chart) {
        this.chart.destroy();
        this.chart = null;
      }
    },

    initChart() {
      this.chartRef = this.$refs.chartRef;
      if (!this.chartRef) return;
      this.chart = new Chart(this.chartRef, {
        type: "line",
        data: {
          labels: this.timeLabelList,
          datasets: [
            {
              label: "温度 ℃",
              data: this.dataList.map(v => v.temperature),
              borderColor: "#007bff",
              backgroundColor: "rgba(0, 123, 255, 0.1)",
              fill: true,
              tension: 0.3,
              pointRadius: 2,
              yAxisID: "y",
            },
            {
              label: "湿度 %RH",
              data: this.dataList.map(v => v.humidity),
              borderColor: "#28a745",
              backgroundColor: "rgba(40, 167, 69, 0.1)",
              fill: true,
              tension: 0.3,
              pointRadius: 2,
              yAxisID: "y1",
            },
          ],
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
          animation: false,
          plugins: {
            legend: {
              position: "top",
              labels: { font: { size: 14 } }
            },
            tooltip: {
              mode: "index",
              intersect: false,
              callbacks: {
                title: items => "采集时间：" + items[0].label
              }
            }
          },
          scales: {
            x: {
              ticks: {
                maxTicksLimit: 10,
                maxRotation: 45,
                minRotation: 45,
                font: { size: 11 }
              },
              grid: { color: "rgba(0,0,0,0.05)" },
              title: { display: true, text: "采集时间", font: { size: 14 } }
            },
            y: {
              type: "linear",
              display: true,
              position: "left",
              title: { display: true, text: "温度 (℃)", color: "#007bff", font: { size: 13 } },
              grid: { drawOnChartArea: false, color: "rgba(0,123,255,0.1)" },
            },
            y1: {
              type: "linear",
              display: true,
              position: "right",
              title: { display: true, text: "湿度 (%RH)", color: "#28a745", font: { size: 13 } },
              grid: { drawOnChartArea: false, color: "rgba(40,167,69,0.1)" },
            },
          },
        },
      });
    },

    // 核心绘图修复：每次更新销毁重建，彻底解决v-show空白
    updateChart() {
      if (this.chart) {
        this.chart.destroy();
        this.chart = null;
      }
      this.initChart();
    },

    async fetchData() {
      if (this.loading) return;
      try {
        this.loading = true;
        this.error = "";
        const res = await fetch(this.apiUrl, {
          method: "GET",
          mode: "cors",
          headers: { Accept: "application/json" },
        });
        if (!res.ok) throw new Error(`HTTP ${res.status}`);
        const result = await res.json();
        if (typeof result.temperature !== "number" || typeof result.humidity !== "number") {
          throw new Error("返回数据格式异常");
        }

        const now = new Date();
        const timeStr = `${now.getHours()}:${now.getMinutes()}:${now.getSeconds()}`;
        this.timeLabelList.push(timeStr);
        this.dataList.push(result);

        if (this.timeLabelList.length > this.maxCache) {
          this.timeLabelList.shift();
          this.dataList.shift();
        }

        this.updateChart();
        this.data = result;
      } catch (err) {
        console.error(err);
        this.error = err.message;
        if (err.message.includes("Failed to fetch") || err.message.includes("CORS")) {
          this.error += " 跨域/设备离线";
        }
      } finally {
        this.loading = false;
      }
    },

    async sendCmd(cmd) {
      if (this.loading) return;
      try {
        const res = await fetch(`${this.esp32IP}/api/cmd`, {
          method: "POST",
          mode: "cors",
          headers: { "Content-Type": "text/plain" },
          body: cmd,
        });
        if (!res.ok) throw new Error("指令发送失败");
        alert("硬件指令下发成功");
        this.error = "";
      } catch (err) {
        this.error = "控制指令发送失败：" + err.message;
      }
    },
  },
};
</script>

<style scoped>
* {
  margin: 0;
  padding: 0;
  box-sizing: border-box;
}
body {
  font-family: "Microsoft Yahei", Arial, sans-serif;
  background: #f0f2f5;
  padding: 24px 12px;
}
#app {
  max-width: 900px;
  margin: 0 auto;
}
h1 {
  text-align: center;
  margin-bottom: 24px;
  color: #222;
  font-size: 26px;
}
h3 {
  margin-bottom: 16px;
  color: #333;
  font-size: 18px;
}
.card {
  background: #fff;
  border-radius: 12px;
  padding: 20px;
  margin-bottom: 20px;
  box-shadow: 0 2px 12px rgba(0, 0, 0, 0.08);
}
.ip-setting {
  display: flex;
  align-items: center;
  gap: 12px;
  flex-wrap: wrap;
}
.ip-setting input {
  flex: 1;
  min-width: 280px;
  padding: 10px 14px;
  border: 1px solid #ddd;
  border-radius: 6px;
  font-size: 15px;
}
.status {
  padding: 14px;
  border-radius: 8px;
  text-align: center;
  font-weight: bold;
  font-size: 16px;
}
.loading {
  background: #fff7e0;
  color: #d48806;
}
.connected {
  background: #e6f7ef;
  color: #009a61;
}
.error-status {
  background: #fee;
  color: #e53e3e;
}
.disconnected {
  background: #f6f7f9;
  color: #666;
}
.chart-wrap {
  width: 100%;
  height: 320px;
}
.data-row {
  display: flex;
  gap: 20px;
  justify-content: center;
}
.data-item {
  flex: 1;
  text-align: center;
  padding: 24px 10px;
  border-radius: 10px;
}
.temp {
  background: #e8f4ff;
}
.hum {
  background: #e6f7ef;
}
.data-item .label {
  font-size: 16px;
  color: #555;
  margin-bottom: 10px;
}
.data-item .value {
  font-size: 28px;
  font-weight: bold;
}
.temp .value {
  color: #007bff;
}
.hum .value {
  color: #28a745;
}
.btn-group {
  display: flex;
  gap: 14px;
  justify-content: center;
}
.refresh-box {
  text-align: center;
  margin: 10px 0 20px;
}
.btn {
  padding: 11px 22px;
  border: none;
  border-radius: 6px;
  font-size: 15px;
  cursor: pointer;
  transition: 0.2s;
}
.btn:disabled {
  background: #ccc !important;
  cursor: not-allowed;
}
.primary {
  background: #007bff;
  color: #fff;
}
.success {
  background: #28a745;
  color: #fff;
}
.danger {
  background: #dc3545;
  color: #fff;
}
.error-card {
  background: #fff2f2;
  border: 1px solid #ffcdcd;
}
.err-text {
  color: #c53030;
  margin: 8px 0 12px;
}
.solutions ol {
  padding-left: 20px;
  line-height: 1.7;
  color: #444;
}
a {
  color: #007bff;
}
</style>
