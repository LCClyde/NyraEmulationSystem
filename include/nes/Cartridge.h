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
#ifndef __NYRA_NES_CARTRIDGE_H__
#define __NYRA_NES_CARTRIDGE_H__

#include <string>
#include <vector>
#include <memory>
#include <nes/Header.h>
#include <nes/Memory.h>
#include <nes/Constants.h>

namespace nyra
{
namespace nes
{
/*
 *  \class - Cartridge
 *  \brief - Handles parsing and storing all information obtained from a
 *           NES cartridge (or in our case a NES rom file).
 */
class Cartridge
{
public:
    /*
     *  \func - Constructor (pathname)
     *  \brief - Creates a cartridge object from a file pathname.
     *
     *  \param pathname - The full path of the file on disk.
     */
    Cartridge(const std::string& pathname);

    /*
     *  \func - getHeader
     *  \brief - Returns the information about the header in the cartridge.
     */
    const Header& getHeader() const
    {
        return mHeader;
    }

    /*
     *  \func - getProgROM
     *  \brief - Returns all known banks of the programming memory.
     */
    const ROMBanks& getProgROM() const
    {
        return mProgROM;
    }

    const ROMBanks& getChrROM() const
    {
        return mChrROM;
    }

private:
    const std::vector<uint8_t> mFile;
    const Header mHeader;
    ROMBanks mProgROM;
    ROMBanks mChrROM;
};
}
}

#endif
