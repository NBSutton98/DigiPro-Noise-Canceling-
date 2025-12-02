/**
 * @file AudioEngine.hpp
 * @brief The Core DSP (Digital Signal Processing) Engine for NClite.
 *
 * This class manages the entire audio processing pipeline. It orchestrates
 * the interaction between the Frequency Domain processor (STFT), the Time Domain
 * processor (Noise Gate), and the corrective EQ/Filtering.
 *
 * Pipeline Order:
 * 1. Input Gain (Mic Boost)
 * 2. Analysis (AI Profiling)
 * 3. Spectral Subtraction (Wiener Filter)
 * 4. Smart Hysteresis Gate
 * 5. Mode-Specific EQ (Broadcast/Isolation)
 * 6. Delta Monitoring (Phase Cancellation)
 * 7. Safety Limiter (Soft Clip)
 */

#pragma once
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include "NLMSFilter.hpp"
#include "STFTProcessor.hpp"
#include "WienerPost.hpp"

class AudioEngine
{
public:
    // ==============================================================================
    // Lifecycle Methods
    // ==============================================================================

    /**
     * @brief Initializes the DSP chain and allocates buffers.
     * @param sampleRate The sample rate of the audio stream (e.g., 44100, 48000).
     * @param maxBlockSize The maximum number of samples expected per callback.
     */
    void prepare(double sampleRate, int maxBlockSize);

    /**
     * @brief The main real-time processing callback.
     * Applies the full DSP chain to the incoming audio buffer in-place.
     *
     * @param buffer The audio buffer containing samples to process.
     * @param start  The starting index within the buffer.
     * @param num    The number of samples to process in this block.
     */
    void process(juce::AudioBuffer<float> &buffer, int start, int num);

    /** Resets all filters, gain states, and logic to their defaults. */
    void resetAll();

    // ==============================================================================
    // Parameter Setters (UI Hooks)
    // ==============================================================================

    /** Bypass the entire engine (Passthrough). */
    void setBypass(bool shouldBypass);

    /**
     * @brief Prevents the algorithm from filtering frequencies in the human vocal range.
     * Helps preserve clarity when noise reduction is aggressive.
     */
    void setVoiceProtect(bool shouldProtect);

    /** Engages a notch filter to remove 50Hz/60Hz ground loop hum. */
    void setHumFix(bool shouldFixHum);

    /**
     * @brief Enables "Delta" monitoring mode.
     * Inverts the phase of the processed signal and sums it with the original.
     * Result: The user hears ONLY the noise that is being removed.
     */
    void setListenDelta(bool shouldListenDelta);

    /**
     * @brief Sets the digital Pre-Amp gain.
     * @param isOn True to enable the boost.
     * @param amountPercent Boost amount (0.0 to 300.0).
     */
    void setMicBoost(bool isOn, float amountPercent);

    /**
     * @brief Selects the active DSP profile.
     * 0 = Standard, 1 = Broadcast (EQ), 2 = Vocal Isolation (Bandpass).
     */
    void setOperationMode(int modeIndex);

    // ==============================================================================
    // AI & Analysis
    // ==============================================================================

    /** Starts the 3-second background noise analysis. */
    void autoSetupBegin();

    /** Finishes analysis and calculates optimal thresholds based on RMS. */
    void autoSetupEnd();

    /** Returns the noise floor detected during the last analysis (0.0 - 1.0). */
    float getDetectedNoiseFloor() const { return detectedNoiseFloor_; }

    // ==============================================================================
    // Meters & Visuals
    // ==============================================================================

    /** Returns the current reduction amount in Decibels (negative value). */
    double getAttenuationDb() const;

    /** Returns the strength of the input signal */
    void setMasterStrength(double strengthPercentage); 

    // Individual Band Strength Setter
    void setBandStrength(int bandIndex, double strengthPercentage);

    // Getter for the 4 band strengths (Needed by MainComponent for autoSetupEnd)
    std::array<double, 4> getBandStrengths() const; 

    // Declaration for the updated getStrength()
    float getStrength() const;

    /** Returns the peak output level (0.0 - 1.0) for clipping detection. */
    float getOutputLevel() const { return lastOutputLevel_; }

private:
    /** Internal helper to update filter coefficients when mode changes. */
    void updateFilters();

    double sampleRate_{48000.0};
    int blockSize_{0};

    // --- Sub-Processors ---
    NLMSFilter nlms_;    // Adaptive filter
    STFTProcessor stft_; // Frequency Domain Processor (Overlap-Add)
    WienerPost post_;    // Spectral Subtraction Logic
    juce::AudioBuffer<float> monoScratch_;

    // --- Parameters ---
    bool bypass_{false};
    double masterStrength01_{0.5}; 
    std::array<double, 4> bandStrengths_{0.5, 0.5, 0.5, 0.5};
    bool voiceProtectOn_{true};
    bool humFixOn_{false};
    bool listenDelta_{false};

    bool micBoostOn_{false};
    float micBoostGain_{1.0f};
    int operationMode_{0};

    // --- DSP Filters (EQ) ---
    // We use ProcessorDuplicator to ensure the filter state is accessible and thread-safe.
    using FilterDup = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>,
                                                     juce::dsp::IIR::Coefficients<float>>;

    // Chain 1: Broadcast Mode (Low Shelf + High Shelf)
    juce::dsp::ProcessorChain<FilterDup, FilterDup> broadcastEQ_;

    // Chain 2: Isolation Mode (High Pass + Low Pass)
    juce::dsp::ProcessorChain<FilterDup, FilterDup> isolationEQ_;

    juce::dsp::ProcessorChain<FilterDup, FilterDup> humFilter_;

    using BandAnalyzer = juce::dsp::ProcessorChain<FilterDup, FilterDup>; 
    std::array<BandAnalyzer, 4> bandAnalyzers_;
    std::array<float, 4> noiseAccumulators_{0.0f};

    // --- AI State ---
    std::atomic<bool> isProfiling_{false};
    float detectedNoiseFloor_{0.0f};
    float noiseAccumulator_{0.0f};
    int noiseFrameCount_{0};

    // --- Noise Gate ---
    juce::LinearSmoothedValue<float> gateGain_{0.0f}; // Smooths volume changes
    int gateHoldCounter_{0};                          // Implements Hysteresis (Hold time)

    // --- Visuals ---
    double attenDb_{0.0};
    std::atomic<float> lastOutputLevel_{0.0f};
};