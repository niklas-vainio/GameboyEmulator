// Created by Niklas on 16/04/2020.
// Class which inherits from sf::SoundStream and plays audio data for the square wave channels
// Frequency controls etc are handled in the apu

#ifndef SquareChannel_h
#define SquareChannel_h

#include <SFML/Audio.hpp>

namespace gb {
    class SquareChannel : public sf::SoundStream {
    public:
        // Function to initialise the stream
        bool open(int _sample_rate, int channel_count) {
            sample_rate = _sample_rate;
            initialize(channel_count, sample_rate);
        }
        
        // Stores which duty cycle is being used (1-4)
        int duty_cycle = 2;
        
    private:
        // Stores the wave patterns
        const bool wave_patterns[4][8] = {
            {1, 0, 0, 0, 0, 0, 0, 0},
            {1, 1, 0, 0, 0, 0, 0, 0},
            {1, 1, 1, 1, 0, 0, 0, 0},
            {1, 1, 1, 1, 1, 1, 0, 0}
        };
        
        // Stores the samples and sample rate
        std::vector<sf::Int16> samples;
        int sample_rate;
        
        // Stores which part of the wave is being used
        int wave_cycle = 0;
        
        // Function to get the next sample value
        sf::Int16 get_next_sample() {
            // Gets current cycle
            bool high = wave_patterns[duty_cycle][wave_cycle];
            
            // Incrments cycle value
            wave_cycle = (wave_cycle + 1) % 8;
            
            // Returns correct sample
            return high ? 32767 : -32768;
        }
        
        // Function to send data when requested
        virtual bool onGetData(Chunk& data){
            samples.clear();
            const int num_samples = 12000;
            
            for (int i = 0; i < (num_samples / (sample_rate / 800)); i ++){
                // Calculates sample value
                sf::Int16 sample = get_next_sample();
                
                // Sends it enough times for a base frequency of 100Hz (modified by changing pitch)
                for (int j = 0; j < (sample_rate / 800); j++) {
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

#endif /* SquareChannel_h */
