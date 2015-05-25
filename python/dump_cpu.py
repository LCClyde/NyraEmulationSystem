
import argparse
from nes import Emulator, DisassemblyVector
from print_disassembly import print_nintendulator

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
            description = 'Dumps NES header information')
    parser.add_argument('pathname', help='specify the NES file to open')
    args = parser.parse_args()
    
    disassembly = DisassemblyVector()
    emulator = Emulator(args.pathname)
    
    keep_going = True
    while keep_going:
        try:
            emulator.tick(disassembly)
        except:
            keep_going = False

        for data in disassembly:
            print print_nintendulator(data)