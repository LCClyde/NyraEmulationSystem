
import argparse
import pygame
import sys
from nes import Controller
from screen import Screen
from fps import FPS
from tas import TAS
from emulator import Emulator

if __name__ == "__main__":
    parser = argparse.ArgumentParser(
            description = 'Dumps NES header information')
    parser.add_argument('pathname', help='specify the NES file to open')
    parser.add_argument('--tas', dest='tas', help='specify a TAS file to run', default=None)
    parser.add_argument('--scale', dest='scale', help='specify the screen scale', default = 1)
    args = parser.parse_args()

    emulator = Emulator(args.pathname)
    screen = Screen(args.scale)
    fps = FPS()
    if args.tas:
        target_fps = 1000.0
        tas = TAS(args.tas)
    else:
        target_fps = 60.0
        tas = None
    
    keep_going = True
    while keep_going:
        try:
            # Get button presses
            if tas == None:
                pressed = pygame.key.get_pressed()
                controller = emulator.controllers[0]
                if pressed[pygame.K_RETURN]:
                    controller.set_key(Controller.BUTTON_START)
                if pressed[pygame.K_RSHIFT]:
                    controller.set_key(Controller.BUTTON_SELECT)
                if pressed[pygame.K_a]:
                    controller.set_key(Controller.BUTTON_LEFT)
                if pressed[pygame.K_d]:
                    controller.set_key(Controller.BUTTON_RIGHT)
                if pressed[pygame.K_w]:
                    controller.set_key(Controller.BUTTON_UP)
                if pressed[pygame.K_s]:
                    controller.set_key(Controller.BUTTON_DOWN)
                if pressed[pygame.K_j]:
                    controller.set_key(Controller.BUTTON_B)
                if pressed[pygame.K_k]:
                    controller.set_key(Controller.BUTTON_A)
            if tas != None:
                tas.update_controller(emulator.controllers[0])
                    
            emulator.tick(screen)
            keep_going = screen.render()
            
            fps.update(target_fps)
            
            
        except Exception, e:
            print 'Exception occurred: ' + str(e)
            keep_going = False