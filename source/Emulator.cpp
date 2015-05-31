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
#include <nes/Emulator.h>
#include <nes/MemoryFactory.h>
#include <nes/Constants.h>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
Emulator::Emulator(const std::string& pathname) :
    mCartridge(pathname),
    mPPU(mCartridge.getChrROM()),
    mMemoryMap(createMemoryMap(mCartridge, mPPU)),
    mCPU(mMemoryMap->readShort(0xFFFC))
{
}

/*****************************************************************************/
void Emulator::tick(uint32_t* buffer,
                    std::vector<Disassembly>* disassembly)
{
    if (disassembly)
    {
        disassembly->clear();
    }

    // Clear buffer
    if (buffer)
    {
        std::fill_n(buffer, NUM_PIXELS, 0);
    }

    do
    {
        mCPU.processScanline(*mMemoryMap,
                             disassembly);
        mPPU.processScanline(mCPU.getInfo(),
                             *mMemoryMap,
                             buffer);

    } while (mCPU.getInfo().scanLine >= 0);
}
}
}
