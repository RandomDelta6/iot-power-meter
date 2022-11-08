#ifndef _util_H_
#define _util_H_
#include <Arduino.h>


#define BEXT(a) a ? F("True") : F("False")

uint32_t getChipId()
{
#ifdef ESP8266
    return ESP.getChipId();
#elif ESP32
    uint32_t chipId = 0;
    for (int i = 0; i < 17; i = i + 8)
    {
        chipId |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
    }
    return chipId;
#endif
}

typedef struct packet_t
{
    double apparent_power;
    double active_power;
    double voltage_rms;
    double current_rms;
    double power_factor;
    double frequency;
    double energy;
    String timestamp;

} packet_t;
#endif
