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
#include <nes/PPU.h>
#include <graphics/WindowSDL.h>

using namespace nyra;

int main(int argc, char** argv)
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: <" << argv[0] << "> <NES File>\n";
            return 1;
        }
 
        const std::string inputPathname = argv[1];

        // Read the cart.
        const nes::Cartridge cart(inputPathname);

        // Chr ROM has 128x128 pixels in it and there are two of them
        const core::Vector2UI size(128, 128);
        graphics::WindowSDL window("Extracted CHR ROM",
                                   size,
                                   core::Vector2I(100, 100));

        const size_t numPixels = size.x * size.y;
        std::vector<uint32_t> pixels(numPixels);
        std::vector<uint8_t> plane1(8);
        std::vector<uint8_t> plane2(8);

        nes::Memory& chrROM = *(cart.getChrROM()[0]);
        for (size_t row = 0; row < 16; ++row)
        {
            for (size_t col = 0; col < 16; ++col)
            {
                size_t address = (row * 16 + col) * 16;
                for (size_t ii = 0; ii < 8; ++ii, ++address)
                {
                    plane1[ii] = chrROM.readByte(address);
                }
                for (size_t ii = 0; ii < 8; ++ii, ++address)
                {
                    plane2[ii] = chrROM.readByte(address);
                } 

                for (size_t ii = 0; ii < 8; ++ii)
                {
                    for (size_t jj = 0; jj < 8; ++jj)
                    {
                        const uint8_t value = (((plane1[ii] >> (7 - jj)) & 0x01) | (((plane2[ii] >> (7 - jj)) & 0x01) << 1)) * 85;
                        const uint32_t rgbValue = (value) | (value << 8) | (value << 16) | (value << 24);
                        pixels[(row * 64 * 16 ) + (col * 8) + (ii * 128) + (jj)] = rgbValue;
                    }
                }
            }
        }
        window.showBuffer(&pixels[0], numPixels * 4);

        // Run
        while (window.update());
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
