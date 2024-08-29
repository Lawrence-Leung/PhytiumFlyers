#!/usr/bin/env python3
from __future__ import print_function, division
from __future__ import unicode_literals
from builtins import chr
from builtins import object
from builtins import bytes
import subprocess
import argparse
import codecs
import datetime
import re
import os
try:
    import queue
except ImportError:
    import Queue as queue
import time
import sys
import serial
import serial.tools.miniterm as miniterm
import types
from distutils.version import StrictVersion
from io import open


from color_print import *
from stoppable_thread import *
from ymodem.ymodem_receiver import *

key_description = miniterm.key_description

# Control-key characters
CTRL_A = '\x01'
CTRL_B = '\x02'
CTRL_C = '\x03'
CTRL_F = '\x06'
CTRL_H = '\x08'
CTRL_R = '\x12'
CTRL_T = '\x14'
CTRL_Y = '\x19'
CTRL_P = '\x10'
CTRL_X = '\x18'
CTRL_L = '\x0c'
CTRL_RBRACKET = '\x1d'  # Ctrl+]

__version__ = "0.1"

# Tags for tuples in queues
TAG_KEY = 0
TAG_SERIAL = 1
TAG_SERIAL_FLUSH = 2

# regex matches an potential PC value (0x4xxxxxxx)
MATCH_PCADDR = re.compile(r'0x4[0-9a-f]{7}', re.IGNORECASE)

DEFAULT_TOOLCHAIN_PREFIX = "aarch64-none-elf-"

DEFAULT_PRINT_FILTER = ""


import re
import sys

class LineMatcher(object):
    """
    Assembles a dictionary of filtering rules based on the --print_filter
    argument of idf_monitor. Then later it is used to match lines and
    determine whether they should be shown on screen or not.
    """
    LEVEL_N = 0
    LEVEL_E = 1
    LEVEL_W = 2
    LEVEL_I = 3
    LEVEL_D = 4
    LEVEL_V = 5

    level = {'N': LEVEL_N, 'E': LEVEL_E, 'W': LEVEL_W, 'I': LEVEL_I, 'D': LEVEL_D,
             'V': LEVEL_V, '*': LEVEL_V, '': LEVEL_V}

    def __init__(self, print_filter):
        self._dict = dict()
        self._re = re.compile(r'^(?:\033\[[01];?[0-9]+m?)?([EWIDV]) \([0-9]+\) ([^:]+): ')
        items = print_filter.split()
        if len(items) == 0:
            self._dict["*"] = self.LEVEL_V  # default is to print everything
        for f in items:
            s = f.split(r':')
            if len(s) == 1:
                # specifying no warning level defaults to verbose level
                lev = self.LEVEL_V
            elif len(s) == 2:
                if len(s[0]) == 0:
                    raise ValueError('No tag specified in filter ' + f)
                try:
                    lev = self.level[s[1].upper()]
                except KeyError:
                    raise ValueError('Unknown warning level in filter ' + f)
            else:
                raise ValueError('Missing ":" in filter ' + f)
            self._dict[s[0]] = lev

    def match(self, line):
        try:
            m = self._re.search(line)
            if m:
                lev = self.level[m.group(1)]
                if m.group(2) in self._dict:
                    return self._dict[m.group(2)] >= lev
                return self._dict.get("*", self.LEVEL_N) >= lev
        except (KeyError, IndexError):
            # Regular line written with something else Phytium*
            # or an empty line.
            pass
        # We need something more than "*.N" for printing.
        return self._dict.get("*", self.LEVEL_N) > self.LEVEL_N



class ConsoleReader(StoppableThread):
    """ Read input keys from the console and push them to the queue,
    until stopped.
    """
    def __init__(self, console, event_queue, test_mode):
        super(ConsoleReader, self).__init__()
        self.console = console
        self.event_queue = event_queue
        self.test_mode = test_mode

    def run(self):
        self.console.setup()
        try:
            while self.alive:
                try:
                    if os.name == 'nt':
                        # Windows kludge: because the console.cancel() method doesn't
                        # seem to work to unblock getkey() on the Windows implementation.
                        #
                        # So we only call getkey() if we know there's a key waiting for us.
                        import msvcrt
                        while not msvcrt.kbhit() and self.alive:
                            time.sleep(0.1)
                        if not self.alive:
                            break
                    elif self.test_mode:
                        # In testing mode the stdin is connected to PTY but is not used for input anything. For PTY
                        # the canceling by fcntl.ioctl isn't working and would hang in self.console.getkey().
                        # Therefore, we avoid calling it.
                        while self.alive:
                            time.sleep(0.1)
                        break
                    c = self.console.getkey()
                except KeyboardInterrupt:
                    c = '\x03'
                if c is not None:
                    self.event_queue.put((TAG_KEY, c), False)
        finally:
            self.console.cleanup()

    def _cancel(self):
        if os.name == 'posix' and not self.test_mode:
            # this is the way cancel() is implemented in pyserial 3.3 or newer,
            # older pyserial (3.1+) has cancellation implemented via 'select',
            # which does not work when console sends an escape Phytium
            #
            # even older pyserial (<3.1) does not have this method
            #
            # on Windows there is a different (also hacky) fix, applied above.
            #
            # note that TIOCSTI is not implemented in WSL / bash-on-Windows.
            # TODO: introduce some workaround to make it work there.
            #
            # Note: This would throw exception in testing mode when the stdin is connected to PTY.
            import fcntl
            import termios
            fcntl.ioctl(self.console.fd, termios.TIOCSTI, b'\0')


class SerialReader(StoppableThread):
    """ Read serial data from the serial port and push to the
    event queue, until stopped.
    """
    def __init__(self, serial, event_queue):
        super(SerialReader, self).__init__()
        self.baud = serial.baudrate
        self.serial = serial
        self.event_queue = event_queue
        if not hasattr(self.serial, 'cancel_read'):
            # enable timeout for checking alive flag,
            # if cancel_read not available
            self.serial.timeout = 0.25

    def run(self):
        if not self.serial.is_open:
            self.serial.baudrate = self.baud
            self.serial.rts = True  # Force an RTS reset on open
            self.serial.open()
            self.serial.rts = False
            self.serial.dtr = self.serial.dtr   # usbser.sys workaround
        try:
            while self.alive:
                data = self.serial.read(self.serial.in_waiting or 1)
                if len(data):
                    self.event_queue.put((TAG_SERIAL, data), False)
        finally:
            self.serial.close()

    def _cancel(self):
        if hasattr(self.serial, 'cancel_read'):
            try:
                self.serial.cancel_read()
            except Exception:
                pass



class SerialStopException(Exception):
    """
    This exception is used for stopping the IDF monitor in testing mode.
    """
    pass


class Monitor(object):
    """
    Monitor application main class.

    This was originally derived from miniterm.Miniterm, but it turned out to be easier to write from scratch for this
    purpose.

    Main difference is that all event processing happens in the main thread, not the worker threads.
    """
    def __init__(self, serial_instance, elf_file,toolchain_prefix,ymodeldest, eol="CR"):
        super(Monitor, self).__init__()
        self.event_queue = queue.Queue()
        self.console = miniterm.Console()
        if os.name == 'nt':
            sys.stderr = ANSIColorConverter(sys.stderr, decode_output=True)
            self.console.output = ANSIColorConverter(self.console.output)
            self.console.byte_output = ANSIColorConverter(self.console.byte_output)

        if StrictVersion(serial.VERSION) < StrictVersion('3.3.0'):
            # Use Console.getkey implementation from 3.3.0 (to be in sync with the ConsoleReader._cancel patch above)
            def getkey_patched(self):
                c = self.enc_stdin.read(1)
                if c == chr(0x7f):
                    c = chr(8)    # map the BS key (which yields DEL) to backspace
                return c

            self.console.getkey = types.MethodType(getkey_patched, self.console)

        socket_mode = serial_instance.port.startswith("socket://")  # testing hook - data from serial can make exit the monitor
        self.serial = serial_instance
        self.console_reader = ConsoleReader(self.console, self.event_queue, socket_mode)
        self.serial_reader = SerialReader(self.serial, self.event_queue)
        self.elf_file = elf_file

        self.make = "make"
        self.encrypted = False
        self.toolchain_prefix = toolchain_prefix
        self.menu_key = CTRL_T
        self.exit_key = CTRL_RBRACKET

        self.translate_eol = {
            "CRLF": lambda c: c.replace("\n", "\r\n"),
            "CR": lambda c: c.replace("\n", "\r"),
            "LF": lambda c: c.replace("\r", "\n"),
        }[eol]

        # ymodem reciver
        
        self.ymodeldest = ymodeldest

        # internal state
        self._pressed_menu_key = False
        self._last_line_part = b""
        self._gdb_buffer = b""
        self._pc_address_buffer = b""
        self._line_matcher = LineMatcher(DEFAULT_PRINT_FILTER)
        self._invoke_processing_last_line_timer = None
        self._force_line_print = False
        self._output_enabled = True
        self._serial_check_exit = socket_mode
        self._log_file = None

    def invoke_processing_last_line(self):
        self.event_queue.put((TAG_SERIAL_FLUSH, b''), False)

    def main_loop(self):
        self.console_reader.start()
        self.serial_reader.start()
        try:
            while self.console_reader.alive and self.serial_reader.alive: 
                (event_tag, data) = self.event_queue.get()
                
                if event_tag == TAG_KEY:
                    self.handle_key(data)
                elif event_tag == TAG_SERIAL:
                    self.handle_serial_input(data)
                    if self._invoke_processing_last_line_timer is not None:
                        self._invoke_processing_last_line_timer.cancel()
                    self._invoke_processing_last_line_timer = threading.Timer(0.1, self.invoke_processing_last_line)
                    self._invoke_processing_last_line_timer.start()
                    # If no futher data is received in the next short period
                    # of time then the _invoke_processing_last_line_timer
                    # generates an event which will result in the finishing of
                    # the last line. This is fix for handling lines sent
                    # without EOL.
                elif event_tag == TAG_SERIAL_FLUSH:
                    self.handle_serial_input(data, finalize_line=True)
                else:
                    raise RuntimeError("Bad event data %r" % ((event_tag,data),))
        except SerialStopException:
            sys.stderr.write(ANSI_NORMAL + "Stopping condition has been received\n")
        finally:
            try:
                self.console_reader.stop()
                self.serial_reader.stop()
                self.stop_logging()
                # Cancelling _invoke_processing_last_line_timer is not
                # important here because receiving empty data doesn't matter.
                self._invoke_processing_last_line_timer = None
            except Exception:
                pass
            sys.stderr.write(ANSI_NORMAL + "\n")

    # 响应终端输入
    def handle_key(self, key): 
        if self._pressed_menu_key:
            self.handle_menu_key(key)
            self._pressed_menu_key = False
        elif key == self.menu_key:
            self._pressed_menu_key = True
        elif key == self.exit_key:
            self.console_reader.stop()
            self.serial_reader.stop()
        else:
            try:
                key = self.translate_eol(key)
                self.serial.write(codecs.encode(key))
            except serial.SerialException:
                pass  # this shouldn't happen, but sometimes port has closed in serial thread
            except UnicodeEncodeError:
                pass  # this can happen if a non-ascii character was passed, ignoring

    def handle_serial_input(self, data, finalize_line=False):
        sp = data.split(b'\n')
        if self._last_line_part != b"":
            # add unprocessed part from previous "data" to the first line
            sp[0] = self._last_line_part + sp[0]
            self._last_line_part = b""
        if sp[-1] != b"":
            # last part is not a full line
            self._last_line_part = sp.pop()

        for line in sp:
            if line != b"":
                if self._serial_check_exit and line == self.exit_key.encode('latin-1'): # 内部关闭
                    raise SerialStopException()
                if self._force_line_print or self._line_matcher.match(line.decode(errors="ignore")):
                    self._print(line + b'\n')
                    self.handle_possible_pc_address_in_line(line)
                self.check_gdbstub_trigger(line)
                self._force_line_print = False

        # Now we have the last part (incomplete line) in _last_line_part. By
        # default we don't touch it and just wait for the arrival of the rest
        # of the line. But after some time when we didn't received it we need
        # to make a decision.
        if self._last_line_part != b"":
            if self._force_line_print or (finalize_line and self._line_matcher.match(self._last_line_part.decode(errors="ignore"))):
                self._force_line_print = True
                self._print(self._last_line_part)
                self.handle_possible_pc_address_in_line(self._last_line_part)
                self.check_gdbstub_trigger(self._last_line_part)
                # It is possible that the incomplete line cuts in half the PC
                # address. A small buffer is kept and will be used the next time
                # handle_possible_pc_address_in_line is invoked to avoid this problem.
                # MATCH_PCADDR matches 10 character long addresses. Therefore, we
                # keep the last 9 characters.
                self._pc_address_buffer = self._last_line_part[-9:]
                # GDB sequence can be cut in half also. GDB sequence is 7
                # characters long, therefore, we save the last 6 characters.
                self._gdb_buffer = self._last_line_part[-6:]
                self._last_line_part = b""
        # else: keeping _last_line_part and it will be processed the next time
        # handle_serial_input is invoked

    def handle_possible_pc_address_in_line(self, line):
        line = self._pc_address_buffer + line
        self._pc_address_buffer = b""
        for m in re.finditer(MATCH_PCADDR, line.decode(errors="ignore")):
            self.lookup_pc_address(m.group())

    def handle_menu_key(self, c):
        if c == self.exit_key or c == self.menu_key:  # send verbatim
            self.serial.write(codecs.encode(c))
        elif c in [CTRL_H, 'h', 'H', '?']:
            red_print(self.get_help_text())
        elif c in [CTRL_A, 'a', 'A']:  # Ymodel reciver
            # "CTRL-A" cannot be captured with the default settings of the Windows command line, therefore, "A" can be used
            # instead
            self.YmodelReciver()
            pass
        elif c == CTRL_Y:  # Toggle output display
            self.output_toggle()
        elif c == CTRL_L:  # Toggle saving output into file
            self.toggle_logging()
        elif c == CTRL_P:
            pass
        elif c in [CTRL_X, 'x', 'X']:  # Exiting from within the menu
            self.console_reader.stop()
            self.serial_reader.stop()
        else:
            red_print('--- unknown menu character {} --'.format(key_description(c)))

    def get_help_text(self):
        return """ 
            ---Phytium Sdk monitor ({version})
            ---based on miniterm from pySerial
            
            --- 
            --- {exit:8} Exit program
            --- {menu:8} Menu escape key, followed by:
            --- Menu keys:
            ---    {menu:14} Send the menu character itself to remote
            ---    {exit:14} Send the exit character itself to remote
            ---    {ymodem:14} Obtain files through the serial port based on ymodem
            ---    {log:14} Toggle saving output into file
        """.format(version=__version__,
           exit=key_description(self.exit_key),
           menu=key_description(self.menu_key),
           ymodem=key_description(CTRL_A) + ' (or A)',
           log=key_description(CTRL_L))

    def __enter__(self):
        """ Use 'with self' to temporarily disable monitoring behaviour """
        self.serial_reader.stop()
        self.console_reader.stop()

    def __exit__(self, *args, **kwargs):
        """ Use 'with self' to temporarily disable monitoring behaviour """
        self.console_reader.start()
        self.serial_reader.start()

    def prompt_next_action(self, reason):
        self.console.setup()  # set up console to trap input characters
        try:
            red_print("""
--- {}
--- Press {} to exit monitor.
--- Press {} to build & flash project.
--- Press {} to build & flash app.
--- Press any other key to resume monitor (resets target).""".format(reason,
                                                                     key_description(self.exit_key),
                                                                     key_description(CTRL_F),
                                                                     key_description(CTRL_A)))
            k = CTRL_T  # ignore CTRL-T here, so people can muscle-memory Ctrl-T Ctrl-F, etc.
            while k == CTRL_T:
                k = self.console.getkey()
        finally:
            self.console.cleanup()
        if k == self.exit_key:
            self.event_queue.put((TAG_KEY, k))
        elif k in [CTRL_F, CTRL_A]:
            self.event_queue.put((TAG_KEY, self.menu_key))
            self.event_queue.put((TAG_KEY, k))

    def run_make(self, target):
        with self:
            if isinstance(self.make, list):
                popen_args = self.make + [target]
            else:
                popen_args = [self.make, target]
            yellow_print("Running %s..." % " ".join(popen_args))
            p = subprocess.Popen(popen_args)
            try:
                p.wait()
            except KeyboardInterrupt:
                p.wait()
            if p.returncode != 0:
                self.prompt_next_action("Build failed")
            else:
                self.output_enable(True)

    def lookup_pc_address(self, pc_addr):
        cmd = ["%saddr2line" % self.toolchain_prefix,
               "-pfiaC", "-e", self.elf_file, pc_addr]
        try:
            translation = subprocess.check_output(cmd, cwd=".")
            if b"?? ??:0" not in translation:
                self._print(translation.decode(), console_printer=yellow_print)
        except OSError as e:
            red_print("%s: %s" % (" ".join(cmd), e))

    def check_gdbstub_trigger(self, line):
        line = self._gdb_buffer + line
        self._gdb_buffer = b""
        m = re.search(b"\\$(T..)#(..)", line)  # look for a gdb "reason" for a break
        if m is not None:
            try:
                chsum = sum(ord(bytes([p])) for p in m.group(1)) & 0xFF
                calc_chsum = int(m.group(2), 16)
            except ValueError:
                return  # payload wasn't valid hex digits
            if chsum == calc_chsum:
                self.run_gdb()
            else:
                red_print("Malformed gdb message... calculated checksum %02x received %02x" % (chsum, calc_chsum))

    def run_gdb(self):
        with self:  # disable console control
            sys.stderr.write(ANSI_NORMAL)
            try:
                cmd = ["%sgdb" % self.toolchain_prefix,
                       "-ex", "set serial baud %d" % self.serial.baudrate,
                       "-ex", "target remote %s" % self.serial.port,
                       "-ex", "interrupt",  # monitor has already parsed the first 'reason' command, need a second
                       self.elf_file]
                process = subprocess.Popen(cmd, cwd=".")
                process.wait()
            except OSError as e:
                red_print("%s: %s" % (" ".join(cmd), e))
            except KeyboardInterrupt:
                pass  # happens on Windows, maybe other OSes
            finally:
                try:
                    # on Linux, maybe other OSes, gdb sometimes seems to be alive even after wait() returns...
                    process.terminate()
                except Exception:
                    pass
                try:
                    # also on Linux, maybe other OSes, gdb sometimes exits uncleanly and breaks the tty mode
                    subprocess.call(["stty", "sane"])
                except Exception:
                    pass  # don't care if there's no stty, we tried...
            self.prompt_next_action("gdb exited")

    def output_enable(self, enable):
        self._output_enabled = enable

    def output_toggle(self):
        self._output_enabled = not self._output_enabled
        yellow_print("\nToggle output display: {}, Type Ctrl-T Ctrl-Y to show/disable output again.".format(self._output_enabled))

    def toggle_logging(self):
        if self._log_file:
            self.stop_logging()
        else:
            self.start_logging()

    def start_logging(self):
        if not self._log_file:
            try:
                name = "log.{}.{}.txt".format(os.path.splitext(os.path.basename(self.elf_file))[0],
                                              datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
                self._log_file = open(name, "wb+")
                yellow_print("\nLogging is enabled into file {}".format(name))
            except Exception as e:
                red_print("\nLog file {} cannot be created: {}".format(name, e))

    def stop_logging(self):
        if self._log_file:
            try:
                name = self._log_file.name
                self._log_file.close()
                yellow_print("\nLogging is disabled and file {} has been closed".format(name))
            except Exception as e:
                red_print("\nLog file cannot be closed: {}".format(e))
            finally:
                self._log_file = None

    def YmodelReciver(self):
        yellow_print("Ymodel Data receiving mode")
        back_timeout = self.serial.timeout
        self.serial_reader.stop()
        self.serial.open()
        self.serial.timeout = 1
        ymodem_receiver(self.serial.read,self.serial.write,self.ymodeldest).run()
        self.serial.timeout = back_timeout
        self.serial_reader = SerialReader(self.serial, self.event_queue)
        self.serial_reader.start()
        pass

    def _print(self, string, console_printer=None):
        if console_printer is None:
            console_printer = self.console.write_bytes
        if self._output_enabled:
            console_printer(string)
        if self._log_file:
            try:
                if isinstance(string, type(u'')):
                    string = string.encode()
                self._log_file.write(string)
            except Exception as e:
                red_print("\nCannot write to file: {}".format(e))
                # don't fill-up the screen with the previous errors (probably consequent prints would fail also)
                self.stop_logging()


