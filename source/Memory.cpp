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
#include <nes/Memory.h>
#include <stdexcept>

namespace
{
/*****************************************************************************/
uint8_t* allocationTransitionBuffer(size_t size, const uint8_t* copyFrom)
{
    uint8_t* ret = new uint8_t[size];
    if (copyFrom)
    {
        std::copy(copyFrom, copyFrom + size, ret);
    }
    else
    {
        std::fill_n(ret, size, 0x00);
    }

    return ret;
}
}

namespace nyra
{
namespace nes
{
/*****************************************************************************/
Memory::Memory(size_t size) :
    mSize(size)
{
}

/*****************************************************************************/
Memory::~Memory()
{
}

/*****************************************************************************/
void Memory::writeByte(size_t address,
                       uint8_t value)
{
    throw std::runtime_error("Cannot write to ram");
}

/*****************************************************************************/
uint8_t Memory::readByte(size_t address)
{
    throw std::runtime_error("Cannot read byte from ram");
}

/*****************************************************************************/
uint16_t Memory::readShort(size_t address)
{
    throw std::runtime_error("Cannot read short from ram");
}

/*****************************************************************************/
ROM::ROM(const uint8_t* buffer,
         size_t size,
         bool takeOwnership) :
    Memory(size),
    mBufferInternal(takeOwnership ? buffer : nullptr),
    mBuffer(buffer)
{
}

/*****************************************************************************/
ROM::~ROM()
{
}

/*****************************************************************************/
RAM::RAM(const uint8_t* buffer,
         size_t size) :
    ROM(allocationTransitionBuffer(size, buffer), size, true),
    // This is a legitimate use of const cast because we know we passed in a
    // non const buffer we just created. The Memory makes it const but we still
    // need access to it with the modifiers we passed it in with.
    mRAMBuffer(const_cast<uint8_t*>(mBuffer))
{
}

/*****************************************************************************/
RAM::RAM(uint8_t* buffer,
         size_t size,
         bool takeOwnership) :
    ROM(buffer, size, takeOwnership),
    mRAMBuffer(buffer)

{
}

/*****************************************************************************/
RAM::RAM(size_t size) :
    ROM(allocationTransitionBuffer(size, nullptr), size, true),
    // This is a legitimate use of const cast because we know we passed in a
    // non const buffer we just created. The Memory makes it const but we still
    // need access to it with the modifiers we passed it in with.
    mRAMBuffer(const_cast<uint8_t*>(mBuffer))
{
}
}
}
