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
#ifndef __NYRA_NES_CONTROLLER_H__
#define __NYRA_NES_CONTROLLER_H__

#include <nes/Memory.h>

namespace nyra
{
namespace nes
{
class Controller : public Memory
{
public:
    enum ControllerBit
    {
        BUTTON_A,
        BUTTON_B,
        BUTTON_SELECT,
        BUTTON_START,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_MAX
    };

    Controller();

    void writeByte(size_t address,
                   uint8_t value);

    uint8_t readByte(size_t address);

    inline void setKey(ControllerBit index, bool value = true)
    {
        if (value)
        {
            mButtonsQueued[index] = value;
        }
    }

private:
    bool mStrobe;
    ControllerBit mIndex;
    bool mButtons[BUTTON_MAX];
    bool mButtonsQueued[BUTTON_MAX];
};
}
}

#endif
