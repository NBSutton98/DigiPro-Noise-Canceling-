#include "AudioEngine.hpp"
#include <algorithm>
#include <cmath>

static inline float mapStrength01_toPostStrength (double s01)
{
    return static_cast<float>(std::clamp(s01, 0.0, 1.0));
}

void AudioEngine::prepare (double sr, int blockSize)
{
    sr_    = sr;
    block_ = blockSize;

    bypass_         = false;
    strength01_     = 0.50;
    
    nlms_.prepare(sr_, 256);
    stft_.prepare(sr_, blockSize);
    post_.prepare(sr_);
    post_.setStrength(mapStrength01_toPostStrength(strength01_));
    
    gateGain_.reset(sr_, 0.05);
    gateGain_.setCurrentAndTargetValue(1.0f); 

    // --- Prepare DSP Chains ---
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sr;
    spec.maximumBlockSize = blockSize;
    spec.numChannels = 2; 

    broadcastEQ_.prepare(spec);
    isolationEQ_.prepare(spec);
    
    updateFilters(); 
}

void AudioEngine::updateFilters()
{
    // FIX: Dereference assignment to avoid "inaccessible" errors
    auto& broadLow = broadcastEQ_.get<0>();
    *broadLow.state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sr_, 150.0f, 0.7f, 1.41f);

    auto& broadHigh = broadcastEQ_.get<1>();
    *broadHigh.state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sr_, 6000.0f, 0.7f, 1.26f);

    auto& isoHighPass = isolationEQ_.get<0>();
    *isoHighPass.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sr_, 300.0f);

    auto& isoLowPass = isolationEQ_.get<1>();
    *isoLowPass.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sr_, 3500.0f);
}

void AudioEngine::process (juce::AudioBuffer<float>& buf, int start, int numSamples)
{
    if (bypass_ || numSamples <= 0) return;

    // --- 0. MIC BOOST ---
    if (micBoostOn_)
    {
        buf.applyGain(micBoostGain_);
    }

    // --- 0.5 SAVE ORIGINAL ---
    juce::AudioBuffer<float> originalInput;
    if (listenDelta_) originalInput.makeCopyOf(buf);

    // --- 1. AI PROFILING ---
    if (isProfiling_)
    {
        float level = buf.getRMSLevel(0, start, numSamples);
        noiseAccumulator_ += level;
        noiseFrameCount_++;
    }

    // --- 2. SPECTRAL CLEANER ---
    // FIX: Explicit casts to float to silence MSVC warnings
    float effectiveStrength = (float)strength01_;
    
    if (operationMode_ == 2) 
        effectiveStrength = std::min(1.0f, (float)(strength01_ * 1.2)); 

    post_.setStrength(mapStrength01_toPostStrength(effectiveStrength));
    juce::AudioBuffer<float> subBuf(buf.getArrayOfWritePointers(), buf.getNumChannels(), start, numSamples);
    stft_.processBlock(subBuf, subBuf, post_);

    // --- 3. SMART GATE ---
    // FIX: Explicit cast to float for threshold calculation
    float activeThreshold = (float)((strength01_ * strength01_) * 0.05); 
    
    int holdSamples = (int)(0.2 * sr_); 
    gateGain_.reset(sr_, 0.1); 

    auto* channelData = buf.getWritePointer(0, start);
    for (int i = 0; i < numSamples; ++i)
    {
        float sample = channelData[i];
        float magnitude = std::abs(sample);
        float targetGain = 0.0f;
        
        if (magnitude > activeThreshold) { targetGain = 1.0f; gateHoldCounter_ = holdSamples; } 
        else {
            if (gateHoldCounter_ > 0) { targetGain = 1.0f; gateHoldCounter_--; }
            else { targetGain = 0.0f; }
        }
        gateGain_.setTargetValue(targetGain);
        float currentGain = gateGain_.getNextValue();
        
        channelData[i] *= currentGain;
        if (buf.getNumChannels() > 1) {
            auto* rightData = buf.getWritePointer(1, start);
            if (rightData) rightData[i] *= currentGain;
        }
    }

    // --- 4. APPLY MODES (EQ) ---
    juce::dsp::AudioBlock<float> block(buf);
    juce::dsp::AudioBlock<float> contextBlock = block.getSubBlock(start, numSamples);
    juce::dsp::ProcessContextReplacing<float> context(contextBlock);

    if (operationMode_ == 1) // Broadcast
    {
        broadcastEQ_.process(context);
    }
    else if (operationMode_ == 2) // Isolation
    {
        isolationEQ_.process(context);
    }

    // --- 5. DELTA LOGIC ---
    if (listenDelta_)
    {
        buf.applyGain(-1.0f);
        for (int ch = 0; ch < buf.getNumChannels(); ++ch)
            buf.addFrom(ch, start, originalInput, ch, start, numSamples);
        buf.applyGain(1.5f); 
    }

    // --- 6. SOFT CLIPPER & PEAK ---
    float maxPeak = 0.0f;
    auto* outL = buf.getWritePointer(0, start);
    auto* outR = (buf.getNumChannels() > 1) ? buf.getWritePointer(1, start) : nullptr;
    for (int i = 0; i < numSamples; ++i)
    {
        outL[i] = std::tanh(outL[i]);
        if (std::abs(outL[i]) > maxPeak) maxPeak = std::abs(outL[i]);
        if (outR) { outR[i] = std::tanh(outR[i]); }
    }
    lastOutputLevel_ = maxPeak;
    attenDb_ = -24.0 * strength01_;
}

// ... Getters/Setters ...
void AudioEngine::autoSetupBegin() { isProfiling_ = true; noiseAccumulator_ = 0; noiseFrameCount_ = 0; strength01_ = 0.0; }
void AudioEngine::autoSetupEnd() {
    isProfiling_ = false;
    if (noiseFrameCount_ > 0) {
        float db = juce::Decibels::gainToDecibels(noiseAccumulator_ / (float)noiseFrameCount_);
        if (db > -50.0f) strength01_ = 0.70;
        else if (db > -70.0f) strength01_ = 0.40;
        else strength01_ = 0.20;
    }
}
void AudioEngine::setBypass(bool b)       { bypass_ = b; }
void AudioEngine::setStrength(double s)   { strength01_ = std::clamp(s, 0.0, 1.0); }
void AudioEngine::setVoiceProtect(bool b) { voiceProtectOn_ = b; }
void AudioEngine::setHumFix(bool b)       { humFixOn_ = b; }
void AudioEngine::setListenDelta(bool b)  { listenDelta_ = b; } 

void AudioEngine::setMicBoost(bool on, float amountPercent) {
    micBoostOn_ = on;
    micBoostGain_ = 1.0f + (amountPercent / 100.0f);
}

void AudioEngine::setOperationMode(int mode) {
    operationMode_ = mode;
    updateFilters(); 
}

void AudioEngine::resetAll() { strength01_ = 0.5; attenDb_ = 0.0; gateGain_.setCurrentAndTargetValue(1.0f); listenDelta_ = false; micBoostOn_ = false; operationMode_ = 0; }
double AudioEngine::getAttenuationDb() const { return attenDb_; }