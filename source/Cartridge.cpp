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
#include <nes/Cartridge.h>
#include <core/File.h>

namespace nyra
{
namespace nes
{
/*****************************************************************************/
static const size_t PRG_ROM_SIZE = 16384;

/*****************************************************************************/
Cartridge::Cartridge(const std::string& pathname) :
    mFile(core::readBinary(pathname)),
    mHeader(mFile),
    mProgROM(mHeader.getProgRomSize())
{
    const uint8_t* ptr = reinterpret_cast<const uint8_t*>(
            &mFile[0] + mHeader.getHeaderSize());

    //! Assign each piece of ROM
    for (size_t ii = 0; ii < mProgROM.size(); ++ii, ptr += PRG_ROM_SIZE)
    {
        mProgROM[ii].reset(new ROM(ptr, PRG_ROM_SIZE, false));
    }
}
}
}
