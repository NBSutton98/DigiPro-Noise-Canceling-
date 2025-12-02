/**
 * @file WienerPost.cpp
 * @brief Implementation of the Spectral Subtraction logic.
 */

#include "WienerPost.hpp"
#include <algorithm>
#include <cmath>
#include <array>

// Crossover frequencies matching AudioEngine.cpp
const std::array<float, 3> crossover_hz = {300.0f, 1500.0f, 5000.0f};

// Set the 4 band strengths
void WienerPost::setStrengths(const float* strengths)
{
    for (int i = 0; i < 4; ++i)
    {
        bandStrengths_[i] = strengths[i];
    }
}

// Helper to map Frequency Bin -> Band Index (0-3)
int WienerPost::getBandIndex(int k, int N) const
{
    // Frequency of bin k = k * (SampleRate / FFTSize)
    // FFTSize is N in this context (RealPacked size)
    const float freq_hz = (float)k * (float)sr_ / (float)N;

    if (freq_hz < crossover_hz[0]) return 0;      // < 300
    else if (freq_hz < crossover_hz[1]) return 1; // 300 - 1500
    else if (freq_hz < crossover_hz[2]) return 2; // 1500 - 5000
    else return 3;                                // > 5000
}

void WienerPost::applyRealPacked(float *X, int N)
{
    const int K = N / 2;

    if ((int)Pn_.size() != K + 1)
    {
        Pn_.assign(K + 1, 1e-6f);
        Sprior_.assign(K + 1, 0.0f);
    }

    const float alpha = 0.92f;
    const float noiseAlpha = 0.95f;

    auto mag2 = [&](int k) -> float
    {
        if (k == 0) return X[0] * X[0];
        if (k == K) return X[1] * X[1];
        const int re = 2 * k;
        const int im = 2 * k + 1;
        float r = X[re];
        float ii = X[im];
        return r * r + ii * ii;
    };

    for (int k = 0; k <= K; ++k)
    {
        int bandIdx = getBandIndex(k, N);
        float currentBandStrength = std::clamp(bandStrengths_[bandIdx], 0.0f, 1.0f);

        float maxReductionDb = -60.0f * currentBandStrength;
        
        const float Gmin = std::pow(10.0f, maxReductionDb / 20.0f);

        float Y2 = mag2(k) + 1e-12f;

        // 1. Estimate Noise Floor
        Pn_[k] = noiseAlpha * Pn_[k] + (1.0f - noiseAlpha) * Y2;

        // 2. A Posteriori SNR
        float postSNR = std::clamp(Y2 / (Pn_[k] + 1e-12f), 0.0f, 1000.0f);

        // 3. A Priori SNR
        float priorSNR = alpha * (Sprior_[k] / (Pn_[k] + 1e-12f))
                        + (1.0f - alpha) * std::max(postSNR - 1.0f, 0.0f);

        // 4. Wiener Gain
        float G = priorSNR / (1.0f + priorSNR);

        // 5. Apply Limit (Gmin is now up to -100dB)
        if (G < Gmin)
            G = Gmin;

        // 6. Apply Gain
        if (k == 0)      X[0] *= G;
        else if (k == K) X[1] *= G;
        else {
            X[2 * k] *= G;
            X[2 * k + 1] *= G;
        }

        // 7. Update State
        Sprior_[k] = G * G * Y2;
    }
}