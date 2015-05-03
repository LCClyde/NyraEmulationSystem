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

#include <nes/PPUHelpers.h>
#include <nes/CPUHelper.h>

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
    PPU();

    /*
     *  \func - tick
     *  \brief - Updates the PPU to match the CPU timing. ideally I think
     *           we want this to be one scanline.
     */
    void tick(const CPUInfo& info);

    /*
     *  \func - getRegisters
     *  \brief - Returns the current PPU register values.
     */
    inline PPURegisters& getRegisters()
    {
        return mRegisters;
    }

private:
    PPURegisters mRegisters;
    int16_t mScanline;
};
}
}
#endif
