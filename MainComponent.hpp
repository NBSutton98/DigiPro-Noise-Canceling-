#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.hpp"

class MainComponent : public juce::AudioProcessorEditor,
                      private juce::Timer
{
public:
    MainComponent(NCliteAudioProcessor &p);
    ~MainComponent() override;

    // Layout & Paint
    void resized() override;
    void paint(juce::Graphics &g) override;

    // Visualization Helpers (Called by Processor)
    void pushInputToMeters(const juce::AudioBuffer<float> &buffer);
    void pushOutputToMeters(const juce::AudioBuffer<float> &buffer);

private:
    void timerCallback() override;
    void setupCallbacks();
    void startAutoSetup();
    void finishAutoSetup();

    enum class CleanState
    {
        Idle,
        Listening,
        Ready
    };
    CleanState state_{CleanState::Idle};

    // Reference to the Logic (Model)
    NCliteAudioProcessor &processor;

    // UI Elements
    juce::ToggleButton toggleOn_{"Processing Active"};
    juce::TextButton resetBtn_{"Reset"};

    juce::TextButton cleanBtn_{"Analyze & Clean Mic"};
    juce::Label pillAtten_;
    juce::Label lStatus_;

    juce::Label lStrength_;
    std::unique_ptr<juce::Label> strengthLabel_;
    juce::Slider strength_;

    juce::ToggleButton voiceProtect_{"Voice Protect"};
    juce::ToggleButton humFix_{"Hum Fix"};
    juce::ToggleButton deltaBtn_{"Hear Noise Only"};

    juce::ToggleButton boostToggle_{"Mic Boost"};
    juce::Slider boostSlider_;
    juce::Label boostLabel_;

    juce::ComboBox mode_;

    juce::Label lIn_, lOut_;
    juce::AudioVisualiserComponent inMeter_{1};
    juce::AudioVisualiserComponent outMeter_{1};

    std::unique_ptr<juce::LookAndFeel> lnf_;
    juce::TooltipWindow tooltipWindow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};