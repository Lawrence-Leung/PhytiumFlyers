#! /usr/bin/env python3
from ymodem.ymodem import *
import serial

class ymodem_receiver(YModem):
    def __init__(self,receiver_getc,receiver_putc,file_path):
        super(ymodem_receiver,self).__init__(receiver_getc,receiver_putc)
        self.file_path = file_path
    
    def run(self):
        received = self.recv_file(self.file_path)
        print(received)
        return 0

