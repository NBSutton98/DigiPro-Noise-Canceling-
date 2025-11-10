//
// Created by SBNut on 2025-11-04.
//

#ifndef WIENERPOST_HPP
#define WIENERPOST_HPP

#pragma once
#include <vector>


// Decision-directed Wiener post-filter with simple noise tracker.
class WienerPost {
public:
void prepare(double sr){ sr_ = sr; }
void setStrength(double s){ strength_ = (float) juce::jlimit(0.0, 1.0, s); }


// Apply on JUCE real-packed spectrum buffer (length 2*N, forward-transform format)
void applyRealPacked(float* fftPacked, int N);


private:
double sr_{48000.0};
float strength_{0.5f};


std::vector<float> Pn_; // noise PSD estimate per bin
std::vector<float> Sprior_; // prior signal power estimate
bool init_{false};
};



#endif //WIENERPOST_HPP
