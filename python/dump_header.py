
import argparse
from nes import Cartridge

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
            description = 'Dumps NES header information')
    parser.add_argument('pathname', help='specify the NES file to open')
    args = parser.parse_args()
    
    cart = Cartridge(args.pathname)
    print cart.get_header()