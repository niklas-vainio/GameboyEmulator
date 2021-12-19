// Created by Niklas on 31/03/2020.
// Creates a class for the ppu which lodas the graphics data from memory
// This class handles the graphcis data - the actual drawing is done by the ViewController Class

#ifndef ppu_h
#define ppu_h

#include <array>
#include <vector>
#include <tuple>

#include "Utils.h"
#include "RegNames.h"
#include "bus.h"

namespace gb {
    class ppu {
    public:
        // Stores the PPU's mode (0 - 3)
        // Mode 0 = h-blank, cpu can access VRAM and OAM
        // Mode 1 = v-blank, cpu can access VRAM and OAM
        // Mode 2 = cpu cannot access OAM
        // Mode 3 = cpu cannot access VRAM or OAM
        int mode = 0;
        
        // Stores how many cycles have elapsed on the current scaline and how many scanlines have elapsed on the current frame
        int num_cycles = 0;
        int num_scanlines = 0;
        int window_scanlines = 0;
        
        // Stores whether vblank is currently occuring
        bool vblank = false;
        
        // Variable which stores whether or not the frame and scanline are over
        bool frame_over = false;
        bool scanline_over = false;
        
        // Stores a frame buffer which is an array of ints, each representing the value of that pixel
        uint8_t frame_buffer[144][160];
        
        // Stores the current tile as an aray of 256 ints, each representing the value of one pixel
        uint8_t current_tile[64];
        
        // Function to store a reference to the bus
        void connect_bus(gb::bus* _bus){
            bus = _bus;
        }
        
        // Function to initialise to default state
        void init(){
            // Reset frame buffer to all zeroes
            for (int x = 0; x < 160; x++){
                for (int y = 0; y < 144; y++){
                    frame_buffer[y][x] = 0;
                }
            }
            
            // Writes zeroes to SCX and SCY
            write(0xFF00 + gb::regNames::SCX, 0);
            write(0xFF00 + gb::regNames::SCY, 0);
        }
        
        
        // Function to get the colour from the BGP pallette
        uint8_t get_background_pallette_data(uint8_t pixel_data){
            // BGP register stores 2 bit colours for each of the pixel values 0-3
            // Value for 3 in bits 7-6, 2 in bits 5-4, 1 in bits 3-2 and 0 in bits 1-0
            return (get_reg(gb::regNames::BGP) & (0b11 << pixel_data * 2)) >> (pixel_data * 2);
        }
        
        // Function to get the colour from the OBP0 or OBP1 pallettes
        uint8_t get_sprite_pallette_data(uint8_t pixel_data, bool pallette_select){
            // OBP0 and OBP1 registers store 2 bit colours for each of the pixel values 0-3 - 0 is always transparent
            // Value for 3 in bits 7-6, 2 in bits 5-4 and 1 in bits 3-2
            if (pallette_select)
                // Return data from OBP1
                return (get_reg(gb::regNames::OBP1) & (0b11 << pixel_data * 2)) >> (pixel_data * 2);
            else
                // Return data from OBP0
                return (get_reg(gb::regNames::OBP0) & (0b11 << pixel_data * 2)) >> (pixel_data * 2);
        }

        // Function which sets current_tile to an array of 64 pixel brightnesses for a given tile
        // Tiles are found in VRAM from 8000 - 97FF (see VRAM.h)
        void get_tile_data(int addr){
            // Each tile is 16 bytes so addr is multiplied by 16
            uint16_t start_addr = 0x8000 + 16 * addr;
            // Creates and populates an array of the 16 bytes being accessed
            uint8_t tile_data[16];
            for (int i = 0; i < 16; i++){
                tile_data[i] = read(start_addr + i);
            }
            // Each pair of bytes corresponds to a row
            // The first byte holds the LSB for each pixel, and the second byte holds the MSB
            for (int row = 0; row < 8; row++){
                // Loops through each of the 8 pixels in the row
                for (int pix = 0; pix < 8; pix++){
                    // Sets the corresponding index in the output to the pixel value
                    current_tile[row * 8 + (7 - pix)] = gb::Utils::get_bit(tile_data[2 * row], pix) + 2 * gb::Utils::get_bit(tile_data[2 * row + 1], pix);
                }
            }
        }
        
        // Function to do one cycle of emulation
        void do_cycle(){
            // Do nothing if LCDC bit 7 is 0
            if(gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 7) == 0)
                return;
            
            num_cycles += 4;
            
            // Sets current mode depending on number of cycles
            if (num_cycles < 80) {
                // If entering mode 2, gives an LCDC Status interrupt if bit 5 of STAT is 1
                if (mode != 2 and gb::Utils::get_bit(get_reg(gb::regNames::STAT), 5))
                    write(0xFF00 + gb::regNames::IF, get_reg(gb::regNames::IF) | 0b00000010);
                mode = 2; // Mode 2 for 80 cycles
            }
            
            else if (num_cycles < 252)
                mode = 3; // Mode 3 for 172 cycles
            
            else if (num_cycles < 456) {
                // Draws the scanline at the start of h-blank
                if (mode != 0)
                    draw_scanline();
                
                // If entering mode 0, gives an LCDC Status interrupt if bit 3 of STAT is 1
                if (mode != 0 and gb::Utils::get_bit(get_reg(gb::regNames::STAT), 3))
                    write(0xFF00 + gb::regNames::IF, get_reg(gb::regNames::IF) | 0b00000010);
                
                mode = 0; // Mode 0 for 204 cycles
            }
            
            else {
                // After 456 cycles, the scanline ends - ppu copies graphics and resets the num_cycles counter
                num_scanlines++;
                
                // Increments window scanlines if window is enabled
                if (gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 5))
                    window_scanlines++;
                
                // If LY = LYC, sets STAT bit 2 and gives an LCDC Status interrupt if bit 6 of STAT is 1
                bool LY_coincidence = (num_scanlines % 154) == get_reg(gb::regNames::LYC);
                uint8_t old_status = get_reg(gb::regNames::STAT);
                write(0xFF00 + gb::regNames::STAT, (old_status & 0b11111011) | LY_coincidence << 2);
                
                if (LY_coincidence and gb::Utils::get_bit(get_reg(gb::regNames::STAT), 6))
                    write(0xFF00 + gb::regNames::IF, get_reg(gb::regNames::IF) | 0b00000010);
                
                num_cycles = 0;
                scanline_over = true;
            }
            
            // After 144 scanlines, V blank occurs for 10 extra scanlines
            if (num_scanlines > 143) {
                // Trigger vblank interrupt by setting bit 0 if the IF register when entering V blank
                if (!vblank) {
                    write(0xFF00 + gb::regNames::IF, get_reg(gb::regNames::IF) | 0b00000001);
                    
                    // Also gives an LCDC Stauts interrupt if bit 4 of STAT is 1
                    if (gb::Utils::get_bit(get_reg(gb::regNames::STAT), 4))
                        write(0xFF00 + gb::regNames::IF, get_reg(gb::regNames::IF) | 0b00000010);
                }
                // Mode 1 for the rest of vblank (4560 cycles)
                vblank = true;
                mode = 1;
            }
            
            if (num_scanlines > 153) {
                // Exits Vblank and starts new frame after 154 scanlines
                num_scanlines = 0;
                window_scanlines = 0;
                vblank = false;
                frame_over = true;
            }
            
            // Writes status and y position to io registers
            write(0xFF00 + gb::regNames::LY, num_scanlines);

            // Sets bits 0 and 1 of STAT to the mode
            uint8_t old_status = get_reg(gb::regNames::STAT);
            write(0xFF00 + gb::regNames::STAT, (old_status & ~(0b11)) | mode);
        }
        
    private:
        // Stores a pointer to the bus
        gb::bus* bus;
        
        // 2 vector of 10 OAM indicies to store which sprites are on the current scanline (max 10)
        std::vector<uint8_t> sprites;
        
        // Helper function to get the data for a sprite in OAM
        uint8_t read_sprite_data(uint8_t sprite_index, uint8_t byte_num){
            return read(0xFE00 + (4 * sprite_index) + byte_num);
        }

        
        // Function to draw one scnaline of graphics
        void draw_scanline(){
            // Do nothing if in vblank
            if (num_scanlines > 143)
                return;
            
            // Othwerise, fetch which sprites are on this scanline
            sprites.clear();
            
            for (int sprite_num = 0; sprite_num < 40; sprite_num++){
                // If the scanline number is between the sprite's y base and the sprite's y position - 16 then it is visible
                // Y position is sprite's top left corner + 16
                // Sprite height is read from LCDC bit 2 (0 = 8x8, 1 = 8x16)
                
                int dy = read_sprite_data(sprite_num, 0) - num_scanlines;
                int dy_min = gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 2) ? 0: 8;
                if (dy > dy_min and dy <= 16) {
                    // Add sprite to the vector of sprites
                    sprites.push_back(sprite_num);
                }
                
                // If there are 10 sprites, break from the loop
                if (sprites.size() >= 10)
                    goto end;
            }
        end:
            // Sorts the sprites if there are any
            if (sprites.size() > 0)
                sort_sprites();
        
            // Render all 160 pixels
            for (int x = 0; x < 160; x++){
                frame_buffer[num_scanlines][x] = draw_pixel(x);
            }
        }
    
        
        // Function which sorts sprite priorities on the same scalnine
        void sort_sprites(){
            // Repeats for the number of elements in sprites
            for (int i = 0; i < sprites.size(); i++) {
                // Finds the highest priority sprite after index i
                int min_value = 0x100;
                int min_index = 0;
                
                for (int j = i; j < sprites.size(); j++) {
                    // Sprites with smaller x coordinate come first
                    int current_value = read_sprite_data(sprites[j], 1);
                    
                    // If current value is less than min_value, set min_value to current_value and store current index
                    if (current_value < min_value) {
                        min_value = current_value;
                        min_index = j;
                    }
                }
                // Swaps it to index i
                uint8_t temp = sprites[i];
                sprites[i] = sprites[min_index];
                sprites[min_index] = temp;
            }
        }
        
        // Fucntion to render a single pixel at a given x position, using the stored scanline number
        uint8_t draw_pixel(int x){
            // Gets the values of the background, window and sprite pixels at this position
            uint8_t background_pixel = get_background_pixel_data(x, num_scanlines);
            int window_pixel = get_window_pixel_data(x, num_scanlines);
            auto [sprite_pixel, sprite_priority, sprite_pallette] = get_sprite_pixel_data(x, num_scanlines);
            
            // Return correct pixel based on priorities
            if (sprite_pixel == 0)
                // Return background/window pixel if sprite pixel is zero, ie transparent
                return get_background_pallette_data((window_pixel == -1) ? background_pixel : window_pixel);

            if (sprite_priority and background_pixel != 0)
                // If sprite priotity is 1, and background pixel is not 0, return background/window pxiel
                return get_background_pallette_data((window_pixel == -1) ? background_pixel : window_pixel);
            
            // Othwerise, return sprite pixel
            return get_sprite_pallette_data(sprite_pixel, sprite_pallette);
        }
        
        // Gets the value of the sprite pixel at this location (0 is no sprite or transparent), its BG priority and its pallette
        std::tuple<uint8_t, bool, bool> get_sprite_pixel_data(int x, int y){
            // If sprites are disabled (LCDC bit 1 = 0), then return a transparent pixel (value 0)
            if (gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 1) == 0)
                return std::make_tuple(0, 0, 0);
            
            // Loops through all sprites on this scanline in priprity order
            for (auto sprite_index:sprites){
                // Checks if current x is between sprite x and sprite x - 8
                int dx = read_sprite_data(sprite_index, 1) - x;
                
                if (dx > 0 and dx <= 8){
                    // If this sprite is visible...
                    int sprite_height = gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 2) ? 16 : 8;
                    uint8_t flags_byte = read_sprite_data(sprite_index, 3);
                    
                    // Calculates the position within the tile to use
                    int fine_x = x - (read_sprite_data(sprite_index, 1) - 8);
                    int fine_y = y - (read_sprite_data(sprite_index, 0) - 16);
                    
                    // Flips y if flags bit 6 is set, and x if flags bit 5 is set
                    fine_x = gb::Utils::get_bit(flags_byte, 5) ? 7 - fine_x : fine_x;
                    fine_y = gb::Utils::get_bit(flags_byte, 6) ? (sprite_height - 1) - fine_y : fine_y;
                    
                    // 8x8 sprites
                    if (sprite_height == 8) {
                        // Get the tile data for the sprite
                        uint8_t tile_index = read_sprite_data(sprite_index, 2);
                        get_tile_data(tile_index);
                        
                    // 8x16 sprites
                    } else if (sprite_height == 16) {
                        //Get the tile data
                        uint8_t tile_index = read_sprite_data(sprite_index, 2) & 0xFE;
                        get_tile_data(fine_y > 7 ? tile_index + 1 : tile_index);
                    }
                    
                    uint8_t pixel_data = current_tile[8 * (fine_y % 8) + fine_x];
                    
                    // If the pixel is not 0 (ie not transparent)...
                    if (pixel_data != 0) {
                        // Returns pixel at that location, along with priority and pallette
                        return std::make_tuple(pixel_data, gb::Utils::get_bit(flags_byte, 7), gb::Utils::get_bit(flags_byte, 4));
                    }
                }
            }
            
            // If no non-transparent sprite pixels were found, returns 0, 0, ,0
            return std::make_tuple(0, 0, 0);
        }
        
        // Gets the value of the background pixel at this location
        uint8_t get_background_pixel_data(int x, int y){
            // If background rendering is disabled (LCDC bit 0 = 0), then return 0
            if (gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 0) == 0)
                return 0;
            
            // Calculates tile position and fine position (position within tile from 0-7)
            int scrolled_x = (x + get_reg(gb::regNames::SCX)) % 256;
            int scrolled_y = (y + get_reg(gb::regNames::SCY)) % 256;
            
            int tile_x = scrolled_x / 8; int tile_y = scrolled_y / 8;
            int fine_x = scrolled_x % 8; int fine_y = scrolled_y % 8;
            
            // Gets the tile index from the tile table in VRAM (9800 - 9BFF or 9C00 - 9FFF depending on LCDC bit 3)
            uint16_t nametable_base = gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 3) ? 0x9C00 : 0x9800;
            uint8_t tile_index = read(nametable_base + tile_x + 32 * tile_y);
            
            // Calculates the address of that tile
            uint16_t tile_addr = get_address_of_background_tile(tile_index);
            // uint16_t tile_addr = 0x8000 + 16 * tile_index;
            
            // Gets the two bytes for the current pixel
            uint8_t pixel_low = read(tile_addr + 2 * fine_y);
            uint8_t pixel_high = read(tile_addr + 2 * fine_y + 1);
            
            // Gets the actual pixel data
            uint8_t pixel_data = gb::Utils::get_bit(pixel_low, (7 - fine_x)) + 2 * gb::Utils::get_bit(pixel_high, (7 - fine_x));
            return pixel_data;
        }
        
        // Gets the value of the window pixel at this location
        int get_window_pixel_data(int x, int y){
            // If background/window rendering is disabled (LCDC bit 0 = 0), then return -1 for no pixel
            if (gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 0) == 0)
                return -1;
            
            // If window rendering is disabled (LCDC bit 5 == 0), then return -1 for no pixel
            if (gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 5) == 0)
                return -1;
            
            // If this pixel is not in the window display area, return -1 for no pixel
            if (x < (get_reg(gb::regNames::WX) - 7) or y < get_reg(gb::regNames::WY))
                return -1;
            
            // Calculates tile position and fine position (position within tile from 0-7)
            int scrolled_x = (x + 7 - get_reg(gb::regNames::WX));
            int scrolled_y = (y - get_reg(gb::regNames::WY));
            
            int tile_x = scrolled_x / 8; int tile_y = scrolled_y / 8;
            int fine_x = scrolled_x % 8; int fine_y = scrolled_y % 8;
            
            // Gets the tile index from the tile table in VRAM (9800 - 9BFF or 9C00 - 9FFF depending on LCDC bit 6)
            uint16_t nametable_base = gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 6) ? 0x9C00 : 0x9800;
            uint8_t tile_index = read(nametable_base + tile_x + 32 * tile_y);
            
            // Calculates the address of that tile
            uint16_t tile_addr = get_address_of_background_tile(tile_index);
            
            // Gets the two bytes for the current pixel
            uint8_t pixel_low = read(tile_addr + 2 * fine_y);
            uint8_t pixel_high = read(tile_addr + 2 * fine_y + 1);
            
            // Gets the actual pixel data
            uint8_t pixel_data = gb::Utils::get_bit(pixel_low, (7 - fine_x)) + 2 * gb::Utils::get_bit(pixel_high, (7 - fine_x));
            return pixel_data;
        }
        
        // Function to translate a 1 byte tile address into an index into VRAM
        uint16_t get_address_of_background_tile(uint8_t tile_index) {
            if (gb::Utils::get_bit(get_reg(gb::regNames::LCDC), 4))
                // If LCDC bit 4 is 1, tiles are read from 0x8000 - each tile is 16 bits
                return 0x8000 + 16 * tile_index;
            else
                // IF LCDC bit 4 is 0, indicies 0-127 are read from 0x9000, and indicies 128-255 are read from 0x8800 - each tile is 16 bits
                return (tile_index > 0x7F) ? 0x8000 + 16 * tile_index : 0x9000 + 16 * tile_index;
        }
        
        // Basic functions to read/write data to/from the bus
        void write(uint16_t addr, uint8_t data){
            bus->write(addr, data);
        }
        
        uint8_t read(uint16_t addr){
            return bus->read(addr);
        }
        // Basic function to get the contents of an io register - faster than reading
        uint8_t get_reg(uint8_t reg_num){
            return bus->get_ioreg(reg_num);
        }
    };
}

#endif /* ppu_h */
