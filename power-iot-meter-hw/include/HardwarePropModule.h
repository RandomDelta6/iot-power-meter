/**
 * @file HardwarePropModule.h
 * @author @anubhav666 @arnabdas01
 * @brief This will use PZEM004T module and UART
 * @version 0.1
 * @date 2022-06-06
 *
 * @copyright Copyright (c) 2022
 *
 */
#ifndef __HardwarePropModule_H__
#define __HardwarePropModule_H__
#include "Arduino.h"
#include "util.h"
#include "PZEM004Tv30.h"
#include "Timing.h"

/**
 * @brief Pin definitions
 * TODO: To be decided Later
 */

#if !defined(PZEM_RX_PIN) && !defined(PZEM_TX_PIN)
#define PZEM_RX_PIN 16
#define PZEM_TX_PIN 17
#endif

#define PZEM_SERIAL Serial2
#define CONSOLE_SERIAL Serial
#define THRESHOLD_MAINS 80

PZEM004Tv30 pzem(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN);

void attach()
{
    // pzem.resetEnergy();
}

bool fetchSensorData(packet_t *data)
{
    // FIXME: detection of power down
    data->voltage_rms = pzem.voltage();
    data->current_rms = pzem.current();
    data->apparent_power = data->voltage_rms * data->current_rms;
    data->active_power = pzem.power();
    data->power_factor = pzem.pf();
    data->energy = pzem.energy();
    data->frequency = pzem.frequency();
    // add timestamp
    data->timestamp = getTimestamp();

    if (isnan(data->voltage_rms) ||
        isnan(data->current_rms) ||
        isnan(data->active_power) ||
        isnan(data->energy) ||
        isnan(data->frequency) ||
        isnan(data->power_factor))
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool fetchSensorDataTest(packet_t *data)
{
    data->current_rms = 5.0;
    data->voltage_rms = 220.0;
    data->apparent_power = data->voltage_rms * data->current_rms;
    data->active_power = data->apparent_power * 0.6;
    data->power_factor = 0.6;
    data->frequency = 50;
    // add timestamp
    data->timestamp = getTimestamp();

    return true;
}

bool checkPowerCut()
{
    return pzem.voltage() <= THRESHOLD_MAINS;
}
#endif