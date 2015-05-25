
import unittest
import os
from nes import CPU, Cartridge, MemoryMap, DisassemblyVector
from print_disassembly import print_nintendulator

class TestCPU(unittest.TestCase):
    def test_cpu(self):
        cart_pathname = os.path.join(
                os.path.dirname(os.path.realpath(__file__)), 'nestest.nes')
        log_pathname = os.path.join(
                os.path.dirname(os.path.realpath(__file__)), 'nestest.log')
        cart = Cartridge(cart_pathname)
        memory = MemoryMap(True)
        memory.set_memory_bank(0x8000, cart.get_prog_rom(0))
        memory.set_memory_bank(0xC000, cart.get_prog_rom(0))

        lines = open(log_pathname).readlines()

        disassembly = DisassemblyVector()
        cpu = CPU(0xC000)
        cpu.get_info().scan_line = 241
        
        keep_going = True
        index = 0
        while keep_going:
            try:
                disassembly.clear()
                cpu.process_scanline(memory, disassembly)
            except:
                keep_going = False
                disassembly.pop()

            for data in disassembly:
                self.assertEqual(print_nintendulator(data),
                                 lines[index].strip())
                index += 1

if __name__ == "__main__":
    unittest.main()
