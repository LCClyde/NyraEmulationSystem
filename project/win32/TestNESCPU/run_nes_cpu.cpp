/******************************************************************************
 * The MIT License(MIT)
 *
 * Copyright(c) 2015 Clyde Stanfield
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files(the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions :
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *****************************************************************************/
#include <iostream>
#include <core/Exception.h>
#include <nes/Cartridge.h>
#include <nes/CPU.h>
#include <nes/Disassembly.h>
#include <core/File.h>
#include <core/StringUtils.h>

using namespace nyra;

int main(int argc, char** argv)
{
    try
    {
        if (argc != 3)
        {
            std::cerr << "Usage: <" << argv[0] << "> <NES File><NES Log>\n";
            return 1;
        }
 
        const std::string inputPathname = argv[1];
        const std::string logPathname = argv[2];

        // Read the log file
        const std::vector<std::string> lines =
                core::split(core::readFile(logPathname), "\n");

        // Read the cart.
        const nes::Cartridge cart(inputPathname);

        // Setup CPU
        nes::CPU cpu(0xC000);

        // Setup memory
        nes::MemoryMap memoryMap;
        nes::RAM ram(0x7F00);
        nes::RAM zeroPage(0x100);
        memoryMap.setMemoryBank(0, zeroPage);
        memoryMap.setMemoryBank(0x0100, ram);
        memoryMap.setMemoryBank(0x8000, *(cart.getProgROM()[0]));
        memoryMap.setMemoryBank(0xC000, *(cart.getProgROM()[0]));

        // Get dissembly
        nes::Disassembly disassembly;

        // Run
        for (size_t ii = 0; ii < 10000; ++ii)
        {
            try
            {
                cpu.tick(memoryMap, &disassembly);
                //std::cout << disassembly << "\n";
            }
            catch (...)
            {
                // Ignore
            }

            std::stringstream ss;
            ss << disassembly;
            if (ss.str() != lines[ii])
            {
                std::cout << "\n\n\nERROR: " << ii + 1 << "\n" << ss.str()
                          << "\n" << lines[ii] << "\n\n\n\n\n";
                return 0;
            }
        }
    }
    catch (core::Exception& ex)
    {
        std::cout << ex.what() << "\n";
        return 1;
    }
    catch (...)
    {
        std::cout << "An unknown error occurred\n";
        return 1;
    }

    return 0;
}
