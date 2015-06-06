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
#include <nes/Constants.h>
#include <iostream>
#include <core/StringConvert.h>

namespace
{
/*****************************************************************************/
uint32_t rgb(uint8_t r, uint8_t g, uint8_t b)
{
    return (r << 16) | (g << 8) | b;
}

/*****************************************************************************/
static const int16_t VBLANK_START = 241;
static const int16_t VBLANK_END = -1;
static const size_t SPRITE_PALETTE_ADDRESS = 0x3F10;
static const size_t BACKGROUND_PALETTE_ADDRESS = 0x3F00;
static const uint32_t RGB_PALLETE[64] =
{
rgb( 84,  84,  84), rgb(  0,  30, 116), rgb(  8,  16, 144), rgb( 48,   0, 136),
rgb( 68,   0, 100), rgb( 92,   0,  48), rgb( 84,   4,   0), rgb( 60,  24,   0),
rgb( 32,  42,   0), rgb(  8,  58,   0), rgb(  0,  64,   0), rgb(  0,  60,   0),
rgb(  0,  50,  60), rgb(  0,   0,   0), rgb(  0,   0,   0), rgb(  0,   0,   0),
rgb(152, 150, 152), rgb(  8,  76, 196), rgb( 48,  50, 236), rgb( 92,  30, 228),
rgb(136,  20, 176), rgb(160,  20, 100), rgb(152,  34,  32), rgb(120,  60,   0),
rgb( 84,  90,   0), rgb( 40, 114,   0), rgb(  8, 124,   0), rgb(  0, 118,  40),
rgb(  0, 102, 120), rgb(  0,   0,   0), rgb(  0,   0,   0), rgb(  0,   0,   0),
rgb(236, 238, 236), rgb( 76, 154, 236), rgb(120, 124, 236), rgb(176,  98, 236),
rgb(228,  84, 236), rgb(236,  88, 180), rgb(236, 106, 100), rgb(212, 136,  32),
rgb(160, 170,   0), rgb(116, 196,   0), rgb( 76, 208,  32), rgb( 56, 204, 108),
rgb( 56, 180, 204), rgb( 60,  60,  60), rgb(  0,   0,   0), rgb(  0,   0,   0),
rgb(236, 238, 236), rgb(168, 204, 236), rgb(188, 188, 236), rgb(212, 178, 236),
rgb(236, 174, 236), rgb(236, 174, 212), rgb(236, 180, 176), rgb(228, 196, 144),
rgb(204, 210, 120), rgb(180, 222, 120), rgb(168, 226, 144), rgb(152, 226, 180),
rgb(160, 214, 228), rgb(160, 162, 160), rgb(  0,   0,   0), rgb(  0,   0,   0)
};
}

namespace nyra
{
namespace nes
{
/*****************************************************************************/
PPU::PPU(const ROMBanks& chrROM,
         Mirroring mirroring) :
    mVRAM(chrROM, mirroring),
    mRegisters(mVRAM),
    mOAM(256)
{
}

/*****************************************************************************/
void PPU::processScanline(CPUInfo& info,
                          const MemoryMap& memory,
                          uint32_t* buffer)
{
    // Check for OAM copy
    if (mRegisters.getOamDma().needsCopy())
    {
        const uint16_t address = mRegisters.getSpriteRamAddress();
        for (size_t ii = 0; ii < mOAM.getSize(); ++ii)
        {
            mOAM.writeByte(ii, memory.readByte(address + ii));
        }
    }

    switch (info.scanLine)
    {
    case VBLANK_START:
        // Set the VBLANK flag
        mRegisters.getRegister(
                PPURegisters::PPUSTATUS)[PPURegisters::VBLANK] = true;

        // Check for NMI
        info.generateNMI =mRegisters.getRegister(
                PPURegisters::PPUCTRL)[PPURegisters::NMI_ENABLED];
        break;
    case VBLANK_END:
        // Set the VBLANK flag
        mRegisters.getRegister(
                PPURegisters::PPUSTATUS)[PPURegisters::VBLANK] = false;
        mRegisters.getRegister(
                PPURegisters::PPUSTATUS)[PPURegisters::SPRITE_HIT_0] = false;
        break;
    }

    if (buffer && mRegisters.getRegister(PPURegisters::PPUMASK)[PPURegisters::SHOW_SPRITES])
    {
        renderScanline(info.scanLine, buffer);
    }
}

/*****************************************************************************/
void PPU::renderScanline(int16_t scanLine,
                         uint32_t* buffer)
{
    //! Make sure this is renderable scanline
    if (scanLine <= VBLANK_END || scanLine >= VBLANK_START - 1)
    {
        return;
    }

    uint32_t* const ptr = buffer + (scanLine * SCREEN_WIDTH);
    const uint32_t backgroundColor =
            RGB_PALLETE[mVRAM.getBackgroundColor()];
    size_t paletteAddress;
    const uint8_t scrollX = mRegisters.getScrollX();
    const size_t backgroundPatternTable =
            mRegisters.getRegister(PPURegisters::PPUCTRL)
            [PPURegisters::BACKGROUND_PATTERN_TABLE] ? 0x1000 : 0x0000;

    //! TODO: This should be & 0x03 and take the y nametable into
    //        consideration
    const uint8_t nametableBaseAddress = mRegisters.getRegister(
            PPURegisters::PPUCTRL).to_ulong() & 0x01;

    //! TODO: Scroll in the y direction
    //! Render background
    const size_t backgroundY = scanLine / 8;

    //! TODO: This needs a lot of clean up.
    for (int32_t ii = 0; ii < 33; ++ii)
    {
        // Get the background X position
        int32_t backgroundX = ii + (scrollX / 8);
        size_t baseNametableAddress;

        // If the background X is greater than 32, we need to go to the next
        // nametable
        if (backgroundX >= 32)
        {
            backgroundX -= 32;
            baseNametableAddress = nametableBaseAddress ? 0x2000 : 0x2400;
        }
        else
        {
            baseNametableAddress = nametableBaseAddress ? 0x2400 : 0x2000;
        }

        // Get the background tile index
        const size_t backgroundIndex = mVRAM.readByte(
                baseNametableAddress + (backgroundY * 32) + backgroundX);

        const size_t pixelPosition = (ii * 8) - (scrollX % 8);
        const size_t address = backgroundPatternTable + (backgroundIndex * 16);

        // Get palette number
        const size_t attributeRow = backgroundY / 4;
        const size_t attributeCol = backgroundX / 4;
        const uint8_t attributeIndex =
                mVRAM.readByte(baseNametableAddress + 0x03C0 +
                        (attributeRow * 8) + attributeCol);

        // The palette that is returned is for four tiles.
        // value = (topleft << 0) |
        //         (topright << 2) |
        //         (bottomleft << 4) |
        //         (bottomright << 6)
        const uint8_t paletteIndex =
                (((backgroundX / 2) % 2) + (((backgroundY / 2) %  2) * 2)) * 2;
        const uint8_t paletteNumber = (attributeIndex >> paletteIndex) & 0x03;

        // Render this background
        for (size_t jj = 0; jj < 8; ++jj)
        {
            // Make sure the pixel is in a valid range
            if (pixelPosition + jj >= 0 && pixelPosition + jj < 256)
            {
                ptr[pixelPosition + jj] = extractPixel(
                        address + (scanLine % 8),
                        7 - jj,
                        BACKGROUND_PALETTE_ADDRESS + (paletteNumber * 4),
                        backgroundColor,
                        paletteAddress);
            }
        }
    }

    const uint16_t spriteAddress = 0;

    //! Get the values
    for (size_t ii = 0; ii < 64 * 4; ii += 4)
    {
        // Get the y position
        const int16_t renderLine = static_cast<int16_t>(
                mOAM.readByte(spriteAddress + ii)) - scanLine + 8;
        if (renderLine < 8 && renderLine >= 0)
        {
            // Get the sprite number
            const size_t address =
                    mOAM.readByte(spriteAddress + ii + 1) * 16;

            const size_t attributes = mOAM.readByte(spriteAddress + ii + 2);
            const size_t paletteNumber = attributes & 0x03;
            const bool flipHorizontally = (attributes & 0x40) > 0;
            const bool flipVertically = (attributes & 0x80) > 0;

            const size_t xPosition = mOAM.readByte(spriteAddress + ii + 3);

            // Render this sprite into this line
            for (size_t jj = 0; jj < 8; ++jj)
            {
                const size_t pixelPosition = xPosition +
                        (flipHorizontally ? (7 - jj) : jj);

                // Note that pixelPosition is unsigned. If it goes negative,
                // it because max unsigned int.
                if (pixelPosition > 255)
                {
                    continue;
                }

                const uint32_t pixel = extractPixel(
                        address + (flipVertically ? renderLine : (7 - renderLine)),
                        7 - jj,
                        SPRITE_PALETTE_ADDRESS + (paletteNumber * 4),
                        backgroundColor,
                        paletteAddress);

                if (paletteAddress != 0)
                {
                    //! TODO: This actually needs to check if the pixel has
                    //        a palette of 0. There is the posibility that
                    //        a palette reuses the background color.
                    // Check for sprite hit 0
                    if (ii == 0 && ptr[pixelPosition] != backgroundColor)
                    {
                        mRegisters.getRegister(PPURegisters::PPUSTATUS)
                                [PPURegisters::SPRITE_HIT_0] = true;
                    }

                    ptr[pixelPosition] = pixel;

                }
            }
        }
    }
}

/*****************************************************************************/
uint32_t PPU::extractPixel(uint32_t address,
                           size_t bitPosition,
                           size_t palette,
                           uint32_t backgroundColor,
                           size_t& paletteAddress)
{
    //! TODO: Is there a reliable way to preprocess this
    //        information? I need more information about CHR ROM
    //        before I decide.If not is there a way to speed this
    //        up and do an entire line at a time followed by
    //        a memcpy?
    const uint8_t val1 =
            (mVRAM.readByte(address) >>
                    (bitPosition)) & 0x01;
    const uint8_t val2 =
            ((mVRAM.readByte(address + 8) >>
                    (bitPosition) & 0x01)) << 1;
    paletteAddress = (val1 | val2);

    if (paletteAddress == 0)
    {
        return backgroundColor;
    }

    return RGB_PALLETE[mVRAM.readByte(palette + paletteAddress)];
}
}
}
