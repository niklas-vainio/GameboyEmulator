// Created by Niklas on 16/04/2020.
// Class which controls volume envelopes for the various channels

#ifndef Envelope_h
#define Envelope_h

namespace gb {
    class Envelope {
    public:
        // Flag which the signals to the APU that a change in volume has occured
        bool update = true;
        
        // Stores the current volume
        int current_volume = 0;
  
        
        // Function to reset when a new value is written to the register
        void reset(uint8_t reg) {
            // Sets initial volume to upper 4 bits
            initial_volume = (reg & 0b11110000) >> 4;
            
            // Sets period to lower 3 bits
            period = reg & 0b00000111;
            
            // Sets incrementing to bit 3
            incrementing = (reg & 0b00001000) >> 3;
            
            // Resets ticks to 0
            ticks = 0;
            
            // Sets update flag
            update = true;
        }
        
        // Function to increment the count
        void count() {
            // If ticks is bigger than 128, return as this is the max
            if (ticks >= 128)
                return;
            
            // Calculates new volume value
            ticks++;
            current_volume = get_volume();
            
            // Sets the flag if the volume has changed
            if (current_volume != prev_volume)
                update = true;
            
            // Bookmarks volume
            prev_volume = current_volume;
            
        }
        
        // Function to get the volume as a float from 0 - 100
        float get_scaled_volume() {
            return 100.0 * ((float)current_volume / 16.0);
        }
        
    private:
        // Stores the initial volume, whether it is incrementing, and the fade period
        int initial_volume = 0;
        int period = 0;
        bool incrementing = true;
        
        // Stores the previous volume value
        int8_t prev_volume = 0;
        
        // Stores the number of ticks
        int ticks = 0;
        
        // Function which calculates the volume value
        int get_volume() {
            // If period is 0, always return initial volume
            if (period == 0)
                return initial_volume;
            
            // Otherwise, do calculation
            int output = initial_volume + (incrementing ? 1 : -1) * (ticks / period);
            if (output > 0xF) output = 0xF;
            if (output < 0) output = 0;
            return output;
        }
    };
}

#endif /* Envelope_h */
