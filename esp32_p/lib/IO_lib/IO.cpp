#include "IO.h"

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

void io_init(void)
{
    pinMode(20,OUTPUT);
    pinMode(19,OUTPUT);

    digitalWrite(19,0);
    digitalWrite(20,1);
}