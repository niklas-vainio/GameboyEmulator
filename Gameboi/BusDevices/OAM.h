// Created by Niklas on 07/04/2020.
// Class to store the sprite attribute table, with information about the sprites
// Addressed from FE00 - FE9F

#ifndef OAM_h
#define OAM_h

#include <stdint.h>

namespace gb {
    class oam {
    private:
        // Array of 160 bytes to store data
        uint8_t sprite_data[0xA0];
        
    public:
        // Constructor
        oam(){
        }
        
        // Functions to read and write data
        uint8_t read(uint16_t addr){
            return sprite_data[addr - 0xFE00];
        }
        
        void write(uint16_t addr, uint8_t data){
            sprite_data[addr - 0xFE00] = data;
        }
    };
}

#endif /* OAM_h */
