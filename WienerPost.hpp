/**
 * @file WienerPost.hpp
 * @brief Spectral Subtraction Algorithm (Wiener Filter).
 */

#pragma once
#include <vector>
#include <array> // Make sure this is included
#include <juce_core/juce_core.h>

class WienerPost
{
public:
    void prepare(double sr) { sr_ = sr; }

    /**
     * @brief Sets the aggression of the filter using 4 multiband values.
     */
    void setStrengths(const float* strengths);

    void applyRealPacked(float *fftPacked, int N);

private:
    double sr_{48000.0};
    
    std::array<float, 4> bandStrengths_{0.5f, 0.5f, 0.5f, 0.5f};

    int getBandIndex(int k, int N) const;

    // --- Spectral State ---
    std::vector<float> Pn_;     
    std::vector<float> Sprior_; 

    bool init_{false};
};