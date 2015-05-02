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
#include <nes/MemoryMap.h>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
MemoryMap::RamMap::const_iterator MemoryMap::getMemoryBank(
        size_t& address) const
{
    RamMap::const_iterator iter = mMap.upper_bound(address);
    --iter;
    address -= iter->first;
    return iter;
}

/*****************************************************************************/
uint16_t MemoryMap::readShort(size_t address) const
{
    size_t modAddress = address;
    RamMap::const_iterator iter = getMemoryBank(modAddress);
    uint16_t ret = iter->second->readByte(modAddress);

    // Check if we are at the end of the bank
    if (iter->first + iter->second->getSize() <= address + 1)
    {
        modAddress = 0;
        // Check if this is zero page
        if (address >= 0x0100)
        {
            ++iter;
        }
    }
    else
    {
        ++modAddress;
    }
    return (iter->second->readByte(modAddress) << 8) | ret;
}
}
}
