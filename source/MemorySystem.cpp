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
MemorySystem::MemorySystem(PPURegisters& ppu,
                           APU& apu,
                           Controller& controller1,
                           Controller& controller2) :
    mRAM(0x0700),
    mZeroPage(0x0100),
    mFill(0x3FE8)
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
    setMemoryBank(0x4000, apu.getRegisters());

    setMemoryBank(0x4014, ppu.getOamDma());
    setMemoryBank(0x4015, apu.getChannelInfo());

    // Controller memory
    setMemoryBank(0x4016, controller1);

    // TODO: 4007 controls both controller2 and the apu frame counter
    //       we need a way to handle this.
    //setMemoryBank(0x4017, controller2);
    setMemoryBank(0x4017, apu.getFrameCounter());

    setMemoryBank(0x4018, mFill);

}

/*****************************************************************************/
MemorySystem::~MemorySystem()
{
}
}
}