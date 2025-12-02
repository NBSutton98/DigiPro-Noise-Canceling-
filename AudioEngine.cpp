#include "AudioEngine.hpp"
#include <algorithm>
#include <cmath>
#include <array>

// Define the crossover frequencies used for multiband analysis
const std::array<float, 3> C = {300.0f, 1500.0f, 5000.0f};

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
    masterStrength01_ = 0.50;
    bandStrengths_.fill(0.5f);

    // Initialize Sub-Processors
    nlms_.prepare(sampleRate_, 256);
    stft_.prepare(sampleRate_, blockSize_);
    post_.prepare(sampleRate_);

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
    humFilter_.prepare(spec);

    humFilter_.reset();

    // Prepare Multiband Analyzers
    for (int i = 0; i < 4; ++i)
        bandAnalyzers_[i].prepare(spec);

    updateFilters(); // Load initial coefficients
    monoScratch_.setSize(1, blockSize_);
}

void AudioEngine::updateFilters()
{
    // 1. Broadcast EQ: Boost Bass (Warmth) and Treble (Air)
    *broadcastEQ_.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate_, 150.0f, 0.7f, 1.41f);
    *broadcastEQ_.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate_, 6000.0f, 0.7f, 1.26f);

    // 2. Isolation EQ: Bandpass (Telephone effect)
    *isolationEQ_.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate_, 300.0f);
    *isolationEQ_.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, 3500.0f);

    // 3. HUM REDUCTION: 4th Order Brick Wall
    const float CUTOFF_FREQ = 65.0f;
    const float Q_VALUE = 0.707f;

    auto hp_coeffs = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate_, CUTOFF_FREQ, Q_VALUE);
    auto ap_coeffs = juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate_, 1000.0f);

    if (humFixOn_)
    {
        *humFilter_.get<0>().state = *hp_coeffs;
        *humFilter_.get<1>().state = *hp_coeffs;
    }
    else
    {
        *humFilter_.get<0>().state = *ap_coeffs;
        *humFilter_.get<1>().state = *ap_coeffs;
    }

    // Configure Multiband Analyzer Filters (Bandpass)
    // Band 0: Low (<300Hz) - Low-Pass at 300Hz
    *bandAnalyzers_[0].get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, C[0], 0.707f);
    *bandAnalyzers_[0].get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate_, 1000.0f);

    // Band 1: Low-Mid (300Hz to 1.5kHz) - Band-Pass
    *bandAnalyzers_[1].get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate_, C[0], 0.707f);
    *bandAnalyzers_[1].get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, C[1], 0.707f);

    // Band 2: Mid-High (1.5kHz to 5kHz) - Band-Pass
    *bandAnalyzers_[2].get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate_, C[1], 0.707f);
    *bandAnalyzers_[2].get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate_, C[2], 0.707f);

    // Band 3: High (>5kHz) - High-Pass at 5kHz
    *bandAnalyzers_[3].get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate_, C[2], 0.707f);
    *bandAnalyzers_[3].get<1>().state = *juce::dsp::IIR::Coefficients<float>::makeAllPass(sampleRate_, 1000.0f);
}

void AudioEngine::process(juce::AudioBuffer<float> &buffer, int start, int numSamples)
{
    if (bypass_ || numSamples <= 0)
        return;

    // ==============================================================================
    // STAGE 1: Input Conditioning (FIXED DELTA SAVE LOCATION)
    // ==============================================================================

    if (micBoostOn_)
    {
        buffer.applyGain(micBoostGain_);
    }

    // Save copy AFTER Mic Boost is applied!
    juce::AudioBuffer<float> originalInput;
    if (listenDelta_)
        originalInput.makeCopyOf(buffer);

    // ==============================================================================
    // STAGE 2: Analysis (AI) - MULTIBAND MEASUREMENT
    // ==============================================================================
    if (isProfiling_)
    {
        // Copy input to a temp buffer to run through the analyzer filters without
        // affecting the main audio path (important for spectral cleaner input).
        juce::AudioBuffer<float> tempBuffer;
        tempBuffer.makeCopyOf(buffer);

        // Analyze noise level in each band
        for (int i = 0; i < 4; ++i)
        {
            // Create AudioBlock for the band analyzer
            juce::dsp::AudioBlock<float> tempBlock(tempBuffer);
            auto sub = tempBlock.getSubBlock((size_t)start, (size_t)numSamples);

            // Apply the band analyzer filter
            bandAnalyzers_[i].process(juce::dsp::ProcessContextReplacing<float>(sub));

            // Measure RMS level of the isolated band
            float level = tempBuffer.getRMSLevel(0, start, numSamples);
            noiseAccumulators_[i] += level;
        }
        noiseFrameCount_++;
    }

    // ==============================================================================
    // STAGE 3: Frequency Domain Processing (Spectral Cleaner) - MULTIBAND CONTROL
    // ==============================================================================

    std::array<float, 4> postStrengths;

    for (int i = 0; i < 4; ++i)
    {
        float bandStr = static_cast<float>(bandStrengths_[i]); // 0..1 from UI

        // Master knob is a global multiplier
        float effectiveStrength = (bandStr * static_cast<float>(masterStrength01_)) * 2.0f;

        // Slightly soften bands 1 & 2 when Voice Protect is on
        if (voiceProtectOn_ && (i == 1 || i == 2))
            effectiveStrength *= 0.9f;

        // Clamp to [0, 1] – this is what WienerPost expects
        effectiveStrength = std::clamp(effectiveStrength, 0.0f, 0.85f);

        postStrengths[i] = effectiveStrength;
    }

    post_.setStrengths(postStrengths.data());

    // --- Mono scratch: process once, apply to all channels ---
    const int numCh = buffer.getNumChannels();

    if (monoScratch_.getNumSamples() < numSamples)
        monoScratch_.setSize(1, numSamples, false, false, true);

    float *mono = monoScratch_.getWritePointer(0);

    if (numCh == 1)
    {
        const float *src = buffer.getReadPointer(0, start);
        std::memcpy(mono, src, (size_t)numSamples * sizeof(float));
    }
    else
    {
        const float *left = buffer.getReadPointer(0, start);
        const float *right = buffer.getReadPointer(1, start);
        for (int i = 0; i < numSamples; ++i)
            mono[i] = 0.5f * (left[i] + right[i]);
    }

    // Run STFT/Wiener on mono
    stft_.processBlock(monoScratch_, monoScratch_, post_);

    // Copy processed mono back into all channels for this block
    for (int ch = 0; ch < numCh; ++ch)
        buffer.copyFrom(ch, start, monoScratch_, 0, 0, numSamples);

    // ==============================================================================
    // STAGE 4: Time Domain Processing (Smart Noise Gate) 
    // ==============================================================================

    if (!listenDelta_)
    {
        // Threshold ranges from approx -70 dB (0.0003) up to -20 dB (0.1) based on master knob.
        float activeThreshold = 0.0015f;

        if (voiceProtectOn_)
            activeThreshold *= 0.5f;

        int holdSamples = (int)(0.15 * sampleRate_); // 150 ms hold

        auto *leftData = buffer.getWritePointer(0, start);
        auto *rightData = (buffer.getNumChannels() > 1)
                              ? buffer.getWritePointer(1, start)
                              : nullptr;

        for (int i = 0; i < numSamples; ++i)
        {
            float magnitude = std::abs(leftData[i]);
            float targetGain;

            // Hysteresis logic
            if (magnitude > activeThreshold)
            {
                targetGain = 1.0f;
                gateHoldCounter_ = holdSamples;
            }
            else if (gateHoldCounter_ > 0)
            {
                targetGain = 1.0f;
                --gateHoldCounter_;
            }
            else
            {
                targetGain = 0.0f;
            }

            float g;

            if (targetGain == 1.0f)
            {
                // Instant Attack - Teleport the gain to 1.0f immediately.
                gateGain_.reset(sampleRate_, 0.01); 
                gateGain_.setTargetValue(1.0f);
                g = gateGain_.getNextValue();
            }
            else // targetGain == 0.0f
            {
                // Smooth Release - Set target and ramp down slowly (0.05s).
                gateGain_.reset(sampleRate_, 0.15); 
                gateGain_.setTargetValue(0.0f);
                g = gateGain_.getNextValue();
            }

            leftData[i] *= g;
            if (rightData)
                rightData[i] *= g;
        }
    }

    // ==============================================================================
    // STAGE 5: Post-Processing (EQ & Modes)
    // ==============================================================================

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::AudioBlock<float> contextBlock = block.getSubBlock(start, numSamples);
    juce::dsp::ProcessContextReplacing<float> context(contextBlock);

    humFilter_.process(context);

    if (operationMode_ == 1) // Broadcast Mode
    {
        broadcastEQ_.process(context);
    }
    else if (operationMode_ == 2) // Isolation Mode
    {
        isolationEQ_.process(context);
    }

    // ==============================================================================
    // STAGE 6: Delta Monitoring
    // ==============================================================================
    if (listenDelta_)
    {
        // 1. Invert the ENTIRE processed signal (Processed * -1)
        buffer.applyGain(-1.0f);

        // 2. Add the original input to the now-inverted processed buffer.
        for (int ch = 0; ch < numCh; ++ch)
        {
            buffer.addFrom(ch, start,
                           originalInput, ch, start, numSamples);
        }

        // 3. Apply the final boost to the resultant Delta signal.
        buffer.applyGain(2.0f);
    }

    // ==============================================================================
    // STAGE 7: Safety & Visuals
    // ==============================================================================

    const float compensatoryGain = 0.707f;
    float maxPeak = 0.0f;
    auto *outL = buffer.getWritePointer(0, start);
    auto *outR = (buffer.getNumChannels() > 1) ? buffer.getWritePointer(1, start) : nullptr;

    for (int i = 0; i < numSamples; ++i)
    {
        outL[i] = std::tanh(outL[i]) * compensatoryGain; // Apply reduction after clip

        // Track peak for UI Redline
        if (std::abs(outL[i]) > maxPeak)
            maxPeak = std::abs(outL[i]);

        if (outR)
        {
            outR[i] = std::tanh(outR[i]) * compensatoryGain; // Apply reduction after clip
        }
    }

    lastOutputLevel_ = maxPeak;
    attenDb_ = -24.0 * masterStrength01_;
}

// ==============================================================================
// Setters & Helpers implementation
// ==============================================================================

//  Master Strength Setter
void AudioEngine::setMasterStrength(double s)
{
    masterStrength01_ = std::clamp(s, 0.0, 1.0);
}

// Individual Band Strength Setter
void AudioEngine::setBandStrength(int bandIndex, double s)
{
    if (bandIndex >= 0 && bandIndex < 4)
    {
        bandStrengths_[bandIndex] = std::clamp(s, 0.0, 1.0);
    }
}

void AudioEngine::autoSetupBegin()
{
    isProfiling_ = true;
    noiseFrameCount_ = 0;

    // Reset all 4 band accumulators
    for (int i = 0; i < 4; ++i)
        noiseAccumulators_[i] = 0.0f;

    // Reset strengths to minimum during profiling
    masterStrength01_ = 0.0;
    bandStrengths_ = {0.0, 0.0, 0.0, 0.0};
}

void AudioEngine::autoSetupEnd()
{
    isProfiling_ = false;

    if (noiseFrameCount_ <= 0)
        return;

    const float LOW_DB_THRESHOLD = -80.0f;
    const float HIGH_DB_THRESHOLD = -40.0f;

    // Calculate and set strengths for all 4 bands
    for (int i = 0; i < 4; ++i)
    {
        float avgGain = noiseAccumulators_[i] / (float)noiseFrameCount_;
        float db = juce::Decibels::gainToDecibels(avgGain);

        float clampedDb = juce::jlimit(LOW_DB_THRESHOLD, HIGH_DB_THRESHOLD, db);

        // Map noise level to band strength (0.1 to 1.0)
        float normalizedStrength = juce::jmap(clampedDb,
                                              LOW_DB_THRESHOLD, HIGH_DB_THRESHOLD,
                                              0.1f, 1.0f);

        bandStrengths_[i] = normalizedStrength;
    }

    // Set master strength to a moderate default (50%) to apply the learned ratios
    masterStrength01_ = 0.5;

    // Reset analysis flags/variables (only required if noiseAccumulator_ was used globally, but it is not)
    // noiseAccumulator_ = 0.0f;
    noiseFrameCount_ = 0;
}

void AudioEngine::setBypass(bool b) { bypass_ = b; }
void AudioEngine::setVoiceProtect(bool b) { voiceProtectOn_ = b; }
void AudioEngine::setHumFix(bool b)
{
    humFixOn_ = b;
    updateFilters();
}
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
    masterStrength01_ = 0.5;
    bandStrengths_ = {0.5, 0.5, 0.5, 0.5};
    attenDb_ = 0.0;
    gateGain_.setCurrentAndTargetValue(1.0f);
    listenDelta_ = false;
    micBoostOn_ = false;
    operationMode_ = 0;
}

double AudioEngine::getAttenuationDb() const { return attenDb_; }
float AudioEngine::getStrength() const { return (float)masterStrength01_; }
std::array<double, 4> AudioEngine::getBandStrengths() const { return bandStrengths_; }