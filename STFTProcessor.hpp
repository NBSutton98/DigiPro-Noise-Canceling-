//
// Created by SBNut on 2025-11-04.
//

#ifndef STFTPROCESSOR_HPP
#define STFTPROCESSOR_HPP

#pragma once
#include <juce_dsp/juce_dsp.h>
#include <complex>
#include <vector>


class WienerPost; // fwd


class STFTProcessor {
public:
void prepare(double sr, int N);
void setFFTSize(int N);
void setWindowType(int id); // 1 Hann, 2 Hamming, 3 Blackman


// Process a single channel in-place
void processChannelInPlace(juce::AudioBuffer<float>& buf, int start, int numSamples, int channel,
WienerPost& post);


private:
void buildWindow();
void ensureBuffers();


double sr_{48000.0};
int N_{1024};
int hop_{256};
int winType_{1};


int order_{10};
juce::dsp::FFT fft_{10};
std::vector<float> window_;
std::vector<float> frame_;
std::vector<float> fftBuf_;
std::vector<float> ola_; // overlap-add ring
int olaWrite_{0};
int framePos_{0};
};



#endif //STFTPROCESSOR_HPP
