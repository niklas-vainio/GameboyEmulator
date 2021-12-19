// Created by Niklas on 26/03/2020.
// Implements CPU Instructions

#include "cpu.h"
#include "Utils.h"
#include <iostream>

void gb::cpu::CB(){
    // Executes instructions with the opcode prefix CB
    opcode = read(PC); PC++;
    Instruction instr = cb_lookup[opcode];
    cycles += instr.cycles;
    (this->*instr.func_ptr)();
}

void gb::cpu::ADC(){
    // ADC - Adds carry bit + register/number to A
    uint8_t old_A = A;
    uint8_t n;
    switch (opcode) {
        case 0xCE:
            // Use immediate value
            n = read(PC); PC++;
            current_instr = "ADC A, #" + gb::Utils::hex_byte(n);
            break;
        case 0x8E:
            // Use (HL)
            n = read(HL()); current_instr = "ADC A, (HL)";
            break;
        case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8F:
            // Use other reigster
            n = *reg_pointers[opcode & 0x07];
            current_instr = "ADC A, " + reg_names[opcode & 0x07];
            break;
        default:
            break;
    }
    
    A = A + n + get_C();
    // Set Z if A == 0, reset N, set H if cary from bit 3, set C if carry
    set_Z(A == 0); set_N(0);
    set_H((old_A & 0xF) + (n & 0xF) + get_C() >= 0x10);
    set_C((old_A + n + get_C()) >= 0x100);
}

void gb::cpu::ADD_16(){
    // Handles 16-bit additions
    if (opcode == 0xE8){
        // ADD SP, n
        uint16_t old_SP = SP;
        uint8_t n = read(PC); PC++;
        SP += n;
        if (n > 0x7F) SP -= 0x100;
        current_instr = "ADD SP, #" + gb::Utils::hex_byte(n);
        
        
        // Resets Z and N, sets H and C if there is a half/full carry in lower byte
        set_Z(0); set_N(0);
        set_H(((old_SP & 0xF) + (n & 0x0F)) >= 0x10);
        set_C(((old_SP & 0xFF) + n) >= 0x100);
    } else {
        // Adds a 16 bit register to HL
        uint16_t nn;
        switch (opcode) {
            case 0x09:
                // ADD HL, BC
                nn = BC(); current_instr = "ADD HL, BC";
                break;
            case 0x19:
                // ADD HL, DE
                nn = DE(); current_instr = "ADD HL, DE";
                break;
            case 0x29:
                // ADD HL, HL
                nn = HL(); current_instr = "ADD HL, HL";
                break;
            case 0x39:
                // ADD HL, SP
                nn = SP; current_instr = "ADD HL, SP";
                break;
            default:
                break;
        }
        
        // Resets N, sets H if carry from bit 11, sets C if carry from bit 15
        set_N(0);
        set_H((nn & 0x0FFF) + (HL() & 0x0FFF) >= 0x1000);
        set_C(nn + HL() > 0xFFFF);
        set_HL(HL() + nn);
    }
}

void gb::cpu::ADD(){
    // ADD - Adds register/number to A
    uint8_t old_A = A;
    uint8_t n;
    switch (opcode) {
        case 0xC6:
            // Use immediate value
            n = read(PC); PC++;
            current_instr = "ADD A, #" + gb::Utils::hex_byte(n);
            break;
        case 0x86:
            // Use (HL)
            n = read(HL()); current_instr = "ADD A, (HL)";
            break;
        case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x87:
            // Use other reigster
            n = *reg_pointers[opcode & 0x07];
            current_instr = "ADD A, " + reg_names[opcode & 0x07];
            break;
        default:
            break;
    }
    
    A += n;
    // Set Z if A == 0, reset N, set H if cary from bit 3, set C if carry
    set_Z(A == 0); set_N(0);
    set_H((old_A & 0xF) + (n & 0xF) >= 0x10);
    set_C(old_A + n >= 0x100);
}

void gb::cpu::AND(){
    // AND - Sets A to A and the target register
    switch (opcode) {
        case 0xE6: {
            // Use immediate value
            uint8_t value = read(PC);
            A &= value; PC ++;
            current_instr = "AND #" + gb::Utils::hex_byte(value);
            break;
        }
        case 0xA6:
            // Use (HL)
            A &= read(HL());
            current_instr = "AND (HL)";
            break;
        case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA7:
            // Use reg from regpointers
            A &= *reg_pointers[opcode & 0x07];
            current_instr = "AND " + reg_names[opcode & 0x07];
            break;
        default:
            break;
    }
    
    // Z set if A == 0, N reset, H set, C reset
    set_Z(A == 0); set_N(0); set_H(1); set_C(0);
}

void gb::cpu::BIT(){
    // BIT - test specific bit in target register and set Z flag accordingly
    int bit_num = (opcode & 0b00111000) >> 3;
    uint8_t data;
    if ((opcode & 0x07) == 6){
        // BIT b, (HL)
        data = read(HL());
        current_instr = "BIT " + std::to_string(bit_num) + ", (HL)";
    } else {
        // BIT b, (some regitser)
        data = *reg_pointers[opcode & 0x07];
        current_instr = "BIT " + std::to_string(bit_num) + ", " + reg_names[opcode & 0x07];
    }
    
    // Z set if the target bit = 0, H set, N reset
    set_Z(gb::Utils::get_bit(data, bit_num) == 0);
    set_H(1); set_N(0);
}

void gb::cpu::CALL(){
    // CALL - push PC onto stack and jump to a given address
    uint16_t nn = read(PC) | (read(PC + 1) << 8);
    PC += 2;
    switch (opcode) {
        case 0xCD:
            // Always call
            write(SP - 1, (PC & 0xFF00) >> 8);
            write(SP - 2, PC & 0x00FF);
            SP -= 2;
            PC = nn;
            current_instr = "CALL $" + gb::Utils::hex_short(nn);
            break;
        case 0xC4: case 0xCC:{
            // C4 - call if Z reset, CC - jump if Z set
            // Takes 12 extra cycles if call occurs
            if (!(get_Z() ^ ((opcode & 0b1000) >> 3))){
                write(SP - 1, (PC & 0xFF00) >> 8);
                write(SP - 2, PC & 0x00FF);
                SP -= 2;
                PC = nn;
                cycles += 12;
            }
            current_instr = ((opcode & 0b1000) >> 3) ? ("CALL Z, $" + gb::Utils::hex_short(nn)) : ("CALL NZ, $" + gb::Utils::hex_short(nn));
            break;
        }
        case 0xD4: case 0xDC:{
            // D4 - call if C reset, DC - call if C set
            // Takes 12 extra cycles if call occurs
            if (!(get_C() ^ ((opcode & 0b1000) >> 3))){
                write(SP - 1, (PC & 0xFF00) >> 8);
                write(SP - 2, PC & 0x00FF);
                SP -= 2;
                PC = nn;
                cycles += 12;
            }
            current_instr = ((opcode & 0b1000) >> 3) ? ("CALL C, $" + gb::Utils::hex_short(nn)) : ("CALL NC, $" + gb::Utils::hex_short(nn));
            break;
        }
        default:
            break;
    }
}

void gb::cpu::CCF(){
    // CCF - complement carry flag (invert it)
    set_C(!get_C()); current_instr = "CCF";
    // Resets N and H flags
    set_N(0); set_H(0);
}

void gb::cpu::CP(){
    // CP - compares A to a value and sets flags accordingly
    uint8_t n;
    switch (opcode) {
        case 0xFE:
            // Use immediate value
            n = read(PC); PC++;
            current_instr = "CP #" + gb::Utils::hex_byte(n);
            break;
        case 0xBE:
            // Use (HL))
            n = read(HL()); current_instr = "CP (HL)";
            break;
        case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBF:
            // Use other register
            n = *reg_pointers[opcode & 0x07];
            current_instr = "CP " + reg_names[opcode & 0x07];
            break;
        default:
            break;
    }
    // Set Z if A == n
    set_Z(A == n);
    // Set N
    set_N(1);
    // Set H if (A & 0xF) < (n & 0xF), and C if A < n
    set_H((A & 0xF) < (n & 0xF)); set_C(A < n);
    
}

void gb::cpu::CPL(){
    // CPL - flips all bits in the A register
    A = ~A; current_instr = "CPL";
    // Sets N and H flags
    set_N(1); set_H(1);
}

void gb::cpu::DAA(){
    // DAA - converts the result of the last operation (in A) to a BCD value
    // I don't really know how this code works
    int correction = 0;
    bool carry = false;
    
    if (get_H() or (!get_N() and ((A & 0x0F) > 9))){
        // Add 6 to first digit
        correction |= 0x06;
    }
    
    if (get_C() or (!get_N() and A > 0x99)){
        // Add 6 to second digit
        correction |= 0x60;
        carry = true;
    }
    
    A += get_N() ? -correction : correction;
    
    current_instr = "DAA";
    
    // Sets Z if A == 0, sets C if A > #99, resets H
    set_Z(A == 0);
    set_C(carry);
    set_H(0);
}

void gb::cpu::DEC(){
    // DEC - decrements target register by 1
    switch (opcode) {
        case 0x35:
            // DEC (HL)
            write(HL(), read(HL()) - 1); current_instr = "DEC (HL)";
            // Setz Z if result is 0, sets N, sets H if there was no borrow from bit 3
            set_Z(read(HL()) == 0); set_N(1); set_H((read(HL()) & 0xF) == 0xF);
            break;
        case 0x05: case 0x0D: case 0x15: case 0x1D: case 0x25: case 0x2D: case 0x3D: {
            // DEC (some register) - register identifier in btis 3-5
            uint8_t* reg_ptr = reg_pointers[(opcode & 0b00111000) >> 3];
            (*reg_ptr)--;
            current_instr = "DEC " + reg_names[(opcode & 0b00111000) >> 3];
            // Setz Z if result is 0, sets N, sets H if there was no borrow from bit 3
            set_Z(*reg_ptr == 0); set_N(1); set_H((*reg_ptr & 0xF) == 0xF);
            break;
        }
        case 0x0B:
            // DEC BC
            set_BC(BC() - 1); current_instr = "DEC BC";
            break;
        case 0x1B:
            // DEC DE
            set_DE(DE() - 1); current_instr = "DEC DE";
            break;
        case 0x2B:
            // DEC HL
            set_HL(HL() - 1); current_instr = "DEC HL";
            break;
        case 0x3B:
            // DEC SP
            SP--; current_instr = "DEC SP";
            break;
        default:
            break;
    }
}

void gb::cpu::DI(){
    // DI - disable interrupts
    IME = false; current_instr = "DI";
}

void gb::cpu::EI(){
    // EI - enable interrputs
    IME = true; current_instr = "EI";
}

void gb::cpu::HALT(){
    // HALT - stops CPU execution until an interrupt occurs (done by setting the halted flag)
    halted = true; current_instr = "HALT";
}

void gb::cpu::INC(){
    // INC - increments target register by 1
    switch (opcode) {
        case 0x34:
            // INC (HL)
            write(HL(), read(HL()) + 1); current_instr = "INC (HL)";
            // Setz Z if result is 0, resets N, sets H if there was a carry from bit 3
            set_Z(read(HL()) == 0); set_N(0); set_H((read(HL()) & 0xF) == 0);
            break;
        case 0x04: case 0x0C: case 0x14: case 0x1C: case 0x24: case 0x2C: case 0x3C: {
            // INC (some register) - register identifier in btis 3-5
            uint8_t* reg_ptr = reg_pointers[(opcode & 0b00111000) >> 3];
            (*reg_ptr)++;
            current_instr = "INC " + reg_names[(opcode & 0b00111000) >> 3];
            // Setz Z if result is 0, resets N, sets H if there was a carry from bit 3
            set_Z(*reg_ptr == 0); set_N(0); set_H((*reg_ptr & 0xF) == 0);
            break;
        }
        case 0x03:
            // INC BC
            set_BC(BC() + 1); current_instr = "INC BC";
            break;
        case 0x13:
            // INC DE
            set_DE(DE() + 1); current_instr = "INC DE";
            break;
        case 0x23:
            // INC HL
            set_HL(HL() + 1); current_instr = "INC HL";
            break;
        case 0x33:
            // INC SP
            SP++; current_instr = "INC SP";
            break;
        default:
            break;
    }
}

void gb::cpu::JP(){
    // JP - jump execution to some address
    switch (opcode) {
        case 0xC3: {
            // Always jump to nn
            uint16_t nn = read(PC) | (read(PC + 1) << 8); PC = nn;
            current_instr = "JP $" + gb::Utils::hex_short(nn);
            break;
        }
        case 0xE9:
            // JP, HL
            PC = HL(); current_instr = "JP HL";
            break;
        case 0xC2: case 0xCA:{
            // C2 - jump if Z reset, CA - jump if Z set
            // Takes 4 extra cycles if jump occurs
            uint16_t nn = read(PC) | (read(PC + 1) << 8); PC += 2;
            if (!(get_Z() ^ ((opcode & 0b1000) >> 3))){
                PC = nn; cycles += 4;
            }
            current_instr = ((opcode & 0b1000) >> 3) ? ("JP Z, $" + gb::Utils::hex_short(nn)) : ("JP NZ, $" + gb::Utils::hex_short(nn));
            break;
        }
        case 0xD2: case 0xDA:{
            // D2 - jump if C reset, DA - jump if C set
            // Takes 4 extra cycles if jump occurs
            uint16_t nn = read(PC) | (read(PC + 1) << 8); PC += 2;
            if (!(get_C() ^ ((opcode & 0b1000) >> 3))){
                PC = nn; cycles += 4;
            }
            current_instr = ((opcode & 0b1000) >> 3) ? ("JP C, $" + gb::Utils::hex_short(nn)) : ("JP NC, $" + gb::Utils::hex_short(nn));
            break;
        }
        default:
            break;
    }
}

void gb::cpu::JR(){
    // JR - signed byte is read and used as a relative offset to the PC
    uint8_t n = read(PC); PC++;
    switch (opcode) {
        case 0x18:
            // Always jump
            PC += n;
            if (n > 0x7F) PC -= 0x100;
            current_instr = "JR #" + gb::Utils::hex_byte(n);
            break;
        
        case 0x20: case 0x28:
            // 20 - jump if Z reset, 28 - jump if Z set
            // Takes 4 extra cycles if jump occurs
            if (!(get_Z() ^ ((opcode & 0b1000) >> 3))){
                PC += n; cycles += 4;
                if (n > 0x7F) PC -= 0x100;
            }
            current_instr = ((opcode & 0b1000) >> 3) ? ("JR Z, #" + gb::Utils::hex_byte(n)) : ("JR NZ, #" + gb::Utils::hex_byte(n));
            break;
            
        case 0x30: case 0x38:
            // 30 - jump if C reset, 38 - jump if C set
            // Takes 4 extra cycles if jump occurs
            if (!(get_C() ^ ((opcode & 0b1000) >> 3))){
                PC += n; cycles += 4;
                if (n > 0x7F) PC -= 0x100;
            }
            current_instr = ((opcode & 0b1000) >> 3) ? ("JR C, #" + gb::Utils::hex_byte(n)) : ("JR NC, #" + gb::Utils::hex_byte(n));
            break;
        default:
            break;
    }
}

void gb::cpu::LD(){
    // Many different instructions to move data around
    switch (opcode){
        // Immediate 16-bit loads ------------------------------------------------------------------------------
        case 0x01: case 0x11: case 0x21: case 0x31:{
            uint16_t data = read(PC) | (read(PC + 1) << 8);
            PC += 2;
            if ((opcode & 0xF0) == 0x30){
                // LD SP, nnnn
                SP = data; current_instr = "LD SP, #" + gb::Utils::hex_short(data);
            } else {
                // LD (BC/DE/HL), nn
                void (gb::cpu::* reg_func)(uint16_t nn) = w_reg_pointers_16[(opcode & 0xF0) >> 4];
                (this->*(reg_func))(data);
                current_instr = "LD " + reg_names_16[(opcode & 0xF0) >> 4] + ", #" + gb::Utils::hex_short(data);
            }
            break;
        }
            
        // Immediate 8-bit loads --------------------------------------------------------------------------------
        case 0x06: case 0x0E: case 0x16: case 0x1E: case 0x26: case 0x2E: case 0x36: case 0x3E:{
            uint8_t n = read(PC); PC++;
            
            // Register identified in bits 3, 4 and 5
            int reg = (opcode & 0b111000) >> 3;
            if (reg == 6){
                // LD (HL), n
                write(HL(), n); current_instr = "LD (HL), #" + gb::Utils::hex_byte(n);
            } else {
                // LD (B/C/D/E/H/L/A), n
                *reg_pointers[reg] = n; current_instr = "LD " + reg_names[reg] + ", #" + gb::Utils::hex_byte(n);
            }
            break;
        }
            
        // LD SP, HL --------------------------------------------------------------------------------------------
        case 0xF9:
            SP = HL();
            current_instr = "LD SP, HL";
            break;
            
        // LD HL, SP + n ----------------------------------------------------------------------------------------
        case 0xF8:{
            uint8_t value = read(PC);
            PC++; set_HL(SP + value);
            if (value > 0x7F) set_HL(HL() - 0x100);
            
            // Z and N reset, C set if there is a carry, H set if there is a half-carry
            set_Z(0); set_N(0);
            set_H(((SP & 0xF) + (value & 0xF)) >= 0x10);
            set_C(((SP & 0xFF) + value) >= 0x100);
            
            current_instr = "LD HL, SP + #" + gb::Utils::hex_byte(value);
            break;
        }
            
        // LD (nn), SP ------------------------------------------------------------------------------------------
        case 0x08:{
            uint16_t nn = read(PC) | (read(PC + 1) << 8);
            write(nn, SP & 0x00FF);
            write(nn + 1, (SP & 0xFF00) >> 8);
            PC += 2;
            current_instr = "LD ($" + gb::Utils::hex_short(nn) + "), SP";
            break;
        }
            
        // Move A to/from $FF page indexed with C ---------------------------------------------------------------
        case 0xF2:
            // LD A, ($FF00 + C)
            A = read(0xFF00 + C);
            current_instr = "LD A, ($FF00 + C)";
            break;
        case 0xE2:
            // LD ($FF00 + C), A
            write(0xFF00 + C, A);
            current_instr = "LD ($FF00 + C), A";
            break;
        // Move A to/from 16-bit address ------------------------------------------------------------------------
        case 0xFA:{
            // LD A, (nn)
            uint16_t nn = read(PC) | (read(PC + 1) << 8);
            A = read(nn); PC += 2;
            current_instr = "LD A, ($" + gb::Utils::hex_short(nn) + ")";
            break;
        }
        case 0xEA:{
            // LD (nn), A
            uint16_t nn = read(PC) | (read(PC + 1) << 8);
            write(nn, A); PC += 2;
            current_instr = "LD ($" + gb::Utils::hex_short(nn) + "), A";
            break;
        }
            
        // Use BC/DE as a pointer to load/store A ------------------------------------------------------------
        case 0x0A:
            // LD A, (BC)
            A = read(BC()); current_instr = "LD A, (BC)";
            break;
        case 0x1A:
            // LD A, (DE)
            A = read(DE()); current_instr = "LD A, (DE)";
            break;
        case 0x02:
            // LD (BC), A
            write(BC(), A); current_instr = "LD (BC), A";
            break;
        case 0x12:
            // LD (DE), A
            write(DE(), A); current_instr = "LD (DE), A";
            break;
        
        // Other register - register loads -------------------------------------------------------------------
        default: {
            // LD (reg 1), (reg 2)
            // Reg 1 comes from bits 3 4 and 5, reg 2 comes from bits 0 1 and 2
            int reg_1 = (opcode & 0b00111000) >> 3;
            int reg_2 = opcode & 0b00000111;
            
            if (reg_1 == 6){
                // LD (HL), (reg 2)
                write(HL(), *reg_pointers[reg_2]);
                current_instr = "LD (HL), " + reg_names[reg_2];
            } else if (reg_2 == 6){
                // LD (reg 1), (HL)
                *reg_pointers[reg_1] = read(HL());
                current_instr = "LD " + reg_names[reg_1] + ", (HL)";
            } else {
                // Normal reg-reg load
                *reg_pointers[reg_1] = *reg_pointers[reg_2];
                current_instr = "LD " + reg_names[reg_1] + ", " + reg_names[reg_2];
            }
            
            break;
        }
    }
}

void gb::cpu::LDD(){
    // LDD - move data between A and memory and decrement HL
    switch (opcode) {
        case 0x3A:
            // LDD A, (HL)
            A = read(HL());
            set_HL(HL() - 1);
            current_instr = "LDD A, (HL)";
            break;
        case 0x32:
            // LDD (HL), A
            write(HL(), A);
            set_HL(HL() - 1);
            current_instr = "LDD (HL), A";
            break;
        default:
            break;
    }
    return;
}

void gb::cpu::LDH(){
    // LDH - move data between A and address at $FF00 + immediate value
    switch (opcode) {
        case 0xE0: {
            // LDH (n), A
            uint8_t value = read(PC);
            PC++;
            write(0xFF00 + value, A);
            current_instr = "LDH ($" + gb::Utils::hex_byte(value) + "), A";
            break;
        }
        case 0xF0: {
            // LDH A, (n)
            uint8_t value = read(PC);
            PC++;
            A = read(0xFF00 + value);
            current_instr = "LDH A, ($" + gb::Utils::hex_byte(value) + ")";
            break;
        }
        default:
            break;
    }
}

void gb::cpu::LDI(){
    // LDI - move data between A and memory and increment HL
    switch (opcode) {
        case 0x2A:
            // LDI A, (HL)
            A = read(HL());
            set_HL(HL() + 1);
            current_instr = "LDI A, (HL)";
            break;
        case 0x22:
            // LDI (HL), A
            write(HL(), A);
            set_HL(HL() + 1);
            current_instr = "LDI (HL), A";
            break;
        default:
            break;
    }
}

void gb::cpu::NOP(){
    // NOP - does nothing
    current_instr = "NOP";
}

void gb::cpu::OR(){
    // OR - Sets A to A or the target register
    switch (opcode) {
        case 0xF6: {
            // Use immediate value
            uint8_t value = read(PC);
            A |= value; PC ++;
            current_instr = "OR #" + gb::Utils::hex_byte(value);
            break;
        }
        case 0xB6:
            // Use (HL)
            A |= read(HL());
            current_instr = "OR (HL)";
            break;
        case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB7:
            // Use reg from regpointers
            A |= *reg_pointers[opcode & 0x07];
            current_instr = "OR " + reg_names[opcode & 0x07];
            break;
        default:
            break;
    }
    // Z set if A == 0, N H and C reset
    set_Z(A == 0); set_N(0); set_H(0); set_C(0);
}

void gb::cpu::POP(){
    // POP - pops register pair (AF/BC/DE/HL) from the stack
    //void (gb::cpu::* reg_func)(uint16_t data) = w_reg_pointers_16[(opcode & 0b00110000) >> 4];
    uint16_t reg_value = read(SP) | (read(SP + 1) << 8);
    SP += 2;
    //(this->*(reg_func))(reg_value);
    switch (opcode) {
        case 0xC1:
            set_BC(reg_value);
            break;
        case 0xD1:
            set_DE(reg_value);
            break;
        case 0xE1:
            set_HL(reg_value);
            break;
        case 0xF1:
            // Masks lower 4 bits as only bits 4-7 in F can be written to
            set_AF(reg_value & 0xFFF0);
            break;
        default:
            break;
    }
    current_instr = "POP " + reg_names_16[(opcode & 0b00110000) >> 4];
}

void gb::cpu::PUSH(){
    // PUSH - pushes register pair (AF/BC/DE/HL) onto the stack
    uint16_t (gb::cpu::* reg_func)() = r_reg_pointers_16[(opcode & 0b00110000) >> 4];
    uint16_t reg_value = (this->*(reg_func))();
    write(SP - 1, (reg_value & 0xFF00) >> 8);
    write(SP - 2, reg_value & 0x00FF);
    SP -= 2;
    current_instr = "PUSH " + reg_names_16[(opcode & 0b00110000) >> 4];
}

void gb::cpu::RES(){
    // RES - resets specific bit in target register to 0
    int bit_num = (opcode & 0b00111000) >> 3;
    if ((opcode & 0x07) == 6){
        // RES b, (HL)
        uint8_t data = read(HL());
        write(HL(), gb::Utils::set_bit(data, bit_num, 0));
        current_instr = "RES " + std::to_string(bit_num) + ", (HL)";
    } else {
        // RES b, (some regitser)
        *reg_pointers[opcode & 0x07] = gb::Utils::set_bit(*reg_pointers[opcode & 0x07], bit_num, 0);
        current_instr = "RES " + std::to_string(bit_num) + ", " + reg_names[opcode & 0x07];
    }
}

void gb::cpu::RET(){
    // RET - pops PC off stack
    switch (opcode) {
        case 0xC9:
            // Always return
            PC = read(SP) | (read(SP + 1) << 8);
            SP += 2;
            current_instr = "RET";
            break;
            
        case 0xC0: case 0xC8:
            // C0 - return if Z reset, C8 - return if Z set
            // Takes 12 extra cycles if return occurs
            if (!(get_Z() ^ ((opcode & 0b1000) >> 3))){
                PC = read(SP) | (read(SP + 1) << 8);
                SP += 2;
                cycles += 12;
            }
            current_instr = ((opcode & 0b1000) >> 3) ? "RET Z" : "RET NZ";
            break;
            
        case 0xD0: case 0xD8:
            // D0 - return if C reset, D8 - return if C set
            // Takes 12 extra cycles if return occurs
            if (!(get_C() ^ ((opcode & 0b1000) >> 3))){
                PC = read(SP) | (read(SP + 1) << 8);
                SP += 2;
                cycles += 12;
            }
            current_instr = ((opcode & 0b1000) >> 3) ? "RET C" : "RET NC";
            break;
            
        default:
            break;
    }
}

void gb::cpu::RETI(){
    // RETI - pops PC off the stack and enables interrupts
    PC = read(SP) | (read(SP + 1) << 8);
    SP += 2;
    IME = true;
    current_instr = "RETI";
}

void gb::cpu::RLA(){
    // RLA - rotates A left through carry, old bit 7 to carry and old carry to bit 0
    bool bit_7 = (A & 0x80) >> 7;
    A = A << 1;
    A |= get_C();
    
    // Z H N reset, C set to old bit 0
    set_Z(0); set_H(0); set_N(0); set_C(bit_7);
    current_instr = "RLA";
}

void gb::cpu::RLC(){
    // RLC - rotates target register left, old bit 7 to carry and bit 0
    if (opcode == 0x06){
        // RLC (HL)
        uint8_t data = read(HL());
        bool bit_7 = (data & 0x80) >> 7;
        data = (data << 1) | bit_7;
        write(HL(), data);
        current_instr = "RLC (HL)";
        
        // Sets Z if result = 0, H N reset, sets C to old bit 7
        set_Z(data == 0); set_H(0); set_N(0); set_C(bit_7);
    } else {
        // RLC (other register)
        int reg = opcode & 0x07;
        bool bit_7 = (*reg_pointers[reg] & 0x80) >> 7;
        *reg_pointers[reg] = (*reg_pointers[reg] << 1) | bit_7;
        current_instr = "RLC " + reg_names[reg];
        
        // Sets Z if result = 0, H N reset, sets C to old bit 7
        set_Z(*reg_pointers[reg] == 0); set_H(0); set_N(0); set_C(bit_7);
    }
}

void gb::cpu::RLCA(){
    // RLCA - rotates A left, old bit 7 to carry and bit 0
    bool bit_7 = (A & 0x80) >> 7;
    A = A << 1;
    A |= bit_7;
    
    // Z H N reset, C set to old bit 0
    set_Z(0); set_H(0); set_N(0); set_C(bit_7);
    current_instr = "RLCA";
}

void gb::cpu::RL(){
    // RL - rotates target register left through carry, carry to bit 0, old bit 7 to carry
    if (opcode == 0x16){
        // RLC (HL)
        uint8_t data = read(HL());
        bool bit_7 = (data & 0x80) >> 7;
        data = (data << 1) | get_C();
        write(HL(), data);
        current_instr = "RL (HL)";
        
        // Sets Z if result = 0, H N reset, sets C to old bit 7
        set_Z(data == 0); set_H(0); set_N(0); set_C(bit_7);
    } else {
        // RL (other register)
        int reg = opcode & 0x07;
        bool bit_7 = (*reg_pointers[reg] & 0x80) >> 7;
        *reg_pointers[reg] = (*reg_pointers[reg] << 1) | get_C();
        current_instr = "RL " + reg_names[reg];
        
        // Sets Z if result = 0, H N reset, sets C to old bit 7
        set_Z(*reg_pointers[reg] == 0); set_H(0); set_N(0); set_C(bit_7);
    }
}

void gb::cpu::RRA(){
    // RRA - rotates A right through carry, old bit 0 to carry and old carry to bit 7
    bool bit_0 = A & 1;
    A = A >> 1;
    A |= get_C() << 7;
    
    // Z H N reset, C set to old bit 0
    set_Z(0); set_H(0); set_N(0); set_C(bit_0);
    current_instr = "RRA";
}

void gb::cpu::RRC(){
    // RRC - rotates target register right, old bit 0 to carry and bit 7
    if (opcode == 0x0E){
        // RRC (HL)
        uint8_t data = read(HL());
        bool bit_0 = data & 1;
        data = (data >> 1) | (bit_0 << 7);
        write(HL(), data);
        current_instr = "RRC (HL)";
        
        // Sets Z if result = 0, H N reset, sets C to old bit 0
        set_Z(data == 0); set_H(0); set_N(0); set_C(bit_0);
    } else {
        // RRC (other register)
        int reg = opcode & 0x07;
        bool bit_0 = *reg_pointers[reg] & 1;
        *reg_pointers[reg] = (*reg_pointers[reg] >> 1) | (bit_0 << 7);
        current_instr = "RRC " + reg_names[reg];
        
        // Sets Z if result = 0, H N reset, sets C to old bit 0
        set_Z(*reg_pointers[reg] == 0); set_H(0); set_N(0); set_C(bit_0);
    }
}

void gb::cpu::RRCA(){
    // RRCA - rotates A right, old bit 0 to bit 7 and carry flag
    bool bit_0 = A & 1;
    A = A >> 1;
    A |= bit_0 << 7;
    
    // Z H N reset, C set to old bit 0
    set_Z(0); set_H(0); set_N(0); set_C(bit_0);
    current_instr = "RRCA";
}

void gb::cpu::RR(){
    // RR - rotates target register right through carry, carry to bit 7, old bit 0 to carry
    if (opcode == 0x1E){
        // RR (HL)
        uint8_t data = read(HL());
        bool bit_0 = data & 1;
        data = (data >> 1) | (get_C() << 7);
        write(HL(), data);
        current_instr = "RR (HL)";
        
        // Sets Z if result = 0, H N reset, sets C to old bit 0
        set_Z(data == 0); set_H(0); set_N(0); set_C(bit_0);
    } else {
        // RRC (other register)
        int reg = opcode & 0x07;
        bool bit_0 = *reg_pointers[reg] & 1;
        *reg_pointers[reg] = (*reg_pointers[reg] >> 1) | (get_C() << 7);
        current_instr = "RR " + reg_names[reg];
        
        // Sets Z if result = 0, H N reset, sets C to old bit 0
        set_Z(*reg_pointers[reg] == 0); set_H(0); set_N(0); set_C(bit_0);
    }
}

void gb::cpu::RST(){
    // RST - push current PC onto the stack and jump to (opcode - 0xC7)
    write(SP - 1, (PC & 0xFF00) >> 8);
    write(SP - 2, PC & 0x00FF);
    SP -= 2;
    PC = opcode - 0xC7;
    current_instr = "RST $00" + gb::Utils::hex_byte(opcode - 0xC7);
}

void gb::cpu::SBC(){
    // SBC - Subtracts register/number and carry bit from A
    uint8_t old_A = A;
    uint8_t n;
    switch (opcode) {
        case 0xDE:
            // Use immediate value
            n = read(PC); PC++;
            current_instr = "SBC A, #" + gb::Utils::hex_byte(n);
            break;
        case 0x9E:
            // Use (HL)
            n = read(HL()); current_instr = "SBC A, (HL)";
            break;
        case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9F:
            // Use other reigster
            n = *reg_pointers[opcode & 0x07];
            current_instr = "SBC A, " + reg_names[opcode & 0x07];
            break;
        default:
            break;
    }
    
    A = A - (n + get_C());
    // Set Z if A == 0, set N, set H if no borrow from bit 3, set C if no borrow
    set_Z(A == 0); set_N(1);
    set_H((old_A & 0xF) < (n & 0xF) + get_C());
    set_C(old_A < n + get_C());
}

void gb::cpu::SCF(){
    // SCF - set carry flag (also resets H and N flags)
    set_N(0); set_H(0); set_C(1);
    current_instr = "SCF";
}

void gb::cpu::SET(){
    // SET - sets specific bit in target register to 1
    int bit_num = (opcode & 0b00111000) >> 3;
    if ((opcode & 0x07) == 6){
        // SET b, (HL)
        uint8_t data = read(HL());
        write(HL(), gb::Utils::set_bit(data, bit_num, 1));
        current_instr = "SET " + std::to_string(bit_num) + ", (HL)";
    } else {
        // SET b, (some regitser)
        *reg_pointers[opcode & 0x07] = gb::Utils::set_bit(*reg_pointers[opcode & 0x07], bit_num, 1);
        current_instr = "SET " + std::to_string(bit_num) + ", " + reg_names[opcode & 0x07];
    }
}

void gb::cpu::SLA(){
    // SLA - shifts target register left and sends old bit 7 to carry
    if (opcode == 0x26) {
        // SLA (HL)
        uint8_t data = read(HL());
        bool bit_7 = (data & 0x80) >> 7;
        data = data << 1;
        write(HL(), data);
        current_instr = "SLA (HL)";
        
        // Sets Z if result = 0, resets H and N, sets C to old bit 7
        set_Z(data == 0); set_H(0); set_N(0); set_C(bit_7);
    } else {
        // RL (other register)
        int reg = opcode & 0x07;
        bool bit_7 = (*reg_pointers[reg] & 0x80) >> 7;
        *reg_pointers[reg] = *reg_pointers[reg] << 1;
        current_instr = "SLA " + reg_names[reg];
        
        // Sets Z if result = 0, H N reset, sets C to old bit 7
        set_Z(*reg_pointers[reg] == 0); set_H(0); set_N(0); set_C(bit_7);
    }
}

void gb::cpu::SRA(){
    // SRA - shifts target register right, old bit 0 to carry, bit 7 keeps original value
    if (opcode == 0x2E){
        // SRA (HL)
        uint8_t data = read(HL());
        bool bit_0 = data & 1;
        uint8_t bit_7 = data & 0x80;
        data = bit_7 | (data >> 1);
        write(HL(), data);
        current_instr = "SRA (HL)";
        
        // Sets Z if result = 0, H N reset, sets C to old bit 0
        set_Z(data == 0); set_H(0); set_N(0); set_C(bit_0);
    } else {
        // SRA (other register)
        int reg = opcode & 0x07;
        bool bit_0 = *reg_pointers[reg] & 1;
        uint8_t bit_7 = *reg_pointers[reg] & 0x80;
        *reg_pointers[reg] = bit_7 | (*reg_pointers[reg] >> 1);
        current_instr = "SRA " + reg_names[reg];
        
        // Sets Z if result = 0, H N reset, sets C to old bit 0
        set_Z(*reg_pointers[reg] == 0); set_H(0); set_N(0); set_C(bit_0);
    }
}

void gb::cpu::SRL(){
    // SRL - shifts target register right, old bit 0 to carry, bit 7 reset to 0
    if (opcode == 0x3E){
        // SRL (HL)
        uint8_t data = read(HL());
        bool bit_0 = data & 1;
        data = data >> 1;
        write(HL(), data);
        current_instr = "SRL (HL)";
        
        // Sets Z if result = 0, H N reset, sets C to old bit 0
        set_Z(data == 0); set_H(0); set_N(0); set_C(bit_0);
    } else {
        // SRA (other register)
        int reg = opcode & 0x07;
        bool bit_0 = *reg_pointers[reg] & 1;
        *reg_pointers[reg] = *reg_pointers[reg] >> 1;
        current_instr = "SRA " + reg_names[reg];
        
        // Sets Z if result = 0, H N reset, sets C to old bit 0
        set_Z(*reg_pointers[reg] == 0); set_H(0); set_N(0); set_C(bit_0);
    }
}

void gb::cpu::STOP(){
    // STOP - Stops CPU execution until a button is pressed (done by setting stopped flag)
    // Opcode is meant to be 0x1000 so reads another byte and gives a warning if it is not 00
    if (read(PC) == 0x00)
        PC++;
    // DEBUG: else
    //    std::cerr << "STOP instrction at $" << gb::Utils::hex_short(PC - 1) << " should have opcode 0x1000!" << std::endl;
        
    stopped = true; current_instr = "STOP";
}

void gb::cpu::SUB(){
    // SUB - Subtracts register/number from A
    uint8_t old_A = A;
    uint8_t n;
    switch (opcode) {
        case 0xD6:
            // Use immediate value
            n = read(PC); PC++;
            current_instr = "SUB #" + gb::Utils::hex_byte(n);
            break;
        case 0x96:
            // Use (HL)
            n = read(HL()); current_instr = "SUB (HL)";
            break;
        case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x97:
            // Use other reigster
            n = *reg_pointers[opcode & 0x07];
            current_instr = "SUB " + reg_names[opcode & 0x07];
            break;
        default:
            break;
    }
    
    A = A - n;
    // Set Z if A == 0, set N, set H if no borrow from bit 3, set C if no borrow
    set_Z(A == 0); set_N(1);
    set_H((old_A & 0xF) < (n & 0xF));
    set_C(old_A < n);
}

void gb::cpu::SWAP(){
    // SWAP - swaps the lower and upper nybbles of target register
    if (opcode == 0x36){
        // SWAP (HL)
        uint8_t data = read(HL());
        data = ((data & 0x0F) << 4) | ((data & 0xF0) >> 4);
        write(HL(), data);
        current_instr = "SWAP (HL)";
        
        // Sets Z is result is 0, resets H N C
        set_Z(data == 0); set_N(0); set_H(0); set_C(0);
    } else {
        // SWAP (other register)
        int reg = opcode & 0x07;
        *reg_pointers[reg] = ((*reg_pointers[reg] & 0x0F) << 4) | ((*reg_pointers[reg] & 0xF0) >> 4);
        current_instr = "SWAP " + reg_names[reg];
        
        // Sets Z is result is 0, resets H N C
        set_Z(*reg_pointers[reg] == 0); set_N(0); set_H(0); set_C(0);
    }
}

void gb::cpu::XOR(){
    // XOR - Sets A to A xor the target register
    switch (opcode) {
        case 0xEE: {
            // Use immediate value
            uint8_t value = read(PC);
            A ^= value; PC ++;
            current_instr = "XOR #" + gb::Utils::hex_byte(value);
            break;
        }
        case 0xAE:
            // Use (HL)
            A ^= read(HL());
            current_instr = "XOR (HL)";
            break;
        case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAF:
            // Use reg from regpointers
            A ^= *reg_pointers[opcode & 0x07];
            current_instr = "XOR " + reg_names[opcode & 0x07];
            break;
        default:
            break;
    }
    
    // C H N set to 0, zero set if result is 0
    set_Z(A == 0); set_H(0); set_N(0); set_C(0);
}

void gb::cpu::XXX(){
    // Catches all invalid instructions
    current_instr = "???";
    // DEBUG: std::cerr << "Invalid opcode " << gb::Utils::hex_byte(opcode) << " at " << gb::Utils::hex_short(PC)  << "!" << std::endl;
}
