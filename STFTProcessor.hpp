#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>

class WienerPost;

class STFTProcessor {
public:
    void prepare(double sr, int maxBlockSize);
    
    // Process a block of audio (Input -> Output) using Overlap-Add
    void processBlock(const juce::AudioBuffer<float>& input, 
                      juce::AudioBuffer<float>& output, 
                      WienerPost& post);

private:
    double sr_ { 48000.0 };

    // FFT Settings
    // 1024 size with 256 hop = 75% overlap (Standard for denoising)
    static constexpr int fftOrder_ = 10;
    static constexpr int fftSize_  = 1 << fftOrder_; // 1024
    static constexpr int hopSize_  = fftSize_ / 4;   // 256
    
    juce::dsp::FFT fft_ { fftOrder_ };
    juce::dsp::WindowingFunction<float> window_ { fftSize_, juce::dsp::WindowingFunction<float>::hann };

    // Buffers
    std::vector<float> inputFifo_;      // History for input
    std::vector<float> outputFifo_;     // Accumulator for output
    std::vector<float> fftWorkBuffer_;  // Temp buffer for FFT calcs
    
    int fifoIndex_ { 0 }; // Current write position in the ring buffers
};