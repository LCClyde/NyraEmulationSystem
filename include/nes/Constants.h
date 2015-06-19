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
#ifndef __NYRA_NES_CONSTANTS_H__
#define __NYRA_NES_CONSTANTS_H__

#include <vector>
#include <memory>

namespace nyra
{
namespace nes
{

/*
 *  \Constant - FLAG_SIZE
 *  \brief - The number of flags in a register. For the 6502 these are
 *            8 bit values.
 */
static const size_t FLAG_SIZE = 8;

static const size_t SCREEN_WIDTH = 256;
static const size_t SCREEN_HEIGHT = 240;
static const size_t NUM_PIXELS = SCREEN_WIDTH * SCREEN_HEIGHT;

//! NTSC CPU clock in MHz
static const double CPU_CLOCK = 1.789773;

/*
 *  \type - ROMBanks
 *  \brief - A vector of ROM objects. This is used to be able to
 *           easily pass around several ROM objects in a form that is
 *           usable in the emulator.
 */
class ROM;
class RAM;
typedef std::vector<std::unique_ptr<ROM> > ROMBanks;
typedef std::vector<std::unique_ptr<RAM> > RAMBanks;

/*
 *  \enum - Mirroring
 *  \brief - Describes how the nametables should be presented to
 *           the player.
 */
enum Mirroring
{
    HORIZONTAL,
    VERTICAL
};
}
}

#endif
