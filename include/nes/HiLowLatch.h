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
#ifndef __NYRA_NES_HI_LOW_LATCH_H__
#define __NYRA_NES_HI_LOW_LATCH_H__

#include <stdint.h>
#include <iostream>

namespace nyra
{
namespace nes
{
class HiLowLatch
{
public:
    HiLowLatch();

    inline void set(uint8_t value)
    {
        if (!mHighSet)
        {
            setHigh(value);
        }
        else
        {
            setLow(value);
        }
    }

    inline void setLow(uint8_t value)
    {
        mValue = (mValue & 0xFF00) | value;
    }

    inline void setHigh(uint8_t value)
    {
        mValue = (mValue & 0x00FF) | (value << 8);
        mHighSet = true;
    }

    inline void reset()
    {
        mHighSet = false;
        mValue = 0;
    }

    inline uint16_t get() const
    {
        return mValue;
    }

    inline void inc(uint8_t amount)
    {
        mValue += amount;
    }

private:
    bool mHighSet;
    uint16_t mValue;
};
}
}

#endif
