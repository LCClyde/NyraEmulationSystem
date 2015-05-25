%module nes

%{
    #include "nes/Header.h"
    #include "nes/Cartridge.h"
    #include "nes/Memory.h"
    #include "nes/CPU.h"
    #include "nes/Disassembly.h"
    #include "nes/OpCode.h"
    #include "nes/CPUHelper.h"
    #include "nes/MemoryMap.h"
    #include "nes/PPUMemory.h"
    #include "nes/Emulator.h"
    #include "nes/PPU.h"
    #include "nes/Mode.h"

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

%rename("%(undercase)s", %$isfunction) "";
%rename("%(undercase)s", %$isvariable) "";

%include "stdint.i"
%include "std_string.i"
%include "std_vector.i"
%include "nes/Memory.h"
%include "nes/Header.h"
%include "nes/Cartridge.h"
%include "nes/CPUHelper.h"
%include "nes/Constants.h"
%include "nes/PPUMemory.h"
%include "nes/PPU.h"
%include "nes/MemoryMap.h"
%include "nes/Mode.h"
%include "nes/OpCode.h"
%include "nes/Disassembly.h"
%include "nes/CPU.h"
%include "nes/Emulator.h"

%template(DisassemblyVector) std::vector<nyra::nes::Disassembly>;

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



