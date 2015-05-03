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
#include <nes/Op6502.h>

namespace nyra
{
namespace nes
{
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
        // Special case for $2002
        if (mMode->getArg() == 0x2002)
        {
            setRegister(0x10,
                        registers.accumulator,
                        registers.statusRegister);
            return;
        }

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
                (1 << nyra::nes::IGNORE);
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
        registers.statusRegister[nyra::nes::CARRY] = 1;
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
        registers.statusRegister[nyra::nes::CARRY] = 0;
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
        registers.statusRegister[nyra::nes::OFLOW] = 0;
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
        registers.statusRegister[nyra::nes::INTERRUPT] = 1;
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
        registers.statusRegister[nyra::nes::DECIMAL] = 1;
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
        registers.statusRegister[nyra::nes::DECIMAL] = 0;
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
            (1 << nyra::nes::IGNORE)) &
            ~(1 << nyra::nes::STACK));
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
                          (1 << nyra::nes::STACK),
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
        return statusRegister[nyra::nes::CARRY];
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
        return statusRegister[nyra::nes::ZERO];
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
        return !statusRegister[nyra::nes::ZERO];
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
        return !statusRegister[nyra::nes::CARRY];
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
        return statusRegister[nyra::nes::OFLOW];
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
        return !statusRegister[nyra::nes::OFLOW];
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
        return !statusRegister[nyra::nes::SIGN];
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
        return statusRegister[nyra::nes::SIGN];
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
}
