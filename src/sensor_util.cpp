#include "sensor_util.h"


void SensorUtil::printAddress(DeviceAddress deviceAddress)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
        Serial.print(" ");
    }
};


void SensorUtil::printTemperature(DeviceAddress deviceAddress)
{
    float tempC = getTempC(deviceAddress);
    if(tempC == DEVICE_DISCONNECTED_C) 
    {
        Serial.println("Error: Could not read temperature data");
        return;
    }
    Serial.print("Temp C: ");
    Serial.print(tempC);
    Serial.print(" Temp F: ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
};


void SensorUtil::printData(DeviceAddress deviceAddress)
{
    Serial.print("Device Address: ");
    printAddress(deviceAddress);
    Serial.print(" ");
    printTemperature(deviceAddress);
    Serial.println();
};


uint8_t SensorUtil::findDevices(int pin)
{
    OneWire ow(pin);

    uint8_t address[8];
    uint8_t count = 0;


    if (ow.search(address))
    {
        Serial.print("\nuint8_t pin");
        Serial.print(pin, DEC);
        Serial.println("[][8] = {");
        do {
            count++;
            Serial.println("  {");
            for (uint8_t i = 0; i < 8; i++)
            {
                Serial.print("0x");
                if (address[i] < 0x10) Serial.print("0");
                Serial.print(address[i], HEX);
                if (i < 7) Serial.print(", ");
            }
            Serial.println("  },");
        } while (ow.search(address));

        Serial.println("};");
        Serial.print("// nr devices found: ");
        Serial.println(count);
    }
    return count;
};