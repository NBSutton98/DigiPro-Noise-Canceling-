#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "AudioEngine.hpp"

class MainComponent : public juce::AudioAppComponent,
                      private juce::Timer
{
public:
    MainComponent();
    ~MainComponent() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;
    void resized() override;
    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;
    void setupCallbacks();
    void startAutoSetup();
    void finishAutoSetup();

    enum class CleanState { Idle, Listening, Ready };
    CleanState state_{ CleanState::Idle };

    AudioEngine engine_;

    // UI Elements
    juce::ToggleButton toggleOn_    { "Processing Active" };
    juce::TextButton   resetBtn_    { "Reset" };
    juce::TextButton   deviceBtn_   { "Settings" };
    juce::TextButton   cleanBtn_    { "Analyze & Clean Mic" };
    juce::Label        pillAtten_;
    juce::Label        lStatus_;
    juce::Label        lStrength_;
    std::unique_ptr<juce::Label> strengthLabel_;
    juce::Slider       strength_;
    
    // Toggles Left
    juce::ToggleButton voiceProtect_ { "Voice Protect" };
    juce::ToggleButton humFix_       { "Hum Fix" };
    juce::ToggleButton deltaBtn_     { "Hear Noise Only" }; 
    
    // NEW: Mic Boost Section
    juce::ToggleButton boostToggle_  { "Mic Boost" };
    juce::Slider       boostSlider_; 
    juce::Label        boostLabel_;

    juce::ComboBox     mode_;
    
    // Meters
    juce::Label        lIn_, lOut_;
    juce::AudioVisualiserComponent inMeter_  { 1 };
    juce::AudioVisualiserComponent outMeter_ { 1 };
    juce::AudioBuffer<float> preTap_;

    std::unique_ptr<juce::LookAndFeel> lnf_;
    juce::TooltipWindow tooltipWindow; 

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};