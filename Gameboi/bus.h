// Created by Niklas on 25/03/2020.
// Class for the bus to handle memory mapping and reading and writing data

#ifndef bus_h
#define bus_h

#include "Cart_ROM.h"
#include "RAM.h"
#include "VRAM.h"
#include "IO_Regs.h"
#include "OAM.h"

#include "MBC_Base.h"
#include "ROM_Only.h"
#include "MBC1.h"
#include "MBC3.h"

#include <iostream>

namespace gb{
    class bus {
    private:
        // Handles cpu reading and writing to devices
        // This class allows devices to be connected to specific memory addresses, according to the gameboy's memory map
        // 64 kB of accessible range (0x0000 - 0xFFFF)
        //
        // Main memory map:
        // 0000 - 3FFF - cartridge bank 0 (16kB)
        // 4000 - 7FFF - swappable cartridge bank (16kB)
        
        // 8000 - 9FFF - Video Ram (VRAM) (8kB)
        
        // A000 - BFFF - Switchable RAM Bank in cartridge (8kB)
        
        // C000 - DFFF - Internal RAM (WRAM) (8kB)
        // E000 - FDFF - Echo of WRAM (8kB)
        
        // FE00 - FE9F - Sprite Attribute Memory (OAM) (160B)
        
        // FEA0 - FEFF - Empty Space (96B)
        
        // FF00 - FF4B - IO Ports (75B)
        // FF4C - FF7F - Empty Space (181B)
        
        // FF80 - FFFE - Internal RAM (HRAM) (127B)
        // FFFF        - Interrupt Enable register
        
        // Stores the 256-byte bios
        uint8_t bios[256];
        
        // Keeps track of the number of cycles since last incrementing DIV and TIMA
        int cycles_since_div_increment = 0;
        int cycles_since_timer_increment = 0;
        
        // Table which maps the values in TAC bits 1-0 to durations between timer increments
        const int timer_increment_durations[4] = {1024, 16, 64, 256};
        
        // Creates objects to store the devices
        gb::cartridge_rom* cart_rom = new gb::cartridge_rom(); // Cartridge ROM
        gb::MBC_Base* mapper; // Mapper from 0x0000 - 0x7FFF and 0xA000 - 0xBFFF

        gb::vram* v_ram = new gb::vram(); // VRAM from 8000 - 9FFF
        gb::ram* work_ram = new gb::ram(0xC000, 0xDFFF); // WRAM from C000 - DFFF
        gb::ram* h_ram = new gb::ram(0xFF00, 0xFFFE); // HRAM from FF00 - FFFE
        gb::oam* oam = new gb::oam(); // OAM from FE00 - FE9F
        gb::io_regs* io_ports = new gb::io_regs(); // IO Ports from FF00 - FF4B
        
        uint8_t IE = 0; // Interrupt enable register at FFFF
        
        // Function to perform OAM DMA
        void do_OAM_DMA() {
            // Gets the starting address from the DMA register
            uint16_t start_addr = read(0xFF46) << 8;

            // Copies the next 160 bytes (A0) into OAM starting at FE00
            for (int i = 0; i < 160; i++){
                write(0xFE00 + i, read(start_addr + i));
            }
        }

    public:
        // Stores the ID of the mapper in use and the various mapper names
        uint8_t mapper_id;
        const std::string mapper_names[0x14] = {
            "ROM Only", "MBC1", "MBC1+RAM", "MBC1+RAM+BATT", "???", "MBC2", "MBC2+BATT", "???",
            "ROM+RAM", "ROM+RAM+BATT", "???", "MM01", "MM01+RAM", "MM01+RAM+BAT", "???", "MBC3+TIMER+BATT",
            "MBC3+TIMER+RAM+BATT","MBC3", "MBC3+RAM", "MBC3+RAM+BATTERY"
        };
        
        // Stores whether the bios is enabled
        bool bios_enabled = true;
        
        // Flags which signal that the APU needs to update audio settings
        bool update_apu = true;
        bool update_square1_envelope = false;
        bool update_square2_envelope = false;
        bool update_noise_envelope = false;
        
        // Function to do one cycle, used for incrementing timers
        void do_cycle() {
            cycles_since_div_increment += 4;
            cycles_since_timer_increment += 4;
            
            // If more than 256 cycles have elapsed since DIV was incrmented, increment DIV and reset count
            if (cycles_since_div_increment >= 256) {
                io_ports->increment_div();
                cycles_since_div_increment = 0;
            }
            
            // Gets how many cycles should have elapsed since last timer increment
            int next_increment = timer_increment_durations[get_ioreg(gb::regNames::TAC) & 0b00000011];
            bool timer_enabled = gb::Utils::get_bit(get_ioreg(gb::regNames::TAC), 2);
            
            // If more than this many cycles have elapsed, and timer is enabled, increment timer and reset count
            if (cycles_since_timer_increment >= next_increment and timer_enabled) {
                bool overflow = io_ports->increment_timer();
                cycles_since_timer_increment = 0;
                
                // If there was an overflow, load the value of TMA into TIMA and trigger a timer overflow interurpt
                if (overflow) {
                    write(0xFF00 + gb::regNames::TIMA, get_ioreg(gb::regNames::TMA));
                    write(0xFF0F, read(0xFF0F) | 0b00000100);
                }
            }
        }
        
        // Function which instructs the mapper to save the contents of non-volatile memory, runs when emulator exits
        void close() {
            mapper->close();
        }
        
        // Function to load the bios into memory
        void load_bios(std::string filepath){
            std::ifstream bios_file;
            bios_file.open(filepath);
            // Give an error if file not found
            if (!bios_file){
                std::cerr << "BIOS file not found!" << std::endl;
                return;
            }
            for (int i = 0; i < 256; i++)
                bios[i] = bios_file.get();
            bios_file.close();
        }
        
        // Function to load a rom file into the cartridge rom
        void load_rom_file(std::string filepath, std::string saves_path){
            cart_rom->load_from_file(filepath);
            
            // Gets the mapper type from the cartridge
            mapper_id = cart_rom->get_mapper_type();

            // Creates the mapper based on the mapper ID
            switch (mapper_id) {
                case 0x00:
                    // ID 00 - ROM only
                    mapper = new ROM_only(cart_rom, false, false);
                    break;
                case 0x01:
                    // ID 01 - MBC1 (no RAM or battery)
                    mapper = new MBC1(cart_rom, false, false);
                    break;
                case 0x02:
                    // ID 02 - MBC1 (RAM but no battery)
                    mapper = new MBC1(cart_rom, true, false);
                    break;
                case 0x03:
                    // ID 03 - MBC1 (RAM and battery)
                    mapper = new MBC1(cart_rom, true, true);
                    break;
                case 0x11:
                    // ID 11 - MBC3 (no RAM or battery)
                    mapper = new MBC3(cart_rom, false, false);
                    break;
                case 0x12:
                    // ID 12 - MBC3 (RAM but no battery)
                    mapper = new MBC3(cart_rom, true, false);
                    break;
                case 0x13:
                    // ID 13 - MBC3 (RAM and battery)
                    mapper = new MBC3(cart_rom, true, true);
                    break;
                default:
                    // If the ID is invalid, gives an error
                    std::cerr << "Mapper ID " << gb::Utils::hex_byte(mapper_id) << " is not supported!" << std::endl;
                    exit(EXIT_FAILURE);
                    break;
            }
            
            // Initialise the mapper
            mapper->set_saves_path(saves_path);
            mapper->init();
        }
        
        // Takes key states from main.cpp and stores them in the io registers
        void store_key_states(bool _a, bool _b, bool _up, bool _down, bool _left, bool _right, bool _start, bool _select){
            io_ports->store_key_states(_a, _b, _up, _down, _left, _right, _start, _select);
        }
        
        // Function to write to any memory address
        void write(uint16_t addr, uint8_t data){
            //if (addr == 0xFF01)
                //std::cout << (char)data;
            // If 01 is written to FF50, disable the bios
            if (addr == 0xFF50 and data == 0x01)
                bios_enabled = false;
            
            // Writes to the correct device
            switch (addr & 0xF000) {
                case 0x0000: case 0x1000: case 0x2000: case 0x3000: case 0x4000: case 0x5000: case 0x6000: case 0x7000:
                    // Mapper rom area from 0000 - 7FFFF
                    mapper->write_rom(addr, data);
                    break;
                case 0x8000: case 0x9000:
                    // VRAM from 8000 - 9FFF
                    v_ram->write(addr, data);
                    break;
                case 0xA000: case 0xB000:
                    // Mapper ram area from A000 - BFFF
                    mapper->write_ram(addr, data);
                    break;
                case 0xC000: case 0xD000:
                    // WRAM from C000 - DFFF
                    work_ram->write(addr, data);
                    break;
                case 0xE000:
                    // WRAM mirror from E000 - EFFF
                    work_ram->write(addr - 0x2000, data);
                    break;
                case 0xF000:
                    switch (addr & 0x0F00) {
                        case 0x000: case 0x100: case 0x200: case 0x300: case 0x400: case 0x500: case 0x600: case 0x700:
                        case 0x800: case 0x900: case 0xA00: case 0xB00: case 0xC00: case 0xD00:
                            // WRAM mirror continued from F000 - FDFF
                            work_ram->write(addr - 0x2000, data);
                            break;
                        case 0xE00:
                            // OAM / empty space from FE00 - FEFF
                            if (addr < 0xFEA0)
                                oam->write(addr, data);
                            break;
                        case 0xF00:
                            // IO registers, hram and interrupt enable register FF00 - FFFF
                            if (addr == 0xFFFF)
                                // IE register at FFFF
                                IE = data;
                            else if (addr >= 0xFF80)
                                // HRAM from FF80 - FFFE
                                h_ram->write(addr, data);
                            else
                                // IO ports from FF00 - FF80
                                io_ports->write(addr, data);
                            break;
                    }
                    break;
            }
            
            // If OAM DMA has been enabled, execute it
            if (io_ports->do_dma){
                io_ports->do_dma = false;
                do_OAM_DMA();
            }
            
            // If address is between FF10 and FF3F, sets the flag for the APU to update
            if (addr >= 0xFF10 and addr < 0xFF40) update_apu = true;
            
            // If address is FF12, sets the flag for the APU to update square wave 1 envelope
            if (addr == 0xFF12) update_square1_envelope = true;
            
            // If address is FF17, sets the flag for the APU to update square wave 2 envelope
            if (addr == 0xFF17) update_square2_envelope = true;
            
            // If address is FF42, sets the flag for the APU to update noise envelope
            if (addr == 0xFF42) update_noise_envelope = true;
        }
        
        // Function to get an io register - faster than reading
        uint8_t get_ioreg(uint8_t reg_num){
            return io_ports->get(reg_num);
        }
        
        // Function to read from any memory address
        uint8_t read(uint16_t addr){
            // If bios is enabled and address is less than 0100 the read from bios
            if (addr < 0x0100 and bios_enabled)
                return bios[addr];

            // Reads from the correct device
            switch (addr & 0xF000) {
                case 0x0000: case 0x1000: case 0x2000: case 0x3000: case 0x4000: case 0x5000: case 0x6000: case 0x7000:
                    // Mapper rom area from 0000 - 7FFFF
                    return mapper->read_rom(addr);
                case 0x8000: case 0x9000:
                    // VRAM from 8000 - 9FFF
                   return  v_ram->read(addr);
                case 0xA000: case 0xB000:
                    // Mapper ram from A000 - BFFF
                    return mapper->read_ram(addr);
                case 0xC000: case 0xD000:
                    // WRAM from C000 - DFFF
                    return work_ram->read(addr);
                case 0xE000:
                    // WRAM mirror from E000 - EFFF
                    return work_ram->read(addr - 0x2000);
                case 0xF000:
                    switch (addr & 0x0F00) {
                        case 0x000: case 0x100: case 0x200: case 0x300: case 0x400: case 0x500: case 0x600: case 0x700:
                        case 0x800: case 0x900: case 0xA00: case 0xB00: case 0xC00: case 0xD00:
                            // WRAM mirror continued from F000 - FDFF
                            return work_ram->read(addr - 0x2000);
                        case 0xE00:
                            // OAM / empty space from FE00 - FEFF
                            if (addr < 0xFEA0)
                                return oam->read(addr);
                            return 0;
                        case 0xF00:
                            // IO registers, hram and interrupt enable register FF00 - FFFF
                            if (addr == 0xFFFF)
                                // IE register at FFFF
                                return IE;
                            else if (addr >= 0xFF80)
                                // HRAM from FF80 - FFFE
                                return h_ram->read(addr);
                            else
                                // IO ports from FF00 - FF80
                                return io_ports->read(addr);
                    }
            }
        }
        
        // Constructor
        bus(){
            // Writes 91 to LCDC at startup
            write(0xFF40, 0x91);
        }
        
        // Destructor
        ~bus(){
            delete cart_rom;
            delete mapper;
            delete work_ram;
            delete v_ram;
            delete h_ram;
            delete oam;
            delete io_ports;
        }
    };
}

#endif /* bus_h */
