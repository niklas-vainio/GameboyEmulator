// Created by Niklas on 25/03/2020.
// Class to control drawing graphics to the screen

#ifndef ViewController_h
#define ViewController_h

#include <string>
#include <SFML/Graphics.hpp>
#include "Utils.h"
#include "RegNames.h"

#include "ppu.h"
#include "cpu.h"
#include "ppu.h"
#include "apu.h"

class ViewController {
private:
    // Stores a window to draw to
    sf::RenderWindow* window;
    
    // Stores the ppu which creates frame data
    gb::ppu* ppu;
    
    // Stores the pallette to convert gameboy color values into actual colors
    // 3 is darkest, 0 is lightest
    // Greyscale pallette:
    // sf::Color master_pallette[4] = {sf::Color(255, 255, 255), sf::Color(170, 170, 170), sf::Color(85, 85, 85),  sf::Color(0, 0, 0)};
    //Green pallette:
    // sf::Color master_pallette[4] = {sf::Color(145, 193, 0), sf::Color(129, 177, 0), sf::Color(1, 101, 45),  sf::Color(0, 55, 2)};
    // SGB Green pallette
    sf::Color master_pallette[4] = {sf::Color(217, 252, 205), sf::Color(117, 198, 104), sf::Color(30, 107, 86),  sf::Color(4, 24, 33)};

public:
    // Constructor binds a window and a ppu
    ViewController(sf::RenderWindow* _window, gb::ppu* _ppu){
        window = _window;
        ppu = _ppu;
    }
    
    // Function to draw a frame in cinematic mode
    void draw_cinematic_frame(sf::View& cinematic_view, sf::VertexArray& screen, gb::bus& bus){
        window->setSize(sf::Vector2u(960, 864));
        window->setView(cinematic_view);
        draw_screen(screen, bus);
    }
    
    
    // Wrapper for the window.draw() method for this class
    void draw(sf::Drawable& object){
        window->draw(object);
    }
    
    // Draws text to the window
    void draw_text(std::string text, const sf::Color& color, const sf::Font& font, float size, float x, float y){
        sf::Text text_obj(text, font, size);
        text_obj.setFillColor(color);
        text_obj.setPosition(x, y);
        draw(text_obj);
    }
    
    // Draws a rectangle to the window
    void draw_rect(const sf::Color& color, float width, float height, float x, float y) {
        sf::RectangleShape rect;
        rect.setFillColor(color);
        rect.setPosition(x, y);
        rect.setSize(sf::Vector2f(width, height));
        draw(rect);
    }
    
    // Draws the state of the cpu registers
    void draw_cpu_state(gb::cpu& cpu,  const sf::Font& font, float x, float y){
        draw_text("CPU Status", sf::Color::Red, font, 30, x, y);
        draw_text("AF " + gb::Utils::hex_short(cpu.AF()) + "   " + gb::Utils::bin_short(cpu.AF()), sf::Color::White, font, 15, x, y + 40);
        draw_text("BC " + gb::Utils::hex_short(cpu.BC()) + "   " + gb::Utils::bin_short(cpu.BC()), sf::Color::White, font, 15, x, y + 60);
        draw_text("DE " + gb::Utils::hex_short(cpu.DE()) + "   " + gb::Utils::bin_short(cpu.DE()), sf::Color::White, font, 15, x, y + 80);
        draw_text("HL " + gb::Utils::hex_short(cpu.HL()) + "   " + gb::Utils::bin_short(cpu.HL()), sf::Color::White, font, 15, x, y + 100);
    
        draw_text("PC " + gb::Utils::hex_short(cpu.PC), sf::Color::White, font, 15, x, y + 130);
        draw_text("SP " + gb::Utils::hex_short(cpu.SP), sf::Color::White, font, 15, x, y + 150);
        
        draw_text("INSTR " + gb::Utils::hex_byte(cpu.read(cpu.PC)), sf::Color::White, font, 15, x + 100, y + 130);
        draw_text(gb::Utils::bin_byte(cpu.read(cpu.PC)), sf::Color::White, font, 15, x + 200, y + 130);
        
        draw_text("Flags", sf::Color::White, font, 15, x + 100, y + 150);
        
        draw_text("Z", cpu.get_Z() ? sf::Color::Green : sf::Color::Red, font, 15, x + 180, y + 150);
        draw_text("N", cpu.get_N() ? sf::Color::Green : sf::Color::Red, font, 15, x + 200, y + 150);
        draw_text("H", cpu.get_H() ? sf::Color::Green : sf::Color::Red, font, 15, x + 220, y + 150);
        draw_text("C", cpu.get_C() ? sf::Color::Green : sf::Color::Red, font, 15, x + 240, y + 150);
        
        //draw_text("CYCLES " + std::to_string(cpu.cycles), sf::Color::White, font, 15, x, y + 190);
        draw_text(cpu.current_instr, sf::Color::Blue, font, 20, x, y + 220);
        
        draw_text("IE " + gb::Utils::bin_byte(cpu.read(0xFFFF)), sf::Color::White, font, 15, x, y + 170);
        draw_text("IME", cpu.IME ? sf::Color::Green : sf::Color::Red, font, 15, x + 150, y + 170);
        draw_text("IF " + gb::Utils::bin_byte(cpu.read(0xFF0F)), sf::Color::White, font, 15, x, y + 190);
    }
    
    // Draws the state of the ppu and its registers
    void draw_ppu_state(gb::bus& bus, const sf::Font& font, float x, float y){
        draw_text("PPU Status", sf::Color::Red, font, 30, x, y);
        draw_text("Mode " + std::to_string(ppu->mode), sf::Color::White, font, 15, x, y + 40);
        draw_text("Clocks " + std::to_string(ppu->num_cycles), sf::Color::White, font, 15, x, y + 60);
        draw_text("Scanlines " + std::to_string(ppu->num_scanlines), sf::Color::White, font, 15, x, y + 80);
        
        draw_text("BGP ", sf::Color::White, font, 15, x + 150, y + 40);
        for (int i = 0; i < 4; i++) {
            draw_rect(master_pallette[ppu->get_background_pallette_data(i)], 15, 15, x + 210 + i * 20, y + 40);
        }
        
        draw_text("OBP0 ", sf::Color::White, font, 15, x + 150, y + 60);
        for (int i = 1; i < 4; i++) {
            draw_rect(master_pallette[ppu->get_sprite_pallette_data(i, 0)], 15, 15, x + 210 + i * 20, y + 60);
        }
        
        draw_text("OBP1 ", sf::Color::White, font, 15, x + 150, y + 80);
        for (int i = 1; i < 4; i++) {
            draw_rect(master_pallette[ppu->get_sprite_pallette_data(i, 1)], 15, 15, x + 210 + i * 20, y + 80);
        }
        
        draw_text("SCX " + gb::Utils::hex_byte(bus.read(0xFF00 + gb::regNames::SCX)), sf::Color::White, font, 15, x, y + 110);
        draw_text("SCY " + gb::Utils::hex_byte(bus.read(0xFF00 + gb::regNames::SCY)), sf::Color::White, font, 15, x, y + 130);
        draw_text("WX " + gb::Utils::hex_byte(bus.read(0xFF00 + gb::regNames::WX)), sf::Color::White, font, 15, x + 100, y + 110);
        draw_text("WY " + gb::Utils::hex_byte(bus.read(0xFF00 + gb::regNames::WY)), sf::Color::White, font, 15, x + 100, y + 130);
        draw_text("LY " + gb::Utils::hex_byte(bus.read(0xFF00 + gb::regNames::LY)), sf::Color::White, font, 15, x + 200, y + 110);
        draw_text("LYC " + gb::Utils::hex_byte(bus.read(0xFF00 + gb::regNames::LYC)), sf::Color::White, font, 15, x + 200, y + 130);
        draw_text("LCDC " + gb::Utils::bin_byte(bus.read(0xFF00 + gb::regNames::LCDC)), sf::Color::White, font, 15, x + 300, y + 110);
        draw_text("STAT " + gb::Utils::bin_byte(bus.read(0xFF00 + gb::regNames::STAT)), sf::Color::White, font, 15, x + 300, y + 130);
    }
    
    // Draws a visual representation of memory to the screen
    void draw_memory(sf::VertexArray& vertices, gb::bus& bus, gb::cpu& cpu, const sf::Font& font){
        // Draw whether the bios is enabled
        if (bus.bios_enabled)
            draw_text("<- BIOS", sf::Color::White, font, 15, 1265, 30);
        
        for (int i = 0; i <= 0xFFFF; i++){
            uint8_t data = bus.read(i);
            // std::cout << gb::Utils::hex_short(i) << ": " << gb::Utils::hex_byte(data) << std::endl;
            vertices[i].color = sf::Color(data, data, data);
        }
        
        // Highlight PC in red, SP in blue and HL in green
        vertices[cpu.PC].color = sf::Color::Red;
        vertices[cpu.SP].color = sf::Color::Blue;
        vertices[cpu.HL()].color = sf::Color::Green;
        
        draw(vertices);
        
        // Draw the name of the current mapper
        draw_text("Mapper   " + bus.mapper_names[bus.mapper_id], sf::Color::White, font, 15, 1000, 300);
    }
    
    // Draws the ppu tiles onto the screen
    void draw_ppu_tiles(float x, float y){
        // Loops through all 384 tiles
        for (int tile_num = 0; tile_num < 384; tile_num++){
            // Directs ppu to read tile data
            ppu->get_tile_data(tile_num);
            
            // Calcultes the starting position of this tile
            float start_x = x + 8 * (tile_num % 16);
            float start_y = y + 1 + 8 * (tile_num / 16);
            
            // Creates a vertex array to draw tile data
            sf::VertexArray vertices(sf::Points, 64);
            
            // Sets the positions and colors of the vertices
            for (int i = 0; i < 64; i++){
                vertices[i].position = sf::Vector2f(start_x + (i % 8), start_y + (i / 8));
                vertices[i].color = master_pallette[ppu->get_background_pallette_data(ppu->current_tile[i])];
            }
            draw(vertices);
        }
    }
    
    // Draws the actual gameboy screen
    void draw_screen(sf::VertexArray& screen_verts, gb::bus& bus){
        // Gets whether LCD is enabled from LCDC bit 7
        bool LCD_enabled = gb::Utils::get_bit(bus.read(0xFF40), 7);
        
        for (int x = 0; x < 160; x++){
            for(int y = 0; y < 144; y++){
                int index = 4 * (y * 160 + x);
                // If screen is disabled, draw all zeroes
                sf::Color color = LCD_enabled ? master_pallette[ppu->frame_buffer[y][x]] : master_pallette[0];
                
                //sf::Color color = sf::Color::Green;
                screen_verts[index + 0].color = color;
                screen_verts[index + 1].color = color;
                screen_verts[index + 2].color = color;
                screen_verts[index + 3].color = color;
            }
        }
        draw(screen_verts);
        
        // Draws a red screen to show the current scanline
        sf::RectangleShape scanline_indicator;
        scanline_indicator.setPosition(640, ppu->num_scanlines * 4);
        scanline_indicator.setSize(sf::Vector2f(4, 4));
        scanline_indicator.setFillColor(sf::Color::Red);
        draw(scanline_indicator);
    }
    
    // Draws the states of the inputs
    void draw_inputs(bool a, bool b, bool up, bool down, bool left, bool right, bool start, bool select,
                     gb::bus& bus, const sf::Font& font, float x, float y){
        draw_text("A", a ? sf::Color::Green : sf::Color::Red, font, 15, x, y);
        draw_text("B", b ? sf::Color::Green : sf::Color::Red, font, 15, x + 20, y);
        draw_text("Start", start ? sf::Color::Green : sf::Color::Red, font, 15, x , y + 20);
        draw_text("Select", select ? sf::Color::Green : sf::Color::Red, font, 15, x, y + 40);
        
        draw_rect(up ? sf::Color::Green : sf::Color::Red, 15, 15, x + 90, y + 10);
        draw_rect(down ? sf::Color::Green : sf::Color::Red, 15, 15, x + 90, y + 40);
        draw_rect(left ? sf::Color::Green : sf::Color::Red, 15, 15, x + 75, y + 25);
        draw_rect(right ? sf::Color::Green : sf::Color::Red, 15, 15, x + 105, y + 25);
        
        draw_text("P1 " + gb::Utils::bin_byte(bus.read(0xFF00)), sf::Color::White, font, 15, x, y + 60);
    }
    
    // Draws the states of sprites in OAM
    void draw_oam_state(gb::bus& bus, const sf::Font& font, float x, float y){
        // Draws header
        draw_text("#  Y X TILE FLAGS", sf::Color::Blue, font, 15, x, y);
        draw_text("#  Y X TILE FLAGS", sf::Color::Blue, font, 15, x + 220, y);
        
        // Draws all tiles, with tiles after 20 being offset right by 220 pixels
        for (int i = 0; i < 40; i++){
            // Reads data from OAM
            uint16_t base_addr = 0xFE00 + 4 * i;
            std::string text = gb::Utils::hex_byte(bus.read(base_addr)) + " " + gb::Utils::hex_byte(bus.read(base_addr + 1)) + "    " + gb::Utils::bin_byte(bus.read(base_addr + 3));
            
            float draw_x = i > 19 ? x + 220 : x;
            float draw_y = i > 19 ? y -240 + 13 * i : y + 20 + 13 * i;
            draw_text(std::to_string(i), sf::Color::Red, font, 10, draw_x, draw_y);
            draw_text(text, sf::Color::White, font, 10, draw_x + 30, draw_y);
            

            // Draws the tile for this sprite, or 2 tiles if 8x16 sprites are enabled
            bool tall_sprites = gb::Utils::get_bit(bus.read(0xFF00 + gb::regNames::LCDC), 2);
            
            sf::VertexArray vertices(sf::Points, 64);
            ppu->get_tile_data(bus.read(base_addr + 2) & (tall_sprites ? 0xFE : 0xFF));
            bool pallette_select = gb::Utils::get_bit(bus.read(base_addr + 3), 4);
            
            // If sprites are tall, offsets all even tiles 5 pixels left, and all odd tiles 5 pixels right
            int offset = tall_sprites ? ((i % 2 == 0) ? -5 : 5) : 0;
            
            // Sets the positions and colors of the vertices
            for (int j = 0; j < 64; j++){
                vertices[j].position = sf::Vector2f(draw_x + 85 + (j % 8) + offset, draw_y + (j / 8) + 3);
                vertices[j].color = master_pallette[ppu->get_sprite_pallette_data(ppu->current_tile[j], pallette_select)];
            }
            draw(vertices);
            
            if (tall_sprites) {
                ppu->get_tile_data(bus.read(base_addr + 2) | 0x01);
                // Draw second tile if 16 x 8 sprites are enabled
                for (int j = 0; j < 64; j++){
                    vertices[j].position = sf::Vector2f(draw_x + 85 + (j % 8) + offset, draw_y + (j / 8) + 11);
                    vertices[j].color = master_pallette[ppu->get_sprite_pallette_data(ppu->current_tile[j], pallette_select)];
                }
                draw(vertices);
            }
        }
    }
    
    // Draws the states of the timer registers
    void draw_timer_states(gb::bus& bus, const sf::Font& font, float x, float y) {
        draw_text("DIV " + gb::Utils::hex_byte(bus.read(0xFF04)), sf::Color::White, font, 15, x, y);
        draw_text("TIMA " + gb::Utils::hex_byte(bus.read(0xFF05)), sf::Color::White, font, 15, x, y + 20);
        draw_text("TMA " + gb::Utils::hex_byte(bus.read(0xFF06)), sf::Color::White, font, 15, x, y + 40);
        draw_text("TAC " + gb::Utils::bin_byte(bus.read(0xFF07)), sf::Color::White, font, 15, x, y + 60);
    }
    
    // Draws the state of the APU
    void draw_apu_state(gb::bus& bus, gb::apu& apu, const sf::Font& font, float x, float y) {
        draw_text("NR10 " + gb::Utils::bin_byte(bus.read(0xFF10)), sf::Color::White, font, 15, x, y);
        draw_text("NR11 " + gb::Utils::bin_byte(bus.read(0xFF11)), sf::Color::White, font, 15, x, y + 20);
        draw_text("NR12 " + gb::Utils::bin_byte(bus.read(0xFF12)), sf::Color::White, font, 15, x, y + 40);
        draw_text("NR13 " + gb::Utils::bin_byte(bus.read(0xFF13)), sf::Color::White, font, 15, x, y + 60);
        draw_text("NR14 " + gb::Utils::bin_byte(bus.read(0xFF14)), sf::Color::White, font, 15, x, y + 80);
        draw_text("Playing", apu.square_wave_1_playing ? sf::Color::Green : sf::Color::Red, font, 15, x + 170, y);
        draw_text("Vol " + std::to_string(apu.square_wave_1_env.current_volume), sf::Color::White, font, 15, x + 170, y + 20);
        
        draw_text("NR21 " + gb::Utils::bin_byte(bus.read(0xFF16)), sf::Color::White, font, 15, x, y + 120);
        draw_text("NR22 " + gb::Utils::bin_byte(bus.read(0xFF17)), sf::Color::White, font, 15, x, y + 140);
        draw_text("NR23 " + gb::Utils::bin_byte(bus.read(0xFF18)), sf::Color::White, font, 15, x, y + 160);
        draw_text("NR24 " + gb::Utils::bin_byte(bus.read(0xFF19)), sf::Color::White, font, 15, x, y + 180);
        draw_text("Playing", apu.square_wave_2_playing ? sf::Color::Green : sf::Color::Red, font, 15, x + 170, y + 120);
        draw_text("Vol " + std::to_string(apu.square_wave_2_env.current_volume), sf::Color::White, font, 15, x + 170, y + 140);
        
        draw_text("NR30 " + gb::Utils::bin_byte(bus.read(0xFF1A)), sf::Color::White, font, 15, x + 250, y);
        draw_text("NR31 " + gb::Utils::bin_byte(bus.read(0xFF1B)), sf::Color::White, font, 15, x + 250, y + 20);
        draw_text("NR32 " + gb::Utils::bin_byte(bus.read(0xFF1C)), sf::Color::White, font, 15, x + 250, y + 40);
        draw_text("NR33 " + gb::Utils::bin_byte(bus.read(0xFF1D)), sf::Color::White, font, 15, x + 250, y + 60);
        draw_text("NR34 " + gb::Utils::bin_byte(bus.read(0xFF1E)), sf::Color::White, font, 15, x + 250, y + 80);
        draw_text("Enabled", gb::Utils::get_bit(bus.read(0xFF1A), 7) ? sf::Color::Green : sf::Color::Red, font, 15, x + 420, y);
        draw_text("Playing", apu.wave_playing ? sf::Color::Green : sf::Color::Red, font, 15, x + 420, y + 20);
        std::string wave_data;
        for (int i = 0; i < 0x10; i++)
            wave_data += gb::Utils::hex_byte(bus.read(0xFF30 + i)) + " ";
        draw_text("WAVE " + wave_data, sf::Color::White, font, 12, x + 250, y + 100);
        
        //draw_text("Playing", apu.square_wave_2_playing ? sf::Color::Green : sf::Color::Red, font, 15, x + 170, y + 120);
        //draw_text("Vol " + std::to_string(apu.square_wave_2_env.current_volume), sf::Color::White, font, 15, x + 170, y + 140);
        
        draw_text("NR41 " + gb::Utils::bin_byte(bus.read(0xFF20)), sf::Color::White, font, 15, x + 340, y + 120);
        draw_text("NR42 " + gb::Utils::bin_byte(bus.read(0xFF21)), sf::Color::White, font, 15, x + 340, y + 140);
        draw_text("NR43 " + gb::Utils::bin_byte(bus.read(0xFF22)), sf::Color::White, font, 15, x + 340, y + 160);
        draw_text("NR44 " + gb::Utils::bin_byte(bus.read(0xFF23)), sf::Color::White, font, 15, x + 340, y + 180);
        draw_text("Playing", apu.noise_playing ? sf::Color::Green : sf::Color::Red, font, 15, x + 510, y + 120);
        draw_text("Vol " + std::to_string(apu.noise_env.current_volume), sf::Color::White, font, 15, x + 510, y + 140);
        
        draw_text("NR50 " + gb::Utils::bin_byte(bus.read(0xFF24)), sf::Color::White, font, 15, x + 0, y + 210);
        draw_text("NR51 " + gb::Utils::bin_byte(bus.read(0xFF25)), sf::Color::White, font, 15, x + 170, y + 210);
        draw_text("NR52 " + gb::Utils::bin_byte(bus.read(0xFF26)), sf::Color::White, font, 15, x + 340, y + 210);
    }
};

#endif /* ViewController_h */
