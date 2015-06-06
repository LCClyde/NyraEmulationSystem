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
#include <iostream>
#include <algorithm>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
MemoryMap::MemoryHandle::MemoryHandle(size_t offset, Memory& memory) :
    memory(&memory),
    offset(offset)
{
}

/*****************************************************************************/
MemoryMap::~MemoryMap()
{
}

/*****************************************************************************/
const MemoryMap::MemoryHandle& MemoryMap::getMemoryBank(size_t& address) const
{
    const MemoryHandle& handle = mMemory[mLookUpTable[address]];
    address -= handle.offset;
    return handle;
}

/*****************************************************************************/
uint16_t MemoryMap::readShort(size_t address) const
{
    size_t modAddress = address;
    const MemoryHandle* handle = &getMemoryBank(modAddress);
    const uint16_t ret = handle->memory->readByte(modAddress);

    // Check if this is zero page
    // TODO: Is this logic correct for VRAM as well?
    if (address >= 0x0100)
    {
        modAddress = address + 1;
        handle = &getMemoryBank(modAddress);
    }
    else
    {
        modAddress = (modAddress + 1) & 0xFF;
    }
    return (handle->memory->readByte(modAddress) << 8) | ret;
}

/*****************************************************************************/
void MemoryMap::lockLookUpTable()
{
    mLookUpTable.clear();
    std::sort(mMemory.begin(), mMemory.end());

    for (size_t ii = 0; ii < mMemory.size(); ++ii)
    {
        for (size_t jj = 0; jj < mMemory[ii].memory->getSize(); ++jj)
        {
            mLookUpTable.push_back(ii);
        }
    }
}
}
}
