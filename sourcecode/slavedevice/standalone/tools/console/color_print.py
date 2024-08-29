#!/usr/bin/env python3
# color_print
# ANSI terminal codes (if changed, regular expressions in LineMatcher need to be udpated)
import sys
ANSI_RED = '\033[1;31m'
ANSI_YELLOW = '\033[0;33m'
ANSI_NORMAL = '\033[0m'


def color_print(message, color):
    """ Print a message to stderr with colored highlighting """
    sys.stderr.write("%s%s%s\n" % (color, message,  ANSI_NORMAL))


def yellow_print(message):
    color_print(message, ANSI_YELLOW)


def red_print(message):
    color_print(message, ANSI_RED)