/*
 * @file sensor_util.h
 *
 *	This header includes a class used for project specific interfacing with the
 *  DS18B20 temperature sensors.
 *
 * @author Hans V. Rasmussen <angdeclo@gmail.com>
 */
#pragma once

#include <OneWire.h>
#include <DallasTemperature.h>
#include "config.h"


class SensorUtil : public DallasTemperature 
{
    public:
        using DallasTemperature::DallasTemperature;

        // Function to read out onewire addresses 
        void printAddress(DeviceAddress deviceAddress);

        // Function to print the temperature for a onewire device
        void printTemperature(DeviceAddress deviceAddress);

        // Function to print the address and temperature for a onewire device
        void printData(DeviceAddress deviceAddress);

        // Function which searches the OneWire bus and prints results
        uint8_t findDevices(int pin);

    protected:
    private:
};