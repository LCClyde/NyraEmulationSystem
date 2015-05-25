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
#include <nes/PPU.h>
#include <limits>

namespace
{
/*****************************************************************************/
static const int16_t VBLANK_START = 241;
static const int16_t VBLANK_END = -1;


}

namespace nyra
{
namespace nes
{
/*****************************************************************************/
PPU::PPU()
{
}

/*****************************************************************************/
void PPU::processScanline(CPUInfo& info)
{
    switch (info.scanLine)
    {
    case VBLANK_START:
        // Set the VBLANK flag
        mRegisters.getRegister(
                PPUMemory::PPUSTATUS)[PPUMemory::VBLANK] = true;

        // Check for NMI
        info.generateNMI =mRegisters.getRegister(
                PPUMemory::PPUCTRL)[PPUMemory::NMI_ENABLED];
        break;
    case VBLANK_END:
        // Set the VBLANK flag
        mRegisters.getRegister(
                PPUMemory::PPUSTATUS)[PPUMemory::VBLANK] = false;
        break;
    }
}
}
}
