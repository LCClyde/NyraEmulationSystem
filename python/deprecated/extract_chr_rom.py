
import argparse
from nes import Cartridge
from screen import Screen

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
            description = 'Dumps NES header information')
    parser.add_argument('pathname', help='specify the NES file to open')
    args = parser.parse_args()
    
    cart = Cartridge(args.pathname)
    screen = Screen((128, 128))
    
    pixels = []
    for ii in range(0, 128 * 128):
        pixels.append(0)
    
    chr_rom = cart.get_chr_rom(0)
    
    # TODO: This should not be done here. The majority of this math should
    #       be in C++. Because this is just a test program, this is not
    #       a high priority until something else needs the functionality.
    for row in range(0, 16):
        for col in range(0, 16):
            address = (row * 16 + col) * 16
            plane1 = []
            plane2 = []
            for ii in range(0, 8):
                plane1.append(chr_rom.read_byte(address))
                address += 1
            for ii in range(0, 8):
                plane2.append(chr_rom.read_byte(address))
                address += 1
            for ii in range(0, 8):
                for jj in range(0, 8):
                    val1 = (plane1[ii] >> (7 - jj)) & 0x01
                    val2 = ((plane2[ii] >> (7 - jj)) & 0x01) << 1
                    value = (val1 | val2) * 85
                    rgb_value = value | (value << 8) | (value << 16)
                    pixels[(row * 64 * 16) + (col * 8) + (ii * 128) + jj] = rgb_value
    
    keep_going = True
    while keep_going:
        try:
            keep_going = screen.render(pixels)
        except:
            keep_going = false