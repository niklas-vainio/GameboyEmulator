// Created by Niklas on 01/04/2020.
// Header file with enum for all of the IO register names

#ifndef RegNames_h
#define RegNames_h

namespace gb {
    enum regNames {
        P1 = 0x00, SB = 0x01, SC = 0x02, DIV = 0x04, TIMA = 0x05, TMA = 0x06,
        TAC = 0x07, IF = 0x0F, NR10 = 0x10, NR11 = 0x11, NR12 = 0x12, NR13 = 0x13,
        NR14 = 0x14, NR21 = 0x16, NR22 = 0x17, NR23 = 0x18, NR24 = 0x19, NR30 = 0x1A,
        NR31 = 0x1B, NR32 = 0x1C, NR33 = 0x1D, NR34 = 0x1E, NR41 = 0x20, NR42 = 0x21,
        NR43 = 0x22, NR44 = 0x23, NR50 = 0x24, NR51 = 0x25, NR52 = 0x26, WAV0 = 0x30,
        WAV1 = 0x31, WAV2 = 0x32, WAV3 = 0x33, WAV4 = 0x34, WAV5 = 0x35, WAV6 = 0x35,
        WAV7 = 0x37, WAV8 = 0x38, WAV9 = 0x39, WAVA = 0x3A, WAVB = 0x3B, WAVC = 0x3C,
        WAVD = 0x3D, WAVE = 0x3E, WAVF = 0x3F, LCDC = 0x40, STAT = 0x41, SCY = 0x42,
        SCX = 0x43, LY = 0x44, LYC = 0x45, DMA = 0x46, BGP = 0x47, OBP0 = 0x48, OBP1 = 0x49,
        WY = 0x4A, WX = 0x4B
    };
}

#endif /* RegNames_h */
