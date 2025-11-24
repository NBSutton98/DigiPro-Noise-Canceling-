/**
 * @file WienerPost.hpp
 * @brief Spectral Subtraction Algorithm (Wiener Filter).
 * * This class implements an adaptive noise reduction filter.
 * It estimates the Power Spectral Density (PSD) of the noise floor
 * and calculates a gain mask (0.0 to 1.0) for each frequency bin.
 * * Core Math: Gain(k) = PriorSNR(k) / (1 + PriorSNR(k))
 */

#pragma once
#include <vector>
#include <juce_core/juce_core.h>

class WienerPost
{
public:
  void prepare(double sr) { sr_ = sr; }

  /**
   * @brief Sets the aggression of the filter (0.0 to 1.0).
   * Higher values allow the filter to reduce noise by up to -40dB.
   */
  void setStrength(double s) { strength_ = (float)juce::jlimit(0.0, 1.0, s); }

  /**
   * @brief Applies the Wiener Gain Mask to a frequency spectrum.
   * @param fftPacked Pointer to the FFT data (Real/Imaginary interleaved).
   * @param N The FFT size (e.g. 1024).
   */
  void applyRealPacked(float *fftPacked, int N);

private:
  double sr_{48000.0};
  float strength_{0.5f};

  // --- Spectral State ---
  std::vector<float> Pn_;     ///< Estimated Noise Power Spectrum
  std::vector<float> Sprior_; ///< A Priori Signal-to-Noise Ratio estimate

  bool init_{false};
};