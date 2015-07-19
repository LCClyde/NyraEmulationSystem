from nes import MemoryMap, Cartridge, CPU, PPU, \
                Controller, APU, create_memory_map
from screen import Screen

class Emulator:
    def __init__(self, pathname):
        self.cartridge = Cartridge(pathname)
        self.ppu = PPU(self.cartridge.chr_rom, self.cartridge.header.mirroring)
        self.apu = APU()
        self.controllers = []
        self.controllers.append(Controller())
        self.controllers.append(Controller())
        self.memory_map = create_memory_map(self.cartridge,
                                            self.ppu,
                                            self.apu,
                                            self.controllers[0],
                                            self.controllers[1])
        self.cpu = CPU(self.memory_map.read_short(0xFFFC))
        self.screen = Screen
    
    def process_scanline(self, buffer):
        self.ppu.process_scanline(self.cpu.info, self.memory_map, buffer)
        self.cpu.process_scanline(self.memory_map)
        
    def tick(self, screen):
        buffer = screen.buffer
        self.process_scanline(buffer)
        # TODO: We can get a significant performance boost by moving this to
        #       C code. It is just not a high priority until we hit below
        #       60 fps.
        while self.cpu.info.scan_line != 241:
            self.process_scanline(buffer)