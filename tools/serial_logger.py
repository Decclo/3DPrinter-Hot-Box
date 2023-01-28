# 
# Script which listens on a given serial interface, and saves the traffic to
# a log file.
#
# Author:        Hans V. Rasmussen
# E-Mail:        angdeclo@gmail.com
# Creation Date: 14/Oct/2020

import argparse
import serial
import datetime
import time
import os

parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument("-p", "--port", help="tty device to log", default="/dev/ttyUSB0")
parser.add_argument("-b", "--baud", help="Baud rate", default=115200, type=int)
args = parser.parse_args()

outputFilePath = os.path.join(os.path.dirname(__file__),
                 datetime.datetime.now().strftime("%Y-%m-%dT%H.%M.%S") + ".bin")

with serial.Serial(args.port, args.baud) as device, open(outputFilePath, mode='wb') as outputFile:
    print("Logging started. Ctrl-C to stop.") 
    try:
        while True:
            time.sleep(1)
            outputFile.write((device.read(device.inWaiting())))
            outputFile.flush()
    except KeyboardInterrupt:
        print("Logging stopped")
