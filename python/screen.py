
import pygame

class Screen:
    def __init__(self):
        pygame.init()
        self.resolution = (256, 240)
        self.screen = pygame.display.set_mode(resolution)
        self.texture = pygame.Surface(resolution)