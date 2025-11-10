//
// Created by SBNut on 2025-11-04.
//

#include "STFTProcessor.h"
#include "WienerPost.h"
#include <cstring>


void STFTProcessor::prepare(double sr, int N){ sr_ = sr; setFFTSize(N); }


void STFTProcessor::setFFTSize(int N){
if (N!=512 && N!=1024 && N!=2048) N = 1024;
N_ = N; hop_ = N_/4; order_ = (int)std::round(std::log2((double)N_));
fft_ = juce::dsp::FFT(order_);
buildWindow(); ensureBuffers();
olaWrite_ = 0; framePos_ = 0;
}


void STFTProcessor::setWindowType(int id){ winType_ = id; buildWindow(); }


void STFTProcessor::buildWindow(){
window_.assign(N_, 1.0f);
for (int n=0;n<N_;++n){
float x = float(n) / float(N_-1);
float w = 1.0f;
switch(winType_){
case 1: w = 0.5f * (1.0f - std::cos(2.0f*juce::MathConstants<float>::pi * x)); break; // Hann
case 2: w = 0.54f - 0.46f*std::cos(2.0f*juce::MathConstants<float>::pi * x); break; // Hamming
case 3: w = 0.42f - 0.5f*std::cos(2.0f*juce::MathConstants<float>::pi * x)
+ 0.08f*std::cos(4.0f*juce::MathConstants<float>::pi * x); break; // Blackman
default: break;
}
window_[n] = w;
}
}


void STFTProcessor::ensureBuffers(){
frame_.assign(N_, 0.0f);
fftBuf_.assign(2*N_, 0.0f); // real/imag interleaved for JUCE FFT
ola_.assign(N_ + hop_, 0.0f);
}


void STFTProcessor::processChannelInPlace(juce::AudioBuffer<float>& buf, int start, int numSamples, int channel,
WienerPost& post){
auto* x = buf.getWritePointer(channel, start);


for (int n=0; n<numSamples; ++n){
// append sample into current frame
frame_[framePos_] = x[n];
++framePos_;


if (framePos_ >= N_){
// window
for (int i=0;i<N_;++i) fftBuf_[2*i] = frame_[i] * window_[i], fftBuf_[2*i+1] = 0.0f;


// FFT
fft_.performRealOnlyForwardTransform(fftBuf_.data());


// Apply spectral post (in-place on packed format)
post.applyRealPacked(fftBuf_.data(), N_);


// iFFT
fft_.performRealOnlyInverseTransform(fftBuf_.data());


}
