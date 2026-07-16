#ifndef _SEND_H
#define _SEND_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "IO.h"

extern WebServer server;
extern float temperature;
extern float humidity;
extern uint16_t Time;


void WiFi_Connect();
void wifi_send();
void setCORS();



#endif // SEND_H