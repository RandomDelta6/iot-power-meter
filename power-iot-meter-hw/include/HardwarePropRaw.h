/**
 * @file HardwarePropRaw.h
 * @author @anindyamitra15 @RandomDelta6
 * @brief This will be measuring the voltage and current and deriving power from that
 * @version 0.1
 * @date 2022-06-06
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __HardwarePropRaw_H__
#define __HardwarePropRaw_H__
#include "Arduino.h"
#include "util.h"
#include "ZMPT101B.h"
#include "ACS712.h"

/**
 * @brief Pin definitions
 * TODO: To be decided Later
 */
#define VOLTAGE_PIN 36 // ANALOG PIN
#define CURRENT_PIN 39 // ANALOG PIN

#define POWER_FACTOR 0.8              // basically cos(36.8698)
#define REACTIVE_POWER_MULTIPLIER 0.6 // basically sin(36.8698)

ZMPT101B voltageSensor(VOLTAGE_PIN);
ACS712 currentSensor(CURRENT_PIN, 5.0, 1023, 100);

void attach()
{
    pinMode(VOLTAGE_PIN, INPUT);
    pinMode(CURRENT_PIN, INPUT);
    Serial.println("Calibrating... Ensure that no current flows through the sensor at this moment");
    voltageSensor.calibrate();
    currentSensor.autoMidPoint();
    delay(100);
    Serial.println("Done!");
}

void fetchSensorData(packet_t *data)
{
    // FIXME: implementation required after deciding on the sensors
}

void fetchSensorDataTest(packet_t *data)
{
    data->current_rms = data->current_rms + 1.073;
    if (data->current_rms > 20)
        data->current_rms = 2;
    data->voltage_rms = 220.0 + random(-300, 300) / 100.00;
    data->apparent_power = data->voltage_rms * data->current_rms;
    data->active_power = data->apparent_power * POWER_FACTOR;
}

#endif