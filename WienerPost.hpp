#pragma once
#include <vector>
#include <juce_core/juce_core.h>   // for juce::Decibels, juce::jlimit

class WienerPost {
public:
  void prepare(double sr){ sr_ = sr; }
  void setStrength(double s){ strength_ = (float) juce::jlimit(0.0, 1.0, s); }
  void applyRealPacked(float* fftPacked, int N);

private:
  double sr_{48000.0};
  float strength_{0.5f};
  std::vector<float> Pn_;
  std::vector<float> Sprior_;
  bool init_{false};
};
