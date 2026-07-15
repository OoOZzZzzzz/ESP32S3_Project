#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>


#define MY_printf               printf
//#define dubge                 //!!!串口调试，打印动态地址

/*此处注释，默认为动态地址，网页中输入连接上的动态地址即可*/

//#define STATICIP              //!!!静态地址宏定义选择十分使用

//!!! 此处必须改成自己的WiFi或热点相关的
const char *SSID     = "你的热点名称";
const char *PASSWORD = "你的热点密码";


WebServer server(80);
int requestCount = 0;

#ifdef STATICIP         //!!!如果使用静态地址，必须改成自己的WiFi或热点相关的
//!!!以下只是例子

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
//     return "{\"temperature\": " + String(random(20, 30)) + 
//             ", \"humidity\": " + String(random(40, 80)) + "}";
// }

//!!! 这里为了方便测试，直接返回固定的温湿度数据，实际使用中可以根据传感器读取的值来返回
//!!! 后续可改为ESP32-S3读取传感器数据，返回JSON格式的温湿度数据
String getSensorData() {
    return "{\"temperature\": " + String(100) + ", \"humidity\": " + String(105) + "}";
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


// IO指示灯配置
void pin_level(uint8_t pin)
{
    if(digitalRead(pin) == 1)
    {
        digitalWrite(pin,0);
    }
    else
    {
        digitalWrite(pin,1);
    }
}

// IO初始化
void io_init(void)
{
    pinMode(20,OUTPUT);
    pinMode(19,OUTPUT);

    digitalWrite(19,0);
    digitalWrite(20,1);
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

    server.onNotFound([]() {
        setCORS();
        server.send(404, "text/plain", "Not Found");
    });

    server.begin();


#ifdef dubge
    Serial0.printf("HTTP server started with CORS support");
    MY_printf("\nWiFi连接成功");
    MY_printf("网关: %s\n", WiFi.gatewayIP().toString().c_str());
    MY_printf("\nWeb服务器已启动");
    MY_printf("访问地址: http://%s\n", WiFi.localIP().toString().c_str());
    MY_printf("\n==================================\n");
#endif



}

void setup()
{
    io_init();
    WiFi_Connect();
}

void loop()
{
    server.handleClient();

    delay(10);
}