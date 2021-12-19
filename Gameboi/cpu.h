// Created by Niklas on 25/03/2020.
// Class for the main Z80-based cpu

#ifndef cpu_h
#define cpu_h

#include "bus.h"
#include "Utils.h"

namespace gb {
    class cpu {
    public:
        // Stores whether interrupt are enabled (IME = interrupt master enable)
        bool IME = true;
        
        // Stores the number of cycles elapsed
        long cycles = 0;
        
        // 8 bit registers A, B, C, D, E, F, H, L
        uint8_t A, B, C, D, E, F, H, L = 0;
        
        // 16 bit reigsters PC and SP, initialised to 0x0100 and 0xFFFE respectively
        uint16_t PC = 0x0000;
        uint16_t SP = 0xFFFE;
        
        // Stores whether the CPU has been stopped or halted by the STOP/HALT instructions
        bool stopped = false; bool halted = false;
        
        // String to store the pneumonic of the current instruction
        std::string current_instr = "";
        
        // Store a reference to the bus
        gb::bus* bus;
        void connect_bus(gb::bus* _bus){
            bus = _bus;
        }
        
        // Functions to return the values of the 16 bit combined registers AF, BC, DE, HL
        uint16_t AF() {return A << 8 | F;}
        uint16_t BC() {return B << 8 | C;}
        uint16_t DE() {return D << 8 | E;}
        uint16_t HL() {return H << 8 | L;}
        
        // Functions to set the values of 16 bit combined registers AF, BC, DE, HL
        void set_AF(uint16_t data) {A = (data & 0xFF00) >> 8; F = (data & 0x00FF);}
        void set_BC(uint16_t data) {B = (data & 0xFF00) >> 8; C = (data & 0x00FF);}
        void set_DE(uint16_t data) {D = (data & 0xFF00) >> 8; E = (data & 0x00FF);}
        void set_HL(uint16_t data) {H = (data & 0xFF00) >> 8; L = (data & 0x00FF);}
        
        // Functions to get, set and reset the flags in the F register
        // Z N H C 0 0 0 0 - Z = zero, N = subtract, H = half-carry, C = carry
        bool get_Z() {return F & 0b10000000;}
        bool get_N() {return F & 0b01000000;}
        bool get_H() {return F & 0b00100000;}
        bool get_C() {return F & 0b00010000;}
        
        void set_Z(bool x) {
            if (x)
                F |= 0b10000000;
            else
                F &= 0b01111111;
        }
        void set_N(bool x) {
            if (x)
                F |= 0b01000000;
            else
                F &= 0b10111111;
        }
        void set_H(bool x) {
            if (x)
                F |= 0b00100000;
            else
                F &= 0b11011111;
        }
        void set_C(bool x) {
            if (x)
                F |= 0b00010000;
            else
                F &= 0b11101111;
        }
        // Functions to read and write to/from the bus
        uint8_t read(uint16_t addr){
            return bus->read(addr);
        }
        void write(uint16_t addr, uint8_t data){
            bus->write(addr, data);
        }
        
        // Runs one fetch decode execute cycle if not stopped or halted
        void run_instruction() {
            // Check for interrupts
            poll_interrupts();
            if (!stopped and !halted){
                opcode = fetch();
                execute();
            } else {
                cycles = 4;
            }
        }
        
        // Declares functions for all instructions - implemented in instructions.cpp
        void ADC(); void ADD(); void ADD_16(); void AND(); void BIT(); void CALL(); void CB(); void CCF();
        void CPL(); void CP(); void DAA(); void DEC(); void DI(); void EI(); void HALT(); void INC(); void JP();
        void JR(); void LD(); void LDD(); void LDH(); void LDI(); void NOP(); void OR(); void POP(); void PUSH();
        void RES(); void RET(); void RETI(); void RLA(); void RLC(); void RLCA(); void RL(); void RRA();
        void RRCA(); void RRC(); void RR(); void RST(); void SBC(); void SCF(); void SET(); void SLA();
        void SRA(); void SRL(); void STOP(); void SUB(); void SWAP(); void XOR(); void XXX();
        
        // Returns the cpu to power up state
        void init(){
            A = 0; B = 0; C = 0; D = 0; E = 0; H = 0; L = 0; F = 0;
            PC = 0x0000;
            SP = 0xFFFE;
        }
        
    private:
        // Stores a table of the return addresses of all the interrupts
        const uint16_t interrupt_vectors[5] = {0x0040, 0x0048, 0x0050, 0x0058, 0x0060};
        
        // Stores the opcode of the last function
        uint8_t opcode;
        
        // Holds a list of registers for easy instruction decoding
        uint8_t* reg_pointers[8] = {&B, &C, &D, &E, &H, &L, &A, &A};
        std::string reg_names[8] = {"B", "C", "D", "E", "H", "L", "(HL)", "A"};
        
        // Same but for 16 bit registers (stored as function pointers)
        uint16_t (gb::cpu::* r_reg_pointers_16[4])() = {&gb::cpu::BC, &gb::cpu::DE, &gb::cpu::HL, &gb::cpu::AF};
        void (gb::cpu::* w_reg_pointers_16[4])(uint16_t data) = {&gb::cpu::set_BC, &gb::cpu::set_DE, &gb::cpu::set_HL, &gb::cpu::set_AF};
        std::string reg_names_16[4] = {"BC", "DE", "HL", "AF"};
        
        // Struct to store instruction data
        struct Instruction{
            int cycles;
            void (gb::cpu::* func_ptr)() ;
        };

        // List of all instruction data stored as Instruction structs
        Instruction lookup[256] = {{4, &gb::cpu::NOP}, {12, &gb::cpu::LD}, {8, &gb::cpu::LD}, {8, &gb::cpu::INC}, {4, &gb::cpu::INC}, {4, &gb::cpu::DEC}, {8, &gb::cpu::LD}, {4, &gb::cpu::RLCA}, {20, &gb::cpu::LD}, {8, &gb::cpu::ADD_16}, {8, &gb::cpu::LD}, {8, &gb::cpu::DEC}, {4, &gb::cpu::INC}, {4, &gb::cpu::DEC}, {8, &gb::cpu::LD}, {4, &gb::cpu::RRCA},
            {4, &gb::cpu::STOP}, {12, &gb::cpu::LD}, {8, &gb::cpu::LD}, {8, &gb::cpu::INC}, {4, &gb::cpu::INC}, {4, &gb::cpu::DEC}, {8, &gb::cpu::LD}, {4, &gb::cpu::RLA}, {12, &gb::cpu::JR}, {8, &gb::cpu::ADD_16}, {8, &gb::cpu::LD}, {8, &gb::cpu::DEC}, {4, &gb::cpu::INC}, {4, &gb::cpu::DEC}, {8, &gb::cpu::LD}, {4, &gb::cpu::RRA},
            {8, &gb::cpu::JR}, {12, &gb::cpu::LD}, {8, &gb::cpu::LDI}, {8, &gb::cpu::INC}, {4, &gb::cpu::INC}, {4, &gb::cpu::DEC}, {8, &gb::cpu::LD}, {4, &gb::cpu::DAA}, {8, &gb::cpu::JR}, {8, &gb::cpu::ADD_16}, {8, &gb::cpu::LDI}, {8, &gb::cpu::DEC}, {4, &gb::cpu::INC}, {4, &gb::cpu::DEC}, {8, &gb::cpu::LD}, {4, &gb::cpu::CPL},
            {8, &gb::cpu::JR}, {12, &gb::cpu::LD}, {8, &gb::cpu::LDD}, {8, &gb::cpu::INC}, {12, &gb::cpu::INC}, {12, &gb::cpu::DEC}, {12, &gb::cpu::LD}, {4, &gb::cpu::SCF}, {8, &gb::cpu::JR}, {8, &gb::cpu::ADD_16}, {8, &gb::cpu::LDD}, {8, &gb::cpu::DEC}, {4, &gb::cpu::INC}, {4, &gb::cpu::DEC}, {8, &gb::cpu::LD}, {4, &gb::cpu::CCF},
            {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {8, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {8, &gb::cpu::LD}, {4, &gb::cpu::LD},
            {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {8, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {8, &gb::cpu::LD}, {4, &gb::cpu::LD},
            {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {8, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {8, &gb::cpu::LD}, {4, &gb::cpu::LD},
            {8, &gb::cpu::LD}, {8, &gb::cpu::LD}, {8, &gb::cpu::LD}, {8, &gb::cpu::LD}, {8, &gb::cpu::LD}, {8, &gb::cpu::LD}, {4, &gb::cpu::HALT}, {8, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {4, &gb::cpu::LD}, {8, &gb::cpu::LD}, {4, &gb::cpu::LD},
            {4, &gb::cpu::ADD}, {4, &gb::cpu::ADD}, {4, &gb::cpu::ADD}, {4, &gb::cpu::ADD}, {4, &gb::cpu::ADD}, {4, &gb::cpu::ADD}, {8, &gb::cpu::ADD}, {4, &gb::cpu::ADD}, {4, &gb::cpu::ADC}, {4, &gb::cpu::ADC}, {4, &gb::cpu::ADC}, {4, &gb::cpu::ADC}, {4, &gb::cpu::ADC}, {4, &gb::cpu::ADC}, {8, &gb::cpu::ADC}, {4, &gb::cpu::ADC},
            {4, &gb::cpu::SUB}, {4, &gb::cpu::SUB}, {4, &gb::cpu::SUB}, {4, &gb::cpu::SUB}, {4, &gb::cpu::SUB}, {4, &gb::cpu::SUB}, {8, &gb::cpu::SUB}, {4, &gb::cpu::SUB}, {4, &gb::cpu::SBC}, {4, &gb::cpu::SBC}, {4, &gb::cpu::SBC}, {4, &gb::cpu::SBC}, {4, &gb::cpu::SBC}, {4, &gb::cpu::SBC}, {8, &gb::cpu::SBC}, {4, &gb::cpu::SBC},
            {4, &gb::cpu::AND}, {4, &gb::cpu::AND}, {4, &gb::cpu::AND}, {4, &gb::cpu::AND}, {4, &gb::cpu::AND}, {4, &gb::cpu::AND}, {8, &gb::cpu::AND}, {4, &gb::cpu::AND}, {4, &gb::cpu::XOR}, {4, &gb::cpu::XOR}, {4, &gb::cpu::XOR}, {4, &gb::cpu::XOR}, {4, &gb::cpu::XOR}, {4, &gb::cpu::XOR}, {8, &gb::cpu::XOR}, {4, &gb::cpu::XOR},
            {4, &gb::cpu::OR}, {4, &gb::cpu::OR}, {4, &gb::cpu::OR}, {4, &gb::cpu::OR}, {4, &gb::cpu::OR}, {4, &gb::cpu::OR}, {8, &gb::cpu::OR}, {4, &gb::cpu::OR}, {4, &gb::cpu::CP}, {4, &gb::cpu::CP}, {4, &gb::cpu::CP}, {4, &gb::cpu::CP}, {4, &gb::cpu::CP}, {4, &gb::cpu::CP}, {8, &gb::cpu::CP}, {4, &gb::cpu::CP},
            {8, &gb::cpu::RET}, {12, &gb::cpu::POP}, {12, &gb::cpu::JP}, {16, &gb::cpu::JP}, {12, &gb::cpu::CALL}, {16, &gb::cpu::PUSH}, {8, &gb::cpu::ADD}, {16, &gb::cpu::RST}, {8, &gb::cpu::RET}, {16, &gb::cpu::RET}, {12, &gb::cpu::JP}, {4, &gb::cpu::CB}, {12, &gb::cpu::CALL}, {24, &gb::cpu::CALL}, {8, &gb::cpu::ADC}, {16, &gb::cpu::RST},
            {8, &gb::cpu::RET}, {12, &gb::cpu::POP}, {12, &gb::cpu::JP}, {4, &gb::cpu::XXX}, {12, &gb::cpu::CALL}, {16, &gb::cpu::PUSH}, {8, &gb::cpu::SUB}, {16, &gb::cpu::RST}, {8, &gb::cpu::RET}, {16, &gb::cpu::RETI}, {12, &gb::cpu::JP}, {4, &gb::cpu::XXX}, {12, &gb::cpu::CALL}, {4, &gb::cpu::XXX}, {8, &gb::cpu::SBC}, {16, &gb::cpu::RST},
            {12, &gb::cpu::LDH}, {12, &gb::cpu::POP}, {8, &gb::cpu::LD}, {4, &gb::cpu::XXX}, {4, &gb::cpu::XXX}, {16, &gb::cpu::PUSH}, {8, &gb::cpu::AND}, {16, &gb::cpu::RST}, {16, &gb::cpu::ADD_16}, {4, &gb::cpu::JP}, {16, &gb::cpu::LD}, {4, &gb::cpu::XXX}, {4, &gb::cpu::XXX}, {4, &gb::cpu::XXX}, {8, &gb::cpu::XOR}, {16, &gb::cpu::RST},
            {12, &gb::cpu::LDH}, {12, &gb::cpu::POP}, {8, &gb::cpu::LD}, {4, &gb::cpu::DI}, {4, &gb::cpu::XXX}, {16, &gb::cpu::PUSH}, {8, &gb::cpu::OR}, {16, &gb::cpu::RST}, {12, &gb::cpu::LD}, {8, &gb::cpu::LD}, {16, &gb::cpu::LD}, {4, &gb::cpu::EI}, {4, &gb::cpu::XXX}, {4, &gb::cpu::XXX}, {8, &gb::cpu::CP}, {16, &gb::cpu::RST}};
        
        // List of all instruction data for opcodes starting in 0xCB (see CB() in instructions.cpp)
        Instruction cb_lookup[256] = {{8, &gb::cpu::RLC}, {8, &gb::cpu::RLC}, {8, &gb::cpu::RLC}, {8, &gb::cpu::RLC}, {8, &gb::cpu::RLC}, {8, &gb::cpu::RLC}, {16, &gb::cpu::RLC}, {8, &gb::cpu::RLC}, {8, &gb::cpu::RRC}, {8, &gb::cpu::RRC}, {8, &gb::cpu::RRC}, {8, &gb::cpu::RRC}, {8, &gb::cpu::RRC}, {8, &gb::cpu::RRC}, {16, &gb::cpu::RRC}, {8, &gb::cpu::RRC},
            {8, &gb::cpu::RL}, {8, &gb::cpu::RL}, {8, &gb::cpu::RL}, {8, &gb::cpu::RL}, {8, &gb::cpu::RL}, {8, &gb::cpu::RL}, {16, &gb::cpu::RL}, {8, &gb::cpu::RL}, {8, &gb::cpu::RR}, {8, &gb::cpu::RR}, {8, &gb::cpu::RR}, {8, &gb::cpu::RR}, {8, &gb::cpu::RR}, {8, &gb::cpu::RR}, {16, &gb::cpu::RR}, {8, &gb::cpu::RR},
            {8, &gb::cpu::SLA}, {8, &gb::cpu::SLA}, {8, &gb::cpu::SLA}, {8, &gb::cpu::SLA}, {8, &gb::cpu::SLA}, {8, &gb::cpu::SLA}, {16, &gb::cpu::SLA}, {8, &gb::cpu::SLA}, {8, &gb::cpu::SRA}, {8, &gb::cpu::SRA}, {8, &gb::cpu::SRA}, {8, &gb::cpu::SRA}, {8, &gb::cpu::SRA}, {8, &gb::cpu::SRA}, {16, &gb::cpu::SRA}, {8, &gb::cpu::SRA},
            {8, &gb::cpu::SWAP}, {8, &gb::cpu::SWAP}, {8, &gb::cpu::SWAP}, {8, &gb::cpu::SWAP}, {8, &gb::cpu::SWAP}, {8, &gb::cpu::SWAP}, {16, &gb::cpu::SWAP}, {8, &gb::cpu::SWAP}, {8, &gb::cpu::SRL}, {8, &gb::cpu::SRL}, {8, &gb::cpu::SRL}, {8, &gb::cpu::SRL}, {8, &gb::cpu::SRL}, {8, &gb::cpu::SRL}, {16, &gb::cpu::SRL}, {8, &gb::cpu::SRL},
            {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {16, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {16, &gb::cpu::BIT}, {8, &gb::cpu::BIT},
            {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {16, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {16, &gb::cpu::BIT}, {8, &gb::cpu::BIT},
            {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {16, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {16, &gb::cpu::BIT}, {8, &gb::cpu::BIT},
            {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {16, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {8, &gb::cpu::BIT}, {16, &gb::cpu::BIT}, {8, &gb::cpu::BIT},
            {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {16, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {16, &gb::cpu::RES}, {8, &gb::cpu::RES},
            {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {16, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {16, &gb::cpu::RES}, {8, &gb::cpu::RES},
            {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {16, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {16, &gb::cpu::RES}, {8, &gb::cpu::RES},
            {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {16, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {8, &gb::cpu::RES}, {16, &gb::cpu::RES}, {8, &gb::cpu::RES},
            {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {16, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {16, &gb::cpu::SET}, {8, &gb::cpu::SET},
            {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {16, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {16, &gb::cpu::SET}, {8, &gb::cpu::SET},
            {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {16, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {16, &gb::cpu::SET}, {8, &gb::cpu::SET},
            {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {16, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {8, &gb::cpu::SET}, {16, &gb::cpu::SET}, {8, &gb::cpu::SET}};
        
        // CPU follows a 3 step instruction process
        // Fetch, decode, execute
        // Fetch and decode are handled in these main functions
        // Execute is handled by calling supplementary functions for each processor instruction in the decode function
        uint8_t fetch(){
            uint8_t output = bus->read(PC);
            PC++;
            return output;
        }

        void execute() {
            // Exectes instruction based on opcode
            cycles = 0;
            Instruction instr = lookup[opcode];
            cycles += instr.cycles;
            (this->*(instr.func_ptr))();
        }
        
        // Function to check for interrupts and perform an ISR if an interrupt occurs
        void poll_interrupts(){
            // Gets the bitwise and of the interrupt flag register (FF0F) and interrupt enable register (FFFF)
            uint8_t interrupts = read(0xFF0F) & read(0xFFFF);
            
            // Return if no interrupts
            if (interrupts == 0)
                return;
            
            // If it is not 0 (ie there is an interrupt) exit from hated state regardless of IME
            halted  = false;
            
            if (IME) {
                // If there is an interrupt and IME is enabled, reset IME
                IME = false;

                // Push PC onto stack
                write(SP - 1, (PC & 0xFF00) >> 8);
                write(SP - 2, PC & 0x00FF);
                SP -= 2;
                
                // Adds 20 cycles
                cycles += 20;
                // Checks interrupt data from bit 4 to bit 0
                for (int i = 4; i >= 0; i--){
                    bool do_interrupt = gb::Utils::get_bit(interrupts, i);
                    if (do_interrupt){
                        // If the correspoding interrupt is set, reset the IF flag, jump to its vector and exit the polling routine
                        PC = interrupt_vectors[i];
                        write(0xFF0F, gb::Utils::set_bit(read(0xFF0F), i, 0));
                        return;
                    }
                }
            }
            
        }
    };
}


#endif /* cpu_h */
