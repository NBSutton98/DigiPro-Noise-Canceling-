/**
 * @file WienerPost.cpp
 * @brief Implementation of the Spectral Subtraction logic.
 */

#include "WienerPost.hpp"
#include <algorithm>
#include <cmath>

void WienerPost::applyRealPacked(float *X, int N)
{
    const int K = N / 2; // Number of frequency bins (Nyquist)

    // Initialize state buffers on first run
    if ((int)Pn_.size() != K + 1)
    {
        Pn_.assign(K + 1, 1e-6f);
        Sprior_.assign(K + 1, 0.0f);
    }

    // Parameters for SNR estimation
    const float alpha = 0.98f;      // Smoothing factor for Decision-Directed approach
    const float noiseAlpha = 0.95f; // Smoothing factor for Noise Estimation

    // --- Dynamic Reduction Limit ---
    // Allows reduction up to -40dB at max strength.
    // If strength is 0, max reduction is 0dB (Passthrough).
    float maxReductionDb = -40.0f * strength_;
    const float Gmin = std::pow(10.0f, maxReductionDb / 20.0f);

    // Helper lambda to calculate Magnitude Squared (Power) of a complex bin
    auto mag2 = [&](int k) -> float
    {
        if (k == 0)
            return X[0] * X[0]; // DC component
        if (k == K)
            return X[1] * X[1]; // Nyquist component

        // Complex components (Real + Imaginary)
        const int re = 2 * k;
        const int im = 2 * k + 1;
        if (im >= 2 * N)
            return 0.0f;

        float r = X[re];
        float ii = X[im];
        return r * r + ii * ii;
    };

    // --- Main Spectral Loop ---
    for (int k = 0; k <= K; ++k)
    {
        float Y2 = mag2(k) + 1e-12f; // Current Power (plus epsilon to avoid divide-by-zero)

        // 1. Estimate Noise Floor (Minimum Statistics Approach)
        // Slowly tracks the lowest energy seen in this bin over time.
        Pn_[k] = noiseAlpha * Pn_[k] + (1.0f - noiseAlpha) * std::min(Pn_[k], Y2);

        // 2. Calculate A Posteriori SNR (Current Frame SNR)
        float postSNR = std::clamp(Y2 / (Pn_[k] + 1e-12f), 0.0f, 1000.0f);

        // 3. Calculate A Priori SNR (Smoothed Historical SNR)
        // Uses Ephraim-Malah Decision-Directed approach for stability.
        float priorSNR = alpha * (Sprior_[k] / (Pn_[k] + 1e-12f)) + (1.0f - alpha) * std::max(postSNR - 1.0f, 0.0f);

        // 4. Calculate Wiener Gain Mask
        // G = SNR / (1 + SNR)
        float G = priorSNR / (1.0f + priorSNR);

        // 5. Apply Reduction Limit (Gmin)
        if (G < Gmin)
            G = Gmin;

        // 6. Apply Gain to Frequency Bin
        if (k == 0)
        {
            X[0] *= G;
        }
        else if (k == K)
        {
            X[1] *= G;
        }
        else
        {
            X[2 * k] *= G;     // Real part
            X[2 * k + 1] *= G; // Imaginary part
        }

        // 7. Update Prior SNR state for next frame
        Sprior_[k] = G * G * Y2;
    }
}