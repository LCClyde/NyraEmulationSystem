
from nes import Controller

class TASState:
    def __init__(self, values):
        # These come in as RLDUTSBA|RLDUTSBA
        key = '.'
        self.right1 = values[3] != key
        self.left1 = values[4] != key
        self.down1 = values[5] != key
        self.up1 = values[6] != key
        self.start1 = values[7] != key
        self.select1 = values[8] != key
        self.b1 = values[9] != key
        self.a1 = values[10] != key

class TAS:
    def __init__(self, pathname):
        f= open(pathname)
        lines = f.readlines()
        
        self.frames = []
        self.current_frame = 0
        for l in lines:
            if l.startswith('|0|'):
                self.frames.append(TASState(l))
                
    def update_controller(self, controller):
        if len(self.frames) > self.current_frame:
            frame = self.frames[self.current_frame]
            controller.set_key(Controller.BUTTON_START, frame.start1)
            controller.set_key(Controller.BUTTON_SELECT, frame.select1)
            controller.set_key(Controller.BUTTON_LEFT, frame.left1)
            controller.set_key(Controller.BUTTON_RIGHT, frame.right1)
            controller.set_key(Controller.BUTTON_UP, frame.up1)
            controller.set_key(Controller.BUTTON_DOWN, frame.down1)
            controller.set_key(Controller.BUTTON_B, frame.b1)
            controller.set_key(Controller.BUTTON_A, frame.a1)
            self.current_frame += 1