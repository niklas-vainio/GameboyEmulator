// Created by Niklas on 09/04/2020.
// Class which controls the whole emulation
// Methods are called by the main.cpp file to simplify code there

#ifndef EmulationController_h
#define EmulationController_h

#include "bus.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"

class EmulationController {
private:
    // Stores pointers to all the hardware components (cpu, bus, ppu etc)
    gb::cpu* cpu;
    gb::bus* bus;
    gb::ppu* ppu;
    gb::apu* apu;
    
    // Stores the path to thr roms and saves folder
    std::string romspath;
    std::string savespath;

public:
    // Constructor takes in pointers to the hardware components as well as roms and saves folders and stores them
    EmulationController(gb::cpu* _cpu, gb::bus* _bus, gb::ppu* _ppu, gb::apu* _apu, std::string _romspath, std::string _savespath) {
        cpu = _cpu;
        bus = _bus;
        ppu = _ppu;
        apu = _apu;

        romspath = _romspath;
        savespath = _savespath;
    }
    
    // Function to initialise the emulation
    void init(std::string bios_path) {
        cpu->connect_bus(bus);
        ppu->connect_bus(bus);
        apu->connect_bus(bus);
        
        cpu->init();
        ppu->init();
        apu->init();
        
        bus->load_bios(bios_path);
    }
    
    // Function to load a rom
    void load_rom(std::string rom_name) {
        bus->load_rom_file(romspath + rom_name + ".gb", savespath + rom_name + "_save.bin");
    }
    
    // Function to emulate one instruction - returns the number of cpu cycles
    long emulate_instruction(){
        cpu->run_instruction();
        for (int i = 0; i < (cpu->cycles / 4); i++) {
            ppu->do_cycle();
            bus->do_cycle();
            apu->do_cycle();
        }
        
        return cpu->cycles;
    }
    
    // Function to emulate one scanline
    void emulate_scanline(){
        while (!ppu->scanline_over) {
            emulate_instruction();
        }
        
        ppu->scanline_over = false;
        return false;
    }
    
    // Function to emulate one frame - returning true pauses emulation for breakpoints
    bool emulate_frame(){
        while (!ppu->frame_over) {
            emulate_instruction();
        }
        
        ppu->frame_over = false;
        return false;
    }

};


#endif /* EmulationController_h */
