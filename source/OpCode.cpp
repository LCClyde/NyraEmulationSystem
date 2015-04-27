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
#include <nes/OpCode.h>
#include <core/StringConvert.h>
#include <core/Exception.h>

namespace
{
/*****************************************************************************/
class ModeAbsolute : public nyra::nes::OpCode::Mode
{
public:
    ModeAbsolute() :
        nyra::nes::OpCode::Mode(true, true)
    {
    }

    std::string toString(const nyra::nes::CPUArgs& args) const
    {
        return "$" + nyra::core::toHexString<uint16_t>(args.darg);
    }

    uint16_t getValue(uint16_t value) const
    {
        return value;
    }
};

class OpNUL : public nyra::nes::OpCode
{
public:
    OpNUL(uint8_t opCode) :
        nyra::nes::OpCode("NUL", "Null Opcode", opCode,
                          0, 0, new ModeAbsolute())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& ,
            nyra::nes::CPUInfo& ) const
    {
        throw nyra::core::Exception("Attempting to run null op: " + 
                nyra::core::toHexString<uint8_t>(mOpCode));
    }
};

/*****************************************************************************/
template <uint8_t OpCodeT, typename ModeT>
class OpJMP : public nyra::nes::OpCode
{
public:
    OpJMP() :
        nyra::nes::OpCode("JMP", "Jump", OpCodeT, 2, 3, new ModeT())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& ,
            nyra::nes::CPUInfo& info) const
    {
        info.programCounter = mMode->getValue(args.darg);
    }
};
}

namespace nyra
{
namespace nes
{
/*****************************************************************************/
void allocateOpCodes(OpCodeArray& opCodes)
{
    opCodes.resize(256);

    // Fill the known opcodes
    opCodes[0x4C].reset(new OpJMP<0x4C, ModeAbsolute>());

    // Fill all other opcodes with null values
    for (size_t ii = 0; ii < opCodes.size(); ++ii)
    {
        if (!opCodes[ii])
        {
            opCodes[ii].reset(new OpNUL(static_cast<uint8_t>(ii)));
        }
    }
}

/*****************************************************************************/
OpCode::Mode::Mode(bool usesArg1, bool usesArg2) :
    mUsesArg1(usesArg1),
    mUsesArg2(usesArg2)
{
}

/*****************************************************************************/
OpCode::Mode::~Mode()
{
}

/*****************************************************************************/
OpCode::OpCode(const std::string& name,
               const std::string& extendedName,
               uint8_t opCode,
               uint8_t length,
               uint8_t time,
               const Mode* mode) :
    mName(name),
    mExtendedName(extendedName),
    mOpCode(opCode),
    mLength(length),
    mTime(time),
    mMode(mode)
{
}

/*****************************************************************************/
OpCode::~OpCode()
{
}
}
}
