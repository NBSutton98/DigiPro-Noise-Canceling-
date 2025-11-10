#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>
class WienerPost;

class STFTProcessor {
public:
  void prepare(double sr, int N);
  void setFFTSize(int N);
  void setWindowType(int id); // 1 Hann, 2 Hamming, 3 Blackman
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
  std::vector<float> window_, frame_, fftBuf_, ola_;
  int framePos_{0};
};
