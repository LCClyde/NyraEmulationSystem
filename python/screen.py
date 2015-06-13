import pygame
import ctypes

class Screen:
    def __init__(self, scale = 4, resolution = (256, 240)):
        pygame.init()
        self.window_size = (resolution[0] * int(scale), resolution[1] * int(scale))
        self.resolution = resolution
        self.screen = pygame.display.set_mode(self.window_size)
        self.texture = pygame.Surface(self.resolution)
        self.rect = pygame.transform.scale(self.texture, (self.window_size)).get_rect()

    @property
    def buffer(self):
        self.buf = pygame.surfarray.pixels2d(self.texture)
        return self.buf.ctypes.data_as(ctypes.c_void_p).value
    
        
    def render(self):
        self.buf = None
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False
        
        self.image = pygame.transform.scale(self.texture, (self.window_size))
        self.screen.blit(self.image, self.rect)
        pygame.display.flip()
        
        return True