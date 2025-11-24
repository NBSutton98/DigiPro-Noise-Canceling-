/**
 * @file STFTProcessor.cpp
 * @brief Implementation of the Overlap-Add STFT engine.
 */

#include "STFTProcessor.hpp"
#include "WienerPost.hpp"

void STFTProcessor::prepare(double sr, int maxBlockSize)
{
    sr_ = sr;

    fifoIndex_ = 0;

    // Resize buffers to hold enough history.
    // 2x FFT size provides ample safety margin for the ring buffer.
    inputFifo_.assign(fftSize_, 0.0f);
    outputFifo_.assign(fftSize_, 0.0f);

    // juce::dsp::FFT requires 2x size workspace for complex numbers (Real/Imag parts)
    fftWorkBuffer_.resize(fftSize_ * 2);
}

void STFTProcessor::processBlock(const juce::AudioBuffer<float> &input,
                                 juce::AudioBuffer<float> &output,
                                 WienerPost &post)
{
    // Process Channel 0 (Mono)
    auto *src = input.getReadPointer(0);
    auto *dst = output.getWritePointer(0);
    const int numSamples = input.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        // 1. Push new sample into Input Ring Buffer
        inputFifo_[fifoIndex_] = src[i];

        // 2. Pull processed sample from Output Ring Buffer
        dst[i] = outputFifo_[fifoIndex_];

        // Clear the output slot so it is ready to accumulate future overlaps
        outputFifo_[fifoIndex_] = 0.0f;

        // 3. Advance Ring Index
        fifoIndex_++;
        if (fifoIndex_ >= fftSize_)
            fifoIndex_ = 0;

        // 4. Process Frame if we hit a Hop boundary (every 256 samples)
        if (fifoIndex_ % hopSize_ == 0)
        {
            // A. Unwrap Ring Buffer into linear workspace
            int idx = fifoIndex_ - fftSize_;
            if (idx < 0)
                idx += fftSize_;

            for (int j = 0; j < fftSize_; ++j)
            {
                fftWorkBuffer_[j] = inputFifo_[idx];
                idx = (idx + 1) % fftSize_;
            }

            // B. Apply Analysis Window (Hann)
            window_.multiplyWithWindowingTable(fftWorkBuffer_.data(), fftSize_);

            // C. Forward FFT (Time -> Frequency)
            fft_.performRealOnlyForwardTransform(fftWorkBuffer_.data());

            // D. SPECTRAL PROCESSING (The "Cleaning" Step)
            post.applyRealPacked(fftWorkBuffer_.data(), fftSize_);

            // E. Inverse FFT (Frequency -> Time)
            fft_.performRealOnlyInverseTransform(fftWorkBuffer_.data());

            // F. Overlap-Add (OLA) Reconstruction
            // Scale factor compensates for window energy loss and overlap gain.
            // For 75% Hann overlap, 1.5f is a typical scaling approximation.
            const float windowCorrection = 1.0f / 1.5f;

            idx = fifoIndex_ - fftSize_;
            if (idx < 0)
                idx += fftSize_;

            for (int j = 0; j < fftSize_; ++j)
            {
                // Accumulate result into output buffer
                outputFifo_[idx] += fftWorkBuffer_[j] * windowCorrection;
                idx = (idx + 1) % fftSize_;
            }
        }
    }
}