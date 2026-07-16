#include "receive.h"

String recvCmd = ""; // 前端指令缓存变量


void handleCmd()
{
    setCORS();
    if (server.hasArg("plain"))
    {
        recvCmd = server.arg("plain");
        Serial0.print("收到前端下发指令：");
        Serial0.println(recvCmd);
    }
    server.send(200, "text/plain", "ok");
}

void cmdProcess(uint8_t ledPin)
{
    if (recvCmd == "") return;

    if (recvCmd == "led_on")
    {
        digitalWrite(ledPin, LOW);
    }
    else if (recvCmd == "led_off")
    {
        digitalWrite(ledPin, HIGH);
    }

    recvCmd = "";
}
