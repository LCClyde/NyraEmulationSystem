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
#ifndef __NYRA_NES_CPU_HELPER_H__
#define __NYRA_NES_CPU_HELPER_H__

#include <stdint.h>
#include <bitset>

namespace nyra
{
namespace nes
{
/*
 *  \class - CPURegisters
 *  \brief - Holds the registers for a 6502 processor.
 */
struct CPURegisters
{
    /*
     *  \enum - Status codes
     *  \brief - Represents the possible bits on the status register.
     */
    enum
    {
        CARRY = 0,
        ZERO,
        INTERRUPT,
        DECIMAL,
        STACK,
        IGNORE,
        OFLOW,
        SIGN
    };

    /*
     *  \func - Constructor
     *  \brief - Creates a CPURegisters object with zero'd out data.
     */
    CPURegisters();

    uint8_t accumulator;
    uint8_t xIndex;
    uint8_t yIndex;
    uint8_t stackPointer;
    std::bitset<8> statusRegister;
};

/*
 *  \class - CPUInfo
 *  \brief - Holds general information about the 6502 CPU processor.
 */
struct CPUInfo
{
    /*
     *  \func - Constructor
     *  \brief - Creates a CPUInfo object with simple information.
     *           In general this should not be used. This is here to allow
     *           objects to be placed in containers with some reasonable values
     */
    CPUInfo();

    /*
     *  \func - Constructor (address)
     *  \brief - Creates a CPUInfo with a starting address. In general this is
     *           best constructor to use with a CPU object.
     */
    CPUInfo(uint16_t programCounter);

    uint16_t programCounter;
    uint16_t cycles;
    int16_t scanLine;
};

/*
 *  \class - CPUArgs
 *  \brief - Convenience class used to make passing around opcode arguments
 *           easier.
 *
 *  \TODO: Should we remove the darg variable? It can easily be obtained
 *         from arg1 and arg2.
 */
struct CPUArgs
{
    /*
     *  \func - Constructor
     *  \brief - Creates a zero'd out CPUArgs object.
     */
    CPUArgs();

    uint8_t opcode;
    uint8_t arg1;
    uint8_t arg2;
    uint16_t darg;
};
}
}

#endif
