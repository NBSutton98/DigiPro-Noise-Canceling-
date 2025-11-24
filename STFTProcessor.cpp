#include "STFTProcessor.hpp"
#include "WienerPost.hpp"

void STFTProcessor::prepare(double sr, int maxBlockSize)
{
    sr_ = sr;
    
    // Reset indices
    fifoIndex_ = 0;

    // Resize buffers. We need enough space for history.
    // 2x FFT size is usually sufficient for the ring buffer logic used here.
    inputFifo_.assign(fftSize_, 0.0f);
    outputFifo_.assign(fftSize_, 0.0f);
    
    // Work buffer needs to be 2x size for juce::dsp::FFT real-to-complex format
    fftWorkBuffer_.resize(fftSize_ * 2); 
}

void STFTProcessor::processBlock(const juce::AudioBuffer<float>& input, 
                                 juce::AudioBuffer<float>& output, 
                                 WienerPost& post)
{
    // For this Lite version, we process Channel 0 (Mono)
    // If you use stereo, you would need two instances of this class.
    auto* src = input.getReadPointer(0);
    auto* dst = output.getWritePointer(0);
    const int numSamples = input.getNumSamples();

    for (int i = 0; i < numSamples; ++i)
    {
        // 1. Push new sample into Input FIFO
        inputFifo_[fifoIndex_] = src[i];

        // 2. Pull processed sample from Output FIFO
        dst[i] = outputFifo_[fifoIndex_];
        
        // Clear the output slot so it is ready to accumulate future overlaps
        outputFifo_[fifoIndex_] = 0.0f;

        // 3. Increment Ring Index
        fifoIndex_++;
        if (fifoIndex_ >= fftSize_)
            fifoIndex_ = 0;

        // 4. Process Frame if we hit a Hop boundary
        // We process whenever we have gathered 'hopSize_' new samples
        if (fifoIndex_ % hopSize_ == 0)
        {
            // Unwrap the Ring Buffer into a linear array for FFT
            int idx = fifoIndex_ - fftSize_;
            if (idx < 0) idx += fftSize_;
            
            for (int j = 0; j < fftSize_; ++j)
            {
                fftWorkBuffer_[j] = inputFifo_[idx];
                idx = (idx + 1) % fftSize_;
            }

            // Apply Analysis Window
            window_.multiplyWithWindowingTable(fftWorkBuffer_.data(), fftSize_);

            // Forward FFT (Time -> Frequency)
            fft_.performRealOnlyForwardTransform(fftWorkBuffer_.data());

            // --- WIENER FILTERING ---
            post.applyRealPacked(fftWorkBuffer_.data(), fftSize_);
            // ------------------------

            // Inverse FFT (Frequency -> Time)
            fft_.performRealOnlyInverseTransform(fftWorkBuffer_.data());

            // Overlap-Add into Output FIFO
            // For Hann window with 75% overlap, we strictly don't need a synthesis window
            // if we scale correctly. A scaling factor is needed because OLA adds up energy.
            // Scaling by 2/3rds of hop ratio is a common approximation, or just empirical tuning.
            const float windowCorrection = 1.0f / 1.5f; // Empirical scaling for Hann 75%

            idx = fifoIndex_ - fftSize_;
            if (idx < 0) idx += fftSize_;

            for (int j = 0; j < fftSize_; ++j)
            {
                // Accumulate result
                outputFifo_[idx] += fftWorkBuffer_[j] * windowCorrection;
                
                idx = (idx + 1) % fftSize_;
            }
        }
    }
}