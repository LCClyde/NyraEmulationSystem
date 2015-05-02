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
#ifndef __NYRA_NES_OP_CODE_H__
#define __NYRA_NES_OP_CODE_H__

#include <stdint.h>
#include <string>
#include <memory>
#include <vector>
#include <nes/CPUHelper.h>
#include <nes/MemoryMap.h>

namespace nyra
{
namespace nes
{
/*
 *  \class OpCode
 *  \brief - Represents a single abstract processing operation. This class
 *           is heavily abstracted. Be careful when trying to use it in new
 *           ways.
 */
class OpCode
{
public:
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
         *  \func - toString
         *  \brief - Outputs the syntax for using this mode.
         *
         *  \param args - The args to use when dumping the information.
         *  \return - The formatted string.
         */
        virtual std::string toString() const = 0;

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
         *  \func - getValue
         *  \brief - Processes a 2 byte value. This is defined in the
         *           specialized modes.
         *
         *  \param value - The value to use for processing.
         *  \param index - An addditional value used for processing
         *  \param memory - The filled out memory banks
         *  \return - The correct output value.
         */
        virtual uint16_t operator()(
                uint16_t value,
                uint16_t index,
                MemoryMap& memory) = 0;

    protected:
        const bool mUsesArg1;
        const bool mUsesArg2;
        uint16_t mParam1;
        uint16_t mParam2;
    };

    /*
     *  \func - Constructor
     *  \brief - Creates a generalized OpCode
     *
     *  \param name - The three letter name of the OpCode
     *  \param extendedName - A more user friendly version of the name.
     *  \param opCode - The opCode index.
     *  \param length - The amount the program counter should move
     *                  after calling the opcode.
     *  \param time - The number of cycles for the opcode.
     *  \param mode - A specialized mode which tells the opcode how to use
     *                parameters.
     */
    OpCode(const std::string& name,
           const std::string& extendedName,
           uint8_t opCode,
           uint8_t length,
           uint8_t time,
           Mode* mode);

    /*
     *  \func - Destructor
     *  \brief - Used to create reliable inheritance.
     */
    virtual ~OpCode();

    void operator()(const CPUArgs& args,
                    CPURegisters& registers,
                    CPUInfo& info,
                    MemoryMap& memory);

    /*
     *  \func - getName
     *  \brief - Returns the three letter name representing this opcode.
     */
    inline std::string getName() const
    {
        return mName;
    }

    /*
     *  \func - getExtendedName
     *  \brief - Returns a user friendly verison of the name of this opcode.
     */
    inline std::string getExtendedName() const
    {
        return mExtendedName;
    }

    /*
     *  \func - getMode
     *  \brief - Returns the mode this opcode uses for addressing.
     */
    inline const Mode& getMode() const
    {
        return *mMode;
    }

    /*
     *  \func - getOpCode
     *  \brief - Returns the identifier value of this opcode.
     */
    inline uint8_t getOpCode() const
    {
        return mOpCode;
    }

protected:
    virtual void op(const CPUArgs& args,
                    CPURegisters& registers,
                    CPUInfo& info,
                    MemoryMap& memory) = 0;

    const std::string mName;
    const std::string mExtendedName;
    const uint8_t mOpCode;
    const uint8_t mLength;
    const uint8_t mTime;
    const std::unique_ptr<Mode> mMode;
};

/*
 *  \type - OpCodeArray
 *  \brief - An array of opcodes which can then be indexed for calling ops.
 */
typedef std::vector<std::unique_ptr<OpCode> > OpCodeArray;

/*
 *  \func - allocateOpCodes
 *  \brief - Sets up an array of opcodes so the correct values are in each
 *           index. This is a pretty specialized function for 6502.
 */
void allocateOpCodes(OpCodeArray& opCodes);
}
}
#endif
