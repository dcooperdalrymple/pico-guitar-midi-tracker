#include "tracker_fft.hpp"

#include "pico/stdlib.h"
#include "pico/float.h"

#include "math.h"
#include "global.h"

TrackerFFT::TrackerFFT(Midi *mid) : Tracker(mid) { };
TrackerFFT::~TrackerFFT() { };

void TrackerFFT::process(size_t len, uint8_t *data) {
    // Check buffer level against trigger level
    if (ac_level < triggerLevel) {
        removeNote();
        return;
    }

    // Convert to float
    float buffer[len];
    for (size_t i = 0; i < len; i++) {
        buffer[i] = ((float)data[i] - dc_offset) / 256.0f;
    }

    // Calculate FFT on buffer
    spectrum(len, buffer);

    // Determine most prominent frequency
    size_t i, imax = 0; // imax = index of max amplitude
    float freq, amax = 0; // freq = detected frequency, amax = max amplitude
    for (i = 0; i < len; i++) {
        if (buffer[i] < amax) continue;
        amax = buffer[i];
        imax = i;
    }
    freq = (float)imax / len * fSAMPLE_RATE / 4; // calculate frequency from index
    detectNote(freq);
};

// Based on https://github.com/adafruit/circuitpython-ulab/blob/master/code/fft.c
void TrackerFFT::spectrum(size_t len, float *data) {
    size_t i;
    if ((len & (len-1)) != 0) return; // Length must be power of 2
    fft(len, data, 1);
    for (i = 0; i < len; i++) {
        data[i] = data[i] * data[i];
    }
};

void TrackerFFT::fft(size_t len, float *data, int isign) {
    int j, m, mmax, istep;
    float swap, temp;
    float wtemp, w, wp, theta;

    j = 0;
    for (size_t i = 0; i < len; i++) {
        if (j > i) {
            swap = data[i];
            data[i] = data[j];
            data[j] = swap;
        }
        m = len >> 1;
        while (j >= m && m > 0) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    mmax = 1;
    while (len > mmax) {
        istep = mmax << 1;
        theta = -2.0 * isign * PI * istep;
        wtemp = sinr(0.5 * theta);
        wp = -1.0 * wtemp * wtemp;
        w = 1.0;
        for (m = 0; m < mmax; m++) {
            for (int i = m; i < len; i += istep) {
                j = i + mmax;
                temp = w * data[j];
                data[j] = data[i] - temp;
                data[i] += temp;
            }
        }
        mmax = istep;
    }
};
