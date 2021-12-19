// Created by Niklas on 31/03/2020.
// Stores and Accesses the ROM information on the cartridge

#ifndef Cart_ROM_h
#define Cart_ROM_h

#include <cstdint>
#include <fstream>
#include <vector>
#include <iostream>

#include "Utils.h"


namespace gb {
    class cartridge_rom {
    private:
        // Stores a vector of the raw ROM data
        std::vector<uint8_t> rom_data;
        
        // Stores a mapping of ram size values to actual ram sizes
        const int ram_sizes[6] = {0, 0x800, 0x2000, 0x8000, 0x20000, 0x10000};
        
        // Stores the size of the rom
        long rom_size;
        
    public:
        // Function to load the rom from a file
        void load_from_file(std::string filepath) {
            std::ifstream input_file;
            input_file.open(filepath);
            
            // Give an error if file not found
            if (!input_file){
                std::cerr << "File not found at " << filepath << "!" << std::endl;
                return;
            }
            
            // Reads first 0x200 bytes of ROM
            for (int i = 0; i < 0x200; i++){
                rom_data.push_back(input_file.get());
            }
            
            // Gets the ROM size from address 0148
            switch (rom_data[0x148]) {
                case 0x52: rom_size = 0x120000; break;
                case 0x53: rom_size = 0x140000; break;
                case 0x54: rom_size = 0x180000; break;
                default: rom_size = 0x8000 << rom_data[0x148]; break;
            }
            
            // Reds the rest of ROM
            for (int i = 0; i < (rom_size - 0x200); i++){
                rom_data.push_back(input_file.get());
            }
            input_file.close();
        }
        
        // Function to get the mapper type at address 0147 in the ROM
        uint8_t get_mapper_type(){
            return rom_data[0x0147];
        }
        
        // Function to get the size of ExRAM in bytes
        int get_ram_size(){
            return ram_sizes[rom_data[0x0149]];
        }
        
        
        // Function to read from the ROM (address is 32 bits as ROMS can be larger than the 64kB address bus)
        uint8_t read(uint32_t addr){
            if (addr < rom_data.size()){
                return rom_data[addr];
            }
            else {
                std::cerr << "ROM Address " << std::hex << addr <<  " out of range!" << std::endl;
                return 0;
            }
        }
    };
}

#endif /* Cart_ROM_h */
