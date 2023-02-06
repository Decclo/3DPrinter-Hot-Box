/*
 * @file main.cpp
 *
 * This code works as a controller for box that is to keep its internal
 * temperature stable.
 *
 * @author Hans V. Rasmussen <angdeclo@gmail.com>
 */


// ##########################################
// ##              Includes                ##
// ##########################################

#include <Arduino.h>
#include <OneWire.h>
#include <ArduinoJson.h>
#include <ArduPID.h>
#include <Streaming.h>

#include "config.h"
#include "sensor_util.h"



// ##########################################
// ##   Objects and Function Declaraions   ##
// ##########################################

// Function that takes over the main loop and gives full control of the device.
void debugMode(SensorUtil *sensors, DeviceAddress devices[], uint8_t dev_elements);



// ##########################################
// ##                Setup                 ##
// ##########################################

// =========== 
// Reconstructing main as found in 
// https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/main.cpp
// ===========

// Declared weak in Arduino.h to allow user redefinitions.
int atexit(void (* /*func*/ )()) { return 0; }

// Weak empty variant initialization function.
// May be redefined by variant files.
void initVariant() __attribute__((weak));
void initVariant() { }

void setupUSB() __attribute__((weak));
void setupUSB() { }

int main(void)
{
	init();

	initVariant();

#if defined(USBCON)
	USBDevice.attach();
#endif

// =========== 
// End of main reconstruction.
// ===========


    // ==== Serial ====
    Serial.begin(115200);
    Serial << "\nSerial initialized!\n\n\n";
    

    // ==== Local Variables ====
    bool relay_state = false;   // Bool which saves the current state of the relay.
    uint8_t fan_value = 220;    // Fan speed from 100 to 255. <100 is off.

    // Onewire initialization
    OneWire oneWire(10);  // on pin 10 (a 4.7K pull-up resistor is necessary).

    // Delay to allow us to turn on any logging scripts that we might have.
    delay(5000);


    // ==== OneWire sensors ====
    Serial << "Initializing OneWire sensors...\n";
    SensorUtil sensors(&oneWire);
    sensors.begin();

    struct
    {
        DeviceAddress inLower = SENSOR_INSIDE_LOWER;
        DeviceAddress inHigher = SENSOR_INSIDE_HIGHER;
        DeviceAddress outRef = SENSOR_OUTSIDE;
    } sensor;

    // Printing a short description of our sensors.
    Serial << "Device 0 Address: ";
    sensors.printAddress(sensor.inLower);
    Serial << "\nDevice 0 Description: Inside, low\n\n";

    Serial << "Device 1 Address: ";
    sensors.printAddress(sensor.inHigher);
    Serial << "\nDevice 1 Description: Inside, High\n\n";

    Serial << "Device 2 Address: ";
    sensors.printAddress(sensor.outRef);
    Serial << "\nDevice 2 Description: Outside, Reference\n\n\n";

    // set the resolution to 12 bit per device.
    // This results in <750ms reading time per sensor with 
    // a resolution of 0.0625 degrees Celcius.
    sensors.setResolution(sensor.inLower, TEMPERATURE_PRECISION);
    sensors.setResolution(sensor.inHigher, TEMPERATURE_PRECISION);
    sensors.setResolution(sensor.outRef, TEMPERATURE_PRECISION);

    Serial << "Device 0 Resolution: " << 
            sensors.getResolution(sensor.inLower) << endl;
    Serial << "Device 1 Resolution: " << 
            sensors.getResolution(sensor.inHigher) << endl;
    Serial << "Device 2 Resolution: " << 
            sensors.getResolution(sensor.outRef) << endl;

    Serial << "OneWire sensors initialized!" << "\n\n";

    // ==== Exponential moving average setup ====
    double tempC_avg = ((sensors.getTempC(sensor.inLower)
                    + sensors.getTempC(sensor.inHigher))/2);

    // ==== Fan Control ====
    // Initialize PWM pin as PWM output.
    pinMode(FAN_PWM_PIN, OUTPUT);

    // ==== Heat Relay Control ====
    // Initialize IO pin as output.
    pinMode(HEAT_RELAY_PIN, OUTPUT);

    // Turn on the fan. For now we'll just keep it constant.
    analogWrite(FAN_PWM_PIN, fan_value);

    // ==== Setup of PWM ====
    unsigned long currentTime = millis();
    unsigned long previousMeasurement = currentTime;
    unsigned long previousActuatorCycle = currentTime;
    unsigned long previousComputeCycle = currentTime;

    // ==== PID Controller Setup ====
    ArduPID myController;

    double setpoint = DESIRED_TEMPERATURE;
    double pwmDuty;
    myController.begin(&tempC_avg, &pwmDuty, &setpoint, PID_P, PID_I, PID_D);

    myController.setOutputLimits(0, 100);
    myController.setBias(25.0);
    //myController.setWindUpLimits(-10, 10); // Groth bounds for the integral term to prevent integral wind-up

    myController.start();

    // Printing a self-made crude JSON settings string.
    Serial << "{\"T_precision\":"     << (uint8_t)TEMPERATURE_PRECISION
            << ",\"MW_Alpha\":"       << (float)MOVING_WINDOW_ALPHA
            << ",\"PWM_Cycle\":"      << (uint16_t)PWM_CYCLE
            << ",\"PID_P\":"          << (float)PID_P
            << ",\"PID_I\":"          << (float)PID_I
            << ",\"PID_D\":"          << (float)PID_D
            << "}" << endl << endl;

    // Give advice on how to enter debugging more, and wait a second so the 
    // user will have chance to read it.
    Serial << "For debugMode please press anykey.\n\n\n";
    delay(1000);


// ##########################################
// ##              Main Loop               ##
// ##########################################

	for (;;)
    {
        currentTime = millis();
        // Check if the user wants to enter debugmode.
        if (Serial.available() > 0)
        {
            DeviceAddress devices[] = { *sensor.inLower, *sensor.inHigher, 
                                        *sensor.outRef};
            debugMode(&sensors, devices, 3);
        }

        // Read all the sensors, and find the mean of the inside sensors.
        sensors.requestTemperatures();
        float tempC_S0 = sensors.getTempC(sensor.inLower);
        float tempC_S1 = sensors.getTempC(sensor.inHigher);
        float tempC_S2 = sensors.getTempC(sensor.outRef);
        tempC_avg = (MOVING_WINDOW_ALPHA * ((tempC_S0 + tempC_S1)/2)) 
                    + ((1.0 - MOVING_WINDOW_ALPHA) * tempC_avg);

        // This is the PID. Contrary to the debug and sensor measurements, 
        // it is set to run once every PWM_CYCLE seconds.
        if (currentTime - previousComputeCycle >= (PWM_CYCLE/4))
        {
            Serial << "PID n+1 computation.\n";
            myController.compute();
            /*myController.debug(&Serial, "myController", 
                PRINT_INPUT    | // Can include or comment out any of these terms to print
                PRINT_OUTPUT   | // in the Serial plotter
                PRINT_SETPOINT |
                PRINT_BIAS     |
                PRINT_P        |
                PRINT_I        |
                PRINT_D
                );*/
            
            // Reset the timer, start next PWM cycle
            previousComputeCycle = currentTime;
        }

        // Actuator control. 
        // Use the owmDuty value from the PID to determine how much of the 
        // PWM cycle we want to turn on the heating element.
        if (currentTime - previousActuatorCycle >= PWM_CYCLE)
        {
            previousActuatorCycle = currentTime;
        }
        Serial << "Cycle time: " << currentTime - previousActuatorCycle << 
                ", PWM Duty: " << (PWM_CYCLE*(pwmDuty/100)) << endl;
        if (currentTime - previousActuatorCycle < (PWM_CYCLE*(pwmDuty/100)))
        {
            digitalWrite(HEAT_RELAY_PIN, HIGH);
            relay_state = true;
        }
        else
        {
            digitalWrite(HEAT_RELAY_PIN, LOW);
            relay_state = false;
        }
        

        // Debugging and logging - Creates a JSON document and sends it over Serial.
        // This piece of code is mostly autogenerated using the ArduinoJson 
        // Assistant: https://arduinojson.org/v6/assistant/
        // The following piece of JSON is used in step 2:
        /*
        {
        "ms": 1000000,
        "sensors": [
        {
        "sensor01": 00.000,
        "sensor02": 00.000,
        "sensor03": 00.000
        }
        ],
        "sensorMean": 00.000,
        "fan": 255,
        "heatingElement": 0
        }
        */
        StaticJsonDocument<96> doc;

        doc["ms"] = millis();
        JsonObject sensors_0 = doc["sensors"].createNestedObject();
        sensors_0["sensor00"] = tempC_S0;
        sensors_0["sensor01"] = tempC_S1;
        sensors_0["sensor02"] = tempC_S2;
        doc["sensorMean"] = tempC_avg;
        doc["fan"] = (uint8_t)pwmDuty;
        doc["heatingElement"] = relay_state;

        serializeJson(doc, Serial);
        Serial.println();

        // From 
        // https://github.com/arduino/ArduinoCore-avr/blob/master/cores/arduino/main.cpp
		if (serialEventRun) serialEventRun();
    }
    return 0;
}



// ##########################################
// ##               Functions              ##
// ##########################################

// Function that takes over the main loop and gives full control of the device.
void debugMode(SensorUtil *sensors, DeviceAddress devices[], uint8_t dev_elements)
{
    int debugmode_enabled = true;
    char rx_byte = 0;

    // Clear any input we might have
    if (Serial.available() > 0) 
    {
        rx_byte = Serial.read();
    }

    // Print the table of choices, read the input, and decide on what to do
    while (debugmode_enabled)
    {
        Serial.println("\nCommands:\n");
        Serial.println("\t1 - Read temperature\n");
        Serial.println("\t2 - Set PWM\n");
        Serial.println("\t3 - Change relay state to on\n");
        Serial.println("\t4 - Change relay state to off\n");
        Serial.println("\t5 - Scan for OneWire sensors\n");
        Serial.println("\t0 - Exit debugMode and return to normal operation\n");
        Serial.println("Choice: ");
        
        // Wait for user input
        while(Serial.available() == 0)
        {
            delay(100);
        }

        // Once the user has sent a command, read and save it. This does not 
        // actually need an if-stament, but I like making safeties.
        if (Serial.available() > 0)     // is a character available?
        {    
            rx_byte = Serial.read();      // get the character
            Serial.print(rx_byte);
            Serial.print("\n");

            // check whether the recieved byte was valid, and execute the command
            if (rx_byte == '1')
            {
                Serial << "\nReading temperature sensors. Press anykey to return.";
                bool read_temp = 1;
                while (read_temp)
                {
                    sensors->requestTemperatures();
                    for (uint8_t i = 0; i < dev_elements; i++)
                    {
                        sensors->printData(devices[i]);
                    }
                    Serial.println();

                    // Checks for a character so that we can break free of this 
                    // while loop
                    char rx_byte_alt = 0;
                    if (Serial.available() > 0)         // is a character available?
                    {    
                        rx_byte_alt = Serial.read();    // get the character
                        read_temp = 0;
                    }
                }
            }
            else if (rx_byte == '2')
            {
                char rx_byte_alt = 0;
                String rx_str_alt = "";
                bool read_str = 1;

                Serial.print("\nPlease input PWM between 100 to 255: ");

                while (read_str)
                {
                    if (Serial.available() > 0)         // is a character available?
                    {    
                        rx_byte_alt = Serial.read();    // get the character
                        
                        if (rx_byte_alt != '\n') 
                        {
                            // a character of the string was received
                            rx_str_alt += rx_byte_alt;
                            Serial.print(rx_byte_alt);
                        }
                        else 
                        {
                            Serial.print("\nSetting pwm to ");
                            Serial.println(rx_str_alt.toInt());
                            Serial.println();
                            Serial.println();
                            analogWrite(FAN_PWM_PIN, rx_str_alt.toInt());
                            read_str = 0;
                        }
                    }
                }
            }
            else if (rx_byte == '3')
            {
                Serial.println("\nTurning on the relay...");
                digitalWrite(HEAT_RELAY_PIN, HIGH);
            }
            else if (rx_byte == '4')
            {
                Serial.println("\nTurning off the relay...");
                digitalWrite(HEAT_RELAY_PIN, LOW);
            }
            else if (rx_byte == '5')
            {
                sensors->findDevices(ONE_WIRE_BUS);
                Serial.println("\n//\n// End oneWireSearch.ino //");
                Serial.println();
            }
            else if (rx_byte == '0')
            {
                debugmode_enabled = false;
            }
            else 
            {
                Serial.println("Command not recognized!");
            }
        }
    }

}