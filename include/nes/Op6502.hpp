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
#ifndef __NYRA_NES_OP_6502_HPP__
#define __NYRA_NES_OP_6502_HPP__

#include <stdint.h>
#include <stdexcept>
#include <nes/Mode6502.hpp>
#include <nes/OpCode.h>
#include <nes/Constants.h>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
uint8_t shiftRight(uint8_t value,
                   bool rotate,
                   std::bitset<FLAG_SIZE>& statusRegister)
{
    uint8_t ret = value >> 1;
    if (rotate)
    {
        ret |= (statusRegister[CARRY] << 7);
    }
    statusRegister[CARRY] = (value & 0x01) != 0;
    return ret;
}

/*****************************************************************************/
uint8_t shiftLeft(uint8_t value,
                  bool rotate,
                  std::bitset<FLAG_SIZE>& statusRegister)
{
    uint8_t ret = value << 1;
    if (rotate)
    {
        ret |= static_cast<uint8_t>(
                statusRegister[CARRY]);
    }
    statusRegister[CARRY] = (value & 0x80) != 0;
    return ret;
}

/*****************************************************************************/
void compare(uint8_t value,
             uint8_t reg,
             std::bitset<FLAG_SIZE>& statusRegister)
{
    statusRegister[CARRY] = reg >= value;
    statusRegister[ZERO] = reg == value;
    statusRegister[SIGN] =
            static_cast<uint8_t>(reg - value) >= 0x80;
}

/*****************************************************************************/
void setRegister(uint8_t value,
                 uint8_t& reg,
                 std::bitset<FLAG_SIZE>& statusRegister)
{
    statusRegister[SIGN] = (value >= 0x80);
    statusRegister[ZERO] = (value == 0);
    reg = value;
}

/*****************************************************************************/
void add(uint8_t value, CPURegisters& registers)
{
    const size_t sum = registers.accumulator + value +
            registers.statusRegister[CARRY];
    registers.statusRegister[OFLOW] =
            ((registers.accumulator ^ sum) & (value ^ sum) & 0x80) != 0;
    registers.statusRegister[CARRY] = (sum > 0xFF) != 0;
    setRegister(static_cast<uint8_t>(sum),
                registers.accumulator, registers.statusRegister);
}

/*****************************************************************************/
void pushStack(uint8_t value, MemoryMap& ram, uint8_t& stackPointer)
{
    ram.writeByte(static_cast<size_t>(stackPointer | 0x100), value);
    --stackPointer;
}

/*****************************************************************************/
uint8_t popStack(MemoryMap& ram, uint8_t& stackPointer)
{
    ++stackPointer;
    return ram.readByte(static_cast<size_t>(stackPointer | 0x100));
}

/*****************************************************************************/
class OpNUL : public OpCode
{
public:
    OpNUL(uint8_t opCode) :
        OpCode("NUL", "Null Opcode", opCode,
               0, 0, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& ,
            CPUInfo& ,
            MemoryMap& )
    {
        throw std::runtime_error("Attempting to run null op");
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpJMP : public OpCode
{
public:
    OpJMP(uint8_t opcode, uint8_t time) :
        OpCode("JMP", "Jump", opcode, 0, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& ,
            CPUInfo& info,
            MemoryMap& )
    {
        info.programCounter = mMode->getArg();
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpLDX : public OpCode
{
public:
    OpLDX(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("LDX", "Load X register",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(mMode->getValue(),
                    registers.xIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpLDY : public OpCode
{
public:
    OpLDY(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("LDY", "Load Y register",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(mMode->getValue(),
                    registers.yIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpLDA : public OpCode
{
public:
    OpLDA(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("LDA", "Load accumulator",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        // Special case for ppu
        /*if (mMode->getArg() == ppu.statusAddress)
        {
            setRegister(static_cast<uint8_t>(ppu.status.to_ulong()),
                        registers.accumulator,
                        registers.statusRegister);
            return;
        }*/

        setRegister(mMode->getValue(),
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpLSR : public OpCode
{
public:
    OpLSR(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("LSR", "Logical shift right",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
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
class OpLSR <ModeAccumulator> : public OpCode
{
public:
    OpLSR(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("LSR", "Logical shift right",
               opcode, length, time, new ModeAccumulator())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
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
class OpASL : public OpCode
{
public:
    OpASL(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("ASL", "Arithmetic shift left",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
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
class OpASL <ModeAccumulator> : public OpCode
{
public:
    OpASL(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("ASL", "Arithmetic shift left",
               opcode, length, time, new ModeAccumulator())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
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
class OpROR : public OpCode
{
public:
    OpROR(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("ROR", "Rotate right",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
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
class OpROR <ModeAccumulator> : public OpCode
{
public:
    OpROR(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("ROR", "Rotate right",
               opcode, length, time, new ModeAccumulator())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
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
class OpROL : public OpCode
{
public:
    OpROL(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("ROL", "Rotate left",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
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
class OpROL <ModeAccumulator> : public OpCode
{
public:
    OpROL(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("ROL", "Rotate left",
               opcode, length, time, new ModeAccumulator())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
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
class OpSTA : public OpCode
{
public:
    OpSTA(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("STA", "Store accumulator",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
    {
        memory.writeByte(mMode->getArg(), registers.accumulator);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpSTX : public OpCode
{
public:
    OpSTX(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("STX", "Store X register",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
    {
        memory.writeByte(mMode->getArg(), registers.xIndex);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpSTY : public OpCode
{
public:
    OpSTY(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("STY", "Store Y register",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
    {
        memory.writeByte(mMode->getArg(), registers.yIndex);
    }
};

/*****************************************************************************/
class OpJSR : public OpCode
{
public:
    OpJSR() :
        OpCode("JSR", "Jump to subroutine",
               0x20, 0, 6, new ModeAbsolute<false>())
    {
    }

    virtual ~OpJSR()
    {
    }

protected:
    virtual void op(CPURegisters& registers,
                    CPUInfo& info,
                    MemoryMap& memory)
    {
        pushStack(((info.programCounter + 2) >> 8) & 0xFF,
                  memory, registers.stackPointer);
        pushStack((info.programCounter + 2) & 0xFF,
                  memory, registers.stackPointer);
        info.programCounter = mMode->getArg();
    }
};

/*****************************************************************************/
class OpJMI : public OpCode
{
public:
    OpJMI() : 
        OpCode("JMI", "Jump to interrupt, NYRA created",
               0x00, 0, 6, new ModeAbsolute<false>())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& info,
            MemoryMap& memory)
    {
        pushStack(((info.programCounter) >> 8) & 0xFF,
                  memory, registers.stackPointer);
        pushStack((info.programCounter) & 0xFF,
                  memory, registers.stackPointer);
        pushStack(static_cast<uint8_t>(registers.statusRegister.to_ulong()),
                  memory, registers.stackPointer);
        info.programCounter = mMode->getArg();
    }
};

/*****************************************************************************/
class OpNOP : public OpCode
{
public:
    OpNOP() :
        OpCode("NOP", "No operation",
               0xEA, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& ,
            CPUInfo& ,
            MemoryMap& )
    {
        // NOP
    }
};

/*****************************************************************************/
class OpRTI : public OpCode
{
public:
    OpRTI() :
        OpCode("RTI", "Return from interrupt",
               0x40, 0, 6, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& info,
            MemoryMap& memory)
    {
        // TODO: Make sure this is correct. I don't think the
        //       stack pointer is manipulated correctly.
        registers.statusRegister = popStack(memory, registers.stackPointer) |
                (1 << IGNORE);
        info.programCounter = popStack(memory, registers.stackPointer) |
                (popStack(memory, registers.stackPointer) << 8);
    }
};

/*****************************************************************************/
class OpRTS : public OpCode
{
public:
    OpRTS() :
        OpCode("RTS", "Return from subroutine",
               0x60, 1, 6, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& info,
            MemoryMap& memory)
    {
        info.programCounter = popStack(memory, registers.stackPointer) |
                              (popStack(memory, registers.stackPointer) << 8);
    }
};

/*****************************************************************************/
class OpINY : public OpCode
{
public:
    OpINY() :
        OpCode("INY", "Increment Y",
               0xC8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(registers.yIndex + 1,
                    registers.yIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpINX : public OpCode
{
public:
    OpINX() :
        OpCode("INX", "Increment X",
               0xC8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(registers.xIndex + 1,
                    registers.xIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpINC : public OpCode
{
public:
    OpINC(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("INC", "Increment memory",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
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
class OpDEC : public OpCode
{
public:
    OpDEC(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("DEC", "Decrement memory",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
    {
        uint8_t garbage;
        setRegister(mMode->getValue() - 1,
                    garbage,
                    registers.statusRegister);
        memory.writeByte(mMode->getArg(), mMode->getValue() - 1);
    }
};

/*****************************************************************************/
class OpDEY : public OpCode
{
public:
    OpDEY() :
        OpCode("DEY", "Decrement Y",
               0x88, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(registers.yIndex - 1,
                    registers.yIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpDEX : public OpCode
{
public:
    OpDEX() :
        OpCode("DEX", "Decrement X",
               0xCA, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(registers.xIndex - 1,
                    registers.xIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpTAX : public OpCode
{
public:
    OpTAX() :
        OpCode("TAX", "Transfer A to X",
               0xAA, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(registers.accumulator,
                    registers.xIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpTXA : public OpCode
{
public:
    OpTXA() :
        OpCode("TXA", "Transfer X to A",
               0x8A, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(registers.xIndex,
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpTAY : public OpCode
{
public:
    OpTAY() :
        OpCode("TAY", "Transfer A to Y",
               0xA8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(registers.accumulator,
                    registers.yIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpTYA : public OpCode
{
public:
    OpTYA() :
        OpCode("TYA", "Transfer Y to A",
               0x98, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(registers.yIndex,
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpSEC : public OpCode
{
public:
    OpSEC() :
        OpCode("SEC", "Set carry",
               0x38, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        registers.statusRegister[CARRY] = 1;
    }
};

/*****************************************************************************/
class OpCLC : public OpCode
{
public:
    OpCLC() :
        OpCode("CLC", "Clear carry",
               0x18, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        registers.statusRegister[CARRY] = 0;
    }
};

/*****************************************************************************/
class OpCLV : public OpCode
{
public:
    OpCLV() :
        OpCode("CLV", "Clear overflow",
               0xB8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        registers.statusRegister[OFLOW] = 0;
    }
};

/*****************************************************************************/
class OpSEI : public OpCode
{
public:
    OpSEI() :
        OpCode("SEI", "Set interrupt",
               0x78, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        registers.statusRegister[INTERRUPT] = 1;
    }
};

/*****************************************************************************/
class OpSED : public OpCode
{
public:
    OpSED() :
        OpCode("SED", "Set decimal",
               0xF8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        registers.statusRegister[DECIMAL] = 1;
    }
};

/*****************************************************************************/
class OpCLD : public OpCode
{
public:
    OpCLD() :
        OpCode("CLD", "Clear decimal",
               0xD8, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        registers.statusRegister[DECIMAL] = 0;
    }
};

/*****************************************************************************/
class OpTSX : public OpCode
{
public:
    OpTSX() :
        OpCode("TSX", "Transfer stack ptr to X",
               0xBA, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(registers.stackPointer,
                    registers.xIndex,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpTXS : public OpCode
{
public:
    OpTXS() :
        OpCode("TXS", "Transfer X to stack ptr",
               0x9A, 1, 2, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        registers.stackPointer = registers.xIndex;
    }
};

/*****************************************************************************/
class OpPLA : public OpCode
{
public:
    OpPLA() :
        OpCode("PLA", "Pull accumulator",
               0x68, 1, 4, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
    {
        setRegister(popStack(memory, registers.stackPointer),
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
class OpPHA : public OpCode
{
public:
    OpPHA() :
        OpCode("PHA", "Push accumulator",
               0x48, 1, 3, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
    {
        pushStack(static_cast<uint8_t>(registers.accumulator),
                  memory, registers.stackPointer);
    }
};

/*****************************************************************************/
class OpPLP : public OpCode
{
public:
    OpPLP() :
        OpCode("PLP", "Pull processor status",
               0x28, 1, 4, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
    {
        registers.statusRegister =
            ((popStack(memory, registers.stackPointer) |
            (1 << IGNORE)) &
            ~(1 << STACK));
    }
};
/*****************************************************************************/
class OpPHP : public OpCode
{
public:
    OpPHP() :
        OpCode("PHP", "Push processor status",
               0x08, 1, 3, new ModeImplied())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& memory)
    {
        pushStack(static_cast<uint8_t>(registers.statusRegister.to_ulong()) |
                          (1 << STACK),
                  memory, registers.stackPointer);
    }
};

/*****************************************************************************/
class OpBranch : public OpCode
{
public:
    OpBranch(const std::string& name,
             const std::string& extendedName,
             uint8_t opcode) :
        OpCode(name, extendedName, opcode,
               0, 2, new ModeRelative())
    {
    }

private:
    virtual bool branchArg(
            const std::bitset<8>& statusRegister) const = 0;

    void op(CPURegisters& registers,
            CPUInfo& info,
            MemoryMap& )
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
        return statusRegister[CARRY];
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
        return statusRegister[ZERO];
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
        return !statusRegister[ZERO];
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
        return !statusRegister[CARRY];
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
        return statusRegister[OFLOW];
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
        return !statusRegister[OFLOW];
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
        return !statusRegister[SIGN];
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
        return statusRegister[SIGN];
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpBIT : public OpCode
{
public:
    OpBIT(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("BIT", "Test bits",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        const size_t param = mMode->getValue();
        registers.statusRegister[ZERO] = (param & registers.accumulator) == 0;
        registers.statusRegister[OFLOW] = (param & (1 << OFLOW)) != 0;
        registers.statusRegister[SIGN] = (param & (1 << SIGN)) != 0;
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpCMP : public OpCode
{
public:
    OpCMP(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("CMP", "Compare accumulator",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        compare(mMode->getValue(),
                registers.accumulator,
                registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpCPY : public OpCode
{
public:
    OpCPY(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("CPY", "Compare Y register",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        compare(mMode->getValue(),
                registers.yIndex,
                registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpCPX : public OpCode
{
public:
    OpCPX(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("CPX", "Compare X register",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        compare(mMode->getValue(),
                registers.xIndex,
                registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpAND : public OpCode
{
public:
    OpAND(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("AND", "Bitwise AND with accumulator",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(mMode->getValue() & registers.accumulator,
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpORA : public OpCode
{
public:
    OpORA(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("ORA", "Bitwise OR with accumulator",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(mMode->getValue() | registers.accumulator,
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpEOR : public OpCode
{
public:
    OpEOR(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("EOR", "Bitwise exclusive OR",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        setRegister(mMode->getValue() ^ registers.accumulator,
                    registers.accumulator,
                    registers.statusRegister);
    }
};

/*****************************************************************************/
template <typename ModeT>
class OpADC : public OpCode
{
public:
    OpADC(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("ADC", "Add with carry",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        add(mMode->getValue(), registers);
    }
};
/*****************************************************************************/
template <typename ModeT>
class OpSBC : public OpCode
{
public:
    OpSBC(uint8_t opcode, uint8_t length, uint8_t time) :
        OpCode("SBC", "Subtract with carry",
               opcode, length, time, new ModeT())
    {
    }

private:
    void op(CPURegisters& registers,
            CPUInfo& ,
            MemoryMap& )
    {
        add(~mMode->getValue(), registers);
    }
};
}
}

#endif
