/**
 * @file NLMSFilter.hpp
 * @brief Normalized Least Mean Squares (NLMS) Adaptive Filter.
 *
 * This class implements a standard adaptive filter designed for Acoustic Echo
 * Cancellation (AEC) or Reference Noise Cancellation. It learns the transfer
 * function between a reference noise source and the primary signal to mathematically
 * subtract the noise.
 */

#ifndef NLMSFILTER_HPP
#define NLMSFILTER_HPP

#pragma once
#include <vector>

class NLMSFilter
{
public:
    /**
     * @brief Initializes the adaptive filter.
     * @param sr Sample Rate (e.g., 48000).
     * @param L Filter Length (Number of Taps/Coefficients).
     */
    void prepare(double sr, int L);

    /**
     * @brief Resizes the filter and clears internal buffers.
     * @param L The new number of taps (minimum 8).
     */
    void setLength(int L);

    /**
     * @brief Sets the learning rate (Step Size).
     * @param mu Controls convergence speed (0.01 - 1.0).
     * Higher = faster adaptation, Lower = more stability.
     */
    void setMu(double mu) { mu_ = mu; }

    /**
     * @brief Freezes the learning process.
     * If true, the filter stops updating weights but continues to apply the
     * current solution to the audio. Useful for "Double-Talk" scenarios.
     */
    void setFrozen(bool f) { frozen_ = f; }

    /**
     * @brief Processes a single sample pair.
     * @param primary The "Dirty" signal (Voice + Noise).
     * @param reference The "Noise Only" reference signal.
     * @return The cleaned sample (Error term).
     */
    float processOne(float primary, float reference);

private:
    double sr_{48000.0};
    int L_{256};         ///< Filter Length (Taps)
    double mu_{0.2};     ///< Normalized Step Size
    bool frozen_{false}; ///< Adaptation State
    double eps_{1e-8};   ///< Regularization term (prevents divide-by-zero)

    std::vector<float> w_;    ///< Filter Weights (Coefficients)
    std::vector<float> xbuf_; ///< Circular Buffer for Reference History
    size_t idx_{0};           ///< Current Write Index
};

#endif // NLMSFILTER_HPP