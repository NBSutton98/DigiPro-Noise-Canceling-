/**
 * @file NLMSFilter.cpp
 * @brief Implementation of the NLMS adaptive algorithm.
 */

#include "NLMSFilter.hpp"
#include <algorithm>
#include <cmath>

void NLMSFilter::prepare(double sr, int L)
{
    sr_ = sr;
    setLength(L);
}

void NLMSFilter::setLength(int L)
{
    L_ = std::max(8, L); // Enforce minimum filter size
    w_.assign(L_, 0.0f);
    xbuf_.assign(L_, 0.0f);
    idx_ = 0;
}

float NLMSFilter::processOne(float primary, float reference)
{
    // 1. Update Reference Buffer
    // Store the latest reference sample in our circular history
    xbuf_[idx_] = reference;

    // 2. Estimate Noise (Convolution)
    // Calculate y_hat = w * x (Dot product of weights and reference history)
    double yhat = 0.0;
    double norm = eps_; // Start with epsilon to prevent division by zero later
    size_t p = idx_;

    for (int i = 0; i < L_; ++i)
    {
        float xi = xbuf_[p];
        yhat += w_[i] * xi;

        // Calculate energy (L2 norm squared) for normalization
        norm += double(xi) * double(xi);

        // Move circular pointer backwards
        p = (p == 0) ? size_t(L_ - 1) : p - 1;
    }

    // 3. Calculate Error (The Output)
    // e = d - y_hat (Primary Signal - Estimated Noise)
    // Ideally, 'e' contains only the voice.
    float e = primary - (float)yhat;

    // 4. Update Weights (Learning Step)
    // Formula: w(n+1) = w(n) + mu * e(n) * x(n) / ||x(n)||^2
    if (!frozen_)
    {
        const double g = mu_ * e / norm; // Normalized step size
        p = idx_;
        for (int i = 0; i < L_; ++i)
        {
            w_[i] += (float)(g * xbuf_[p]);
            p = (p == 0) ? size_t(L_ - 1) : p - 1;
        }
    }

    // 5. Advance Circular Index
    idx_ = (idx_ + 1) % (size_t)L_;

    return e;
}