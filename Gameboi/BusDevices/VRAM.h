// Created by Niklas on 31/03/2020.
// Stores the gameboy's vram - tile data and tile name tables
// Addressed from 8000 - 9FFF

#ifndef VRAM_h
#define VRAM_h

#include <iostream>
#include "Utils.h"

namespace gb {
    class vram {
    private:
        // 3 arrays of 2048 bytes representing the 3 tile data tables (8000 - 87FF, 8800 - 8FFF, 9000 - 97FF)
        uint8_t tile_data_0[2048]; uint8_t tile_data_1[2048]; uint8_t tile_data_2[2048];
        
        // 2 arrays of 1024 bytes to store the 2 tile name tables (9800 - 9BFF, 9C00 - 9FFF)
        uint8_t tile_names_0[1024]; uint8_t tile_names_1[1024];
        
    public:
        // Constructor
        vram(){
        }
        
        void write(uint16_t addr, uint8_t data){
            switch (addr & 0xFF00) {
                case 0x8000: case 0x8100: case 0x8200: case 0x8300: case 0x8400: case 0x8500: case 0x8600: case 0x8700:
                    // 8000 - 87FF - write to tile data table 0
                    tile_data_0[addr - 0x8000] = data;
                    break;
                    
                case 0x8800: case 0x8900: case 0x8A00: case 0x8B00: case 0x8C00: case 0x8D00: case 0x8E00: case 0x8F00:
                    // 8800 - 8FFF - write to tile data table 1
                    tile_data_1[addr - 0x8800] = data;
                    break;
                    
                case 0x9000: case 0x9100: case 0x9200: case 0x9300: case 0x9400: case 0x9500: case 0x9600: case 0x9700:
                    // 9000 - 97FF - write to tile data table 2
                    tile_data_2[addr - 0x9000] = data;
                    break;
                    
                case 0x9800: case 0x9900: case 0x9A00: case 0x9B00:
                    // 9800 - 9BFF - write to tile names table 0
                    tile_names_0[addr - 0x9800] = data;
                    break;
                
                case 0x9C00: case 0x9D00: case 0x9E00: case 0x9F00:
                    // 9C00 - 9FFF - write to tile data table 1
                    tile_names_1[addr - 0x9C00] = data;
                    break;
                default:
                    // Invalid address, gives an error
                    std::cerr << "Attemp to write to invalid address " << gb::Utils::hex_short(addr) << "in VRAM";
            }
        }
        
        uint8_t read(uint16_t addr){
            switch (addr & 0xFF00) {
                case 0x8000: case 0x8100: case 0x8200: case 0x8300: case 0x8400: case 0x8500: case 0x8600: case 0x8700:
                    // 8000 - 87FF - read from tile data table 0
                    return tile_data_0[addr - 0x8000];
                    break;
                case 0x8800: case 0x8900: case 0x8A00: case 0x8B00: case 0x8C00: case 0x8D00: case 0x8E00: case 0x8F00:
                    // 8800 - 8FFF - read from tile data table 1
                    return tile_data_1[addr - 0x8800];
                    break;
                case 0x9000: case 0x9100: case 0x9200: case 0x9300: case 0x9400: case 0x9500: case 0x9600: case 0x9700:
                    // 9000 - 97FF - read from tile data table 2
                    return tile_data_2[addr - 0x9000];
                    break;
                case 0x9800: case 0x9900: case 0x9A00: case 0x9B00:
                    // 9800 - 9BFF - read from tile names table 0
                    return tile_names_0[addr - 0x9800];
                    break;
                case 0x9C00: case 0x9D00: case 0x9E00: case 0x9F00:
                    // 9C00 - 9FFF - read from tile data table 1
                    return tile_names_1[addr - 0x9C00];
                    break;
                default:
                    // Invalid address, gives an error
                    std::cerr << "Attemp to read from invalid address " << gb::Utils::hex_short(addr) << "in VRAM";
                    return 0;
            }
        }
    };
}
#endif /* VRAM_h */
