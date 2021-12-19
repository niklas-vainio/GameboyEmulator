// Created by Niklas on 31/03/2020.
// Stores the Gameboy's IO data such as sound and LCD control
// Addressed from FF00 - FF4B

#ifndef IO_Regs_h
#define IO_Regs_h

#include "Utils.h"
#include "RegNames.h"

namespace gb {
    class io_regs {
    private:
        // Creates an array to store register states
        uint8_t reg_data[0x4C];
        
        // Stores the states of all buttons
        bool a, b, up, down, left, right, start, select = false;
        
        // Functon to return the state of the the P1 register (FF00) which stores which buttons are pressed
        uint8_t P1(){
            if (gb::Utils::get_bit(reg_data[0], 4) == 0){
                // If bit 4 is a 0, then bits 0-4 are set to the states of right, left, up, down (0 = pressed, 1 = released)
                uint8_t data = reg_data[0];
                uint8_t mask = (down << 3) | (up << 2) | (left << 1) | right;
                return (data | 0b00001111) & ~mask;
            } else if (gb::Utils::get_bit(reg_data[0], 5) == 0){
                // If bit 5 is a 0, then bits 0-4 are set to the states of start, select, B and A (0 = pressed, 1 = released)
                uint8_t data = reg_data[0];
                uint8_t mask = (start << 3) | (select << 2) | (b << 1) | a;
                return (data | 0b00001111) & ~mask;
            }
            // If neither of those conditions are met (one should be), then the existing contents of FF00 are returned
            return reg_data[0];
        }
        
    public:
        // Constructor
        io_regs(){
        }
        
        // Flag indicting whether the bus should perform OAM DMA
        bool do_dma = false;
        
        // Function to store key states fed by the bus
        void store_key_states(bool _a, bool _b, bool _up, bool _down, bool _left, bool _right, bool _start, bool _select){
            a = _a; b = _b;
            up = _up; down = _down; left = _left; right = _right;
            start = _start; select = _select;
        }
        
        // Fucntion to increment the value in the DIV register - should run every 256 cycles
        void increment_div(){
            reg_data[4] = reg_data[4] + 1;
        }
        
        // Function to increment the value in the TIMA register - returns true if it overflows
        bool increment_timer(){
            reg_data[5] = reg_data[5] + 1;
            return reg_data[5] == 0;
        }
        
        // Functions to read and write to the registers
        void write(uint16_t addr, uint8_t data){
            // Writes if address is between FF00 and FF4B, otherwise does nothing
            if ((addr >= 0xFF00) and (addr <= 0xFF4B)){
                // Writing to address FF46 initiates OAM DMA
                if (addr == 0xFF46)
                    do_dma = true;
                
                // Writing to DIV (FF04) resets it to 0
                if (addr == 0xFF04){
                    reg_data[0x04] = 0;
                    return;
                }
                    
                reg_data[addr - 0xFF00] = data;
            }
        }
        
        uint8_t read(uint16_t addr){
            // Reads if address is between FF00 and FF4B, otherwise does nothing
            if ((addr >= 0xFF00) and (addr <= 0xFF4B)){
                // If address = FF00 calls the function to return the value of P1
                if (addr == 0xFF00){
                    return P1();
                }
                
                // Otherwise, simply read from a register
                return reg_data[addr - 0xFF00];
            } else
                return 0;
        }
        
        // Function to get an item by its register ID for use in other devices eg PPU
        uint8_t get(uint8_t reg_num){
            return reg_data[reg_num];
        }
    };
}

#endif /* IO_Regs_h */
