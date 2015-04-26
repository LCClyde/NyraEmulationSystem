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
#include <nes/Header.h>
#include <core/Exception.h>

namespace
{
/*****************************************************************************/
static const size_t PROG_SIZE_LOCATION = 4;
static const size_t CHR_SIZE_LOCATION = 5;

/*****************************************************************************/
/*
 * Byte 6
 * 7       0
 * ---------
 * NNNN FTBM
 *
 * N: Lower 4 bits of the mapper number
 * F: Four screen mode. 0 = no, 1 = yes.
 *    (When set, the M bit has no effect)
 * T: Trainer.  0 = no trainer present, 1 = 512 byte
 *    trainer at 7000-71FFh
 * B: SRAM at 6000-7FFFh battery backed.  0= no, 1 = yes
 * M: Mirroring.  0 = horizontal, 1 = vertical.
 */
/*****************************************************************************/
static const size_t FLAG_6 = 6;

/*****************************************************************************/
/*
 * Byte 7
 * 7       0
 * ---------
 * NNNN SSPV
 *
 * N: Upper 4 bits of the mapper number
 * S: When equal to binary 10, use NES 2.0 rules;
 *    otherwise, use other rules.
 * P: Playchoice 10.  When set, this is a PC-10 game
 * V: Vs. Unisystem.  When set, this is a Vs. game
 */
/*****************************************************************************/
static const size_t FLAG_7 = 7;
}

namespace nyra
{
namespace nes
{
/*****************************************************************************/
const size_t Header::HEADER_SIZE = 16;

/*****************************************************************************/
Header::Header(const std::vector<uint8_t>& binary) :
    mNESIdentifier(binary.size() >= HEADER_SIZE ?
            std::string(reinterpret_cast<const char*>(&binary[0]), 0, 3) :
            throw core::Exception("Incorrect NES file size")),
    mProgSize(binary[PROG_SIZE_LOCATION]),
    mChrRomSize(binary[CHR_SIZE_LOCATION]),
    mMapperNumber((binary[FLAG_6] >> 4) | (binary[FLAG_7] & 0xF0)),
    mFourScreenMode((binary[FLAG_6] & 0x8) != 0),
    mTrainer((binary[FLAG_6] & 0x04) != 0),
    mBatteryBack((binary[FLAG_6] & 0x02) != 0),
    mMirroring((binary[FLAG_6] & 0x01) == 0 ? HORIZONTAL : VERTICAL),
    mPlayChoice10((binary[FLAG_7] & 0x02) != 0),
    mVsUnisystem((binary[FLAG_7] & 0x01) != 0),
    mIsNes2_0((binary[FLAG_7] & 0x08) == 0x08)

{
    if (mNESIdentifier != "NES")
    {
        throw core::Exception("Unknown header type: " + mNESIdentifier);
    }
}

/*****************************************************************************/
std::ostream& operator<<(std::ostream& os,
                         const Header& header)
{
    std::string yesNo[2] = {"No", "Yes"};
    std::string mirroring[2] = {"HORIZONTAL", "VERTICAL"};

    os << "NES Identifier:      " << header.getNESIdentifier()
       << "\nProg Size:           " << header.getProgRomSize()
       << "\nChr Rom Size:        " << header.getChrRomSize()
       << "\nMapper Number:       " << header.getMapperNumber()
       << "\nFour Screen Mode:    " << yesNo[header.getIsFourScreenMode()]
       << "\nTrainer:             " << yesNo[header.getHasTrainer()]
       << "\nBattery Back:        " << yesNo[header.getHasBatteryBack()]
       << "\nMirroring:           " << mirroring[header.getMirroring()]
       << "\nPlayChoice-10:       " << yesNo[header.getIsPlayChoice10()]
       << "\nVS System:           " << yesNo[header.getIsVsUnisystem()]
       << "\nNES 2.0:             " << yesNo[header.getIsNes2_0()];
    return os;
}
}
}
