#!/usr/bin/env python3
from monitor import *
from color_print import *
from tftp_flash import *

def main():
    parser = argparse.ArgumentParser("monitor - a serial output monitor for standalone-sdk")
    
    # 需要打开monitor
    parser.add_argument(
        '--monitor',
        help='Whether to open monitor  ,0 is close',
        type=int,
        default = 0
    )
    
    # 烧录模式
    parser.add_argument(
        '--load', '-ld',
        help='Loading firmware, 0 is tftp',
    )
    # 串口部分
    parser.add_argument(
        '--port', '-p',
        help='Serial port device',
        default=os.environ.get('ESPTOOL_PORT', '/dev/ttyUSB0')
    )

    parser.add_argument(
        '--baud', '-b',
        help='Serial port baud rate',
        type=int,
        default="115200")

    parser.add_argument(
        '--toolchain-prefix',
        help="Triplet prefix to add before cross-toolchain names",
        default=DEFAULT_TOOLCHAIN_PREFIX)


    parser.add_argument(
        "--eol",
        choices=['CR', 'LF', 'CRLF'],
        type=lambda c: c.upper(),
        help="End of line to use when sending to the serial port",
        default='CR')

    parser.add_argument(
        '--elf_file', help='ELF file of application',
        type=argparse.FileType('rb'))

    # 烧录部分
    # boot address
    parser.add_argument(
        '--bootaddr', '-ba',
        help='Boot address to load image',
        default='0x90100000'
    )

    # board ip
    parser.add_argument(
        '--boardip', '-bi',
        help='IPv4 address for developer board',
        default='192.168.4.20'
    )

    # host ip
    parser.add_argument(
        '--hostip', '-hi',
        help='IPv4 address for developer host',
        default='192.168.4.51'
    )

    # gateway ip
    parser.add_argument(
        '--gatewayip', '-gi',
        help='IPv4 address for developer host-board gateway',
        default='192.168.4.1'
    )

    # ymodem reciver
    parser.add_argument(
        '--ymodeldest', '-ryd',
        help='Default ymodem receiving destination',
        default='./'
    )
    

    args = parser.parse_args()
    
    if args.load == '0': # 烧录方式选择
        # 增加烧录流程
        tftp_boot = tftp_flash(args.port,args.baud) 
        ret = tftp_boot.flash(args.elf_file.name,args.bootaddr,args.boardip,args.hostip,args.gatewayip)
        if ret != 0:
            exit(0)
    
    if args.monitor == 0:
        exit(0)

    # monitor 流程
    # 初始化串口工具
    serial_instance = serial.serial_for_url(args.port, args.baud,
                                            do_not_open=True)
    serial_instance.dtr = False
    serial_instance.rts = False

    monitor = Monitor(serial_instance,args.elf_file.name,args.toolchain_prefix,args.ymodeldest,args.eol) 

    yellow_print('--- sdk_monitor on {p.name} {p.baudrate} ---'.format(
        p=serial_instance))
    yellow_print('--- Quit: {} | Menu: {} | Help: {} followed by {} ---'.format(
        key_description(monitor.exit_key),
        key_description(monitor.menu_key),
        key_description(monitor.menu_key),
        key_description(CTRL_H)))
    
    #monitor.lookup_pc_address('0x80100000')
    monitor.main_loop()

if __name__ == "__main__":
    main()
