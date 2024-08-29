#! /usr/bin/env python3
# Copyright : (C) 2022 Phytium Information Technology, Inc. 
# All Rights Reserved.
 
# This program is OPEN SOURCE software: you can redistribute it and/or modify it  
# under the terms of the Phytium Public License as published by the Phytium Technology Co.,Ltd,  
# either version 1.0 of the License, or (at your option) any later version. 
 
# This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;  
# without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the Phytium Public License for more details. 
 
# FilePath: install.py
# Date: 2023-01-28 08:19:30
# LastEditTime: 2023-01-28 08:19:30
# Description:  This file is for send cmd and recv response by serial

# Modify History: 
#  Ver   Who        Date         Changes
# ----- ------     --------    --------------------------------------
# 1.0   zhugengyu  2023-1-28   init commit

import os
import sys
import math
import time
import serial
import re
import logging
import argparse
from threading import Event
from ymodem.modem import Modem

LOG_FORMAT = "%(asctime)s - %(levelname)s - %(message)s"
DATE_FORMAT = "%m/%d/%Y %H:%M:%S %p"

def serial_tx(serial_port, send_cmd):
    #check to see if the connection is open
    if serial_port.isOpen():
        serial_port.write(bytes('\r', encoding='utf-8'))

        # while true, send 'r' char to receive the output from the sensor
        serial_port.write(bytes('{} \r'.format(send_cmd), encoding='utf-8'))
        return True
    else:
        return False

def serial_rx(serial_port):
    cmd_echo = ''
    title_passed = 0

    #check to see if the connection is open
    if serial_port.isOpen():
        # loop to print all the output lines
        while True:
            #read the output and store it in a variable
            response = serial_port.readline()

            #Check to see if all the lines of the output are printed
            if (len(response)) > 0:
                cmd_resp = (response.rstrip()).decode(encoding='utf-8', errors='replace')
                cmd_echo += cmd_resp + '\n'
            else:
                break

    return cmd_echo

def check_response(cmd, cmd_echo, exp_pattern):
    result = re.search(exp_pattern, cmd_echo)
    if result:
        print('{} Passed !!!'.format(cmd))
        logging.info('{} Passed !!!'.format(cmd))
        return True
    else:
        print('{} Failed !!!'.format(cmd))
        logging.error('{} Failed !!!'.format(cmd))
        return False

def serial_tx_then_check_response(serial_port, send_cmd, exp_pattern, exp_time):
    # send command string to serial
    if not serial_tx(serial_port, send_cmd):
        return False

    time.sleep(exp_time)

	# get command response from serial
    cmd_echo = serial_rx(serial_port)

	# log the command and response
    logging.info("==> {}".format(send_cmd))
    logging.info("<== {}".format(cmd_echo))

	# check if command response follow expected pattern
    return check_response(send_cmd, cmd_echo, exp_pattern)

parser = argparse.ArgumentParser()
parser.description='please enter two parameters <Serial-Port> <Baudrate> and <FlashBin> ...'
parser.add_argument("-p", "--port", help="serial port to connect board", type=str, default='/dev/ttyS13')
parser.add_argument("-b", "--baud", help="serial port baudrate", type=int, default=115200)
parser.add_argument("-c", "--command", help="command send by serial port",  type=str, default="version")
parser.add_argument("-r", "--response", help="response pattern expect to reeive from serial port",  type=str, default="U-Boot 2022.01")
parser.add_argument("-t", "--time", help="command execute time in second",  type=str, default="0.5")
parser.add_argument("-l", "--log", help="logging files", type=str, default='./test.log')
args = parser.parse_args()

if __name__ == "__main__":
    # Setup logging to text file
    logging.basicConfig(filename=args.log, level=logging.DEBUG, format=LOG_FORMAT, datefmt=DATE_FORMAT)

    # Open a serial connection
    new_port = serial.Serial(port=args.port, baudrate=args.baud, parity="N", bytesize=8, stopbits=1, timeout=1)
    if not new_port.isOpen():
        print('Failed to open serial {}'.format(args.port))
        exit(1)

    comd_item = args.command.split(',')
    resp_item = args.response.split(',')
    time_item = args.time.split(',')

    if len(comd_item) != len(resp_item):
        print('Invalid input')
        exit(2)

    # Function call
    print('\r********************************')
    for idx in range(len(comd_item)):
        command = comd_item[idx].strip()
        response = resp_item[idx].strip()
        exc_time_s = time_item[idx].strip()

        if len(exc_time_s) > 0:
            exc_time = float(exc_time_s)
        else:
            exc_time = 0.5 # by default 0.5 second

        print('{}'.format(command))
        if len(response) > 0:
            # write to serial and get response
            success = serial_tx_then_check_response(new_port, command, response, exc_time)
        else:
            # just write to serial and do not wait response
            success = serial_tx(new_port, command)
            time.sleep(exc_time)

        if not success:
            break        

    print('********************************')
    # close the serial connection after the function is over
    new_port.close()