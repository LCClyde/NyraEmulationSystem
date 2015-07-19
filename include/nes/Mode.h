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
#ifndef __NYRA_NES_MODE_H__
#define __NYRA_NES_MODE_H__

#include <nes/CPUHelper.h>
#include <nes/MemoryMap.h>

namespace nyra
{
namespace nes
{
/*
 *  \class - Mode
 *  \brief - Used to determine how the arguments are used. Like the OpCode
 *           this is heavily abstracted.
 */
class Mode
{
public:
    /*
     *  \func - Constructor
     *  \brief - Sets up a Mode object.
     *
     *  \param useArg1 - Does this Mode use the first byte argument?
     *  \param useArg2 - Does this Mode use the second byte argument?
     *  \NOTE: If the mode uses both arguments as a single 2 byte value,
     *         pass in true for both parameters.
     */
    Mode(bool usesArg1, bool usesArg2);

    /*
     *  \func - Destructor
     *  \brief - Used to create reliable inheritance.
     */
    virtual ~Mode();

    /*
     *  \func - useArg1
     *  \brief - returns true if this mode uses arg1.
     */
    virtual bool usesArg1() const
    {
        return mUsesArg1;
    }

    /*
     *  \func - useArg2
     *  \brief - returns true if this mode uses arg2.
     */
    virtual bool usesArg2() const
    {
        return mUsesArg2;
    }

    /*
     *  \func - getArg
     *  \brief - Gets the argument.
     */
    inline uint16_t getArg() const
    {
        return mArg;
    }

    /*
     *  \func - getValue
     *  \brief - Gets the calculated value. This is mode independent.
     */
    inline uint8_t getValue() const
    {
        return mValue;
    }

    /*
     *  \func - getValue
     *  \brief - Process all mode information
     *
     *  \param args - The input arguments to the mode.
     *  \param registers - The current CPU registers
     *  \param memory - The filled out memory banks
     *  \param info - The current CPU info struct.
     */
    virtual void operator()(const CPUArgs& args,
                            const CPURegisters& registers,
                            const MemoryMap& memory,
                            CPUInfo& info) = 0;

protected:
    const bool mUsesArg1;
    const bool mUsesArg2;
    uint16_t mArg;
    uint8_t mValue;
};
}
}

#endif
