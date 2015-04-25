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
#ifndef __NYRA_NES_HEADER_H__
#define __NYRA_NES_HEADER_H__

#include <ostream>
#include <vector>
#include <stdint.h>

namespace nyra
{
namespace nes
{
/*
 *  \class - Header
 *  \brief - Holds information about the file and contents of a single game
 */
class Header
{
public:
    /*
     *  \enum - Mirroring
     *  \brief - Describes how the nametables should be presented to
     *           the player.
     */
    enum Mirroring
    {
        HORIZONTAL,
        VERTICAL
    };

    /*
     *  \func - Constructor (string)
     *  \brief - Creates a Header from a string.
     *
     *  \param binary - This is the file contents of the cartridge.
     *         The Header will read from this to construct its values.
     *  \throw - If the stream does not look like a NES header.
     */
    Header(const std::vector<uint8_t>& binary);

    /*
     *  \func - getProgRomSize
     *  \brief - Returns the size of the read only memory that contains the
     *           programming logic.
     */
    inline size_t getProgRomSize() const
    {
        return mProgSize;
    }

    /*
     *  \func - getChrRomSize
     *  \brief - Returns the size of the character read only memory.
     */
    inline size_t getChrRomSize() const
    {
        return mChrRomSize;
    }

    /*
     *  \func - getMapperNumber
     *  \brief - Returns the mapper number associated with this NES game.
     */
    inline size_t getMapperNumber() const
    {
        return mMapperNumber;
    }

    /*
     *  \func - getIsFourScreenMode
     *  \brief - Returns true if this cartridge is setup for four
     *           screen mirroring.
     */
    inline bool getIsFourScreenMode() const
    {
        return mFourScreenMode;
    }

    /*
     *  \func - getHasTrainer
     *  \brief - Returns true if the NES header has trainer memory
     *           associated with it. This will be very rare to see.
     */
    inline bool getHasTrainer() const
    {
        return mTrainer;
    }

    /*
     *  \func - getHasBatteryBack
     *  \brief - Returns true if this cartridge contains battery-backed
     *           persistent memory.
     */
    inline bool getHasBatteryBack() const
    {
        return mBatteryBack;
    }

    /*
     *  \func - getMirroring
     *  \brief - Returns the direction of nametable mirroring
     */
    inline Mirroring getMirroring() const
    {
        return mMirroring;
    }

    /*
     *  \func - getIsPlayChoice10
     *  \brief - Returns if the cartridge supports PlayChoice-10. This is used
     *           to make game look better with 2C03 RGB PPU. It handles
     *           color emphasis differently than a standard NES PPU.
     */
    inline bool getIsPlayChoice10() const
    {
        return mPlayChoice10;
    }

    /*
     *  \func - getIsVsUnisystem
     *  \brief - Returns true if game is actually for the Nintendo VS System.
     */
    inline bool getIsVsUnisystem() const
    {
        return mVsUnisystem;
    }

    /*
     *  \func - getisNes2_0
     *  \brief - Returns true if this game uses an NES 2.0 header.
     */
    inline bool getIsNes2_0() const
    {
        return mIsNes2_0;
    }

    /*
     *  \func - getHeaderSize
     *  \brief - Returns the number of bytes a header should be.
     */
    inline size_t getHeaderSize() const
    {
        return HEADER_SIZE;
    }

    /*
     *  \func - getNESIdentifier
     *  \brief - Returns the NES identifier value. This shoudl always be
     *           "NES" if the header was properly read.
     */
    inline std::string getNESIdentifier() const
    {
        return mNESIdentifier;
    }

private:
    static const size_t HEADER_SIZE;

    const std::string mNESIdentifier;
    const size_t mProgSize;
    const size_t mChrRomSize;
    const size_t mMapperNumber;
    const bool mFourScreenMode;
    const bool mTrainer;
    const bool mBatteryBack;
    const Mirroring mMirroring;
    const bool mPlayChoice10;
    const bool mVsUnisystem;
    const bool mIsNes2_0;
};

/*
 *  \func - operator <<
 *  \brief - Dumps all relavant data from the header to an output stream.
 */
std::ostream& operator<<(std::ostream& os,
                         const Header& header);
}
}

#endif
