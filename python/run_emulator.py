
import argparse
import ctypes
from nes import Emulator, DisassemblyVector
from print_disassembly import print_nintendulator
from screen import Screen

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
            description = 'Dumps NES header information')
    parser.add_argument('pathname', help='specify the NES file to open')
    args = parser.parse_args()

    emulator = Emulator(args.pathname)
    screen = Screen()
    
    keep_going = True
    while keep_going:
        try:
            emulator.tick(screen.buffer)
            keep_going = screen.render()
        except Exception, e:
            print 'Exception occurred: ' + str(e)
            keep_going = False