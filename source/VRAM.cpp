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
#include <nes/VRAM.h>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
VRAM::VRAM(const ROMBanks& chrROM,
           Mirroring mirroring) :
    MemoryMap(),
    mNametables(2),
    mUniversalBackgroundColor(4),
    mPalettes(8),
    mFill(0x0F00)
{
    setMemoryBank(0x0000, *chrROM[0]);
    setMemoryBank(0x1000, *chrROM[1]);

    mNametables[0].reset(new RAM(0x400));
    mNametables[1].reset(new RAM(0x400));

    if (mirroring == HORIZONTAL)
    {
        setMemoryBank(0x2000, *mNametables[0]);
        setMemoryBank(0x2400, *mNametables[0]);
        setMemoryBank(0x2800, *mNametables[1]);
        setMemoryBank(0x2C00, *mNametables[1]);
    }
    else if (mirroring == VERTICAL)
    {
        setMemoryBank(0x2000, *mNametables[0]);
        setMemoryBank(0x2400, *mNametables[1]);
        setMemoryBank(0x2800, *mNametables[0]);
        setMemoryBank(0x2C00, *mNametables[1]);
    }
    // TODO: Implement single screen and four screen

    // TODO: This should actually be a mirror of $2000 - $2EFF
    setMemoryBank(0x3000, mFill);

    for (size_t ii = 0; ii < 4; ++ii)
    {
        mUniversalBackgroundColor[ii].reset(new RAM(1));
        mPalettes[ii].reset(new RAM(3));
        mPalettes[ii + 4].reset(new RAM(3));

        const size_t address = ii * 4;
        // TODO: This should actually be mUniversalBackgroundColor[ii].
        //       But that makes things more difficult and has no functional
        //       purpose other than being techinically correct.
        setMemoryBank(0x3F00 + address, *mUniversalBackgroundColor[ii]);
        setMemoryBank(0x3F10 + address, *mUniversalBackgroundColor[ii]);
        setMemoryBank(0x3F01 + address, *mPalettes[ii]);
        setMemoryBank(0x3F11 + address, *mPalettes[ii + 4]);
    }

    lockLookUpTable();
}
}
}
