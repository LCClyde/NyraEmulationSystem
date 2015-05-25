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
#include <nes/CPU.h>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
const size_t CPU::INTERRUPT_OPCODE = 0x100;

/*****************************************************************************/
CPU::CPU(uint16_t startAddress) :
    mInfo(startAddress)
{
    allocateOpCodes(mOpCodes);
}

/*****************************************************************************/
void CPU::processScanline(MemoryMap& ram,
                          std::vector<Disassembly>* disassembly)
{
    // Process one scanline
    const int16_t scanline = mInfo.scanLine;

    // Check for interrupts
    if (mInfo.generateNMI)
    {
        ram.getOpInfo(0XFFF9, mArgs);
        (*mOpCodes[INTERRUPT_OPCODE])(mArgs, mRegisters, mInfo, ram);
        mInfo.generateNMI = false;
    }

    while (scanline == mInfo.scanLine)
    {
        ram.getOpInfo(mInfo.programCounter,
                      mArgs);

        if (disassembly)
        {
            disassembly->push_back(Disassembly(
                    *mOpCodes[mArgs.opcode],
                    mArgs,
                    mRegisters,
                    mInfo));
        }

        (*mOpCodes[mArgs.opcode])(mArgs, mRegisters, mInfo, ram);

        if (disassembly)
        {
            Disassembly& dis = disassembly->back();
            dis.setModeString(dis.getOpCode().getMode().toString());
        }
    }
}
}
}
