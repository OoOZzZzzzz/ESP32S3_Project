#include "send.h"
#include "receive.h"


#define MY_printf               printf

//!!!串口调试，打印动态地址
#define dubge

/*此处注释，默认为动态地址，网页中输入连接上的动态地址即可*/
//!!!静态地址宏定义选择十分使用
//#define STATICIP

//!!! 此处必须改成自己的WiFi或热点相关的
const char *SSID     = "你的热点名称";
const char *PASSWORD = "你的热点密码";


WebServer server(80);
int requestCount = 0;

//!!!如果使用静态地址，必须改成自己的WiFi或热点相关的
#ifdef STATICIP

IPAddress local_IP(192, 168, 248, 100);         //静态地址
IPAddress gateway(192, 168, 248, 122);          //网关
IPAddress subnet(255, 255, 255, 0);             //子网掩码
#endif

// 设置 CORS 头部
void setCORS() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
    server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
}


// String getSensorData() {
//     return "{\"temperature\": " + String(100) + ", \"humidity\": " + String(105) + "}";
// }

//!!! 这里为了方便测试，模拟正弦温湿度数据，实际使用中可以根据传感器读取的值来返回
//!!! 后续可改为ESP32-S3读取传感器数据，返回JSON格式的温湿度数据

float temperature = 25.0f;
float humidity = 50.0f;
uint16_t Time = 0;

String getSensorData() {

    // 正弦模拟温度 22 ~ 28℃
    temperature = 25 + 3 * sin(Time * 0.1);
    // 湿度 40 ~ 60%RH
    humidity = 50 + 10 * sin(Time * 0.15);

    Time+=5;

    return "{\"temperature\": " + String(temperature,1) + ", \"humidity\": " + String(humidity,1) + "}";
}


void handleRoot() {
    setCORS();
    server.send(200, "text/plain", "ESP32-S3 Server is Running!");
}

void handleAPIData() {
    setCORS();
    server.send(200, "application/json", getSensorData());
}

// 专门处理 OPTIONS 预检请求
void handleOptions() {
    setCORS();
    server.send(200);
}

//    WiFi的初始化和连接
void WiFi_Connect()
{
    Serial0.begin(115200);
    //SerialPort.begin(115200, SERIAL_8N1, 44, 43);

#ifdef dubge
    MY_printf("\n\nESP32-WROOM-32 Web服务器启动");
    MY_printf("\n==================================\n");
    Serial0.printf("连接WiFi...\n");
#endif

#ifdef STATICIP
    WiFi.config(local_IP, gateway, subnet);
#endif

    WiFi.begin(SSID, PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    { //这里是阻塞程序，直到连接成功
        delay(300);
        pin_level(20);
        pin_level(19);
#ifdef dubge
        Serial0.printf(".");//不知道为何不起作用
#endif
    }

    // 设置路由
    server.on("/", handleRoot);
    server.on("/api/data", HTTP_GET, handleAPIData);
    server.on("/api/data", HTTP_OPTIONS, handleOptions); // 预检请求

    server.on("/api/cmd", HTTP_POST, handleCmd);


    server.onNotFound([]() {
        setCORS();
        server.send(404, "text/plain", "Not Found");
    });

    server.begin();


#ifdef dubge
    Serial0.printf("HTTP server started with CORS support");
    MY_printf("\nWiFi连接成功");
    MY_printf("网关: %s\n", WiFi.gatewayIP().toString().c_str());
    MY_printf("\nWeb服务器已启动OK");
    MY_printf("访问地址: http://%s\n", WiFi.localIP().toString().c_str());
    MY_printf("\n==================================\n");
#endif

}

void wifi_send()
{
    server.handleClient();
}