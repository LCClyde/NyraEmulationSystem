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
#include <nes/PPURegisters.h>
#include <core/Exception.h>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
OamDma::OamDma(HiLowLatch& spriteRamAddress) :
    Memory(1),
    mSpriteRamAddress(spriteRamAddress)
{
}

/*****************************************************************************/
PPURegisters::PPURegisters(MemoryMap& vram) :
    Memory(8),
    mOamDma(mSpriteRamAddress),
    mVRAM(vram)
{
}

/*****************************************************************************/
uint8_t PPURegisters::readByte(size_t address)
{
    uint8_t value = static_cast<uint8_t>(mMemory[address].to_ulong());
    switch (address)
    {
    case PPUSTATUS:
        mMemory[PPUSTATUS][VBLANK] = false;
        mPPUAddress.reset();
        break;
    case PPUDATA:
        //mPPUAddress.inc(mMemory[PPUCTRL][VRAM_INC ] ? 32 : 1);
        break;

    default:
        // Do Nothing
        break;
    }
    return value;
}


/*****************************************************************************/
void PPURegisters::writeByte(size_t address, uint8_t value)
{
    switch (address)
    {
    case OAMADDR:
        mSpriteRamAddress.setLow(value);
        break;
    case PPUADDR:
        mPPUAddress.set(value);
        break;
    case PPUDATA:
        mVRAM.writeByte(mPPUAddress.get(), value);
        mPPUAddress.inc(mMemory[PPUCTRL][VRAM_INC ] ? 32 : 1);
        break;
    default:
        mMemory[address] = value;
        break;
    }
}
}
}
