#pragma once
#include <juce_dsp/juce_dsp.h>
#include <juce_audio_basics/juce_audio_basics.h> 
#include <atomic>
#include "NLMSFilter.hpp"
#include "STFTProcessor.hpp"
#include "WienerPost.hpp"

class AudioEngine {
public:
    void prepare (double sampleRate, int maxBlock);
    void process (juce::AudioBuffer<float>& buffer, int start, int num);

    // UI Hooks
    void setBypass       (bool b);
    void setStrength     (double s01);
    void setVoiceProtect (bool b);
    void setHumFix       (bool b);
    void setListenDelta  (bool b);
    void resetAll();
    
    // Boost & Modes
    void setMicBoost(bool on, float amountPercent);
    void setOperationMode(int mode);

    // AI
    void autoSetupBegin();
    void autoSetupEnd();
    float getDetectedNoiseFloor() const { return detectedNoiseFloor_; }

    // Readout
    double getAttenuationDb() const;
    float getOutputLevel() const { return lastOutputLevel_; }

private:
    void updateFilters();

    double sr_ { 48000.0 };
    int    block_ { 0 };

    NLMSFilter      nlms_;
    STFTProcessor   stft_;
    WienerPost      post_;

    // Params
    bool   bypass_         { false };
    double strength01_     { 0.5 };
    bool   voiceProtectOn_ { true };
    bool   humFixOn_       { true };
    bool   listenDelta_    { false };
    
    // New Params
    bool  micBoostOn_      { false };
    float micBoostGain_    { 1.0f }; 
    int   operationMode_   { 0 };

    // --- FIX: Use ProcessorDuplicator ---
    // This wrapper guarantees that 'state' is publicly accessible.
    using FilterDup = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;

    // Chain 1: Broadcast (Low Shelf + High Shelf)
    juce::dsp::ProcessorChain<FilterDup, FilterDup> broadcastEQ_;
    
    // Chain 2: Isolation (High Pass + Low Pass)
    juce::dsp::ProcessorChain<FilterDup, FilterDup> isolationEQ_;

    // AI State
    std::atomic<bool> isProfiling_ { false };
    float detectedNoiseFloor_ { 0.0f };
    float noiseAccumulator_   { 0.0f };
    int   noiseFrameCount_    { 0 };

    // Gate
    juce::LinearSmoothedValue<float> gateGain_ { 0.0f };
    int gateHoldCounter_ { 0 };

    // UI Readout
    double attenDb_ { 0.0 };
    std::atomic<float> lastOutputLevel_ { 0.0f };
};