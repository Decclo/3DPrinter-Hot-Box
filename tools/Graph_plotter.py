# 
# Script which plots the JSON output of the 3DPrinter Hot-Box
#
# Author:        Hans V. Rasmussen
# E-Mail:        angdeclo@gmail.com
# Creation Date: 14/Oct/2020


from matplotlib import pyplot as plt
from itertools import tee
from colorama import Fore, Back, Style
import numpy as np
import argparse
import re
import json


''' The JSON should be in the following format:
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
'''


parser = argparse.ArgumentParser(description='Reads a logfile for the Temperated Box and creates relevant graphs.')
parser.add_argument('filepath', metavar='FILE', type=str,
                    help='Path to input file')
parser.add_argument('-o', '--output', type=str,
                    default="",
                    help='Save the extracted JSON to a file')
parser.add_argument('-t', '--target-temperature', type=int, dest='target_temp',
                    default=None,
                    help='Add the target temperature to the plot')
parser.add_argument('-ma', '--moving-average', type=int, dest='moving_average',
                    default=None,
                    help='Add a moving average of sensorMean to sensor plot')

args = parser.parse_args()



#########################
## Functions & Classes ##
#########################

# Defining a rolling window function
# source: https://stackoverflow.com/a/6822907
def window(iterable, size):
    iters = tee(iterable, size)
    for i in range(1, size):
        for each in iters[i:]:
            next(each, None)
    return zip(*iters)  



###################
## Preprocessing ##
###################

# Load the logfile
with open(args.filepath) as f:
    logfile = f.read().splitlines()


# Filter logfile so we only get the JSON data
logfile = [line for line in logfile
        if re.search(r'^\{.*\}$', line)]


# If the --output option was specified, save the extracted JSON to the specified file
print(Fore.BLUE + '[INFO] ' + Style.RESET_ALL + 'Writing output to ' + args.output)
try:
    if(args.output != ""):
        f = open(args.output, "w")
        for line in logfile:
            f.write(line)
            f.write("\n")
        f.close()
except:
    print(Fore.YELLOW + '[WARNING] ' + Style.RESET_ALL + 'Cannot save extracted JSON to specified location')    


# Iterate through the json data and pick out the values we want
data_raw = [[],[],[],[],[],[],[]]
for line in logfile:
    data = json.loads(line)
    data_raw[0].append(data['ms']/60000)
    data_raw[1].append(data['sensors'][0]['sensor00'])
    data_raw[2].append(data['sensors'][0]['sensor01'])
    data_raw[3].append(data['sensors'][0]['sensor02'])
    data_raw[4].append(data['sensorMean'])
    data_raw[5].append(data['fan'] / 255)
    data_raw[6].append(data['heatingElement'])


if (args.moving_average != None):
    # Finding the average of the last x values of sensorMean
    window_size = args.moving_average
    moving_average = []
    for i in range(window_size - 1):
        moving_average.append(None)
    for each in window(data_raw[4], window_size):
        moving_average.append(sum(each)/len(each))


if (args.target_temp != None):
    Temp_wanted_list = []
    for i in data_raw[4]:
        Temp_wanted_list.append(args.target_temp)
    

# Find the average of sensorMean and make a list for plotting
sensorMean_average = sum(data_raw[4])/len(data_raw[4])
sensorMean_average_list = []
for i in data_raw[4]:
    sensorMean_average_list.append(sensorMean_average)
print(Fore.BLUE + '[INFO] ' + Style.RESET_ALL + 'The average of sensorMean is: ', sensorMean_average)



#####################
## Construct plots ##
#####################

# Make a figure that consists of two plots, an upper for sensors, and lower for actuators
# Let's make the upper one 3/4 of the rows, and the lower 1/4
a1 = plt.subplot2grid((4,2),(0,0),rowspan = 3, colspan = 2)
a2 = plt.subplot2grid((4,2),(3,0),colspan = 2)


# The upper plot contains the sensor values, as well as basic analytics
if (args.target_temp != None):
    a1.plot(data_raw[0], Temp_wanted_list, color='green', label=('Target temperature'), linestyle = 'dotted')
a1.plot(data_raw[0], data_raw[1], color='orange',  label='Sensor00 (lower)')
a1.plot(data_raw[0], data_raw[2], color='red',  label='Sensor01 (upper)')
a1.plot(data_raw[0], data_raw[4], color='magenta',  label='Sensor inside Mean', linestyle = 'dashed')
if (args.moving_average != None):     # Only draw the moving average if moving_average is defined
    try:        # Try to draw the moving average, if this gives and error, then it is most likely caused by a too large window
        a1.plot(data_raw[0], moving_average, label=('Moving average, resolution: ' + str(window_size)))
    except:
        print(Fore.YELLOW + '[WARNING] ' + Style.RESET_ALL + 'Not enough data to create moving average with specified window')
a1.plot(data_raw[0], sensorMean_average_list, color='magenta', label=('Average: ' + str(round(sensorMean_average, 2))), linestyle = 'dotted')
a1.plot(data_raw[0], data_raw[3], color='darkgreen',  label='sensor02 (outside)')
a1.set_title('Sensors')
a1.set_ylabel("Temperature [C]")
#a1.set_ylim([-0.1, 40.1])
a1.grid()
a1.legend()


# The lower plot contains the actuators, scaled to a 0 to 1 range
a2.plot(data_raw[0], data_raw[5], label='Fan (scaled to 0:1)')
a2.plot(data_raw[0], data_raw[6], label='Heating element (digital)')
a2.set_title('Actuators')
a2.set_xlabel("Minutes since start")
a2.set_ylabel("Value")
a2.set_ylim([-0.1, 1.1])
a2.grid()
a2.legend()


# Visualize the plot
plt.tight_layout()
plt.show()