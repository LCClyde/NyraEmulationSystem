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
void compare(uint8_t value, uint8_t reg, std::bitset<8>& statusRegister)
{
    statusRegister[nyra::nes::CPURegisters::CARRY] = reg >= value;
    statusRegister[nyra::nes::CPURegisters::ZERO] = reg == value;
    statusRegister[nyra::nes::CPURegisters::SIGN] =
            static_cast<uint8_t>(reg - value) >= 0x80;
}

/*****************************************************************************/
void branch(bool branchOn,
            uint16_t offset,
            uint16_t& programCounter,
            uint16_t& cycles)
{
    if (branchOn)
    {
        programCounter = offset;
        cycles += 3;
    }
    else
    {
        programCounter += 2;
    }
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

    uint16_t operator()(
            uint16_t value,
            nyra::nes::MemoryMap& ,
            uint16_t )
    {
        return value;
    }
};

/*****************************************************************************/
class ModeAbsoluteOutput : public nyra::nes::OpCode::Mode
{
public:
    ModeAbsoluteOutput() :
        nyra::nes::OpCode::Mode(true, true),
        mParam(0)
    {
    }

    std::string toString(const nyra::nes::CPUArgs& args) const
    {
        return "$" + nyra::core::toHexString<uint16_t>(args.darg) +
                " = " + nyra::core::toHexString<uint8_t>(mParam);
    }

    uint16_t operator()(
            uint16_t value,
            nyra::nes::MemoryMap& memory,
            uint16_t )
    {
        mParam = memory.readByte(value);
        return mParam;
    }

    uint8_t mParam;
};

/*****************************************************************************/
class ModeRelative : public nyra::nes::OpCode::Mode
{
public:
    ModeRelative() :
        nyra::nes::OpCode::Mode(true, false),
        mParam(0)
    {
    }

    std::string toString(const nyra::nes::CPUArgs& ) const
    {
        return "$" + nyra::core::toHexString<uint16_t>(mParam);
    }
    
    uint16_t operator()(
            uint16_t value,
            nyra::nes::MemoryMap& ,
            uint16_t programCounter)
    {
        mParam = programCounter + static_cast<int8_t>(value) + 2;
        return mParam;
    }

private:
    uint16_t mParam;
};

/*****************************************************************************/
class ModeZeroPage : public nyra::nes::OpCode::Mode
{
public:
    ModeZeroPage() :
        nyra::nes::OpCode::Mode(true, false),
        mParam(0)
    {
    }

    std::string toString(const nyra::nes::CPUArgs& args) const
    {
        return "$" + nyra::core::toHexString<uint8_t>(args.arg1) + " = " + 
                nyra::core::toHexString<uint8_t>(mParam);
    }

    uint16_t operator()(
            uint16_t value,
            nyra::nes::MemoryMap& memory,
            uint16_t )
    {
        mParam = memory.readByte(value);
        return mParam;

    }

private:
    uint8_t mParam;
};

/*****************************************************************************/
class ModeImmediate : public nyra::nes::OpCode::Mode
{
public:
    ModeImmediate() :
        nyra::nes::OpCode::Mode(true, false)
    {
    }

    std::string toString(const nyra::nes::CPUArgs& args) const
    {
        return "#$" + nyra::core::toHexString<uint8_t>(args.arg1);
    }

    uint16_t operator()(
            uint16_t value,
            nyra::nes::MemoryMap& ,
            uint16_t )
    {
        return value;
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

    std::string toString(const nyra::nes::CPUArgs& ) const
    {
        return "";
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& ,
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
    OpJMP(uint8_t opcode) :
        nyra::nes::OpCode("JMP", "Jump", opcode, 0, 3, new ModeT())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& ,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        info.programCounter = (*mMode)(args.darg, memory, 0);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        setRegister(static_cast<uint8_t>((*mMode)(args.arg1, memory, 0)),
                    registers.xIndex, registers.statusRegister);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        setRegister(static_cast<uint8_t>((*mMode)(args.arg1, memory, 0)),
                    registers.yIndex, registers.statusRegister);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        setRegister(static_cast<uint8_t>((*mMode)(args.darg, memory, 0)),
                    registers.accumulator, registers.statusRegister);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        // Save off the param value for proper printing
        (*mMode)(args.arg1, memory, 0);
        memory.writeByte(args.arg1, registers.accumulator);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        // Save off the param value for proper printing
        (*mMode)(args.arg1, memory, 0);
        memory.writeByte(args.arg1, registers.xIndex);
    }
};

/*****************************************************************************/
class OpJSR : public nyra::nes::OpCode
{
public:
    OpJSR() :
        nyra::nes::OpCode("JSR", "Jump to subroutine",
                          0x20, 0, 6, new ModeAbsolute())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        pushStack(((info.programCounter + 2) >> 8) & 0xFF,
                  memory, registers.stackPointer);
        pushStack((info.programCounter + 2) & 0xFF,
                  memory, registers.stackPointer);
        info.programCounter = (*mMode)(args.darg, memory, 0);
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& ,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& )
    {
        setRegister(registers.xIndex + 1,
                    registers.xIndex,
                    registers.statusRegister);
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
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
    void op(const nyra::nes::CPUArgs& ,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        pushStack(static_cast<uint8_t>(registers.statusRegister.to_ulong()) |
                          (1 << nyra::nes::CPURegisters::STACK),
                  memory, registers.stackPointer);
    }
};

/*****************************************************************************/
class OpBCS : public nyra::nes::OpCode
{
public:
    OpBCS() :
        nyra::nes::OpCode("BCS", "Branch on carry set",
                          0xB0, 0, 2, new ModeRelative())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        branch(registers.statusRegister[nyra::nes::CPURegisters::CARRY],
               (*mMode)(args.arg1, memory, info.programCounter),
               info.programCounter, info.cycles);
    }
};

/*****************************************************************************/
class OpBEQ : public nyra::nes::OpCode
{
public:
    OpBEQ() :
        nyra::nes::OpCode("BEQ", "Branch on equal",
                          0xF0, 0, 2, new ModeRelative())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        branch(registers.statusRegister[nyra::nes::CPURegisters::ZERO],
               (*mMode)(args.arg1, memory, info.programCounter),
               info.programCounter, info.cycles);
    }
};

/*****************************************************************************/
class OpBNE : public nyra::nes::OpCode
{
public:
    OpBNE() :
        nyra::nes::OpCode("BNE", "Branch on not equal",
                          0xD0, 0, 2, new ModeRelative())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        branch(!registers.statusRegister[nyra::nes::CPURegisters::ZERO],
               (*mMode)(args.arg1, memory, info.programCounter),
               info.programCounter, info.cycles);
    }
};

/*****************************************************************************/
class OpBCC : public nyra::nes::OpCode
{
public:
    OpBCC() :
        nyra::nes::OpCode("BCC", "Branch on carry clear",
                          0x90, 0, 2, new ModeRelative())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        branch(!registers.statusRegister[nyra::nes::CPURegisters::CARRY],
               (*mMode)(args.arg1, memory, info.programCounter),
               info.programCounter, info.cycles);
    }
};

/*****************************************************************************/
class OpBVS : public nyra::nes::OpCode
{
public:
    OpBVS() :
        nyra::nes::OpCode("BVS", "Branch on overflow set",
                          0x70, 0, 2, new ModeRelative())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        branch(registers.statusRegister[nyra::nes::CPURegisters::OFLOW],
               (*mMode)(args.arg1, memory, info.programCounter),
               info.programCounter, info.cycles);
    }
};

/*****************************************************************************/
class OpBVC : public nyra::nes::OpCode
{
public:
    OpBVC() :
        nyra::nes::OpCode("BVC", "Branch on overflow clear",
                          0x50, 0, 2, new ModeRelative())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        branch(!registers.statusRegister[nyra::nes::CPURegisters::OFLOW],
               (*mMode)(args.arg1, memory, info.programCounter),
               info.programCounter, info.cycles);
    }
};

/*****************************************************************************/
class OpBPL : public nyra::nes::OpCode
{
public:
    OpBPL() :
        nyra::nes::OpCode("BPL", "Branch on plus",
                          0x10, 0, 2, new ModeRelative())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        branch(!registers.statusRegister[nyra::nes::CPURegisters::SIGN],
               (*mMode)(args.arg1, memory, info.programCounter),
               info.programCounter, info.cycles);
    }
};

/*****************************************************************************/
class OpBMI : public nyra::nes::OpCode
{
public:
    OpBMI() :
        nyra::nes::OpCode("BMI", "Branch on minus",
                          0x30, 0, 2, new ModeRelative())
    {
    }

private:
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& info,
            nyra::nes::MemoryMap& memory)
    {
        branch(registers.statusRegister[nyra::nes::CPURegisters::SIGN],
               (*mMode)(args.arg1, memory, info.programCounter),
               info.programCounter, info.cycles);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const size_t param = (*mMode)(args.arg1, memory, 0);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t param = static_cast<uint8_t>(
                (*mMode)(args.arg1, memory, 0));
        compare(param, registers.accumulator, registers.statusRegister);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t param = static_cast<uint8_t>(
                (*mMode)(args.arg1, memory, 0));
        compare(param, registers.yIndex, registers.statusRegister);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t param = static_cast<uint8_t>(
                (*mMode)(args.arg1, memory, 0));
        compare(param, registers.xIndex, registers.statusRegister);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const size_t param = (*mMode)(args.arg1, memory, 0);
        setRegister(param & registers.accumulator,
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t param = static_cast<uint8_t>(
                (*mMode)(args.arg1, memory, 0));
        setRegister(param | registers.accumulator,
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t param = static_cast<uint8_t>(
                (*mMode)(args.arg1, memory, 0));
        setRegister(param ^ registers.accumulator,
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t param = static_cast<uint8_t>(
                (*mMode)(args.arg1, memory, 0));
        add(param, registers);
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
    void op(const nyra::nes::CPUArgs& args,
            nyra::nes::CPURegisters& registers,
            nyra::nes::CPUInfo& ,
            nyra::nes::MemoryMap& memory)
    {
        const uint8_t param = static_cast<uint8_t>(
                (*mMode)(args.arg1, memory, 0));
        add(~param, registers);
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
    opCodes[0x08].reset(new OpPHP());
    opCodes[0x09].reset(new OpORA<ModeImmediate>(0x09, 2, 2));
    opCodes[0x10].reset(new OpBPL());
    opCodes[0x18].reset(new OpCLC());
    opCodes[0x20].reset(new OpJSR());
    opCodes[0x24].reset(new OpBIT<ModeZeroPage>(0x24, 2, 3));
    opCodes[0x25].reset(new OpAND<ModeZeroPage>(0x25, 2, 3));
    opCodes[0x28].reset(new OpPLP());
    opCodes[0x29].reset(new OpAND<ModeImmediate>(0x29, 2, 2));
    opCodes[0x30].reset(new OpBMI());
    opCodes[0x38].reset(new OpSEC());
    opCodes[0x40].reset(new OpRTI());
    opCodes[0x4C].reset(new OpJMP<ModeAbsolute>(0x4C));
    opCodes[0x48].reset(new OpPHA());
    opCodes[0x49].reset(new OpEOR<ModeImmediate>(0x49, 2, 2));
    opCodes[0x50].reset(new OpBVC());
    opCodes[0x60].reset(new OpRTS());
    opCodes[0x68].reset(new OpPLA());
    opCodes[0x69].reset(new OpADC<ModeImmediate>(0x69, 2, 2));
    opCodes[0x70].reset(new OpBVS());
    opCodes[0x78].reset(new OpSEI());
    opCodes[0x85].reset(new OpSTA<ModeZeroPage>(0x85, 2, 3));
    opCodes[0x86].reset(new OpSTX<ModeZeroPage>(0x86, 2, 3));
    opCodes[0x88].reset(new OpDEY());
    opCodes[0x8A].reset(new OpTXA());
    opCodes[0x8E].reset(new OpSTX<ModeAbsoluteOutput>(0x8E, 3, 4));
    opCodes[0x90].reset(new OpBCC());
    opCodes[0x98].reset(new OpTYA());
    opCodes[0x9A].reset(new OpTXS());
    opCodes[0xA0].reset(new OpLDY<ModeImmediate>(0xA0, 2, 2));
    opCodes[0xA2].reset(new OpLDX<ModeImmediate>(0xA2, 2, 2));
    opCodes[0xA8].reset(new OpTAY());
    opCodes[0xA9].reset(new OpLDA<ModeImmediate>(0xA9, 2, 2));
    opCodes[0xAA].reset(new OpTAX());
    opCodes[0xAD].reset(new OpLDA<ModeAbsoluteOutput>(0xAD, 3, 4));
    opCodes[0xAE].reset(new OpLDX<ModeAbsoluteOutput>(0xAE, 3, 4));
    opCodes[0xB0].reset(new OpBCS());
    opCodes[0xB8].reset(new OpCLV());
    opCodes[0xBA].reset(new OpTSX());
    opCodes[0xC0].reset(new OpCPY<ModeImmediate>(0xC0, 2, 2));
    opCodes[0xC8].reset(new OpINY());
    opCodes[0xC9].reset(new OpCMP<ModeImmediate>(0xC9, 2, 2));
    opCodes[0xCA].reset(new OpDEX());
    opCodes[0xD0].reset(new OpBNE());
    opCodes[0xD8].reset(new OpCLD());
    opCodes[0xE0].reset(new OpCPX<ModeImmediate>(0xE0, 2, 2));
    opCodes[0xE8].reset(new OpINX());
    opCodes[0xE9].reset(new OpSBC<ModeImmediate>(0xE9, 2, 2));
    opCodes[0xEA].reset(new OpNOP());
    opCodes[0xF0].reset(new OpBEQ());
    opCodes[0xF8].reset(new OpSED());

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
        op(args, registers, info, memory);
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
