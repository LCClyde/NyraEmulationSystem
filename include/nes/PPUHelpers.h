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
#ifndef __NYRA_NES_PPU_HELPERS_H__
#define __NYRA_NES_PPU_HELPERS_H__

#include <stdint.h>
#include <bitset>
#include <nes/Constants.h>

namespace nyra
{
namespace nes
{
/*
 *  \enum - Status Flags
 *  \brief - Locations of flags for the PPUSTATUS variable.
 */
enum
{
    SPRITE_OVERFLOW = 5,
    SPRITE_HIT_0,
    VBLANK
};

/*
 *  \class - PPURegisters
 *  \brief - Holds the PPU specific register values.
 */
struct PPURegisters
{
    /*
     *  \func - Constructor
     *  \brief - Sets up a zero'd out registers struct.
     */
    PPURegisters();

    const size_t statusAddress;

    std::bitset<FLAG_SIZE> controller;
    std::bitset<FLAG_SIZE> mask;
    std::bitset<FLAG_SIZE> status;
};
}
}
#endif
