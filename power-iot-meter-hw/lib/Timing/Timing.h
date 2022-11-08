#ifndef __TIMING_H__
#define __TIMING_H__
#include "Arduino.h"

/**
 * @file Timing.h
 * @author @anindyamitra15 (you@domain.com)
 * @brief
 * Fetch time from NTC, according to Timezone
 * Store time in internal RTC and tick
 * fetch timestamp string from RTC time
 *
 * @version 0.1
 * @date 2022-05-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <Arduino.h>
#include "Time.h"
#include "FileOperation.h"

#define NTP_SERVER "pool.ntp.org"
#define POWER_CUT "/lastPowerCut"

const long gmtOffset_sec = 19800; // +5:30 hrs
const int daylightOffset_sec = 0;

void printLocalTime();
void set_time_from_ntp();
String getTimestamp();
void powerCutTimeStore(fs::SPIFFSFS &filesystem);
String powerCutTimeFetch(fs::SPIFFSFS &filesystem);


void set_time_from_ntp()
{
    configTime(gmtOffset_sec, daylightOffset_sec, NTP_SERVER);
    printLocalTime();
}

String getTimestamp()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return "";
    }
    char buffer[80];
    //YYYY-MM-DD HH:MM
    strftime (buffer, 80, "%Y-%m-%d %H:%M", &timeinfo);
    return buffer;
}

void powerCutTimeStore(fs::SPIFFSFS &filesystem)
{
    String time = getTimestamp();
    FileOperation::writeFile(filesystem, POWER_CUT, time);
}

String powerCutTimeFetch(fs::SPIFFSFS &filesystem)
{
    return FileOperation::readFile(filesystem, POWER_CUT);
}

void printLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
    Serial.print("Day of week: ");
    Serial.println(&timeinfo, "%A");
    Serial.print("Month: ");
    Serial.println(&timeinfo, "%B");
    Serial.print("Day of Month: ");
    Serial.println(&timeinfo, "%d");
    Serial.print("Year: ");
    Serial.println(&timeinfo, "%Y");
    Serial.print("Hour: ");
    Serial.println(&timeinfo, "%H");
    Serial.print("Hour (12 hour format): ");
    Serial.println(&timeinfo, "%I");
    Serial.print("Minute: ");
    Serial.println(&timeinfo, "%M");
    Serial.print("Second: ");
    Serial.println(&timeinfo, "%S");

    Serial.println("Time variables");
    char timeHour[3];
    strftime(timeHour, 3, "%H", &timeinfo);
    Serial.println(timeHour);
    char timeWeekDay[10];
    strftime(timeWeekDay, 10, "%A", &timeinfo);
    Serial.println(timeWeekDay);
    Serial.println();
}

#endif