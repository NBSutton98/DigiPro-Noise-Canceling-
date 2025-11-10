//
// Created by SBNut on 2025-11-04.
//

#ifndef AUDIOENGINE_HPP
#define AUDIOENGINE_HPP

#pragma once
#include <juce_dsp/juce_dsp.h>
#include "NLMSFilter.hpp"
#include "STFTProcessor.hpp"
#include "WienerPost.hpp"


class AudioEngine {
public:
void prepare(double sr, int blockSize);
void process(juce::AudioBuffer<float>& buf, int start, int numSamples);


void setMu(double v) { nlms_.setMu(v); }
void setFilterLength(int L) { nlms_.setLength(L); }
void setFrozen(bool f) { nlms_.setFrozen(f); }


void setFFTSize(int N) { stft_.setFFTSize(N); }
void setWindowType(int id) { stft_.setWindowType(id); }
void setPostStrength(double s){ post_.setStrength(s); }
void enablePost(bool e) { usePost_ = e; }


private:
double sr_{}; int block_{0};
NLMSFilter nlms_;
STFTProcessor stft_;
WienerPost post_;
bool usePost_{true};
};

#endif //AUDIOENGINE_HPP
