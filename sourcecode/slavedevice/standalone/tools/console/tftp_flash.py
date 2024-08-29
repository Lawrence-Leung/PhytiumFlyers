#!/usr/bin/env python3
import time
import serial
import os


class tftp_flash(object):

    no_elf_bootup_rx = [b'ft2004#', b'FT2004#', b'd2000#', b'D2000#', b'e2000#', b'E2000#'] 
    elf_bootup_rx = [b'phytium:/$']

    def __init__(self,port, baud = 115200):
        self.console = serial.Serial(
            port = port,
            baudrate = baud,
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.EIGHTBITS,
            timeout=1
        )
        
    def send_cmd(self, tx, rx, wait, wait_time=1):
        self.console.write(tx)
        self.console.write(b'\r\r\r\r\r')
        if wait:
            time.sleep(wait_time)
        info = self.console.read(100000)
        try:
            print(info.decode('utf-8'))
        except:
            pass
        if self.is_title_of(info, rx):
            # print("{} success".format(tx))
            return True
        else:
            # print("{} failed".format(tx))
            return False

    def is_title_of(self, info, titles):
        for t in titles:
            if t in info:
                return True

        return False

    def get_cur_sh(self, no_elf_bootup_rx, elf_boot_up_rx):
        self.console.write(b'\r\r\r\r\r')
        info = self.console.read(100000)
        if self.is_title_of(info, no_elf_bootup_rx):
            return "no_elf_bootup"
        elif self.is_title_of(info, elf_boot_up_rx):
            return "elf_bootup"
        else:
            return "unkonwn shell"

    def show_elf_version(self, no_elf_bootup_rx, elf_boot_up_rx):
        if "elf_bootup" == self.get_cur_sh(no_elf_bootup_rx, elf_boot_up_rx):
            self.console.write(b'\r')
            self.console.write(b'version')
            self.console.write(b'\r')
        return


    def flash(self,firmware_name,bootaddr,local_ip,host_ip,gateway_ip):
        tx = ''
        ret = True
        # check current shell header, is it u-boot or elf image
        cur_shell = self.get_cur_sh(self.no_elf_bootup_rx, self.elf_bootup_rx)

        if "no_elf_bootup" == cur_shell:    
        # reset u-boot, wait 10 seconds
        # ret = self.send_cmd(b'reset', no_elf_bootup_rx, True, 10)
        # if True != ret:
        #     exit(1)
            pass
        elif "elf_bootup" == cur_shell:
            # reset elf, wait 10 seconds
            ret = self.send_cmd(b'reboot', self.no_elf_bootup_rx, True, 10)
            if True != ret:
                return -1
        else:
            print(cur_shell)
            return -1    

         # set ip addr of board
        tx = 'setenv ipaddr {}'.format(local_ip)
        ret = self.send_cmd(bytes(tx, encoding="utf8"), self.no_elf_bootup_rx, False)
        if True != ret:
            return -2

        # set ip addr of server host
        tx = 'setenv serverip {}'.format(host_ip)
        ret = self.send_cmd(bytes(tx, encoding="utf8"), self.no_elf_bootup_rx, False)
        if True != ret:
            return -3

        # set ip gateway
        tx = 'setenv gatewayip {}'.format(gateway_ip)
        ret = self.send_cmd(bytes(tx, encoding="utf8"), self.no_elf_bootup_rx, True)
        if True != ret:
            return -4
  
        # boot elf image
        tx = 'tftpboot {} {}'.format(bootaddr, os.path.basename(firmware_name))
        ret = self.send_cmd(bytes(tx, encoding="utf8"), self.no_elf_bootup_rx, True, 20)
        if True != ret:
            return -5

        # unpack elf image and jump
        tx = 'bootelf -p {}'.format(bootaddr)
        self.send_cmd(bytes(tx, encoding="utf8"), self.elf_bootup_rx, True, 10)
        if True != ret:
            return -6

        return 0

        

