// Created by Niklas on 21/04/2020.
// Class which inherits from sf::SoundStream and plays audio data for the wave channel
// Frequency controls etc are handled in the apu

#ifndef WaveChannel_h
#define WaveChannel_h

#include <SFML/Audio.hpp>
#include "bus.h"

namespace gb {
    class WaveChannel : public sf::SoundStream {
    public:
        // Function to initialise the stream
        bool open(int _sample_rate, int channel_count, gb::bus* _bus) {
            bus = _bus;
            sample_rate = _sample_rate;
            initialize(channel_count, sample_rate);
        }
        
        // Stores a pointer to the bus
        gb::bus* bus;
        
        // Stores the wave sample data
        uint8_t wave_samples[32];
        
        // Stores the volume setting
        int volume = 1;
        
    private:
        // Stores the samples and sample rate
        std::vector<sf::Int16> samples;
        int sample_rate;
        
        // Stores which part of the wave is being used
        int wave_cycle = 0;
        
        // Function to get fetch the wave sample data from the bus
        void read_wave_samples() {
            for (int i = 0; i < 0x10; i++) {
                // Reads raw data from FF30 + i
                uint8_t raw_data = bus->read(0xFF30 + i);
                
                // Stores high nybble then low nybble
                wave_samples[2 * i + 0] = raw_data >> 4;
                wave_samples[2 * i + 1] = raw_data & 0x0F;
            }
            
        }
        
        // Function to get the next sample value
        sf::Int16 get_next_sample() {
            // If volume is 0, returns 0
            if (volume == 0) return 0;
            
            // Gets the appropriate sample
            int8_t sample_raw = wave_samples[wave_cycle];
            sample_raw >>= (volume - 1);
            
            // Increments cycle counter
            wave_cycle = (wave_cycle + 1) % 32;
            
            // Returns the scaled sample
            return (sample_raw - 8) * 4096;
        }
        
        // Function to send data when requested
        virtual bool onGetData(Chunk& data){
            samples.clear();
            const int num_samples = 3000;
            
            // Reads samples from the bus
            read_wave_samples();
            
            for (int i = 0; i < (num_samples / (sample_rate / 3200)); i ++){
                // Calculates sample value
                sf::Int16 sample = get_next_sample();
                
                // Sends it enough times for a base frequency of 100Hz (modified by changing pitch)
                for (int j = 0; j < (sample_rate / 3200); j++) {
                    samples.push_back(sample);
                }
            }
            
            // Fill the chunk with audio data from the stream source
            data.samples = &samples[0];
            data.sampleCount = samples.size();
            
            // Return true to continue playing
            return true;
        }
        
        // Unimplementnted
        virtual void onSeek(sf::Time timeOffset){}
    };
}

#endif /* WaveChannel_h */
