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
#ifndef __NYRA_NES_DISASSEMBLY_H__
#define __NYRA_NES_DISASSEMBLY_H__

#include <stdint.h>
#include <nes/OpCode.h>
#include <nes/CPUHelper.h>

namespace nyra
{
namespace nes
{
/*
 *  \class - Disassembly
 *  \brief - Can be used to store information about a CPU tick. This can then
 *           be easily dumped to stream.
 */
class Disassembly
{
public:
    /*
     *  \func - Constructor
     *  \brief - The default constructor with all zero'd out values. This is
     *           here to allows a Disassembly object to go into containers.
     *           It is not safe to try to use an object constructed this way.
     */
    Disassembly();

    /*
     *  \func - Constructor (info)
     *  \brief - Creates a Disassembly from passed in values. All values will
     *           be copied except for the OpCode which is held onto by
     *           reference.
     *           Be sure the OpCodes outlive the Disassembly object.
     *
     *  \param opCode - The op code processed this tick. Because of the way
     *         OpCodes cannot be copy constructed, this is held onto
     *         internally by reference.
     *  \param args - A filled out CPUArgs structure.
     *  \param registers - A filled out CPURegisters structure.
     *  \param info - A filled out CPUInfo structure.
     */
    Disassembly(const OpCode& opCode,
                const CPUArgs& args,
                const CPURegisters& registers,
                const CPUInfo info);

    /*
     *  \func - Constructor (copy)
     *  \brief - Copies a Disassembly object.
     *
     *  \param other - The Disassembly to copy.
     */
    Disassembly(const Disassembly& other);

    /*
     *  \func operator (assignment)
     *  \brief - Assigns a Disassembly object.
     *
     *  \param other - The Disassembly to assign from.
     *  \return - The assigned object.
     */
    Disassembly& operator=(Disassembly other);

    const OpCode& getOpCode() const
    {
        return *mOpCode;
    }

    const CPUArgs& getArgs() const
    {
        return mArgs;
    }

    const CPURegisters& getRegisters() const
    {
        return mRegisters;
    }

    const CPUInfo& getInfo() const
    {
        return mInfo;
    }

    void setModeString(const std::string& mode)
    {
        mModeString = mode;
    }

    std::string getModeString() const
    {
        return mModeString;
    }

private:
    void swap(Disassembly& first, Disassembly& second);

    //! TODO: Can we get rid of this pointer and find a way to copy construct
    //        an OpCode? Then this can live independent of the CPU.
    //! TODO: This needs a lot of work. The OpCode mode should not control the
    //        string output. This should be contolled in python.
    const OpCode* mOpCode;
    CPUArgs mArgs;
    CPURegisters mRegisters;
    CPUInfo mInfo;
    std::string mModeString;
};
}
}

#endif
