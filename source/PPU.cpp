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
static const uint32_t NULL_PIXEL = 0xFFFFFFFF;
}

namespace nyra
{
namespace nes
{
/*****************************************************************************/
PPU::PPU(const ROMBanks& chrROM) :
    mVRAM(chrROM),
    mRegisters(mVRAM)
{
}

/*****************************************************************************/
void PPU::processScanline(CPUInfo& info,
                          const MemoryMap& memory,
                          uint32_t* buffer)
{
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
        break;
    }

    if (buffer && mRegisters.getRegister(PPURegisters::PPUMASK)[PPURegisters::SHOW_SPRITES])
    {
        renderScanline(info.scanLine, memory, buffer);
    }
}

/*****************************************************************************/
void PPU::renderScanline(int16_t scanLine,
                         const MemoryMap& memory,
                         uint32_t* buffer)
{
    //! Make sure this is renderable scanline
    if (scanLine <= VBLANK_END || scanLine >= VBLANK_START - 1)
    {
        return;
    }

    uint32_t* const ptr = buffer + (scanLine * SCREEN_WIDTH);

    const uint16_t spriteAddress = mRegisters.getSpriteRamAddress();

    //! Get the values
    for (size_t ii = 0; ii < 64 * 4; ii += 4)
    {
        // Get the y position
        const int16_t renderLine =
                memory.readByte(spriteAddress + ii) - scanLine;
        if (renderLine < 8 && renderLine >= 0)
        {
            // Get the sprite number
            const size_t address =
                    memory.readByte(spriteAddress + ii + 1) * 16;

            const size_t attributes = memory.readByte(spriteAddress + ii + 2);
            const size_t paletteNumber = attributes & 0x03;
            const bool flipHorizontally = (attributes & 0x40) > 0;

            const size_t xPosition = memory.readByte(spriteAddress + ii + 3);

            // Render this sprite into this line
            for (size_t ii = 0; ii < 8; ++ii)
            {
                const size_t pixelPosition = xPosition +
                        (flipHorizontally ? (7 - ii) : ii);

                // Note that pixelPosition is unsigned. If it goes negative,
                // it because max unsigned int.
                if (pixelPosition > 255)
                {
                    continue;
                }

                const uint32_t pixel = extractPixel(address + (7 - renderLine),
                                                    7 - ii,
                                                    paletteNumber);
                if (pixel != NULL_PIXEL)
                {
                    ptr[pixelPosition] = pixel;
                }
            }
        }
    }
}

/*****************************************************************************/
uint32_t PPU::extractPixel(uint32_t address,
                           size_t bitPosition,
                           size_t palette)
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
    const uint8_t addressOffset = (val1 | val2);
    if (addressOffset != 0)
    {
        const size_t paletteAddress = SPRITE_PALETTE_ADDRESS + (palette * 4) + addressOffset;
        return RGB_PALLETE[mVRAM.readByte(paletteAddress)];
    }
    return NULL_PIXEL;
}
}
}
