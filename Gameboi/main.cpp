// Gameboi By Niklas Vainio - Started on 25/3/20, completed on ???
// This is designed to emulate simple gameboy games

// SFML Libraries
#include <SFML/Graphics.hpp>
#include <string.h>
#include <iostream>

// Custom includes
#include "ViewController.h"
#include "EmulationController.h"

#define SCREEN_SCALE 4

using namespace sf;
int main(int, const char **) {
    // Initialisation --------------------------------------------------------------------
    srand(time(NULL));
    // Sotres paths to files folder and roms folder
    const std::string filepath = "/Users/niklas/Desktop/PROGRAMMING/C++/Gameboi/Files/";
    const std::string romspath = "/Users/niklas/Desktop/PROGRAMMING/C++/Gameboi/Roms/";
    const std::string savespath = "/Users/niklas/Desktop/PROGRAMMING/C++/Gameboi/Saves/";

    // Constants to map the gameboy buttons (a, b, up, down, left, right, start, select) to keyboard keys
    const Keyboard::Key a_key = Keyboard::Key::X;
    const Keyboard::Key b_key = Keyboard::Key::Z;
    const Keyboard::Key up_key = Keyboard::Key::Up;
    const Keyboard::Key down_key = Keyboard::Key::Down;
    const Keyboard::Key left_key = Keyboard::Key::Left;
    const Keyboard::Key right_key = Keyboard::Key::Right;
    const Keyboard::Key start_key = Keyboard::Key::Enter;
    const Keyboard::Key select_key = Keyboard::Key::Tab;
    
    // Array to store all key values for quick looping
    const Keyboard::Key all_keys[8] = {a_key, b_key, up_key, down_key, left_key, right_key, start_key, select_key};
    
    // Keeps track of whether the cpu is executing
    bool executing = true;
    bool do_once = false;
    bool do_one_scanline = false;
    bool do_one_frame = false;
    
    RenderWindow window(VideoMode(1360, 850), "SFML window", Style::Close);
    //window.setFramerateLimit(60);
    window.setVerticalSyncEnabled(true);

    // Keeps track of whether emulator is in cinematic mode
    bool cinematic_mode = true;
    
    // Creates a view for cinematic mode
    View cinematic_view(Vector2f(318, 286), Vector2f(641, 577));
    cinematic_view.setViewport(FloatRect(0, 0, 1, 1));
    
    // Creates a default camera
    View default_view(Vector2f(680, 425), Vector2f(1360, 850));
    
    // Stores background color
    const Color bg_color(0, 0, 50);
    
    // Initialise main hardware components (cpu, bus etc) and passes them into the emulation controller
    gb::cpu cpu; gb::ppu ppu; gb::bus bus; gb::apu apu;
    EmulationController emulator(&cpu, &bus, &ppu, &apu, romspath, savespath);
    
    emulator.init(filepath + "bios.bin");
    emulator.load_rom("tetris");

    // Initialise the main view controller with a reference to the window
    ViewController view(&window, &ppu);
    
    // Creates a vertex array for visualising memory contents
    float visualiser_x = 1000;
    float visualiser_y = 40;
    VertexArray memory_visualisation(Points, 256 * 256);
    for (int x = 0; x < 256; x++){
        for (int y = 0; y < 256; y++){
            int index = y * 256 + x;
            memory_visualisation[index].position = Vector2f(visualiser_x + x, visualiser_y + y);
        }
    }
    
    // Creates a vertex array for drawing the actual screen
    VertexArray screen(Quads, 4 * 160 * 144);
    for (int x = 0; x < 160; x++){
        for (int y = 0; y < 144; y++){
            int index = 4 * (y * 160 + x);
            screen[index + 0].position = Vector2f(SCREEN_SCALE * x, SCREEN_SCALE * (y + 1));
            screen[index + 1].position = Vector2f(SCREEN_SCALE * x, SCREEN_SCALE * y);
            screen[index + 2].position = Vector2f(SCREEN_SCALE * (x + 1), SCREEN_SCALE * y);
            screen[index + 3].position = Vector2f(SCREEN_SCALE * (x + 1), SCREEN_SCALE * (y + 1));
        }
    }

    // Set the Icon
    Image icon;
    if (!icon.loadFromFile(filepath + "icon.png")) {
        return EXIT_FAILURE;
    }
    window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
    window.setTitle("Gameboi Emulator");

    // Loads arcade classic font
    Font font;
    if (!font.loadFromFile(filepath + "Pixel_NES.otf")) {
        return EXIT_FAILURE;
    }
    
    // Main Game Loop
    while (window.isOpen())
    {
        // Event handling ----------------------------------------------------------------
        Event event;
        while (window.pollEvent(event))
        {
            // Any key pressed: check if it is one of the buttons and if so, cpu exits from stopped state
            if (event.type == Event::KeyPressed){
                for (Keyboard::Key i: all_keys){
                    if (event.key.code == i)
                        cpu.stopped = false;
                }
            }
            
            // Close window: save battery-backed ram and exit
            if (event.type == Event::Closed) {
                apu.stop_all();
                bus.close();
                window.close();
            }
            
            // Space pressed: toggle cpu executing
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space){
                executing = !executing;
            }
            
            // Enter: set do_once to true
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::I){
                do_once = true;
            }
            
            // S: set do_one scanline to true
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::S){
                do_one_scanline = true;
            }
            
            // F: set do_one_frame to true
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::F){
                do_one_frame = true;
            }
            
            // C: toggle cinematic mode
            if (event.type == Event::KeyPressed && event.key.code == Keyboard::C){
                cinematic_mode = !cinematic_mode;
            }
        }
        // Sends the states of pressed keys to the bus
        bus.store_key_states(Keyboard::isKeyPressed(a_key), Keyboard::isKeyPressed(b_key),
                             Keyboard::isKeyPressed(up_key), Keyboard::isKeyPressed(down_key),
                             Keyboard::isKeyPressed(left_key), Keyboard::isKeyPressed(right_key),
                             Keyboard::isKeyPressed(start_key), Keyboard::isKeyPressed(select_key));
        
        // Processing -------------------------------------------------------------------
        if (executing or do_one_frame) {
            // Emulate one frame
            if (emulator.emulate_frame())
                executing = false;
            
            do_one_frame = false;
            
        } else if (do_one_scanline){
            // Emulate one scanline
            emulator.emulate_scanline();
            do_one_scanline = false;
            
        } else if (do_once){
            // Emulate one instruction
            emulator.emulate_instruction();
            do_once = false;
        }
        
        // Drawing ----------------------------------------------------------------------
        window.clear(bg_color);
        
        // If in cinematic mode, set window size to 960 * 864 and only draw screen
        if (cinematic_mode){
            view.draw_cinematic_frame(cinematic_view, screen, bus);
        } else {
            // Otherwise, set window size to 1360 * 850 and draw all debug info
            window.setSize(Vector2u(1360, 850));
            window.setView(default_view);
            
            view.draw_text("Inputs", Color::Red, font, 30, 650, 500);
            view.draw_inputs(Keyboard::isKeyPressed(a_key), Keyboard::isKeyPressed(b_key),
                             Keyboard::isKeyPressed(up_key), Keyboard::isKeyPressed(down_key),
                             Keyboard::isKeyPressed(left_key), Keyboard::isKeyPressed(right_key),
                             Keyboard::isKeyPressed(start_key), Keyboard::isKeyPressed(select_key),
                             bus, font, 650, 540);
            
            view.draw_cpu_state(cpu, font, 650, 0);
            view.draw_ppu_state(bus, font, 900, 330);
            
            view.draw_text("Memory Contents", Color::Red, font, 30, visualiser_x, visualiser_y - 40);
            view.draw_memory(memory_visualisation, bus, cpu, font);
            
            view.draw_text("Tile Data", Color::Red, font, 30, 650, 260);
            view.draw_ppu_tiles(650, 300);
            
            view.draw_text("OAM Data", Color::Red, font, 30, 900, 480);
            view.draw_oam_state(bus, font, 900, 520);
            
            view.draw_text("Timers", Color::Red, font, 30, 650, 620);
            view.draw_timer_states(bus, font, 650, 660);
            
            view.draw_text("APU Status", Color::Red, font, 30, 10, 580);
            view.draw_apu_state(bus, apu, font, 10, 620);
            
            view.draw_screen(screen, bus);
        }
        // End of Frame Code ------------------------------------------------------------
        window.display();
    }

    return EXIT_SUCCESS;
}
