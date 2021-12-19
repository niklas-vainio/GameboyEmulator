// Created by Niklas on 25/03/2020.
// Stores ram of any size

#ifndef RAM_h
#define RAM_h

#include <iostream>
#include <vector>

namespace gb {
    class ram {
    private:
        // Stores the start and end locations
        uint16_t start_addr, end_addr;
        
        // Stores the size in bytes
        int size;
        
        // Array to store actual data
        std::vector<uint8_t> memory;
    public:
        
        void randomise() {
            for (int i = 0; i < size; i++){
                memory[i] = rand() % 256;
            }
        }
        
        // Constructor
        ram(uint16_t _start_addr, uint16_t _end_addr){
            srand(time(NULL));
            
            start_addr = _start_addr;
            end_addr = _end_addr;
            size = 1 + end_addr - start_addr;
            
            memory.reserve(size);
            
            for (int i = 0; i < size; i ++){
                // Resets all bytes to 0
                memory[i] = 0x00;
                // DEBUG: prints out all bytes
                /*std::cout << gb::Utils::hex_byte(memory[i]) << " ";
                if (i % 16 == 15)
                    std::cout << std::endl;*/
            }
        }
        
        void write(uint16_t addr, uint8_t data){
            memory[addr - start_addr] = data;
        }
        
        uint8_t read(uint16_t addr){
            return memory[addr - start_addr];
        }
    };
}

#endif /* RAM_h */
