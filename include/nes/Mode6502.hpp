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
#ifndef __NYRA_NES_MODE_6502_HPP__
#define __NYRA_NES_MODE_6502_HPP__

#include <nes/OpCode.h>

/*****************************************************************************/
// TODO: Most of the modes have extra unnecessary operations which are used to
//       match a known debug string output. There needs to be a way to remove
//       theses. Maybe with a #ifdef DEBUG or something similar.
/*****************************************************************************/
namespace nyra
{
namespace nes
{
/*****************************************************************************/
class ModeAccumulator : public Mode
{
public:
    ModeAccumulator() :
        Mode(false, false)
    {
    }

    std::string toString() const
    {
        return "A";
    }

    void operator()(const CPUArgs& ,
                    const CPURegisters& registers,
                    const MemoryMap& ,
                    CPUInfo& )
    {
        mValue = registers.accumulator;
        mArg = mValue;
    }
};

/*****************************************************************************/
template <bool OutputT>
class ModeAbsolute : public Mode
{
public:
    ModeAbsolute() :
        Mode(true, true)
    {
    }

    std::string toString() const
    {
        const std::string outputString = OutputT ?
                " = " + core::toHexString<uint8_t>(mValue) : "";
        return "$" + core::toHexString<uint16_t>(mArg) + outputString;
    }

    void operator()(const CPUArgs& args,
                    const CPURegisters& ,
                    const MemoryMap& memory,
                    CPUInfo& )
    {
        mArg = args.darg;
        mValue = OutputT ? memory.readByte(mArg) : 0;
    }
};

/*****************************************************************************/
class ModeIndirect : public Mode
{
public:
    ModeIndirect() :
        Mode(true, true),
        mOrigArg(0),
        mCorrectArg(0)
    {
    }

    std::string toString() const
    {
        return "($" + core::toHexString<uint16_t>(mOrigArg) + ") = " +
                core::toHexString<uint16_t>(mCorrectArg);
    }

    void operator()(const CPUArgs& args,
                    const CPURegisters& ,
                    const MemoryMap& memory,
                    CPUInfo& )
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
class ModeIndirectX : public Mode
{
public:
    ModeIndirectX() :
        Mode(true, false),
        mOrigArg(0),
        mModArg(0)
    {
    }

    std::string toString() const
    {
        return "($" + core::toHexString<uint8_t>(mOrigArg) + ",X) @ " +
                core::toHexString<uint8_t>(mModArg) + " = " +
                core::toHexString<uint16_t>(mArg) + " = " +
                core::toHexString<uint8_t>(mValue);
    }

    void operator()(const CPUArgs& args,
                    const CPURegisters& registers,
                    const MemoryMap& memory,
                    CPUInfo& )
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
class ModeZeroPageN : public Mode
{
public:
    ModeZeroPageN(char index) :
        Mode(true, false),
        mOrigArg(0),
        mIndex(index)
    {
    }

    std::string toString() const
    {
        return "$" + core::toHexString<uint8_t>(mOrigArg) +
                "," + mIndex + " @ " +
                core::toHexString<uint8_t>(
                        static_cast<uint8_t>(mArg)) + " = " +
                core::toHexString<uint8_t>(mValue);
    }

    void operator()(const CPUArgs& args,
                    const CPURegisters& registers,
                    const MemoryMap& memory,
                    CPUInfo& )
    {
        mOrigArg = args.arg1;
        mArg = (mOrigArg + getIndex(registers)) & 0xFF;
        mValue = memory.readByte(mArg);
    }

private:
    virtual uint8_t getIndex(
            const CPURegisters& registers) const = 0;

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
            const CPURegisters& registers) const
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
            const CPURegisters& registers) const
    {
        return registers.yIndex;
    }
};

/*****************************************************************************/
template <bool ExtraCycleT>
class ModeAbsoluteN : public Mode
{
public:
    ModeAbsoluteN(char index) :
        Mode(true, true),
        mOrigArg(0),
        mIndex(index)
    {
    }

    std::string toString() const
    {
        return "$" + core::toHexString<uint16_t>(mOrigArg) +
                "," + mIndex + " @ " +
                core::toHexString<uint16_t>(mArg) + " = " +
                core::toHexString<uint8_t>(mValue);
    }

    void operator()(const CPUArgs& args,
                    const CPURegisters& registers,
                    const MemoryMap& memory,
                    CPUInfo& info)
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
            const CPURegisters& registers) const = 0;

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
            const CPURegisters& registers) const
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
            const CPURegisters& registers) const
    {
        return registers.xIndex;
    }
};

/*****************************************************************************/
template <bool ExtraCycleT>
class ModeIndirectY : public Mode
{
public:
    ModeIndirectY() :
        Mode(true, false),
        mOrigArg(0),
        mModArg(0)
    {
    }

    std::string toString() const
    {
        return "($" + core::toHexString<uint8_t>(mOrigArg) + "),Y = " +
                core::toHexString<uint16_t>(mModArg) + " @ " +
                core::toHexString<uint16_t>(mArg) + " = " +
                core::toHexString<uint8_t>(mValue);
    }

    void operator()(const CPUArgs& args,
                    const CPURegisters& registers,
                    const MemoryMap& memory,
                    CPUInfo& info)
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
class ModeRelative : public Mode
{
public:
    ModeRelative() :
        Mode(true, false)
    {
    }

    std::string toString() const
    {
        return "$" + core::toHexString<uint16_t>(mArg);
    }

    void operator()(const CPUArgs& args,
                    const CPURegisters& ,
                    const MemoryMap& ,
                    CPUInfo& info)
    {
        // Value is never used for relative mode
        mArg = info.programCounter + static_cast<int8_t>(args.arg1) + 2;

    }
};

/*****************************************************************************/
class ModeZeroPage : public Mode
{
public:
    ModeZeroPage() :
        Mode(true, false)
    {
    }

    std::string toString() const
    {
        return "$" + core::toHexString<uint8_t>(
                static_cast<uint8_t>(mArg)) + " = " + 
                core::toHexString<uint8_t>(mValue);
    }

    void operator()(const CPUArgs& args,
                    const CPURegisters& ,
                    const MemoryMap& memory,
                    CPUInfo& )
    {
        mArg = args.arg1;
        mValue = memory.readByte(mArg);
    }
};

/*****************************************************************************/
class ModeImmediate : public Mode
{
public:
    ModeImmediate() :
        Mode(true, false)
    {
    }

    std::string toString() const
    {
        return "#$" + core::toHexString<uint8_t>(mValue);
    }

    void operator()(const CPUArgs& args,
                    const CPURegisters& ,
                    const MemoryMap& ,
                    CPUInfo& )
    {
        mValue = args.arg1;
        mArg = mValue;
    }
};

/*****************************************************************************/
class ModeImplied : public Mode
{
public:
    ModeImplied() :
        Mode(false, false)
    {
    }

    std::string toString() const
    {
        return "";
    }

    void operator()(const CPUArgs& ,
                    const CPURegisters& ,
                    const MemoryMap& ,
                    CPUInfo& )
    {
        // Nothing to do here
    }
};
}
}
#endif
