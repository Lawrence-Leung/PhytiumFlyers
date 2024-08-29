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
# Description:  This file is for flashing files by ymodem

# Modify History: 
#  Ver   Who        Date         Changes
# ----- ------     --------    --------------------------------------
# 1.0   zhugengyu  2023-1-28   init commit

import os
import sys
import math
import time
import serial
import logging
import argparse
from ymodem.modem import Modem

class TaskProgressBar:
    def __init__(self):
        self.bar_width = 50
        self.last_task_name = ""
        self.current_task_start_time = -1

    def show(self, task_index, task_name, total, success, failed):
        if task_name != self.last_task_name:
            self.current_task_start_time = time.perf_counter()
            if self.last_task_name != "":
                print('\n', end="")
            self.last_task_name = task_name

        success_width = math.ceil(success * self.bar_width / total)

        a = "#" * success_width
        b = "." * (self.bar_width - success_width)
        progress = (success_width / self.bar_width) * 100
        cost = time.perf_counter() - self.current_task_start_time

        print(f"\r{task_index} - {task_name} {progress:.2f}% [{a}->{b}]{cost:.2f}s", end="")


parser = argparse.ArgumentParser()
parser.description='please enter two parameters <Serial-Port> <Baudrate> and <FlashBin> ...'
parser.add_argument("-p", "--port", help="serial port to connect board", type=str, default="/dev/ttyS13")
parser.add_argument("-b", "--baud", help="serial port baudrate", type=str, default="115200")
parser.add_argument("-f", "--file", help="export PATH for RTT",  type=str, default="./baremetal.bin")
args = parser.parse_args()

if __name__ == '__main__':
    logging.basicConfig(level=logging.INFO, format='%(message)s')

    serial_io = serial.Serial()
    serial_io.port = args.port
    serial_io.baudrate = args.baud
    serial_io.parity = "N"
    serial_io.bytesize = 8
    serial_io.stopbits = 1
    serial_io.timeout = 2

    try:
        serial_io.open()
    except Exception as e:
        raise Exception("Failed to open serial port!")
    
    def sender_read(size, timeout=3):
        serial_io.timeout = timeout
        return serial_io.read(size) or None

    def sender_write(data, timeout=3):
        serial_io.writeTimeout = timeout
        return serial_io.write(data)

    sender = Modem(sender_read, sender_write)

    os.chdir(sys.path[0])
    file_path1 = os.path.abspath(args.file)
    progress_bar = TaskProgressBar()
    sender.send([file_path1], callback=progress_bar.show)

    print('\r')
    serial_io.close()
