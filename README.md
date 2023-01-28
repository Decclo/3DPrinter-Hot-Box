An enclosure meant to house a resin 3D printer, holding the temperature at a stable value to ensure the best results. The box allows for the printer inside to be placed in a cold and even freezing environment, such as a shed or porch, to keep potential toxic fumes away from the usual living area.

Visual Studio Code from Microsoft with PlatformIO was used as IDE, though the project can be run using PlatformIO Core with no IDE.

# Components needed:
1. An isolated box, in this case with a wooden frame with a double layered plexiglass front panel, removable top, and styrofoam on the inside.
2. [A power supply which covers all our power requirements](https://www.amazon.de/gp/product/B07BLR16PB/ref=ppx_yo_dt_b_asin_title_o00_s00?ie=UTF8&psc=1).
3. [Arduino Nano](https://www.banggood.com/Geekcreit-ATmega328P-Nano-V3-Module-Improved-Version-No-Cable-Development-Board-Geekcreit-for-Arduino-products-that-work-with-official-Arduino-boards-p-959231.html?cur_warehouse=CN&rmmds=search) microcontroller.
4. A heating unit, in this case a [150W 12V DC PTC Fan Heater](https://www.banggood.com/150W-12V-DC-PTC-Fan-Heater-Constant-Temperature-With-Connection-Cable-p-1120251.html?cur_warehouse=CN&rmmds=search) with integrated fan.
5. A motor controller to take care of the fan, such as the [L298N Dual H bridge motor driver](https://www.banggood.com/Wholesale-L298N-Dual-H-Bridge-Stepper-Motor-Driver-Board-p-42826.html?cur_warehouse=CN&rmmds=search), which also features a 5v voltage regulator.
6. A [relay](https://www.banggood.com/3Pcs-5V-Relay-5-12V-TTL-Signal-1-Channel-Module-High-Level-Expansion-Board-p-1178211.html?cur_warehouse=CN&rmmds=search) to turn the heating element on and off.
7. We'll also need some temperature sensors for feedback, my personal favorite being the [DS18B20 OneWire digital temperature probes](https://www.banggood.com/DS18B20-Waterproof-Digital-Temperature-Temp-Sensor-Probe-1M-2M-3M-5M-10M-15M-p-1211828.html?cur_warehouse=CN&ID=45763&rmmds=search).
8. At last, we'll need some assorted cables and soldering skills. A power socket rail would also be neat so we have easy power access for the printer.

# Assembly
The box really depends on your tastes and needs. We made the box of plywood and isolated it using polysterene. On the front we installed handles, a double-layer plexiglass window, and a closing mechanism to hold it in place. The top was made to be removable, so it would be easier to handle the 3D printer. This leaves the sides, bottom, and back as one module.

We mounted the electronics on a backplate on the inside as to protect them from the outside environment. This is usually considered a bad practice, but we deemed it acceptable as the box is meant to house electronics and likely will never exceed 30 degrees celcius anyway.


![signal-2021-03-24-194901_002](https://user-images.githubusercontent.com/25606661/112367431-30895d80-8cda-11eb-955a-b8b967ac7dbb.jpeg)
![signal-2021-03-24-194901_003](https://user-images.githubusercontent.com/25606661/112367433-3121f400-8cda-11eb-8844-0a34f85c6d95.jpeg)

The power supply is connected in the following way:
```
Power supply (12v output)
|-> L298N motor driver
| |-> Fan (from motor A out)
| |-> Relay (from L298N 5v relugated output)
| |-> DS18B20 temperature sensors (from L298N 5v relugated output)
|-> Arduino (VIN)
|-> Relay (switch)
| |-> Heating unit
```

While the positive rail can be wired as stated above, there are no limits for the ground rail. Preferable every component would connect to every other components ground, but since this would be cumbersome, the components which give signals to other components have an additional ground wire along the signal wire. This results in a mesh-like ground rail.

It should also be noted that the 12v input connected to the Arduino Nano's VIN is made with a removable connector, as to be able to change the power source from the power supply and to a computer to be able to upload code to the arduino without risking the power flowing the wrong way through the device and potentially burning the computers' USB port.

**Always make sure the arduino's vin pin is not connected when connecting it to the computer using USB!**

The devices are connected to the Arduino Nano as follows:
  - Pin 10: Onewire (additionally has 4.7kOhm pullup resistor to arduino's 5v rail)  
  - Pin 09: L298N Motor driver PWM for fan  
  - Pin 11: Digital output for relay controlling heating element

Note that the pins referred to are the ones marked with purple ont he image below.

![Arduino Nano Pinout from Wikipedia.com](https://upload.wikimedia.org/wikipedia/commons/e/e4/Arduino-nano-pinout.png)
*From wikipedia.com*

![signal-2021-03-24-194901_001](https://user-images.githubusercontent.com/25606661/112367429-2ff0c700-8cda-11eb-82a0-a78cce96e969.jpeg)
![signal-2021-03-27-220648](https://user-images.githubusercontent.com/25606661/112734892-d3471380-8f48-11eb-9a5a-19a19f92f553.jpeg)

  
# Repository Setup
This repository was made with VSCode in mind, though platformIO has been used with the core in mind, meaning that compiling and flashing can be done using `pio` from the terminal.


  1. Download this repository if you haven't done so already.
  2. Initialize the required modules by running `git submodule init && git submodule update` in either your local terminal or VSCode's terminal (requires you to be in the repository directory)
  3. Either install PlatformIO in VSCode as an extension, or install the [PlatformIO Core](https://docs.platformio.org/en/stable/core/installation/methods/index.html) using any available method.

# Compiling and Flashing
## VSCode
You can compile the code from VSCode by pressing the 'PlatformIO:Build' button on the blue bar at the bottom. Likewise, flashing can be done by pressing the right-pointing-arrow next to the ckeckmark. Board and uplaod preferences can be changed in `platformio.ini`.


## PlatformIO Core
To use the Pio Core, first navigate to the workdirectory.

Compiling:
```
pio run -e pro16MHzatmega328
```

Uploading:
```
pio run -e pro16MHzatmega328 --target upload
# or:
pio run -e pro16MHzatmega328 --target upload --upload-port /dev/ttyUSB0
```

Clean build:
```
pio run -e pro16MHzatmega328 --target clean
```

Monitor:
```
pio device monitor
```

As with VSCode, additional boards and preferences can be set in the file called `platformio.ini`.

# Tests
Two tests were made. 

## Test 01 - Empty Box
The first test was without the printer inside, meaning that only the raspberry pi overseeing the microcontroller and the heating system are producing heat. As one can see in the plots below, the heating element is only in use around 10% of the time with and outside temperature of <10C. The temperature inside the box is varying with around +-1C (from 26C to 28C) with a mean of 27.07C. This is using a desired temperature of 27C with an offset of -0.4C and an allowed deviation of 0.2C.
![2021-03-27T10_empty_test](https://user-images.githubusercontent.com/25606661/112734271-fc65a500-8f44-11eb-9e64-c7fa682b964e.png)


## Test 02 - Live run with printer inside
The second test was with the printer inside, printing a batch of objects. The second test was using the same settings as Test 01, but as can be seen in the image below the printer was producing enough heat to prevent the box from cooling down, meaning that the temperature control only was used for bringing the box to operational temperature but not to keep it hot. This raises concern as the box might oberheat with higher outside temperatures or if the printer or other components might produce more heat. Possible solutions are to partly open the door to let some cold air in, or perhaps introducing some kind of cooling mechanism.
It should be noted that the sudden spikes at ~6000s and ~9000s are caused by opening the box to check the print, with the small spike at ~600s being the act of opening the box and pressing the start button.
![2021-03-27T14_First_printer_test](https://user-images.githubusercontent.com/25606661/112734319-4f3f5c80-8f45-11eb-8d3c-6685207f6c76.png)



