#include "AudioEngine.hpp"
#include <algorithm>
#include <cmath>

// Helper: Maps the UI Slider (0-1) to a usable DSP strength value.
static inline float mapStrength01_toPostStrength(double s01)
{
    return static_cast<float>(std::clamp(s01, 0.0, 1.0));
}

void AudioEngine::prepare(double sr, int maxBlockSize)
{
    sampleRate_ = sr;
    blockSize_ = maxBlockSize;

    // Default Initial State
    bypass_ = false;
    strength01_ = 0.50;

    // Initialize Sub-Processors
    nlms_.prepare(sampleRate_, 256);
    stft_.prepare(sampleRate_, blockSize_);
    post_.prepare(sampleRate_);
    post_.setStrength(mapStrength01_toPostStrength(strength01_));

    // Initialize Gate (Start Open)
    gateGain_.reset(sampleRate_, 0.05); // 50ms ramp time
    gateGain_.setCurrentAndTargetValue(1.0f);

    // --- Initialize DSP Chains ---
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate_;
    spec.maximumBlockSize = blockSize_;
    spec.numChannels = 2; // Duplicators handle stereo automatically

    broadcastEQ_.prepare(spec);
    isolationEQ_.prepare(spec);

    updateFilters(); // Load initial coefficients
}

void AudioEngine::updateFilters()
{
    // We access the filter state directly via the Duplicator.
    // dereferencing (*state) allows us to assign the new coefficients safely.

    // 1. Broadcast EQ: Boost Bass (Warmth) and Treble (Air)
    *broadcastEQ_.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate_, 150.0f, 0.7f, 1.41f);
    *broadcastEQ_.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate_, 6000.0f, 0.7f, 1.26f);

    // 2. Isolation EQ: Bandpass (Telephone effect)
    *isolationEQ_.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate_, 300.0f);
    *isolationEQ_.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, 3500.0f);
}

void AudioEngine::process(juce::AudioBuffer<float> &buffer, int start, int numSamples)
{
    if (bypass_ || numSamples <= 0)
        return;

    // ==============================================================================
    // STAGE 1: Input Conditioning
    // ==============================================================================

    // Apply Mic Boost if enabled
    if (micBoostOn_)
    {
        buffer.applyGain(micBoostGain_);
    }

    // Save a clean copy of the input if we are in "Delta Listen" mode
    juce::AudioBuffer<float> originalInput;
    if (listenDelta_)
        originalInput.makeCopyOf(buffer);

    // ==============================================================================
    // STAGE 2: Analysis (AI)
    // ==============================================================================
    if (isProfiling_)
    {
        // Measure the noise floor RMS
        float level = buffer.getRMSLevel(0, start, numSamples);
        noiseAccumulator_ += level;
        noiseFrameCount_++;
    }

    // ==============================================================================
    // STAGE 3: Frequency Domain Processing (Spectral Cleaner)
    // ==============================================================================

    // Calculate effective strength (Boosted slightly in Isolation mode)
    float effectiveStrength = (float)strength01_;
    if (operationMode_ == 2)
        effectiveStrength = std::min(1.0f, (float)(strength01_ * 1.2));

    // Run the STFT (Short-Time Fourier Transform) pipeline
    post_.setStrength(mapStrength01_toPostStrength(effectiveStrength));

    // Create a sub-view of the buffer for processing
    juce::AudioBuffer<float> subBuf(buffer.getArrayOfWritePointers(), buffer.getNumChannels(), start, numSamples);
    stft_.processBlock(subBuf, subBuf, post_);

    // ==============================================================================
    // STAGE 4: Time Domain Processing (Smart Noise Gate)
    // ==============================================================================

    // Threshold Curve: Quadratic (strength^2) for finer control at low levels
    float activeThreshold = (float)((strength01_ * strength01_) * 0.05);

    // Hold Time: 200ms to prevent chopping off the ends of words
    int holdSamples = (int)(0.2 * sampleRate_);

    gateGain_.reset(sampleRate_, 0.1); // 100ms release time

    auto *channelData = buffer.getWritePointer(0, start);

    for (int i = 0; i < numSamples; ++i)
    {
        float sample = channelData[i];
        float magnitude = std::abs(sample);
        float targetGain = 0.0f;

        // Gate Logic with Hysteresis
        if (magnitude > activeThreshold)
        {
            // Open Gate immediately
            targetGain = 1.0f;
            gateHoldCounter_ = holdSamples;
        }
        else
        {
            // Signal is quiet. Are we holding?
            if (gateHoldCounter_ > 0)
            {
                targetGain = 1.0f; // Keep open
                gateHoldCounter_--;
            }
            else
            {
                targetGain = 0.0f; // Close Gate
            }
        }

        gateGain_.setTargetValue(targetGain);
        float currentGain = gateGain_.getNextValue();

        // Apply gain to Mono (Left)
        channelData[i] *= currentGain;

        // Apply same gain to Right channel if Stereo
        if (buffer.getNumChannels() > 1)
        {
            auto *rightData = buffer.getWritePointer(1, start);
            if (rightData)
                rightData[i] *= currentGain;
        }
    }

    // ==============================================================================
    // STAGE 5: Post-Processing (EQ & Modes)
    // ==============================================================================

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> contextBlock = block.getSubBlock(start, numSamples);
    juce::dsp::ProcessContextReplacing<float> context(contextBlock);

    if (operationMode_ == 1) // Broadcast Mode
    {
        broadcastEQ_.process(context);
    }
    else if (operationMode_ == 2) // Isolation Mode
    {
        isolationEQ_.process(context);
    }

    // ==============================================================================
    // STAGE 6: Delta Monitoring (Debug)
    // ==============================================================================
    if (listenDelta_)
    {
        // Math: Noise = Original + (Processed * -1)
        buffer.applyGain(-1.0f);
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addFrom(ch, start, originalInput, ch, start, numSamples);

        // Slight boost to make the removed noise audible
        buffer.applyGain(1.0f);
    }

    // ==============================================================================
    // STAGE 7: Safety & Visuals
    // ==============================================================================

    float maxPeak = 0.0f;
    auto *outL = buffer.getWritePointer(0, start);
    auto *outR = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1, start) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        // Soft Clipper (tanh) to prevent digital distortion > 0dB
        outL[i] = std::tanh(outL[i]);

        // Track peak for UI Redline
        if (std::abs(outL[i]) > maxPeak)
            maxPeak = std::abs(outL[i]);

        if (outR)
        {
            outR[i] = std::tanh(outR[i]);
        }
    }

    lastOutputLevel_ = maxPeak;
    attenDb_ = -24.0 * strength01_;
}

// ==============================================================================
// Setters & Helpers implementation
// ==============================================================================

void AudioEngine::autoSetupBegin()
{
    isProfiling_ = true;
    noiseAccumulator_ = 0;
    noiseFrameCount_ = 0;
    strength01_ = 0.0;
}

void AudioEngine::autoSetupEnd()
{
    isProfiling_ = false;

    float avgGain = 0.0f; 
    noiseAccumulator_ = 0.0f;
    
    if (noiseFrameCount_ > 0)
    {
        avgGain = noiseAccumulator_ / (float)noiseFrameCount_;

        float db = juce::Decibels::gainToDecibels(avgGain);
        const float LOW_DB_THRESHOLD = -80.0f;
        const float HIGH_DB_THRESHOLD = -40.0f; 

        float clampedDb = juce::jlimit(LOW_DB_THRESHOLD, HIGH_DB_THRESHOLD, db);

        float normalizedStrength = juce::jmap(clampedDb, 
                                              LOW_DB_THRESHOLD, 
                                              HIGH_DB_THRESHOLD, 
                                              0.1f, 
                                              1.0f  
                                             );

        strength01_ = normalizedStrength;
        noiseAccumulator_ = 0.0f;
        noiseFrameCount_ = 0;
    }
}

void AudioEngine::setBypass(bool b) { bypass_ = b; }
void AudioEngine::setStrength(double s) { strength01_ = std::clamp(s, 0.0, 1.0); }
void AudioEngine::setVoiceProtect(bool b) { voiceProtectOn_ = b; }
void AudioEngine::setHumFix(bool b) { humFixOn_ = b; }
void AudioEngine::setListenDelta(bool b) { listenDelta_ = b; }

void AudioEngine::setMicBoost(bool on, float amountPercent)
{
    micBoostOn_ = on;
    // Map 0-300% to 1.0x - 4.0x gain multiplier
    micBoostGain_ = 1.0f + (amountPercent / 100.0f);
}

void AudioEngine::setOperationMode(int mode)
{
    operationMode_ = mode;
    updateFilters();
}

void AudioEngine::resetAll()
{
    strength01_ = 0.5;
    attenDb_ = 0.0;
    gateGain_.setCurrentAndTargetValue(1.0f);
    listenDelta_ = false;
    micBoostOn_ = false;
    operationMode_ = 0;
}

double AudioEngine::getAttenuationDb() const { return attenDb_; }