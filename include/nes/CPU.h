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
#ifndef __NYRA_NES_CPU_H__
#define __NYRA_NES_CPU_H__

#include <stdint.h>
#include <nes/MemoryMap.h>
#include <nes/CPUHelper.h>
#include <nes/OpCode.h>

namespace nyra
{
namespace nes
{
/*
 *  \class - CPU
 *  \brief - Emulates the 6502 central processing unit. Run tick to process
 *           an opcode.
 *  TODO: This could be generalized for any CPU. It would need to be setup
 *        to allow the number of bits to be set, probably through a template.
 */
class CPU
{
public:
    /*
     *  \func - Constructor (address)
     *  \brief - Creates a CPU object with a starting memory address.
     *
     *  \param startAddress - The location to start reading opcodes from.
     *  TODO: Should this also have a version that takes in a MemoryMap and
     *        resolves the startAddress itself?
     */
    CPU(uint16_t startAddress);

    /*
     *  \func - tick
     *  \brief - Processes a single opcode.
     *
     *  \param memory - All the available memory as swappable banks.
     *  \param disassembly [OPTIONAL] - Allows you pass in a disassembly
     *         object which can then be used to view information about
     *         the processed opcode.
     *         Remove this to increase performance.
     */
    void processScanline(MemoryMap& memory);

    void processScanline(const std::unique_ptr<MemoryMap>& memory)
    {
        processScanline(*memory);
    }

    /*
     *  \func - getInfo
     *  \brief - Returns the CPU implementation info.
     */
    inline const CPUInfo& getInfo() const
    {
        return mInfo;
    }

    inline CPUInfo& getInfo()
    {
        return mInfo;
    }

private:
    static const size_t INTERRUPT_OPCODE;
    CPURegisters mRegisters;
    CPUInfo mInfo;
    CPUArgs mArgs;
    OpCodeArray mOpCodes;
};
}
}

#endif
