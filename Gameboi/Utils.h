// Created by Niklas on 25/03/2020.
// Namespace containing useful functions such as converting strings to hex
#ifndef Utils_h
#define Utils_h

// For returning strings as hex
#include <string>
#include <iostream>

namespace gb{
    namespace Utils {
        inline bool get_bit(char byte, int bitnum){
            // Returns the bit in a specific byte according to the bitnum
            // MSB = 7, LSB = 0
            return (byte & (1 << bitnum)) >> bitnum;
        }
        
        inline char set_bit(char byte, int bitnum, bool value){
            // Sets the bit in the bitnum position the value given
            // MSB = 7, LSB = 0
            if (value)
                // Set bit to 1
                return byte | (1 << bitnum);
            else
                // Set bit to 0
                return byte & ~(1 << bitnum);
        }
        
        inline std::string dec_to_hex(int num){
            // Converts any number into a hexadecimal string
            char hex_chars[16] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
            std::string output = "";
            
            while (num > 0){
                int digit = num % 16;
                output = hex_chars[digit] + output;
                num /= 16;
            }
            
            return output;
        }
        
        inline std::string dec_to_bin(int num){
            // Converts any number into a binary string
            char hex_chars[16] = {'0','1'};
            std::string output = "";
            
            while (num > 0){
                int digit = num % 2;
                output = hex_chars[digit] + output;
                num /= 2;
            }
            return output;
        }
        
        inline std::string hex_byte(uint8_t input){
            // Formats a byte as 2 hex digits
            std::string raw = dec_to_hex(input);
            int loops = 2 - raw.length();
            for (int i = 0; i < loops; i++)
                raw = "0" + raw;
            return raw;
        }
        
       inline std::string hex_short(uint16_t input){
            // Formats a short as 4 hex digits
           // Formats a byte as 2 hex digits
           std::string raw = dec_to_hex(input);
           int loops = 4 - raw.length();
           for (int i = 0; i < loops; i++)
               raw = "0" + raw;
           return raw;
        }
        
        inline std::string bin_byte(uint8_t input){
            // Formats a byte as 2 hex digits
            std::string raw = dec_to_bin(input);
            int loops = 8 - raw.length();
            for (int i = 0; i < loops; i++)
                raw = "0" + raw;
            return raw;
        }
        
        inline std::string bin_short(uint16_t input){
            // Formats a short as 4 hex digits
            std::string raw_high = bin_byte((input & 0xFF00) >> 8);
            std::string raw_lo = bin_byte(input & 0x00FF);
            return raw_high + "  " + raw_lo;
        }
        
    };
}

#endif /* Utils_h */
