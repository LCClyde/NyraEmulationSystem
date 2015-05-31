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
#include <nes/MemorySystem.h>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
MemorySystem::MemorySystem(PPURegisters& ppu) :
    mRAM(0x0700),
    mZeroPage(0x0100),
    mFill1(0x14),
    mFill2(0x3FEC)
{
    // Mirror the RAM to 0x2000
    for (size_t ii = 0; ii < 0x2000;
            ii += (mRAM.getSize() + mZeroPage.getSize()))
    {
        setMemoryBank(ii, mZeroPage);
        setMemoryBank(ii  + mZeroPage.getSize(), mRAM);
    }

    // PPU Memory
    for (size_t ii = 0x2000; ii < 0x4000; ii += ppu.getSize())
    {
        setMemoryBank(ii, ppu);
    }

    //! TODO: This is a tempory hack to prevent from
    //        reading and writing to a NULL register.
    setMemoryBank(0x4000, mFill1);

    setMemoryBank(0x4014, ppu.getOamDma());

    setMemoryBank(0x4015, mFill2);

}

/*****************************************************************************/
MemorySystem::~MemorySystem()
{
}
}
}