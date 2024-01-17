#pragma once

#include "pico/stdlib.h"
#include "tracker.hpp"
#include "midi.hpp"

class TrackerFFT : public Tracker {

public:
    TrackerFFT(Midi *mid);
    ~TrackerFFT();
    
    void process(size_t len, uint8_t *data);

private:
    void spectrum(size_t len, float *data);
    void fft(size_t len, float *data, int isign);

};
