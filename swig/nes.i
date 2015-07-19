%module nes

%{
    #include "nes/Header.h"
    #include "nes/Cartridge.h"
    #include "nes/Memory.h"
    #include "nes/CPU.h"
    #include "nes/OpCode.h"
    #include "nes/CPUHelper.h"
    #include "nes/MemoryMap.h"
    #include "nes/MemoryFactory.h"
    #include "nes/PPURegisters.h"
    #include "nes/PPU.h"
    #include "nes/Mode.h"
    #include "nes/Controller.h"
    #include "nes/APU.h"

    #include <sstream>
%}

%include "exception.i"

%exception
{
  try
  {
    $action
  }
  catch (const std::exception& e)
  {
    SWIG_exception(SWIG_RuntimeError, e.what());
  }
}

%include "attribute.i"
%include "stdint.i"
%include "std_string.i"
%include "std_vector.i"
%include "std_shared_ptr.i"

%shared_ptr(nyra::nes::MemoryMap)

%attribute(nyra::nes::Header, nyra::nes::Mirroring, mirroring, getMirroring)
%attribute2(nyra::nes::Cartridge, nyra::nes::Header, header, getHeader)
%attribute2(nyra::nes::Cartridge, nyra::nes::ROMBanks, chr_rom, getChrROM)
%attribute2(nyra::nes::OpCode, nyra::nes::Mode, mode, getMode)
%attribute(nyra::nes::Mode, bool, uses_arg1, usesArg1)
%attribute(nyra::nes::Mode, bool, uses_arg2, usesArg2)
%attributestring(nyra::nes::OpCode, std::string, name, getName)
%attribute2(nyra::nes::CPU, nyra::nes::CPUInfo, info, getInfo)

%rename("%(undercase)s", %$isfunction) "";
%rename("%(undercase)s", %$isvariable) "";

%include "nes/Constants.h"
%include "nes/Memory.h"
%include "nes/Header.h"
%include "nes/Cartridge.h"
%include "nes/CPUHelper.h"
%include "nes/HiLowLatch.h"
%include "nes/MemoryMap.h"
%include "nes/PPURegisters.h"
%include "nes/PPU.h"
%include "nes/Controller.h"
%include "nes/APU.h"
%include "nes/MemoryFactory.h"
%include "nes/Mode.h"
%include "nes/OpCode.h"
%include "nes/CPU.h"

%template(PixelVector) std::vector<uint32_t>;

%extend nyra::nes::Header
{
    std::string __str__() const
    {
        std::ostringstream oss;
        oss << *self;
        return oss.str().c_str();
    }
};

%extend nyra::nes::CPURegisters
{
    uint8_t get_status() const
    {
        return static_cast<uint8_t>(self->statusRegister.to_ulong());
    }
};

%extend nyra::nes::PPU
{
    void processScanline(CPUInfo& info, const MemoryMap& memory, size_t buffer)
    {
        $self->processScanline(info, memory, reinterpret_cast<uint32_t*>(buffer));
    }
}



