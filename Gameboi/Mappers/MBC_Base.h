// Created by Niklas on 10/04/2020.
// Base class which all MBCS inherit from

#ifndef MBC_Base_h
#define MBC_Base_h

#include "Cart_ROM.h"

namespace gb {
    class MBC_Base {
    protected:
        // Stores a pointer to the cartridge ROM
        gb::cartridge_rom* cartridge;
        
        // Stores whether the cartridge has ram and/or a battery
        bool has_ram; bool has_battery;
        
        // Path to file where non-volatile memory is stored
        std::string memory_path;
        
    public:
        // Constructor which takes in a pointer to the cartridge and whether the cartridge has ram or a battery
        MBC_Base(gb::cartridge_rom* _cartridge, bool _has_ram, bool _has_battery){
            cartridge = _cartridge;
            has_ram = _has_ram;
            has_battery = _has_battery;
        }
        
        // Destructor
        ~MBC_Base(){}
        
        // Virtual method to initialise the mapper
        virtual void init() {};
        
        // Virtual method to shut down the mapper when the emulator exits
        virtual void close() {};
        
        // Method to set path to non-volatile memory
        void set_saves_path(std::string path) {
            memory_path = path;
        }
        
        // Virtual methods to read/write in the ROM area (0x0000 - 0x7FFF in gameboy address bus)
        virtual uint8_t read_rom(uint16_t addr) {};
        virtual void write_rom(uint16_t addr, uint8_t data) {};
        
        // Virtual methods to read/write in the ExRAM area (0xA000 - 0xBFF in gameboy address bus)
        virtual uint8_t read_ram(uint16_t addr) {};
        virtual void write_ram(uint16_t addr, uint8_t data) {};
        
    };
}

#endif /* MBC_Base_h */

