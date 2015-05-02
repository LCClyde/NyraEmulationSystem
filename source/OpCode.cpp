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
static const uint16_t CYCLES_PER_SCANLINE = 341;
static const uint16_t MAX_SCANLINES = 260;

/*****************************************************************************/
uint8_t shiftRight(uint8_t value, bool rotate, std::bitset<8>& statusRegister)
{
    uint8_t ret = value >> 1;
    if (rotate)
    {
        ret |= (statusRegister[nyra::nes::CPURegisters::CARRY] << 7);
    }
    statusRegister[nyra::nes::CPURegisters::CARRY] = (value & 0x01) != 0;
    return ret;
}

/*****************************************************************************/
uint8_t shiftLeft(uint8_t value, bool rotate, std::bitset<8>& statusRegister)
{
    uint8_t ret = value << 1;
    if (rotate)
    {
        ret |= static_cast<uint8_t>(
                statusRegister[nyra::nes::CPURegisters::CARRY]);
    }
    statusRegister[nyra::nes::CPURegisters::CARRY] = (value & 0x80) != 0;
    return ret;
}

/*****************************************************************************/
void compare(uint8_t value, uint8_t reg, std::bitset<8>& statusRegister)
{
    statusRegister[nyra::nes::CPURegisters::CARRY] = reg >= value;
    statusRegister[nyra::nes::CPURegisters::ZERO] = reg == value;
    statusRegister[nyra::nes::CPURegisters::SIGN] =
            static_cast<uint8_t>(reg - value) >= 0x80;
}

/*****************************************************************************/
void setRegister(uint8_t value, uint8_t& reg, std::bitset<8>& statusRegister)
{
    statusRegister[nyra::nes::CPURegisters::SIGN] = (value >= 0x80);
    statusRegister[nyra::nes::CPURegisters::ZERO] = (value == 0);
    reg = value;
}

/*****************************************************************************/
void add(uint8_t value, nyra::nes::CPURegisters& registers)
{
    const size_t sum = registers.accumulator + value +
            registers.statusRegister[nyra::nes::CPURegisters::CARRY];
    registers.statusRegister[nyra::nes::CPURegisters::OFLOW] =
            ((registers.accumulator ^ sum) & (value ^ sum) & 0x80) != 0;
    registers.statusRegister[nyra::nes::CPURegisters::CARRY] =
            (sum > 0xFF) != 0;
    setRegister(static_cast<uint8_t>(sum),
                registers.accumulator, registers.statusRegister);
}

/*****************************************************************************/
void pushStack(uint8_t value, nyra::nes::MemoryMap& ram, uint8_t& stackPointer)
{
    ram.writeByte(static_cast<size_t>(stackPointer | 0x100), value);
    --stackPointer;
}

/*****************************************************************************/
uint8_t popStack(nyra::nes::MemoryMap& ram, uint8_t& stackPointer)
{
    ++stackPointer;
    return ram.readByte(static_cast<size_t>(stackPointer | 0x100));
}

/*****************************************************************************/
// TODO: Most of the modes have extra unnecessary operations which are used to
//       match a known debug string output. There needs to be a way to remove
//       theses. Maybe with a #ifdef DEBUG or something similar.
/*****************************************************************************/

/*****************************************************************************/
class ModeAccumulator : public nyra::nes::OpCode::Mode
{
public:
    ModeAccumulator() :
        nyra::nes::OpCode::Mode(false, false)
    {
    }

    std::string toString() const
    {
        return "A";
    }

    void operator()(const nyra::nes::CPUArgs& ,
                    const nyra::nes::CPURegisters& registers,
                    const nyra::nes::MemoryMap& ,
                    nyra::nes::CPUInfo& )
    {
        mValue = registers.accumulator;
        mArg = mValue;
    }
};

/*****************************************************************************/
template <bool OutputT>
class ModeAbsolute : public nyra::nes::OpCode::Mode
{
public:
    ModeAbsolute() :
        nyra::nes::OpCode::Mode(true, true)
    {
    }

    std::string toString() const
    {
        const std::string outputString = OutputT ?
                " = " + nyra::core::toHexString<uint8_t>(mValue) : "";
        return "$" + nyra::core::toHexString<uint16_t>(mArg) + outputString;
    }

    void operator()(const nyra::nes::CPUArgs& args,
                    const nyra::nes::CPURegisters& ,
                    const nyra::nes::MemoryMap& memory,
                    nyra::nes::CPUInfo& )
    {
        mArg = args.darg;
        mValue = memory.readByte(mArg);
    }
};

/*****************************************************************************/
class ModeIndirect : public nyra::nes::OpCode::Mode
{
public:
    ModeIndirect() :
        nyra::nes::OpCode::Mode(true, true),
        mOrigArg(0),
        mCorrectArg(0)
    {
    }

    std::string toString() const
    {
        return "($" + nyra::core::toHexString<uint16_t>(mOrigArg) + ") = " +
                nyra::core::toHexString<uint16_t>(mCorrectArg);
    }

    void operator()(const nyra::nes::CPUArgs& args,
                    const nyra::nes::CPURegisters& ,
                    const nyra::nes::MemoryMap& memory,
                    nyra::nes::CPUInfo& )
    {
        mOrigArg = args.darg;
        mCorrectArg = memory.readShort(mOrigArg);

        // There is a bug in 6502. If we try to get the address at 0xXXFF,
        // it does not go to the next digit properly.
        if (args.arg1 == 0xFF)
        {
            const uint8_t high = memory.readByte(args.darg);
            const uint16_t low = memory.readByte(args.arg2 << 8);
            mArg = (low << 8) | high;
        }
        else
        {
            mArg = mCorrectArg;
        }
    }

private:
    uint16_t mOrigArg;
    uint16_t mCorrectArg;
};

/*****************************************************************************/
class ModeIndirectX : public nyra::nes::OpCode::Mode
{
public:
    ModeIndirectX() :
        nyra::nes::OpCode::Mode(true, false),
        mOrigArg(0),
        mModArg(0)
    {
    }

    std::string toString() const
    {
        return "($" + nyra::core::toHexString<uint8_t>(mOrigArg) + ",X) @ " +
                nyra::core::toHexString<uint8_t>(mModArg) + " = " +
                nyra::core::toHexString<uint16_t>(mArg) + " = " +
                nyra::core::toHexString<uint8_t>(mValue);
    }

    void operator()(const nyra::nes::CPUArgs& args,
                    const nyra::nes::CPURegisters& registers,
                    const nyra::nes::MemoryMap& memory,
                    nyra::nes::CPUInfo& )
    {
        mOrigArg = args.arg1;
        mModArg = (args.arg1 + registers.xIndex) & 0xFF;
        mArg = memory.readShort(mModArg);
        mValue = memory.readByte(mArg);
    }

private:
    uint8_t mOrigArg;
    uint8_t mModArg;
};

/*****************************************************************************/
class ModeZeroPageN : public nyra::nes::OpCode::Mode
{
public:
    ModeZeroPageN(char index) :
        nyra::nes::OpCode::Mode(true, false),
        mOrigArg(0),
        mIndex(index)
    {
    }

    std::string toString() const
    {
        return "$" + nyra::core::toHexString<uint8_t>(mOrigArg) +
                "," + mIndex + " @ " +
                nyra::core::toHexString<uint8_t>(
                        static_cast<uint8_t>(mArg)) + " = " +
                nyra::core::toHexString<uint8_t>(mValue);
    }

    void operator()(const nyra::nes::CPUArgs& args,
                    const nyra::nes::CPURegisters& registers,
                    const nyra::nes::MemoryMap& memory,
                    nyra::nes::CPUInfo& )
    {
        mOrigArg = args.arg1;
        mArg = (mOrigArg + getIndex(registers)) & 0xFF;
        mValue = memory.readByte(mArg);
    }

private:
    virtual uint8_t getIndex(
            const nyra::nes::CPURegisters& registers) const = 0;

    uint8_t mOrigArg;
    const char mIndex;
};

/*****************************************************************************/
class ModeZeroPageX : public ModeZeroPageN
{
public:
    ModeZeroPageX() :
        ModeZeroPageN('X')
    {
    }

private:
    virtual uint8_t getIndex(
            const nyra::nes::CPURegisters& registers) const
    {
        return registers.xIndex;
    }
};

/*****************************************************************************/
class ModeZeroPageY : public ModeZeroPageN
{
public:
    ModeZeroPageY() :
        ModeZeroPageN('Y')
    {
    }

private:
    virtual uint8_t getIndex(
            const nyra::nes::CPURegisters& registers) const
    {
        return registers.yIndex;
    }
};

/*****************************************************************************/
template <bool ExtraCycleT>
class ModeAbsoluteN : public nyra::nes::OpCode::Mode
{
public:
    ModeAbsoluteN(char index) :
        nyra::nes::OpCode::Mode(true, true),
        mOrigArg(0),
        mIndex(index)
    {
    }

    std::string toString() const
    {
        return "$" + nyra::core::toHexString<uint16_t>(mOrigArg) +
                "," + mIndex + " @ " +
                nyra::core::toHexString<uint16_t>(mArg) + " = " +
                nyra::core::toHexString<uint8_t>(mValue);
    }

    void operator()(const nyra::nes::CPUArgs& args,
                    const nyra::nes::CPURegisters& registers,
                    const nyra::nes::MemoryMap& memory,
                    nyra::nes::CPUInfo& info)
    {
        mOrigArg = args.darg;
        mArg = mOrigArg + getIndex(registers);
        mValue = memory.readByte(mArg);

        if (ExtraCycleT)
        {
            if ((mOrigArg & 0xFF00) != (mArg & 0xFF00))
            {
                info.cycles += 3;
            }
        }
    }

private:
    virtual uint8_t getIndex(
            const nyra::nes::CPURegisters& registers) const = 0;

    uint16_t mOrigArg;
    const char mIndex;
};

/*****************************************************************************/
template <bool ExtraCycleT>
class ModeAbsoluteY : public ModeAbsoluteN<ExtraCycleT>
{
public:
    ModeAbsoluteY() :
        ModeAbsoluteN('Y')
    {
    }

private:
    virtual uint8_t getIndex(
            const nyra::nes::CPURegisters& registers) const
    {
        return registers.yIndex;
    }
};

/*****************************************************************************/
template <bool ExtraCycleT>
class ModeAbsoluteX : public ModeAbsoluteN<ExtraCycleT>
{
public:
    ModeAbsoluteX() :
        ModeAbsoluteN('X')
    {
    }

private:
    virtual uint8_t getIndex(
            const nyra::nes::CPURegisters& registers) const
    {
        return registers.xIndex;
    }
};

/*****************************************************************************/
template <bool ExtraCycleT>
class ModeIndirectY : public nyra::nes::OpCode::Mode
{
public:
    ModeIndirectY() :
        nyra::nes::OpCode::Mode(true, false),
        mOrigArg(0),
        mModArg(0)
    {
    }

    std::string toString() const
    {
        return "($" + nyra::core::toHexString<uint8_t>(mOrigArg) + "),Y = " +
                nyra::core::toHexString<uint16_t>(mModArg) + " @ " +
                nyra::core::toHexString<uint16_t>(mArg) + " = " +
                nyra::core::toHexString<uint8_t>(mValue);
    }

    void operator()(const nyra::nes::CPUArgs& args,
                    const nyra::nes::CPURegisters& registers,
                    const nyra::nes::MemoryMap& memory,
                    nyra::nes::CPUInfo& info)
    {
        mOrigArg = args.arg1;
        mModArg = memory.readShort(mOrigArg);
        mArg = mModArg + registers.yIndex;
        mValue = memory.readByte(mArg);

        if (ExtraCycleT)
        {
            if ((mOrigArg == 0xFF) || ((mModArg & 0xFF00) != (mArg & 0xFF00)))
            {
                info.cycles += 3;
            }
        }
    }

private:
    uint8_t mOrigArg;
    uint16_t mModArg;
};

/*****************************************************************************/
class ModeRelative : public nyra::nes::OpCode::Mode
{
public:
    ModeRelative() :
        nyra::nes::OpCode::Mode(true, false)
    {
    }

    std::string toString() const
    {
        return "$" + nyra::core::toHexString<uint16_t>(mArg);
    }

    void operator()(const nyra::nes::CPUArgs& args,
                    const nyra::nes::CPURegisters& ,
                    const nyra::nes::MemoryMap& ,
                    nyra::nes::CPUInfo& info)
    {
        // Value is never used for relative mode
        mArg = info.programCounter + args.arg1 + 2;
    }
};

/*****************************************************************************/
class ModeZeroPage : public nyra::nes::OpCode::Mode
{
public:
    ModeZeroPage() :
        nyra::nes::OpCode::Mode(true, false)
    {
    }

    std::string toString() const
    {
        return "$" + nyra::core::toHexString<uint8_t>(
                static_cast<uint8_t>(mArg)) + " = " + 
                nyra::core::toHexString<uint8_t>(mValue);
    }

    void operator()(const nyra::nes::CPUArgs& args,
                    const nyra::nes::CPURegisters& ,
                    const nyra::nes::MemoryMap& memory,
                    nyra::nes::CPUInfo& )
    {
        mArg = args.arg1;
        mValue = memory.readByte(mArg);
    }
};

/*****************************************************************************/
class ModeImmediate : public nyra::nes::OpCode::Mode
{
public:
    ModeImmediate() :
        nyra::nes::OpCode::Mode(true, false)
    {
    }

    std::string toString() const
    {
        return "#$" + nyra::core::toHexString<uint8_t>(mValue);
    }

    void operator()(const nyra::nes::CPUArgs& args,
                    const nyra::nes::CPURegisters& ,
                    const nyra::nes::MemoryMap& ,
                    nyra::nes::CPUInfo& )
    {
        mValue = args.arg1;
        mArg = mValue;
    }
};

/*****************************************************************************/
class ModeImplied : public nyra::nes::OpCode::Mode
{
public:
    ModeImplied() :
        nyra::nes::OpCode::Mode(false, false)
    {
    }

    std::string toString() const
    {
        return "";
    }

    void operator()(const nyra::nes::CPUArgs& ,
                    const nyra::nes::CPURegisters& ,
                    const nyra::nes::MemoryMap& ,
                    nyra::nes::CPUInfo& )
    {
        // Nothing to do here
    }
};


/*****************************************************************************/
class OpNUL : public nyra::nes::OpCode
{
public:
    OpNUL(uint8_t opCode) :
        nyra::nes::OpCode("NUL", "Null Opcode", opCode,
                          0, 0, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& ,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        throw nyra::core::Exception("Attempting to run null op: " + 
                nyra::core::toHexString<uint8_t>(mOpCode));
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpJMP : public nyra::nes::OpCode
{
public:
    OpJMP(uint8_t opcode, uint8_t time) :
        nyra::nes::OpCode("JMP", "Jump", opcode, 0, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& ,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& )
    {
        info.programCounter = mMode->getArg();
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpLDX : public nyra::nes::OpCode
{
public:
    OpLDX(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("LDX", "Load X register",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(mMode->getValue(),
                    registers.xIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpLDY : public nyra::nes::OpCode
{
public:
    OpLDY(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("LDY", "Load Y register",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(mMode->getValue(),
                    registers.yIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpLDA : public nyra::nes::OpCode
{
public:
    OpLDA(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("LDA", "Load accumulator",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(mMode->getValue(),
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpLSR : public nyra::nes::OpCode
{
public:
    OpLSR(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("LSR", "Logical shift right",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t value = shiftRight(mMode->getValue(),
                                         false,
                                         registers.statusRegister);
        uint8_t garbage;
        setRegister(value, garbage, registers.statusRegister);
        memory.writeByte(mMode->getArg(), value);
    }
};

/*****************************************************************************/
template <>
class OpLSR <ModeAccumulator> : public nyra::nes::OpCode
{
public:
    OpLSR(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("LSR", "Logical shift right",
                          opcode, length, time, new ModeAccumulator())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(shiftRight(mMode->getValue(),
                               false,
                               registers.statusRegister),
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpASL : public nyra::nes::OpCode
{
public:
    OpASL(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("ASL", "Arithmetic shift left",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t value = shiftLeft(mMode->getValue(),
                                         false,
                                         registers.statusRegister);
        uint8_t garbage;
        setRegister(value, garbage, registers.statusRegister);
        memory.writeByte(mMode->getArg(), value);
    }
};

/*****************************************************************************/
template <>
class OpASL <ModeAccumulator> : public nyra::nes::OpCode
{
public:
    OpASL(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("ASL", "Arithmetic shift left",
                          opcode, length, time, new ModeAccumulator())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(shiftLeft(mMode->getValue(),
                              false,
                              registers.statusRegister),
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpROR : public nyra::nes::OpCode
{
public:
    OpROR(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("ROR", "Rotate right",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t value = shiftRight(mMode->getValue(),
                                         true,
                                         registers.statusRegister);
        uint8_t garbage;
        setRegister(value, garbage, registers.statusRegister);
        memory.writeByte(mMode->getArg(), value);
    }
};

/*****************************************************************************/
template <>
class OpROR <ModeAccumulator> : public nyra::nes::OpCode
{
public:
    OpROR(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("ROR", "Rotate right",
                          opcode, length, time, new ModeAccumulator())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(shiftRight(mMode->getValue(),
                               true,
                               registers.statusRegister),
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpROL : public nyra::nes::OpCode
{
public:
    OpROL(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("ROL", "Rotate left",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t value = shiftLeft(mMode->getValue(),
                                        true,
                                        registers.statusRegister);
        uint8_t garbage;
        setRegister(value, garbage, registers.statusRegister);
        memory.writeByte(mMode->getArg(), value);
    }
};

/*****************************************************************************/
template <>
class OpROL <ModeAccumulator> : public nyra::nes::OpCode
{
public:
    OpROL(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("ROL", "Rotate left",
                          opcode, length, time, new ModeAccumulator())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(shiftLeft(mMode->getValue(),
                              true,
                              registers.statusRegister),
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpSTA : public nyra::nes::OpCode
{
public:
    OpSTA(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("STA", "Store accumulator",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        memory.writeByte(mMode->getArg(), registers.accumulator);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpSTX : public nyra::nes::OpCode
{
public:
    OpSTX(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("STX", "Store X register",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        memory.writeByte(mMode->getArg(), registers.xIndex);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpSTY : public nyra::nes::OpCode
{
public:
    OpSTY(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("STY", "Store Y register",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        memory.writeByte(mMode->getArg(), registers.yIndex);
    }
};

/*****************************************************************************/
class OpJSR : public nyra::nes::OpCode
{
public:
    OpJSR() :
        nyra::nes::OpCode("JSR", "Jump to subroutine",
                          0x20, 0, 6, new ModeAbsolute<false>())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        pushStack(((info.programCounter + 2) >> 8) & 0xFF,
                  memory, registers.stackPointer);
        pushStack((info.programCounter + 2) & 0xFF,
                  memory, registers.stackPointer);
        info.programCounter = mMode->getArg();
    }
};

/*****************************************************************************/
class OpNOP : public nyra::nes::OpCode
{
public:
    OpNOP() :
        nyra::nes::OpCode("NOP", "No operation",
                          0xEA, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& ,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        // NOP
    }
};

/*****************************************************************************/
class OpRTI : public nyra::nes::OpCode
{
public:
    OpRTI() :
        nyra::nes::OpCode("RTI", "Return from interrupt",
                          0x40, 0, 6, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        // TODO: Make sure this is correct. I don't think the
        //       stack pointer is manipulated correctly.
        registers.statusRegister = popStack(memory, registers.stackPointer) |
                (1 << nyra::nes::CPURegisters::IGNORE);
        info.programCounter = popStack(memory, registers.stackPointer) |
                (popStack(memory, registers.stackPointer) << 8);
    }
};

/*****************************************************************************/
class OpRTS : public nyra::nes::OpCode
{
public:
    OpRTS() :
        nyra::nes::OpCode("RTS", "Return from subroutine",
                          0x60, 1, 6, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        info.programCounter = popStack(memory, registers.stackPointer) |
                              (popStack(memory, registers.stackPointer) << 8);
    }
};

/*****************************************************************************/
class OpINY : public nyra::nes::OpCode
{
public:
    OpINY() :
        nyra::nes::OpCode("INY", "Increment Y",
                          0xC8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(registers.yIndex + 1,
                    registers.yIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpINX : public nyra::nes::OpCode
{
public:
    OpINX() :
        nyra::nes::OpCode("INX", "Increment X",
                          0xC8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(registers.xIndex + 1,
                    registers.xIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpINC : public nyra::nes::OpCode
{
public:
    OpINC(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("INC", "Increment memory",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        uint8_t garbage;
        setRegister(mMode->getValue() + 1,
                    garbage,
                    registers.statusRegister);
        memory.writeByte(mMode->getArg(), mMode->getValue() + 1);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpDEC : public nyra::nes::OpCode
{
public:
    OpDEC(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("DEC", "Decrement memory",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        uint8_t garbage;
        setRegister(mMode->getValue() - 1,
                    garbage,
                    registers.statusRegister);
        memory.writeByte(mMode->getArg(), mMode->getValue() - 1);
    }
};

/*****************************************************************************/
class OpDEY : public nyra::nes::OpCode
{
public:
    OpDEY() :
        nyra::nes::OpCode("DEY", "Decrement Y",
                          0x88, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(registers.yIndex - 1,
                    registers.yIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpDEX : public nyra::nes::OpCode
{
public:
    OpDEX() :
        nyra::nes::OpCode("DEX", "Decrement X",
                          0xCA, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(registers.xIndex - 1,
                    registers.xIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpTAX : public nyra::nes::OpCode
{
public:
    OpTAX() :
        nyra::nes::OpCode("TAX", "Transfer A to X",
                          0xAA, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(registers.accumulator,
                    registers.xIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpTXA : public nyra::nes::OpCode
{
public:
    OpTXA() :
        nyra::nes::OpCode("TXA", "Transfer X to A",
                          0x8A, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(registers.xIndex,
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpTAY : public nyra::nes::OpCode
{
public:
    OpTAY() :
        nyra::nes::OpCode("TAY", "Transfer A to Y",
                          0xA8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(registers.accumulator,
                    registers.yIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpTYA : public nyra::nes::OpCode
{
public:
    OpTYA() :
        nyra::nes::OpCode("TYA", "Transfer Y to A",
                          0x98, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(registers.yIndex,
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpSEC : public nyra::nes::OpCode
{
public:
    OpSEC() :
        nyra::nes::OpCode("SEC", "Set carry",
                          0x38, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        registers.statusRegister[nyra::nes::CPURegisters::CARRY] = 1;
    }
};

/*****************************************************************************/
class OpCLC : public nyra::nes::OpCode
{
public:
    OpCLC() :
        nyra::nes::OpCode("CLC", "Clear carry",
                          0x18, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        registers.statusRegister[nyra::nes::CPURegisters::CARRY] = 0;
    }
};

/*****************************************************************************/
class OpCLV : public nyra::nes::OpCode
{
public:
    OpCLV() :
        nyra::nes::OpCode("CLV", "Clear overflow",
                          0xB8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        registers.statusRegister[nyra::nes::CPURegisters::OFLOW] = 0;
    }
};

/*****************************************************************************/
class OpSEI : public nyra::nes::OpCode
{
public:
    OpSEI() :
        nyra::nes::OpCode("SEI", "Set interrupt",
                          0x78, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        registers.statusRegister[nyra::nes::CPURegisters::INTERRUPT] = 1;
    }
};

/*****************************************************************************/
class OpSED : public nyra::nes::OpCode
{
public:
    OpSED() :
        nyra::nes::OpCode("SED", "Set decimal",
                          0xF8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        registers.statusRegister[nyra::nes::CPURegisters::DECIMAL] = 1;
    }
};

/*****************************************************************************/
class OpCLD : public nyra::nes::OpCode
{
public:
    OpCLD() :
        nyra::nes::OpCode("CLD", "Clear decimal",
                          0xD8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        registers.statusRegister[nyra::nes::CPURegisters::DECIMAL] = 0;
    }
};

/*****************************************************************************/
class OpTSX : public nyra::nes::OpCode
{
public:
    OpTSX() :
        nyra::nes::OpCode("TSX", "Transfer stack ptr to X",
                          0xBA, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(registers.stackPointer,
                    registers.xIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpTXS : public nyra::nes::OpCode
{
public:
    OpTXS() :
        nyra::nes::OpCode("TXS", "Transfer X to stack ptr",
                          0x9A, 1, 2, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        registers.stackPointer = registers.xIndex;
    }
};

/*****************************************************************************/
class OpPLA : public nyra::nes::OpCode
{
public:
    OpPLA() :
        nyra::nes::OpCode("PLA", "Pull accumulator",
                          0x68, 1, 4, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        setRegister(popStack(memory, registers.stackPointer),
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpPHA : public nyra::nes::OpCode
{
public:
    OpPHA() :
        nyra::nes::OpCode("PHA", "Push accumulator",
                          0x48, 1, 3, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        pushStack(static_cast<uint8_t>(registers.accumulator),
                  memory, registers.stackPointer);
    }
};

/*****************************************************************************/
class OpPLP : public nyra::nes::OpCode
{
public:
    OpPLP() :
        nyra::nes::OpCode("PLP", "Pull processor status",
                          0x28, 1, 4, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        registers.statusRegister =
            ((popStack(memory, registers.stackPointer) |
            (1 << nyra::nes::CPURegisters::IGNORE)) &
            ~(1 << nyra::nes::CPURegisters::STACK));
    }
};
/*****************************************************************************/
class OpPHP : public nyra::nes::OpCode
{
public:
    OpPHP() :
        nyra::nes::OpCode("PHP", "Push processor status",
                          0x08, 1, 3, new ModeImplied())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        pushStack(static_cast<uint8_t>(registers.statusRegister.to_ulong()) |
                          (1 << nyra::nes::CPURegisters::STACK),
                  memory, registers.stackPointer);
    }
};

/*****************************************************************************/
class OpBranch : public nyra::nes::OpCode
{
public:
    OpBranch(const std::string& name,
             const std::string& extendedName,
             uint8_t opcode) :
        nyra::nes::OpCode(name, extendedName, opcode,
                          0, 2, new ModeRelative())
    {
    }

private:
    virtual bool branchArg(
            const std::bitset<8>& statusRegister) const = 0;

    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& )
    {
        if (branchArg(registers.statusRegister))
        {
            info.programCounter = mMode->getArg();
            info.cycles += 3;
        }
        else
        {
            info.programCounter += 2;
        }
    }

};

/*****************************************************************************/
class OpBCS : public OpBranch
{
public:
    OpBCS() : OpBranch("BCS", "Branch on carry set", 0xB0)
    {
    }

private:
    bool branchArg(const std::bitset<8>& statusRegister) const
    {
        return statusRegister[nyra::nes::CPURegisters::CARRY];
    }
};

/*****************************************************************************/
class OpBEQ : public OpBranch
{
public:
    OpBEQ() : OpBranch("BEQ", "Branch on equal", 0xF0)
    {
    }

private:
    bool branchArg(const std::bitset<8>& statusRegister) const
    {
        return statusRegister[nyra::nes::CPURegisters::ZERO];
    }
};

/*****************************************************************************/
class OpBNE : public OpBranch
{
public:
    OpBNE() : OpBranch("BNE", "Branch on not equal", 0xD0)
    {
    }

private:
    bool branchArg(const std::bitset<8>& statusRegister) const
    {
        return !statusRegister[nyra::nes::CPURegisters::ZERO];
    }
};

/*****************************************************************************/
class OpBCC : public OpBranch
{
public:
    OpBCC() : OpBranch("BCC", "Branch on carry clear", 0x90)
    {
    }

private:
    bool branchArg(const std::bitset<8>& statusRegister) const
    {
        return !statusRegister[nyra::nes::CPURegisters::CARRY];
    }
};

/*****************************************************************************/
class OpBVS : public OpBranch
{
public:
    OpBVS() : OpBranch("BVS", "Branch on overflow set", 0x70)
    {
    }

private:
    bool branchArg(const std::bitset<8>& statusRegister) const
    {
        return statusRegister[nyra::nes::CPURegisters::OFLOW];
    }
};

/*****************************************************************************/
class OpBVC : public OpBranch
{
public:
    OpBVC() : OpBranch("BVC", "Branch on overflow clear",0x50)
    {
    }

private:
    bool branchArg(const std::bitset<8>& statusRegister) const
    {
        return !statusRegister[nyra::nes::CPURegisters::OFLOW];
    }
};

/*****************************************************************************/
class OpBPL : public OpBranch
{
public:
    OpBPL() : OpBranch("BPL", "Branch on plus", 0x10)
    {
    }

private:
    bool branchArg(const std::bitset<8>& statusRegister) const
    {
        return !statusRegister[nyra::nes::CPURegisters::SIGN];
    }
};

/*****************************************************************************/
class OpBMI : public OpBranch
{
public:
    OpBMI() : OpBranch("BMI", "Branch on minus", 0x30)
    {
    }

private:
    bool branchArg(const std::bitset<8>& statusRegister) const
    {
        return statusRegister[nyra::nes::CPURegisters::SIGN];
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpBIT : public nyra::nes::OpCode
{
public:
    OpBIT(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("BIT", "Test bits",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        const size_t param = mMode->getValue();
        registers.statusRegister[nyra::nes::CPURegisters::ZERO] =
                (param & registers.accumulator) == 0;
        registers.statusRegister[nyra::nes::CPURegisters::OFLOW] =
                (param & (1 << nyra::nes::CPURegisters::OFLOW)) != 0;
        registers.statusRegister[nyra::nes::CPURegisters::SIGN] =
                (param & (1 << nyra::nes::CPURegisters::SIGN)) != 0;
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpCMP : public nyra::nes::OpCode
{
public:
    OpCMP(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("CMP", "Compare accumulator",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        compare(mMode->getValue(),
                registers.accumulator,
                registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpCPY : public nyra::nes::OpCode
{
public:
    OpCPY(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("CPY", "Compare Y register",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        compare(mMode->getValue(),
                registers.yIndex,
                registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpCPX : public nyra::nes::OpCode
{
public:
    OpCPX(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("CPX", "Compare X register",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        compare(mMode->getValue(),
                registers.xIndex,
                registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpAND : public nyra::nes::OpCode
{
public:
    OpAND(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("AND", "Bitwise AND with accumulator",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(mMode->getValue() & registers.accumulator,
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpORA : public nyra::nes::OpCode
{
public:
    OpORA(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("ORA", "Bitwise OR with accumulator",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(mMode->getValue() | registers.accumulator,
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpEOR : public nyra::nes::OpCode
{
public:
    OpEOR(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("EOR", "Bitwise exclusive OR",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(mMode->getValue() ^ registers.accumulator,
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpADC : public nyra::nes::OpCode
{
public:
    OpADC(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("ADC", "Add with carry",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        add(mMode->getValue(), registers);
    }
};
/*****************************************************************************/
template <typename ModeT>
class OpSBC : public nyra::nes::OpCode
{
public:
    OpSBC(uint8_t opcode, uint8_t length, uint8_t time) :
        nyra::nes::OpCode("SBC", "Subtract with carry",
                          opcode, length, time, new ModeT())
    {
    }

private:
    void op(nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        add(~mMode->getValue(), registers);
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
    opCodes[0x01].reset(new OpORA<ModeIndirectX>(0x01, 2, 6));
    opCodes[0x05].reset(new OpORA<ModeZeroPage>(0x05, 2, 3));
    opCodes[0x06].reset(new OpASL<ModeZeroPage>(0x06, 2, 5));
    opCodes[0x08].reset(new OpPHP());
    opCodes[0x09].reset(new OpORA<ModeImmediate>(0x09, 2, 2));
    opCodes[0x0A].reset(new OpASL<ModeAccumulator>(0x0A, 1, 2));
    opCodes[0x0D].reset(new OpORA<ModeAbsolute<true> >(0x0D, 3, 4));
    opCodes[0x0E].reset(new OpASL<ModeAbsolute<true> >(0x0E, 3, 6));
    opCodes[0x10].reset(new OpBPL());
    opCodes[0x11].reset(new OpORA<ModeIndirectY<true> >(0x11, 2, 5));
    opCodes[0x15].reset(new OpORA<ModeZeroPageX>(0x15, 2, 4));
    opCodes[0x16].reset(new OpASL<ModeZeroPageX>(0x16, 2, 6));
    opCodes[0x18].reset(new OpCLC());
    opCodes[0x19].reset(new OpORA<ModeAbsoluteY<true> >(0x19, 3, 4));
    opCodes[0x1D].reset(new OpORA<ModeAbsoluteX<true> >(0x1D, 3, 4));
    opCodes[0x1E].reset(new OpASL<ModeAbsoluteX<false> >(0x1E, 3, 7));
    opCodes[0x20].reset(new OpJSR());
    opCodes[0x21].reset(new OpAND<ModeIndirectX>(0x21, 2, 6));
    opCodes[0x24].reset(new OpBIT<ModeZeroPage>(0x24, 2, 3));
    opCodes[0x25].reset(new OpAND<ModeZeroPage>(0x25, 2, 3));
    opCodes[0x26].reset(new OpROL<ModeZeroPage>(0x26, 2, 5));
    opCodes[0x28].reset(new OpPLP());
    opCodes[0x29].reset(new OpAND<ModeImmediate>(0x29, 2, 2));
    opCodes[0x2A].reset(new OpROL<ModeAccumulator>(0x2A, 1, 2));
    opCodes[0x2C].reset(new OpBIT<ModeAbsolute<true> >(0x2C, 3, 4));
    opCodes[0x2D].reset(new OpAND<ModeAbsolute<true> >(0x2D, 3, 4));
    opCodes[0x2E].reset(new OpROL<ModeAbsolute<true> >(0x2E, 3, 6));
    opCodes[0x30].reset(new OpBMI());
    opCodes[0x31].reset(new OpAND<ModeIndirectY<true> >(0x31, 2, 5));
    opCodes[0x35].reset(new OpAND<ModeZeroPageX>(0x35, 2, 4));
    opCodes[0x36].reset(new OpROL<ModeZeroPageX>(0x36, 2, 6));
    opCodes[0x38].reset(new OpSEC());
    opCodes[0x39].reset(new OpAND<ModeAbsoluteY<true> >(0x39, 3, 4));
    opCodes[0x3D].reset(new OpAND<ModeAbsoluteX<true> >(0x3D, 3, 4));
    opCodes[0x3E].reset(new OpROL<ModeAbsoluteX<false> >(0x3E, 3, 7));
    opCodes[0x40].reset(new OpRTI());
    opCodes[0x41].reset(new OpEOR<ModeIndirectX>(0x41, 2, 6));
    opCodes[0x45].reset(new OpEOR<ModeZeroPage>(0x45, 2, 3));
    opCodes[0x46].reset(new OpLSR<ModeZeroPage>(0x46, 2, 5));
    opCodes[0x4C].reset(new OpJMP<ModeAbsolute<false> >(0x4C, 3));
    opCodes[0x4D].reset(new OpEOR<ModeAbsolute<true> >(0x4D, 3, 4));
    opCodes[0x48].reset(new OpPHA());
    opCodes[0x49].reset(new OpEOR<ModeImmediate>(0x49, 2, 2));
    opCodes[0x4A].reset(new OpLSR<ModeAccumulator>(0x4A, 1, 2));
    opCodes[0x4E].reset(new OpLSR<ModeAbsolute<true> >(0x4E, 3, 6));
    opCodes[0x50].reset(new OpBVC());
    opCodes[0x51].reset(new OpEOR<ModeIndirectY<true> >(0x51, 2, 5));
    opCodes[0x55].reset(new OpEOR<ModeZeroPageX>(0x55, 2, 4));
    opCodes[0x56].reset(new OpLSR<ModeZeroPageX>(0x56, 2, 6));
    opCodes[0x59].reset(new OpEOR<ModeAbsoluteY<true> >(0x59, 3, 4));
    opCodes[0x5D].reset(new OpEOR<ModeAbsoluteX<true> >(0x5D, 3, 4));
    opCodes[0x5E].reset(new OpLSR<ModeAbsoluteX<false> >(0x5E, 3, 7));
    opCodes[0x60].reset(new OpRTS());
    opCodes[0x61].reset(new OpADC<ModeIndirectX>(0x61, 2, 6));
    opCodes[0x65].reset(new OpADC<ModeZeroPage>(0x65, 2, 3));
    opCodes[0x66].reset(new OpROR<ModeZeroPage>(0x66, 2, 5));
    opCodes[0x68].reset(new OpPLA());
    opCodes[0x69].reset(new OpADC<ModeImmediate>(0x69, 2, 2));
    opCodes[0x6A].reset(new OpROR<ModeAccumulator>(0x6A, 1, 2));
    opCodes[0x6C].reset(new OpJMP<ModeIndirect>(0x6C, 5));
    opCodes[0x6D].reset(new OpADC<ModeAbsolute<true> >(0x6D, 3, 4));
    opCodes[0x6E].reset(new OpROR<ModeAbsolute<true> >(0x6E, 3, 6));
    opCodes[0x70].reset(new OpBVS());
    opCodes[0x71].reset(new OpADC<ModeIndirectY<true> >(0x71, 2, 5));
    opCodes[0x75].reset(new OpADC<ModeZeroPageX>(0x75, 2, 4));
    opCodes[0x76].reset(new OpROR<ModeZeroPageX>(0x76, 2, 6));
    opCodes[0x78].reset(new OpSEI());
    opCodes[0x79].reset(new OpADC<ModeAbsoluteY<true> >(0x79, 3, 4));
    opCodes[0x7D].reset(new OpADC<ModeAbsoluteX<true> >(0x7D, 3, 4));
    opCodes[0x7E].reset(new OpROR<ModeAbsoluteX<false> >(0x7E, 3, 7));
    opCodes[0x81].reset(new OpSTA<ModeIndirectX>(0x81, 2, 6));
    opCodes[0x84].reset(new OpSTY<ModeZeroPage>(0x84, 2, 3));
    opCodes[0x85].reset(new OpSTA<ModeZeroPage>(0x85, 2, 3));
    opCodes[0x86].reset(new OpSTX<ModeZeroPage>(0x86, 2, 3));
    opCodes[0x88].reset(new OpDEY());
    opCodes[0x8A].reset(new OpTXA());
    opCodes[0x8C].reset(new OpSTY<ModeAbsolute<true> >(0x8C, 3, 4));
    opCodes[0x8D].reset(new OpSTA<ModeAbsolute<true> >(0x8D, 3, 4));
    opCodes[0x8E].reset(new OpSTX<ModeAbsolute<true> >(0x8E, 3, 4));
    opCodes[0x90].reset(new OpBCC());
    opCodes[0x91].reset(new OpSTA<ModeIndirectY<false> >(0x91, 2, 6));
    opCodes[0x94].reset(new OpSTY<ModeZeroPageX>(0x94, 2, 4));
    opCodes[0x95].reset(new OpSTA<ModeZeroPageX>(0x95, 2, 4));
    opCodes[0x96].reset(new OpSTX<ModeZeroPageY>(0x96, 2, 4));
    opCodes[0x98].reset(new OpTYA());
    opCodes[0x99].reset(new OpSTA<ModeAbsoluteY<false> >(0x99, 3, 5));
    opCodes[0x9A].reset(new OpTXS());
    opCodes[0x9D].reset(new OpSTA<ModeAbsoluteX<false> >(0x9D, 3, 5));
    opCodes[0xA0].reset(new OpLDY<ModeImmediate>(0xA0, 2, 2));
    opCodes[0xA1].reset(new OpLDA<ModeIndirectX>(0xA1, 2, 6));
    opCodes[0xA2].reset(new OpLDX<ModeImmediate>(0xA2, 2, 2));
    opCodes[0xA4].reset(new OpLDY<ModeZeroPage>(0xA4, 2, 3));
    opCodes[0xA5].reset(new OpLDA<ModeZeroPage>(0xA5, 2, 3));
    opCodes[0xA6].reset(new OpLDX<ModeZeroPage>(0xA6, 2, 3));
    opCodes[0xA8].reset(new OpTAY());
    opCodes[0xA9].reset(new OpLDA<ModeImmediate>(0xA9, 2, 2));
    opCodes[0xAA].reset(new OpTAX());
    opCodes[0xAC].reset(new OpLDY<ModeAbsolute<true> >(0xAC, 3, 4));
    opCodes[0xAD].reset(new OpLDA<ModeAbsolute<true> >(0xAD, 3, 4));
    opCodes[0xAE].reset(new OpLDX<ModeAbsolute<true> >(0xAE, 3, 4));
    opCodes[0xB0].reset(new OpBCS());
    opCodes[0xB1].reset(new OpLDA<ModeIndirectY<true> >(0xB1, 2, 5));
    opCodes[0xB4].reset(new OpLDY<ModeZeroPageX>(0xB4, 2, 4));
    opCodes[0xB5].reset(new OpLDA<ModeZeroPageX>(0xB5, 2, 4));
    opCodes[0xB6].reset(new OpLDX<ModeZeroPageY>(0xB6, 2, 4));
    opCodes[0xB8].reset(new OpCLV());
    opCodes[0xB9].reset(new OpLDA<ModeAbsoluteY<true> >(0xB9, 3, 4));
    opCodes[0xBA].reset(new OpTSX());
    opCodes[0xBC].reset(new OpLDY<ModeAbsoluteX<true> >(0xBC, 3, 4));
    opCodes[0xBD].reset(new OpLDA<ModeAbsoluteX<true> >(0xBD, 3, 4));
    opCodes[0xBE].reset(new OpLDX<ModeAbsoluteY<true> >(0xBE, 3, 4));
    opCodes[0xC0].reset(new OpCPY<ModeImmediate>(0xC0, 2, 2));
    opCodes[0xC1].reset(new OpCMP<ModeIndirectX>(0xC1, 2, 6));
    opCodes[0xC4].reset(new OpCPY<ModeZeroPage>(0xC4, 2, 3));
    opCodes[0xC5].reset(new OpCMP<ModeZeroPage>(0xC5, 2, 3));
    opCodes[0xC6].reset(new OpDEC<ModeZeroPage>(0xC6, 2, 5));
    opCodes[0xC8].reset(new OpINY());
    opCodes[0xC9].reset(new OpCMP<ModeImmediate>(0xC9, 2, 2));
    opCodes[0xCA].reset(new OpDEX());
    opCodes[0xCC].reset(new OpCPY<ModeAbsolute<true> >(0xCC, 3, 4));
    opCodes[0xCD].reset(new OpCMP<ModeAbsolute<true> >(0xCD, 3, 4));
    opCodes[0xCE].reset(new OpDEC<ModeAbsolute<true> >(0xCE, 3, 6));
    opCodes[0xD0].reset(new OpBNE());
    opCodes[0xD1].reset(new OpCMP<ModeIndirectY<true> >(0xD1, 2, 5));
    opCodes[0xD5].reset(new OpCMP<ModeZeroPageX>(0xD5, 2, 4));
    opCodes[0xD6].reset(new OpDEC<ModeZeroPageX>(0xD6, 2, 6));
    opCodes[0xD8].reset(new OpCLD());
    opCodes[0xD9].reset(new OpCMP<ModeAbsoluteY<true> >(0xD9, 3, 4));
    opCodes[0xDD].reset(new OpCMP<ModeAbsoluteX<true> >(0xDD, 3, 4));
    opCodes[0xDE].reset(new OpDEC<ModeAbsoluteX<false> >(0xDE, 3, 7));
    opCodes[0xE0].reset(new OpCPX<ModeImmediate>(0xE0, 2, 2));
    opCodes[0xE1].reset(new OpSBC<ModeIndirectX>(0xE1, 2, 6));
    opCodes[0xE4].reset(new OpCPX<ModeZeroPage>(0xE4, 2, 3));
    opCodes[0xE5].reset(new OpSBC<ModeZeroPage>(0xE5, 2, 3));
    opCodes[0xE6].reset(new OpINC<ModeZeroPage>(0xE6, 2, 5));
    opCodes[0xE8].reset(new OpINX());
    opCodes[0xE9].reset(new OpSBC<ModeImmediate>(0xE9, 2, 2));
    opCodes[0xEA].reset(new OpNOP());
    opCodes[0xEC].reset(new OpCPX<ModeAbsolute<true> >(0xEC, 3, 4));
    opCodes[0xED].reset(new OpSBC<ModeAbsolute<true> >(0xED, 3, 4));
    opCodes[0xEE].reset(new OpINC<ModeAbsolute<true> >(0xEE, 3, 6));
    opCodes[0xF0].reset(new OpBEQ());
    opCodes[0xF1].reset(new OpSBC<ModeIndirectY<true> >(0xF1, 2, 5));
    opCodes[0xF5].reset(new OpSBC<ModeZeroPageX>(0xF5, 2, 4));
    opCodes[0xF6].reset(new OpINC<ModeZeroPageX>(0xF6, 2, 6));
    opCodes[0xF8].reset(new OpSED());
    opCodes[0xF9].reset(new OpSBC<ModeAbsoluteY<true> >(0xF9, 3, 4));
    opCodes[0xFD].reset(new OpSBC<ModeAbsoluteX<true> >(0xFD, 3, 4));
    opCodes[0xFE].reset(new OpINC<ModeAbsoluteX<false> >(0xFE, 3, 7));

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
               Mode* mode) :
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

/*****************************************************************************/
void OpCode::operator()(const CPUArgs& args,
                        CPURegisters& registers,
                        CPUInfo& info,
                        MemoryMap& memory)
{
    // Setup the mode values
    (*mMode)(args, registers, memory, info);

    op(registers, info, memory);
    info.programCounter += mLength;
    info.cycles += mTime * 3;
    if (info.cycles >= CYCLES_PER_SCANLINE)
    {
        info.cycles -= CYCLES_PER_SCANLINE;
        ++info.scanLine;
        if (info.scanLine > MAX_SCANLINES)
        {
            info.scanLine = -1;
        }
    }
}
}
}
