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
#include <nes/Op6502.hpp>
#include <nes/Mode6502.hpp>

namespace
{
/*****************************************************************************/
static const uint16_t CYCLES_PER_SCANLINE = 341;
static const uint16_t MAX_SCANLINES = 260;
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
