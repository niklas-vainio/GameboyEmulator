// Created by Niklas on 15/04/2020.
// Class which handles plaing the gameboy's audio

#ifndef apu_h
#define apu_h

#include <SFML/Audio.hpp>
#include <vector>
#include <cmath>
#include <iostream>

#include "RegNames.h"
#include "Utils.h"
#include "bus.h"

#include "Envelope.h"
#include "FreqSweep.h"
#include "NoiseChannel.h"
#include "WaveChannel.h"
#include "SquareChannel.h"

#define SAMPLE_RATE 44100
#define TWO_PI 6.28318

namespace gb {
    class apu {
    public:
        // Stores whether each sound channel is playing
        bool square_wave_1_playing = false;
        bool square_wave_2_playing = false;
        bool wave_playing = false;
        bool noise_playing = false;
        
        // Volume envelopes for each channel
        gb::Envelope square_wave_1_env;
        gb::Envelope square_wave_2_env;
        gb::Envelope noise_env;

        // Constructor
        apu(){}
        
        // Destructor
        ~apu(){}
        
        // Function to store a pointer to the bus
        void connect_bus(gb::bus* _bus){
            bus = _bus;
        }

        // Function to initialise the apu
        void init() {
            square_wave_1.open(SAMPLE_RATE, 1);
            square_wave_1.setLoop(true);
            square_wave_1.play();
            
            square_wave_2.open(SAMPLE_RATE, 1);
            square_wave_2.setLoop(true);
            square_wave_2.play();
            
            wave_channel.open(SAMPLE_RATE, 1, bus);
            wave_channel.setLoop(true);
            //wave_channel.setPitch(10.0);
            wave_channel.play();
            
            noise_channel.open(SAMPLE_RATE, 1);
            noise_channel.setLoop(true);
            noise_channel.play();
            
            // Update all sound devices to load initial settings
            update_all();
        }
        
        // Function to stop all sounds
        void stop_all() {
            square_wave_1.stop();
            square_wave_2.stop();
            wave_channel.stop();
            noise_channel.stop();
        }
        
        // Function to run every cycle
        void do_cycle() {
            // Increment timers
            cycles_since_length_decrement += 4;
            cycles_since_envelope += 4;
            
            // Reset envelopes if flags are set
            if (bus->update_square1_envelope) {
                square_wave_1_env.reset(get_reg(gb::regNames::NR12));
                bus->update_square1_envelope = false;
            }
            if (bus->update_square2_envelope) {
                square_wave_2_env.reset(get_reg(gb::regNames::NR22));
                bus->update_square2_envelope = false;
            }
            if (bus->update_noise_envelope) {
                noise_env.reset(get_reg(gb::regNames::NR42));
                bus->update_noise_envelope = false;
            }
            
            // If 16384 cycles have elapsed, decrement length counters
            if (cycles_since_length_decrement >= 16384) {
                decrement_length_counters();
                cycles_since_length_decrement = 0;
            }
            
            // If 65536 cycles have elapsed, update envelopes
            if (cycles_since_envelope >= 65536) {
                update_envelopes();
                cycles_since_envelope = 0;
            }
            
            // Updates the volumes for each channel
            update_all_volumes();
            
            // If sound registers have been changed...
            if (bus->update_apu) {
                // Update all settings
                update_all();
                
                // Reset update flag
                bus->update_apu = false;
            }
        }
        
    private:
        // Stores a pointer to the bus
        gb::bus* bus;

        // Initialises SFML sound objects for each of the sound channels
        gb::SquareChannel square_wave_1;
        gb::SquareChannel square_wave_2;
        gb::WaveChannel wave_channel;
        gb::NoiseChannel noise_channel;

        // Stores the number of cycles since decrementing the various length counters
        long cycles_since_length_decrement = 0;
        long cycles_since_envelope = 0;
        
        // Stores a table of wave channel volume tables
        const double wave_channel_volumes[4] = {0.0, 100.0, 50.0, 25.0};
   
        // Keeps track of the number of updates (for debug/optimisation)
        long num_updates = 0;
        
        // Function to update the audio settings after the APU register contents change
        void update_all() {
            num_updates++;
            // DEBUG: std::cout << "UPDATE " << num_updates << std::endl;
            
            // If NR52 bit 7 is 0, mute all sound and return
            if ((get_reg(gb::regNames::NR52) & 0x80) == 0) {
                square_wave_1_playing = false;
                square_wave_2_playing = false;
                noise_playing = false;
                update_all_volumes();
                return;
            }
            
            // Update square wave channels
            update_square_channels();
            
            // Update wave channel
            update_wave_channel();
            
            // Update noise channel
            update_noise_channel();
            
            // Update all volumes
            update_all_volumes();
        }
        
        
        // Function to update all volumes
        void update_all_volumes() {
            if (square_wave_1_env.update)
                update_volume(square_wave_1, square_wave_1_env, square_wave_1_playing);
            if (square_wave_2_env.update)
                update_volume(square_wave_2, square_wave_2_env, square_wave_2_playing);
            if (noise_env.update)
                update_volume(noise_channel, noise_env, noise_playing);
            
            // Update wave channel volume
        }
        
        // Function to update square wave channels
        void update_square_channels() {
            // If a 1 is written to the highest bit of NR14, this bit is reset and square wave 1 is enabled
            if ((get_reg(gb::regNames::NR14) & 0x80) != 0) {
                write(0xFF00 + gb::regNames::NR14, get_reg(gb::regNames::NR14) & 0x7F);
                square_wave_1_playing = true;
                
            }
            
            // If a 1 is written to the highest bit of NR24, this bit is reset and square wave 2 is enabled
            if ((get_reg(gb::regNames::NR24) & 0x80) != 0) {
                write(0xFF00 + gb::regNames::NR24, get_reg(gb::regNames::NR24) & 0x7F);
                square_wave_2_playing = true;
            }
            
            // If length counter in NR11 is 0, and bit 6 of NR 14 is 1, square wave 1 is disabled
            if ((get_reg(gb::regNames::NR11) & 0b00111111) == 0 and gb::Utils::get_bit(get_reg(gb::regNames::NR14), 6)) {
                square_wave_1_playing = false;
                bus->update_square1_envelope = true;
            }
            
            // If length counter in NR21 is 0, and bit 6 of NR 24 is 1, square wave 1 is disabled
            if ((get_reg(gb::regNames::NR21) & 0b00111111) == 0 and gb::Utils::get_bit(get_reg(gb::regNames::NR24), 6)){
                square_wave_2_playing = false;
                bus->update_square2_envelope = true;
            }
            
            // Sets the duty cycles
            square_wave_1.duty_cycle = (get_reg(gb::regNames::NR11) & 0b11000000) >> 6;
            square_wave_2.duty_cycle = (get_reg(gb::regNames::NR21) & 0b11000000) >> 6;
            
            // Sets the frequencies
            uint16_t square_wave_1_freq_raw = ((get_reg(gb::regNames::NR14) & 0b111) << 8) | get_reg(gb::regNames::NR13);
            double square_wave_1_freq = 131072.0 / (2048.0 - square_wave_1_freq_raw);
            square_wave_1.setPitch(square_wave_1_freq / 100.0);
            
            uint16_t square_wave_2_freq_raw = ((get_reg(gb::regNames::NR24) & 0b111) << 8) | get_reg(gb::regNames::NR23);
            double square_wave_2_freq = 131072.0 / (2048.0 - square_wave_2_freq_raw);
            square_wave_2.setPitch(square_wave_2_freq / 100.0);
        }
        
        // Update wave channel
        void update_wave_channel() {
            // If a 1 is written to the highest bit of NR34, this bit is reset and wave channel is enabled
            if ((get_reg(gb::regNames::NR34) & 0x80) != 0) {
                write(0xFF00 + gb::regNames::NR34, get_reg(gb::regNames::NR34) & 0x7F);
                wave_playing = true;
            }
            
            // Updates volume
            int channel_volume_raw = (get_reg(gb::regNames::NR32) & 0b01100000) >> 5;
            double channel_volume = wave_channel_volumes[channel_volume_raw];
            
            wave_channel.setVolume(wave_playing ? channel_volume : 0.0);
            
            // Sets wave volume to 0 if disabled
            if (gb::Utils::get_bit(get_reg(gb::regNames::NR30), 7) == 0)
                wave_channel.setVolume(0);
            
            // Sets the shift volume value to bits 6-5 of NR32
            
            // Sets the frequency
            uint16_t wave_freq_raw = ((get_reg(gb::regNames::NR34) & 0b111) << 8) | get_reg(gb::regNames::NR33);
            double wave_freq = 65536.0 / (2048.0 - wave_freq_raw);
            wave_channel.setPitch(wave_freq / 100.0);
        }
        
        // Update noise channels
        void update_noise_channel() {
            // If a 1 is written to the highest bit of NR44, this bit is reset and noise is enabled
            if ((get_reg(gb::regNames::NR44) & 0x80) != 0) {
                write(0xFF00 + gb::regNames::NR44, get_reg(gb::regNames::NR44) & 0x7F);
                noise_playing = true;
            }
            
            // If length counter in NR41 is 0, and bit 6 of NR 44 is 1, noise is disabled
            if ((get_reg(gb::regNames::NR41) & 0b00111111) == 0 and gb::Utils::get_bit(get_reg(gb::regNames::NR44), 6))
                noise_playing = false;
            
            // Sets the frequency
            uint8_t s = (get_reg(gb::regNames::NR43) & 0b11110000) >> 4;
            uint8_t r_raw = get_reg(gb::regNames::NR43) & 0b00000111;
            double r = (r_raw == 0) ? 0.5 : (double)r_raw;
            
            double noise_freq = 524288.0 / (r * (double)(2 << (s + 1)));
            noise_channel.setPitch(noise_freq / 1000.0);
            
            // Sets the LFSR width
            noise_channel.mode_15 = !gb::Utils::get_bit(get_reg(gb::regNames::NR43), 3);
        }

        
        // Functions to update the volume of each channel
        void update_volume(sf::SoundStream& channel, gb::Envelope& envelope, bool playing) {
            // Reset update flag
            envelope.update = false;
            
            // If the channel is not playing, set its volume to 0
            if (!playing)
                channel.setVolume(0.0);
            else
                // Othwerise, use envelope calculation
                channel.setVolume(envelope.get_scaled_volume());
        }
        
        // Function to decrement the note length counters
        void decrement_length_counters() {
            // Decrements lower 6 bits of NR11 by 1
            uint8_t pulse_1_length = get_reg(gb::regNames::NR11) & 0b00111111;
            if (pulse_1_length > 0) {
                pulse_1_length--;
                write(0xFF00 + gb::regNames::NR11, (get_reg(gb::regNames::NR11) & 0b11000000) | pulse_1_length);
            }

            // Decrements lower 6 bits of NR21 by 1
            uint8_t pulse_2_length = get_reg(gb::regNames::NR21) & 0b00111111;
            if (pulse_2_length > 0) {
                pulse_2_length--;
                write(0xFF00 + gb::regNames::NR21, (get_reg(gb::regNames::NR21) & 0b11000000) | pulse_2_length);
            }
            
            // Decrements NR31 by 1
            uint8_t wave_length = get_reg(gb::regNames::NR31);
            if (wave_length > 0) {
                wave_length--;
                write(0xFF00 + gb::regNames::NR21, (get_reg(gb::regNames::NR21) & 0b11000000) | wave_length);
            }
            
            // Decrements lower 6 bits of NR41 by 1
            uint8_t noise_length = get_reg(gb::regNames::NR41) & 0b00111111;
            if (noise_length > 0) {
                noise_length--;
                write(0xFF00 + gb::regNames::NR41, (get_reg(gb::regNames::NR41) & 0b11000000) | noise_length);
            }
        }
        
        // Function to increment the square wave envelope counters
        void update_envelopes() {
            if (square_wave_1_playing)
                square_wave_1_env.count();
            if (square_wave_2_playing)
                square_wave_2_env.count();
            if (noise_playing)
                noise_env.count();
        }
                
        // Basic functions to read from/write to the bus
        uint8_t read(uint16_t addr){
            return bus->read(addr);
        }
        
        void write(uint16_t addr, uint8_t data){
            bus->write(addr, data);
        }
        
        // Function to get the contents of an io register (faste than reading)
        uint8_t get_reg(uint8_t reg_name){
            return bus->get_ioreg(reg_name);
        }

    };
}

#endif /* apu_h */
