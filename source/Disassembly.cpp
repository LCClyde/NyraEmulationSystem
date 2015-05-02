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
#include <nes/Disassembly.h>
#include <core/StringConvert.h>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
Disassembly::Disassembly() :
    mOpCode(nullptr)
{
}

/*****************************************************************************/
Disassembly::Disassembly(const OpCode& opCode,
                         const CPUArgs& args,
                         const CPURegisters& registers,
                         const CPUInfo info) :
    mOpCode(&opCode),
    mArgs(args),
    mRegisters(registers),
    mInfo(info)
{
}

/*****************************************************************************/
Disassembly::Disassembly(const Disassembly& other) :
    mOpCode(other.mOpCode),
    mArgs(other.mArgs),
    mRegisters(other.mRegisters),
    mInfo(other.mInfo)
{
}

/*****************************************************************************/
Disassembly& Disassembly::operator=(Disassembly other)
{
    swap(*this, other);
    return *this;
}

/*****************************************************************************/
void Disassembly::swap(Disassembly& first, Disassembly& second)
{
    std::swap(first.mOpCode, second.mOpCode);
    std::swap(first.mArgs, second.mArgs);
    std::swap(first.mRegisters, second.mRegisters);
    std::swap(first.mInfo, second.mInfo);
}

/*****************************************************************************/
std::ostream& operator<<(std::ostream& os,
                         const Disassembly& data)
{
    os << core::toHexString<uint16_t>(data.mInfo.programCounter) << "  "
       << core::toHexString<uint8_t>(data.mArgs.opcode) << " ";

    std::string params;
    if (data.mOpCode->getMode().usesArg1())
    {
        params += core::toHexString<uint8_t>(data.mArgs.arg1) + " ";
    }
    if (data.mOpCode->getMode().usesArg2())
    {
        params += core::toHexString<uint8_t>(data.mArgs.arg2);
    }
    params = core::pad(params, ' ', 7, true);
    os << params
       << core::pad(data.mOpCode->getName() + " " +
                    data.mOpCode->getMode().toString(),
                    ' ', 32, true)
       << "A:" << core::toHexString<uint8_t>(data.mRegisters.accumulator)
       << " X:" << core::toHexString<uint8_t>(data.mRegisters.xIndex)
       << " Y:" << core::toHexString<uint8_t>(data.mRegisters.yIndex)
       << " P:" << core::toHexString<uint8_t>(static_cast<uint8_t>(
                data.mRegisters.statusRegister.to_ulong()))
       << " SP:" << core::toHexString<uint8_t>(data.mRegisters.stackPointer)
       << " CYC:" << core::pad(core::toString(data.mInfo.cycles), ' ', 3)
       << " SL:" << core::toString(data.mInfo.scanLine);
    return os;
}
}
}
