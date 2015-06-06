import pygame
import timeit
import time

class FPS:
    def __init__(self):
        self.fps = 0
        self.prev_time = timeit.default_timer()
        self.delta_time = 10000
        self.total_time = 0.0
        
    def update_time(self):
        curent_time = timeit.default_timer()
        self.delta_time = curent_time - self.prev_time
        self.total_time += self.delta_time
        self.prev_time = curent_time
        

    def update(self, throttle = None):
        # Update fps
        self.fps += 1
        self.update_time()
        if self.total_time >= 1.0:
            pygame.display.set_caption('FPS: ' + str(self.fps))
            self.fps = 0
            self.total_time = 0.0

        # TODO: This is not high enough precision.
        #       for now it is close enough.
        if throttle != None:
            sleep_time = (1.0 / throttle) - self.delta_time
            if sleep_time > 0.0:
                time.sleep(sleep_time)
                self.update_time()
