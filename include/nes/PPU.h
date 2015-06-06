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
#ifndef __NYRA_NES_PPU_H__
#define __NYRA_NES_PPU_H__

#include <vector>
#include <nes/Memory.h>
#include <nes/CPUHelper.h>
#include <nes/PPURegisters.h>
#include <nes/VRAM.h>
#include <nes/Constants.h>

namespace nyra
{
namespace nes
{
/*
 *  \class - PPU
 *  \brief - The picture processing unit handles rendering the image to the
 *           screen. In the actual NES this runs in parallel with the CPU.
 *           This implemention instead will run sequentially until it catches
 *           up to the CPU.
 */
class PPU
{
public:
    /*
     *  \func - Constructor
     *  \brief - Sets a default internal structure for the PPU.
     */
    PPU(const ROMBanks& chrROM,
        Mirroring mirroring);

    /*
     *  \func - tick
     *  \brief - Updates the PPU to match the CPU timing. ideally I think
     *           we want this to be one scanline.
     */
    void processScanline(CPUInfo& info,
                         const MemoryMap& memory,
                         uint32_t* buffer = nullptr);

    void renderScanline(int16_t scanLine,
                        uint32_t* buffer = nullptr);

    uint32_t extractPixel(uint32_t address,
                          size_t bitPosition,
                          size_t palette,
                          uint32_t backgroundColor,
                          size_t& paletteAddress);

    inline PPURegisters& getRegisers()
    {
        return mRegisters;
    }

    inline const PPURegisters& getRegisters() const
    {
        return mRegisters;
    }

private:
    VRAM mVRAM;
    PPURegisters mRegisters;
    RAM mOAM;
};
}
}
#endif
