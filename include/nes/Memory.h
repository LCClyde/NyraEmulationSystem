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
#ifndef __NYRA_NES_MEMORY_H__
#define __NYRA_NES_MEMORY_H__

#include <stdint.h>
#include <memory>

namespace nyra
{
namespace nes
{
/*
 *  \class Memory
 *  \brief A generalized memory access class. Long term this is meant to
 *         be very reusable for any general memory configuration.
 *
 *  TODO: Add endianess support
 *  TODO: Should this have logic to throw when an index is out of bounds?
 *  TODO: Support Memory other than 8 bit.
 */
template <typename T>
class Memory
{
public:
    /*
     *  \func - Constructor (buffer)
     *  \brief - Creates a Memory object from an existing buffer. The object
     *           does not take ownership unless you tell it to do so.
     *           This will accept the buffer as is. It will not modify any
     *           values until explicitly told to do so.
     *
     *  \param buffer - The buffer of memory.
     *  \param size - The size of the buffer.
     *  \param takeOwnership - If this is true the RAM class will handle
     *         freeing this buffer.
     */
    Memory(T* buffer,
           size_t size,
           bool takeOwnership = false) :
        mBufferInternal(takeOwnership ? buffer : nullptr),
        mBuffer(buffer),
        mSize(size)
    {
    }

    /*
     *  \func - Destructor
     *  \brief - Base class destructor
     */
    virtual ~Memory()
    {
    }

    /*
     *  \func - writeByte
     *  \brief - Writes a single byte into memory at any address.
     *
     *  \param address - The address to write to byte at.
     *  \param value - The 1 byte value to write.
     *  \throw - If this is called from a ROM object, it will throw.
     */
    virtual void writeByte(size_t address,
                           uint8_t value) = 0;

    /*
     *  \func - readByte
     *  \brief - Reads a single byte from the memory structure.
     *
     *  \param address - The address to read from.
     */
    inline uint8_t readByte(size_t address) const
    {
        return mBuffer[address];
    }

    /*
     *  \func - readShort
     *  \brief - Reads a single short from the memory structure.
     *
     *  \param address - The address to read from.
     *  TODO: This is setup for a little endian system. We need a big
     *        endian version as well.
     */
    inline uint16_t readShort(size_t address) const
    {
        return ((readByte(address + 1) << 8 | readByte(address)));
    }

protected:
    const std::unique_ptr<T[]> mBufferInternal;
    T* const mBuffer;
    const size_t mSize;
};

/*
 *  \class - RAM
 *  \brief - Exposes 8 bit random access memory.
 */
class RAM : public Memory<uint8_t>
{
public:
    /*
     *  \func - Constructor (buffer)
     *  \brief - Creates a RAM object from an existing buffer. The RAM object
     *           does not take ownership unless you tell it to do so.
     *           This will accept the buffer as is. It will not modify any
     *           values until explicitly told to do so.
     *
     *  \param buffer - The buffer for the RAM.
     *  \param size - The size of the buffer.
     *  \param takeOwnership - If this is true the RAM class will handle
     *         freeing this buffer.
     */
    RAM(uint8_t* buffer,
        size_t size,
        bool takeOwnership = false);

    /*
     *  \func - Constructor (size)
     *  \brief - Creates and allocates a new piece of memory. This can then be
     *           placed in a memory map to be used in a larger application.
     *           Memory will start zero'd out.
     *
     *  \param size - The size in bytes of the memory.
     */
    RAM(size_t size);

    /*
     *  \func - writeByte
     *  \brief - Writes a single byte into memory at any address.
     *
     *  \param address - The address to write to byte at.
     *  \param value - The 1 byte value to write.
     */
    inline void writeByte(size_t address,
                          uint8_t value)
    {
        mBuffer[address] = value;
    }
};

/*
 *  \class - ROM
 *  \brief - Exposes 8 bit read only memory.
 */
class ROM : public Memory<const uint8_t>
{
public:
    /*
     *  \func - Constructor (buffer)
     *  \brief - Creates a ROM object from an existing buffer. The ROM object
     *           does not take ownership unless you tell it to do so.
     *           This will accept the buffer as is.
     *
     *  \param buffer - The buffer for the ROM.
     *  \param size - The size of the buffer.
     *  \param takeOwnership - If this is true the RAM class will handle
     *         freeing this buffer.
     */
    ROM(const uint8_t* buffer,
        size_t size,
        bool takeOwnership = false);

    /*
     *  \func - writeByte
     *  \brief - ROM does not implement this function.
     *
     *  \throw - Calling will function will throw.
     */
     void writeByte(size_t address,
                    uint8_t value);
};
}
}
#endif
