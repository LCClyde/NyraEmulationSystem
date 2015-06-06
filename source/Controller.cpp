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
#include <nes/Controller.h>
#include <iostream>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
Controller::Controller() :
    Memory(1),
    mStrobe(false),
    mIndex(BUTTON_A)
{
    std::fill_n(mButtons, static_cast<size_t>(BUTTON_MAX), false);
    std::fill_n(mButtonsQueued, static_cast<size_t>(BUTTON_MAX), false);
}

/*****************************************************************************/
void Controller::writeByte(size_t ,
                           uint8_t value)
{
    mStrobe = ((value & 0x01) == 0x01);
    mIndex = BUTTON_A;
    if (mStrobe)
    {
        std::copy(mButtonsQueued, mButtonsQueued + BUTTON_MAX, mButtons);
        std::fill_n(mButtonsQueued, static_cast<size_t>(BUTTON_MAX), false);
    }
}

/*****************************************************************************/
uint8_t Controller::readByte(size_t )
{
    // TODO: This will probably never be used?
    //       It is currently unexpectly entered because of the way the memory
    //       is handled. This should be reworked so it never enters here as a
    //       side effect.
    if (mStrobe)
    {
        // This should actually strobe the controller?
        return mButtons[BUTTON_A] ? 0x01 : 0x00;
    }
    const uint8_t ret = mButtons[mIndex] ? 0x01 : 0x00;
    mIndex = static_cast<ControllerBit>((mIndex + 1) % BUTTON_MAX);
    return ret;
}
}
}
