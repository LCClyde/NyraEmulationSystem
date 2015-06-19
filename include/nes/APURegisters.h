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
#ifndef __NYRA_NES_APU_REGISTERS_H__
#define __NYRA_NES_APU_REGISTERS_H__

#include <nes/Constants.h>

namespace nyra
{
namespace nes
{
class Pulse
{
public:
    Pulse(const uint8_t& envelope,
          const uint8_t& sweep,
          const uint8_t& timer,
          const uint8_t& lengthCounter);

    inline uint8_t getDuty() const
    {
        return (mEnvelope & 0xC0) >> 6;
    }

    inline bool getEnvelopLoop() const
    {
        return (mEnvelope & 0x20) != 0;
    }

    inline bool getConstantVolume() const
    {
        return (mEnvelope & 0x10) != 0;
    }

    inline uint8_t getVolume() const
    {
        return (mEnvelope & 0x0F);
    }

    inline bool getSweepUnitEnabled() const
    {
        return (mSweep & 0x80) != 0;
    }

    inline uint8_t getPeriod() const
    {
        return (mSweep &  0x70) >> 4;
    }

    inline bool getNegate() const
    {
        return (mSweep & 0x08) != 0;
    }

    inline uint8_t getSweep() const
    {
        return (mSweep & 0x07);
    }

    inline uint16_t getTimer() const
    {
        return static_cast<uint16_t>(mTimer) |
                (static_cast<uint16_t>(mLengthCounter & 0x07) << 8);
    }

    inline uint8_t getLengthCounter() const
    {
        return mLengthCounter >> 3;
    }

    inline double getFrequency() const
    {
        return CPU_CLOCK / (16.0 * (getTimer() + 1));
    }

private:
    const uint8_t& mEnvelope;
    const uint8_t& mSweep;
    const uint8_t& mTimer;
    const uint8_t& mLengthCounter;
};
}
}

#endif
