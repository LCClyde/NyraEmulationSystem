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
#include <core/Exception.h>

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
        std::fill_n(ret, size, 0xFF);
    }

    return ret;
}
}

namespace nyra
{
namespace nes
{
/*****************************************************************************/
Memory::Memory(const uint8_t* buffer,
               size_t size,
               bool takeOwnership) :
    mBufferInternal(takeOwnership ? buffer : nullptr),
    mBuffer(buffer),
    mSize(size)
{
}

/*****************************************************************************/
void Memory::writeByte(size_t , uint8_t )
{
    throw core::Exception("Cannot write memory to ROM!");
}

/*****************************************************************************/
RAM::RAM(const uint8_t* buffer,
         size_t size) :
    Memory(allocationTransitionBuffer(size, buffer), size, true),
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
    Memory(buffer, size, takeOwnership),
    mRAMBuffer(buffer)
{
}

/*****************************************************************************/
RAM::RAM(size_t size) :
    Memory(allocationTransitionBuffer(size, nullptr), size, true),
    // This is a legitimate use of const cast because we know we passed in a
    // non const buffer we just created. The Memory makes it const but we still
    // need access to it with the modifiers we passed it in with.
    mRAMBuffer(const_cast<uint8_t*>(mBuffer))
{
}
}
}
