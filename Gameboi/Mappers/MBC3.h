// Created by Niklas on 19/04/2020.
// Class for the MBC3 mapper
// Stores up to 2MB ROM and 32KB RAM

#ifndef MBC3_h
#define MBC3_h

#include "MBC_Base.h"

namespace gb {
    class MBC3 : public MBC_Base {
    protected:
        // Stores the current rom and ram bank values
        uint8_t rom_bank = 1;
        uint8_t ram_bank = 0;
        
        // Creates an std::vector to store ram
        std::vector<uint8_t> ram;
        
        // Stores whether RAM is enabled and the ram size
        bool ram_enabled = false;
        long ram_size;
        
    public:
        // Constructor from parent class
        MBC3(gb::cartridge_rom* _cartridge, bool _has_ram, bool _has_battery) : MBC_Base(_cartridge, _has_ram, _has_battery){}
        
        void init() {
            // Reserves the correct amount of space in the ram vector
            ram_size = cartridge->get_ram_size();
            ram.reserve(ram_size);
            
            // If there is a battery, loads the contents of battery backed ram from saves file
            if (has_battery) {
                std::ifstream input_file;
                input_file.open(memory_path, std::ios::in | std::ios::binary);
                
                // If the file already exists, then load data
                if (input_file)
                    for (int i = 0; i < ram_size; i++)
                        ram.push_back(input_file.get());
                
                input_file.close();
            }
        }
        
        void close() {
            // If there is a battery, saves the contents of battery backed ram to save file
            if (has_battery) {
                std::ofstream output_file;
                output_file.open(memory_path, std::ios::out | std::ios::binary | std::ios::trunc);
                
                for (int i = 0; i < ram_size; i++)
                    output_file << ram[i];
                
                output_file.close();
            }
        }
        
        uint8_t read_rom(uint16_t addr) {
            // Reading from 0x0000 - 0x3FFF reads from that address in the cartridge (bank 0)
            if (addr < 0x4000)
                return cartridge->read(addr);
            else {
                // Reading from 0x4000 - 0x7FFF reads from the selected bank
                return cartridge->read(0x4000 * rom_bank + (addr - 0x4000));
            }
        }
        
        void write_rom(uint16_t addr, uint8_t data) {
            // Writing to an address between 0x0000 and 0x1FFF enables/disables ram
            if (addr < 0x2000) {
                // XA enables ram, X0 disables ram
                if ((data & 0x0F) == 0xA) ram_enabled = true;
                if ((data & 0x0F) == 0x0) ram_enabled = false;
            }
            
            // Writing to an address between 0x2000 and 0x3FFF sets the rom bank to the lower 7 bits of the value written
            else if (addr < 0x4000) {
                rom_bank = data & 0b01111111;
                
                // If the current bank is 0x00,1 is added to the bank number
                if (rom_bank == 0)
                    rom_bank++;
            }
            
            // Writing to an address between 0x4000 and 0x5FFF sets the ram bank to the lower 2 bits of the value written
            else if (addr < 0x6000) {
                if (ram_bank <= 0x03)
                    ram_bank = data & 0b00000011;
            }
            // Writing to an address between 0x6000 and 0x7FFF latches RTC data (unimplemented)
            else if (addr < 0x8000) {
            }
            
        }
        
        // Functions to read from/write to ram
        uint8_t read_ram(uint16_t addr) {
            // If no ram or ram is disabled, return 0
            if (!has_ram or !ram_enabled)
                return 0;
            
            // Othwerise, uses the ram bank number
            return ram[0x2000 * ram_bank + (addr - 0xA000)];
        }
        
        void write_ram(uint16_t addr, uint8_t data) {
            // If no ram or ram is disabled, return
            if (!has_ram or !ram_enabled)
                return;
            
            // Othwerise, uses the ram bank number
            ram[0x2000 * ram_bank + (addr - 0xA000)] = data;
        }
    };
}

#endif /* MBC3_h */
