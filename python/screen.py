import pygame
import timeit
import ctypes

class Screen:
    def __init__(self, resolution=(256, 240)):
        pygame.init()
        self.resolution = resolution
        self.screen = pygame.display.set_mode(self.resolution)
        self.texture = pygame.Surface(self.resolution)
        self.rect = self.texture.get_rect()
        self.fps = 0
        self.prev_time = timeit.default_timer()

    @property
    def buffer(self):
        self.buf = pygame.surfarray.pixels2d(self.texture)
        return self.buf.ctypes.data_as(ctypes.c_void_p).value
    
        
    def render(self):
        self.buf = None
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                return False
        
        self.screen.blit(self.texture, self.rect)
        pygame.display.flip()
        
        # Update fps
        self.fps += 1
        time = timeit.default_timer()
        self.current_time = time - self.prev_time
        if self.current_time >= 1.0:
            pygame.display.set_caption('FPS: ' + str(self.fps))
            self.fps = 0
            self.prev_time = time
        
        return True