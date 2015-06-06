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
#ifndef __NYRA_NES_PPU_REGISTERS_H__
#define __NYRA_NES_PPU_REGISTERS_H__

#include <nes/Memory.h>
#include <nes/MemoryMap.h>
#include <nes/Constants.h>
#include <nes/HiLowLatch.h>
#include <bitset>
#include <vector>

namespace nyra
{
namespace nes
{
class OamDma : public Memory
{
public:
    OamDma(HiLowLatch& spriteRamAddress);

    uint8_t readByte(size_t )
    {
        return static_cast<uint8_t>(mMemory.to_ulong());
    }

    virtual void writeByte(size_t ,
                           uint8_t value)
    {
        mSpriteRamAddress.setHigh(value);
    }

    std::bitset<FLAG_SIZE>& getRegister()
    {
        return mMemory;
    }
private:
    std::bitset<FLAG_SIZE> mMemory;
    HiLowLatch& mSpriteRamAddress;
};

class PPURegisters : public Memory
{
public:
    enum Register
    {
        PPUCTRL = 0,
        PPUMASK,
        PPUSTATUS,
        OAMADDR,
        OAMDATA,
        PPUSCROLL,
        PPUADDR,
        PPUDATA,
        MAX_REGISTER,
        OAMDMA
    };

    enum
    {
        SPRITE_OFLOW = 5,
        SPRITE_HIT_0,
        VBLANK
    };

    enum
    {
        NAMETABLE_ADDRESS_LOW = 0,
        NAMETABLE_ADDRESS_HIGH,
        VRAM_INC,
        SPRITE_PATTERN_TABLE,
        BACKGROUND_PATTERN_TABLE,
        SPRITE_SIZE,
        MASTER_SLAVE,
        NMI_ENABLED
    };

    enum
    {
        GRAYSCALE = 0,
        BACKGROUND_LEFTMOST,
        SPRITES_LEFTMOST,
        SHOW_BACKGROUND,
        SHOW_SPRITES,
        EMPHASIZE_REF,
        EMPHASIZE_GREEN,
        EMPHASIZE_BLUE
    };

    PPURegisters(MemoryMap& vram);

    uint8_t readByte(size_t address);

    virtual void writeByte(size_t address,
                           uint8_t value);

    std::bitset<FLAG_SIZE>& getRegister(Register reg)
    {
        return reg != OAMDMA ? mMemory[reg] : mOamDma.getRegister();
    }

    OamDma& getOamDma()
    {
        return mOamDma;
    }

    uint16_t getSpriteRamAddress() const
    {
        return mSpriteRamAddress.get();
    }

    uint8_t getScrollX() const
    {
        return mScrollPosition.getHigh();
    }

    uint8_t getScrollY() const
    {
        return mScrollPosition.getLow();
    }

protected:
    std::bitset<FLAG_SIZE> mMemory[MAX_REGISTER];

    HiLowLatch mSpriteRamAddress;
    HiLowLatch mPPUAddress;

    // High is x, low is y
    HiLowLatch mScrollPosition;
    OamDma mOamDma;
    MemoryMap& mVRAM;
    uint8_t mByteBuffer;
};
}
}

#endif
