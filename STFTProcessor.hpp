/**
 * @file STFTProcessor.hpp
 * @brief Short-Time Fourier Transform (STFT) Engine.
 * * This class handles the conversion between Time Domain (waveform) and
 * Frequency Domain (spectral) data. It uses a Circular Buffer (FIFO) to
 * manage the overlap-add reconstruction process, ensuring artifact-free
 * processing even when FFT window sizes do not match audio callback block sizes.
 */

#pragma once
#include <juce_dsp/juce_dsp.h>
#include <vector>

class WienerPost;

class STFTProcessor
{
public:
    // ==============================================================================
    // Lifecycle
    // ==============================================================================

    /**
     * @brief Initializes the FFT engine and windowing functions.
     * @param sr The sample rate.
     * @param maxBlockSize Maximum expected audio block size.
     */
    void prepare(double sr, int maxBlockSize);

    /**
     * @brief Processes a block of audio through the STFT pipeline.
     * * Pipeline:
     * 1. Push audio into Ring Buffer.
     * 2. When enough data exists, extract a Frame (1024 samples).
     * 3. Apply Window Function (Hann).
     * 4. Perform Forward FFT (Time -> Freq).
     * 5. Call 'post' processor (Wiener Filter) to modify spectrum.
     * 6. Perform Inverse FFT (Freq -> Time).
     * 7. Overlap-Add result into Output Ring Buffer.
     * * @param input Source audio.
     * @param output Destination audio.
     * @param post The spectral processing algorithm to apply (Wiener).
     */
    void processBlock(const juce::AudioBuffer<float> &input,
                      juce::AudioBuffer<float> &output,
                      WienerPost &post);

private:
    double sr_{48000.0};

    // --- FFT Configuration ---
    // Size: 1024 samples (approx 21ms at 48kHz).
    // Hop: 256 samples (75% Overlap) for smooth reconstruction.
    static constexpr int fftOrder_ = 10;
    static constexpr int fftSize_ = 1 << fftOrder_; // 1024
    static constexpr int hopSize_ = fftSize_ / 4;   // 256

    juce::dsp::FFT fft_{fftOrder_};

    // Hann window reduces spectral leakage at frame boundaries
    juce::dsp::WindowingFunction<float> window_{fftSize_, juce::dsp::WindowingFunction<float>::hann};

    // --- Circular Buffers ---
    // FIFO buffers allow us to decouple the FFT frame size from the audio hardware buffer size.
    std::vector<float> inputFifo_;     ///< Stores incoming raw samples
    std::vector<float> outputFifo_;    ///< Accumulates processed overlap-add samples
    std::vector<float> fftWorkBuffer_; ///< Temporary workspace for FFT calculations

    int fifoIndex_{0}; ///< Current write position in the ring buffer
};