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
#include <core/OptionsParser.h>

using namespace nyra;

int main(int argc, char** argv)
{
    try
    {
        core::OptionsParser options;
        options.addOption("-a --start-address", "startAddress",
                          "Specify the address to start execution at");
        options.addOption("", "inputPathname",
                          "Specify the .nes file to run");
        options.parse(argc, argv);
 
        const std::string inputPathname =
                options.get<std::string>("inputPathname");

        // Read the cart.
        const nes::Cartridge cart(inputPathname);

        // Setup memory
        nes::MemoryMap memoryMap(true);
        memoryMap.setMemoryBank(0x8000, *(cart.getProgROM()[0]));
        memoryMap.setMemoryBank(0xC000, *(cart.getProgROM()[0]));

        const uint16_t startingAddress = options.has("startAddress") ?
                options.get<uint16_t>("startAddress") :
                memoryMap.readShort(0xFFFD);

        // Setup CPU
        nes::CPU cpu(startingAddress);

        // Get dissembly
        nes::Disassembly disassembly;

        // Run
        while (true)
        {
            cpu.tick(memoryMap, &disassembly);

            std::cout << disassembly << "\n";
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
