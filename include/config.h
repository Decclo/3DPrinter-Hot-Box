/*
 * @file config.h
 *
 *	Includes common configuration.
 *
 * @author Hans V. Rasmussen <angdeclo@gmail.com>
 */
#pragma once

#include <DallasTemperature.h>

// ##########################################
// ##               Settings               ##
// ##########################################
/*
 * This section defines the settings for the program, such as pin numbers and
 * operation modi.
 * For tuning the device, one would change DESIRED_TEMPERATURE and 
 * TEMPERATURE_DEVIATION. As the heating element is rather quick to heat up 
 * but needs a lot of time to cool down, the TEMPERATURE_OFFSET should be set 
 * to the difference between the wanted value and average measured value.
 * If the measured value is lower than the wanted, then TEMPERATURE_OFFSET 
 * needs to be positive.
*/

// Desired temperature in degrees celsius.
const double DESIRED_TEMPERATURE = 27;

// How much deviation (+-DESIRED_TEMPERATURE) is allowed before correcting.
const double TEMPERATURE_DEVIATION = 0.05;

// Offset from the desired temperature and average measured temperature, 
// found by using the Graph_Plotter.py
const double TEMPERATURE_OFFSET = -0.4;

// Exponential moving window for evening out spikes in the temperature
// measurement.
#define MOVING_WINDOW_ALPHA     0.166

// Pin for onewire interface. The sensors are installed in series running 
// without parasitic power.
#define ONE_WIRE_BUS            10

// Precision of the temperature sensor
// 09 equals a precision of 0.5C and measurement time <93.75ms
// 10 equals a precision of 0.25C and measurement time <187.5ms
// 11 equals a precision of 0.125C and measurement time <375ms
// 12 equals a precision of 0.0625C and measurement time <750ms
#define TEMPERATURE_PRECISION   12

// Pin on which to run the PWM for the motor controller in charge of the fan. 
// The H-bridge is hard-wired.
#define FAN_PWM                 9

// Pin controlling the relay in charge of the heating element.
#define HEAT_RELAY_PIN          11

// Addresses for the sensors
#define SENSOR_INSIDE_LOWER     {0x28, 0x69, 0xA1, 0x69, 0x35, 0x19, 0x01, 0x5D}
#define SENSOR_INSIDE_HIGHER    {0x28, 0xD5, 0x55, 0x6B, 0x35, 0x19, 0x01, 0x99}
#define SENSOR_OUTSIDE          {0x28, 0x96, 0x5F, 0xE3, 0x22, 0x20, 0x01, 0x62}