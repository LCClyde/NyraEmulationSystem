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
#ifndef __NYRA_NES_MEMORY_MAP_H__
#define __NYRA_NES_MEMORY_MAP_H__

#include <stdint.h>
#include <map>
#include <nes/Memory.h>
#include <nes/CPUHelper.h>

namespace nyra
{
namespace nes
{
/*
 *  \class - MemoryMap
 *  \brief - Holds banks of memory which can then be read as it it was one
 *           contiguous buffer.
 *  TODO: Reimplement this using a look up table. A map should be assigned to
 *        and then locked. Once locked it creates the look up table of values.
 *        this removes the lower bound logic and allows direct memory access.
 */
class MemoryMap
{
private:
    // TODO: Ideally this should hold a reference to a Memory, not a pointer.
    //       A first quick pass at this seemed to show this was not possible.
    //       This needs to be investigated further.
    typedef std::map<size_t, Memory*> RamMap;

public:
    virtual ~MemoryMap();

    /*
     *  \func - setMemoryBank
     *  \brief - Adds a memory bank into the map. It is the user's job to
     *           ensure banks do not cross. If they do the behavior is
     *           undefined.
     *
     *  \param memoryOffset - The starting address of the memory.
     *  \param memory - The memory object that will go into this address.
     */
    inline void setMemoryBank(size_t memoryOffset, Memory& memory)
    {
        mMap[memoryOffset] = &memory;
    }

    /*
     *  \func - writeByte
     *  \brief - Write a single byte into MemoryMap.
     *
     *  \param address - The global address to write to
     *  \param value - The value to write.
     */
    inline void writeByte(size_t address, uint8_t value)
    {
        RamMap::const_iterator iter = getMemoryBank(address);
        iter->second->writeByte(address, value);
    }

    /*
     *  \func - getOpInfo
     *  \brief - Used to get all potential information about an operation
     *           in ROM.
     *
     *  \param address - The global address of the op.
     *  \param args[OUTPUT] - The output for all op arg information.
     */
    inline void getOpInfo(size_t address,
                          CPUArgs& args)
    {
        RamMap::const_iterator iter = getMemoryBank(address);
        args.opcode = iter->second->readByte(address);
        args.arg1 = iter->second->readByte(address + 1);
        args.arg2 = iter->second->readByte(address + 2);

        // TODO: Is it safe to get the dword in this way? Should we instead
        //       call MemoryMap::readShort as this has extra statements to
        //       protect against going out of bounds in a memory bank.
        args.darg = iter->second->readShort(address + 1);
    }

    /*
     *  \func readByte
     *  \brief - Reads a single byte from global memory.
     *
     *  \param address - The global adddress to read from.
     *  \return - The value at that address.
     */
    inline uint8_t readByte(size_t address) const
    {
        RamMap::const_iterator iter = getMemoryBank(address);
        return iter->second->readByte(address);
    }

    /*
     *  \func readShort
     *  \brief - Reads a single short from global memory. This properly
     *           handles reaching the end of a bank.

     *
     *  \param address - The global adddress to read from.
     *  \return - The value at that address.
     */
    uint16_t readShort(size_t address) const;

private:
    RamMap::const_iterator getMemoryBank(size_t& address) const;

    RamMap mMap;
};
}
}

#endif
