
import argparse
import pygame
import sys
import time
from nes import Emulator, DisassemblyVector, Controller
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
            time.sleep(0.01)
            pressed = pygame.key.get_pressed()
            if pressed[pygame.K_RETURN]:
                emulator.controller_1.set_key(Controller.BUTTON_START)
            if pressed[pygame.K_RSHIFT]:
                emulator.controller_1.set_key(Controller.BUTTON_SELECT)
            if pressed[pygame.K_a]:
                emulator.controller_1.set_key(Controller.BUTTON_LEFT)
            if pressed[pygame.K_d]:
                emulator.controller_1.set_key(Controller.BUTTON_RIGHT)
            if pressed[pygame.K_w]:
                emulator.controller_1.set_key(Controller.BUTTON_UP)
            if pressed[pygame.K_s]:
                emulator.controller_1.set_key(Controller.BUTTON_DOWN)
            if pressed[pygame.K_j]:
                emulator.controller_1.set_key(Controller.BUTTON_B)
            if pressed[pygame.K_k]:
                emulator.controller_1.set_key(Controller.BUTTON_A)

            emulator.tick(screen.buffer)
            keep_going = screen.render()
        except Exception, e:
            print 'Exception occurred: ' + str(e)
            keep_going = False