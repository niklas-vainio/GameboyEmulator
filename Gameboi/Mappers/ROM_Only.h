// Created by Niklas on 10/04/2020.
// Class to store a ROM only MBC (id 00), which has no RAM and maps directly to the cartridge ROM

#ifndef ROM_Only_h
#define ROM_Only_h

#include "MBC_Base.h"

namespace gb {
    class ROM_only : public MBC_Base {
    protected:
    public:
        // Constructor from parent class
        ROM_only(gb::cartridge_rom* _cartridge, bool _has_ram, bool _has_battery) : MBC_Base(_cartridge, _has_ram, _has_battery){}
        
        // Reading from ROM just reads from the same address as is in the cartridge
        uint8_t read_rom(uint16_t addr) {
            return cartridge->read(addr);
        }
        
        // Writing to ROM does nothing
        void write_rom(uint16_t addr, uint8_t data) {}
        
        // Reading from/writing to RAM does nothing
        uint8_t read_ram(uint16_t addr) {return 0;}
        void write_ram(uint16_t addr, uint8_t data) {}
    };
}

#endif /* ROM_Only_h */
