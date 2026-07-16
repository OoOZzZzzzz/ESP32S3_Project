#ifndef _RECEIVE_H
#define _RECEIVE_H

#include <WebServer.h>
#include <Arduino.h>
#include "send.h"

extern String recvCmd;

void handleCmd();
void cmdProcess(uint8_t ledPin);

#endif // _RECEIVE_H