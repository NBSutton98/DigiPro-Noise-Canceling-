#include "STFTProcessor.hpp"
#include "WienerPost.hpp"
#include <cstring>
#include <cmath>

void STFTProcessor::prepare(double sr, int N){ sr_ = sr; setFFTSize(N); }
void STFTProcessor::setFFTSize(int N){
  if (N!=512 && N!=1024 && N!=2048) N = 1024;
  N_ = N; hop_ = N_/4; order_ = (int)std::round(std::log2((double)N_));
  fft_ = juce::dsp::FFT(order_);
  buildWindow(); ensureBuffers();
  framePos_ = 0;
}
void STFTProcessor::setWindowType(int id){ winType_ = id; buildWindow(); }
void STFTProcessor::buildWindow(){
  window_.assign(N_, 1.0f);
  for (int n=0;n<N_;++n){
    float x = float(n) / float(N_-1);
    float w = 1.0f;
    switch(winType_){
      case 1: w = 0.5f * (1.0f - std::cos(2.0f*juce::MathConstants<float>::pi * x)); break;
      case 2: w = 0.54f - 0.46f*std::cos(2.0f*juce::MathConstants<float>::pi * x); break;
      case 3: w = 0.42f - 0.5f*std::cos(2.0f*juce::MathConstants<float>::pi * x)
                    + 0.08f*std::cos(4.0f*juce::MathConstants<float>::pi * x); break;
    }
    window_[n] = w;
  }
}
void STFTProcessor::ensureBuffers(){
  frame_.assign(N_, 0.0f);
  fftBuf_.assign(2*N_, 0.0f);
  ola_.assign(N_ + hop_, 0.0f);
}
void STFTProcessor::processChannelInPlace(juce::AudioBuffer<float>& buf, int start, int numSamples, int channel,
                                          WienerPost& post){
  auto* x = buf.getWritePointer(channel, start);
  for (int n=0; n<numSamples; ++n) {
    frame_[framePos_] = x[n];
    ++framePos_;

    if (framePos_ >= N_) {
        for (int i=0;i<N_;++i){ fftBuf_[2*i] = frame_[i]*window_[i]; fftBuf_[2*i+1] = 0.0f; }
        fft_.performRealOnlyForwardTransform(fftBuf_.data());
        post.applyRealPacked(fftBuf_.data(), N_);
        fft_.performRealOnlyInverseTransform(fftBuf_.data());

        // write back with scaling; keep indices inside [0, numSamples)
        const float scale = 2.0f / float(N_);
        for (int i=0;i<N_;++i){
            int outIdx = (n - (N_-1)) + i;
            if (outIdx >= 0 && outIdx < numSamples)
                x[outIdx] += fftBuf_[i] * window_[i] * scale;
        }

        // slide window by hop
        if (hop_ < N_) {
            std::memmove(frame_.data(), frame_.data() + hop_, sizeof(float)*(N_ - hop_));
            std::fill(frame_.begin() + (N_ - hop_), frame_.end(), 0.0f);
            framePos_ = N_ - hop_;
        } else {
            std::fill(frame_.begin(), frame_.end(), 0.0f);
            framePos_ = 0;
        }
    }
}
}
