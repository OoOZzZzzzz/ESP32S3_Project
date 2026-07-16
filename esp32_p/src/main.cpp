#include "heardfile.h"





void setup()
{
    io_init();
    WiFi_Connect();
}

void loop()
{
    wifi_send();
    cmdProcess(19);
    delay(10);

}