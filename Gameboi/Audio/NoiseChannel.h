// Created by Niklas on 15/04/2020.
// Class which inherits from sf::SoundStream and plays audio data for the noise channel
// Frequency controls etc are handled in the apu

#ifndef NoiseChannel_h
#define NoiseChannel_h

#include <SFML/Audio.hpp>

namespace gb {
    class NoiseChannel : public sf::SoundStream {
    public:
        // Stores whether the LFSR is operating in 15 bit mode
        bool mode_15 = true;
        
        // Function to initialise the stream
        bool open(int _sample_rate, int channel_count) {
            sample_rate = _sample_rate;
            initialize(channel_count, sample_rate);
        }
        
    private:
        // Stores the samples and sample rate
        std::vector<sf::Int16> samples;
        int sample_rate;
        
        // Stores the 16-bit LFSR
        uint16_t LFSR = 0xFFFF;
        
        // Function to get the next sample value
        sf::Int16 get_next_sample() {
            // Last bit of LFSR is the output
            bool output = LFSR & 1;
            
            // XORs last two bits of LFSR
            bool new_bit = ((LFSR & 0b10) >> 1) ^ (LFSR & 0b01);
            
            if (mode_15)
                // If in mode 15, this bit is placed at bit 14
                LFSR = (LFSR >> 1) | (new_bit << 14);
            else
                // Otherwise, this bit is placed in bit 6
                LFSR = ((LFSR >> 1) & ~(1 << 6)) | (new_bit << 6);
            
            // Returns either 32767 or -32768 depending on output bit
            return output ? -32768 : 32767;
        }
        
        // Function to send data when requested
        virtual bool onGetData(Chunk& data){
            samples.clear();
            const int num_samples = 48000;
            
            // Sends one second of data
            for (int i = 0; i < (num_samples / (sample_rate / 1000)); i ++){
                // Calculates sample value
                sf::Int16 sample = get_next_sample();
                
                //Sends it enough times for a base frequency of 100Hz (modified by changing pitch)
                for (int j = 0; j < (sample_rate / 1000); j++) {
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


#endif /* NoiseChannel_h */
