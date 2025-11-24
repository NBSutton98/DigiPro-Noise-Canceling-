/**
 * @file MainComponent.hpp
 * @brief The Graphical User Interface (GUI) for NClite.
 *
 * This class acts as the "View" and "Controller" in the MVC pattern.
 * It owns the AudioEngine (Model), manages the layout of widgets,
 * and handles user interactions via lambda callbacks.
 */

#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "AudioEngine.hpp"

class MainComponent : public juce::AudioAppComponent,
                      private juce::Timer
{
public:
    // ==============================================================================
    // Lifecycle
    // ==============================================================================
    MainComponent();
    ~MainComponent() override;

    // ==============================================================================
    // Audio Callbacks (Inherited from AudioAppComponent)
    // ==============================================================================

    /** Called before playback starts. Allocates buffers and prepares the Engine. */
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;

    /**
     * @brief The Audio Thread Callback.
     * 1. Copies audio to the Input Meter buffer.
     * 2. Passes the buffer to AudioEngine::process().
     * 3. Copies the result to the Output Meter buffer.
     */
    void getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill) override;

    /** Called when playback stops. Cleans up memory. */
    void releaseResources() override;

    // ==============================================================================
    // Graphics Callbacks (Inherited from Component)
    // ==============================================================================

    /** Calculates the position and size of every widget when the window resizes. */
    void resized() override;

    /** Draws the background gradient and structural lines. */
    void paint(juce::Graphics &g) override;

private:
    // ==============================================================================
    // Internal Logic
    // ==============================================================================

    /** Runs at 30Hz (frames per second) to update meters and status text. */
    void timerCallback() override;

    /** Connects UI elements (Buttons/Sliders) to Engine functions via Lambdas. */
    void setupCallbacks();

    /** Initiates the AI profiling sequence. */
    void startAutoSetup();

    /** Finalizes the AI sequence and updates the UI with new thresholds. */
    void finishAutoSetup();

    // State machine for the AI calibration process
    enum class CleanState
    {
        Idle,
        Listening,
        Ready
    };
    CleanState state_{CleanState::Idle};

    // The DSP Brain
    AudioEngine engine_;

    // ==============================================================================
    // UI Widgets
    // ==============================================================================

    // --- Header ---
    juce::ToggleButton toggleOn_{"Processing Active"};
    juce::TextButton resetBtn_{"Reset"};
    juce::TextButton deviceBtn_{"Settings"};

    // --- Hero Section ---
    juce::TextButton cleanBtn_{"Analyze & Clean Mic"};
    juce::Label pillAtten_; // The "Badges" (e.g. -12 dB)
    juce::Label lStatus_;   // The text status bar at the bottom

    // --- Main Controls ---
    juce::Label lStrength_;
    std::unique_ptr<juce::Label> strengthLabel_;
    juce::Slider strength_; // The central Arc Knob

    // --- Left Column (Toggles) ---
    juce::ToggleButton voiceProtect_{"Voice Protect"};
    juce::ToggleButton humFix_{"Hum Fix"};
    juce::ToggleButton deltaBtn_{"Hear Noise Only"};

    // --- Left Column (Pre-Amp) ---
    juce::ToggleButton boostToggle_{"Mic Boost"};
    juce::Slider boostSlider_; // The linear "Fader"
    juce::Label boostLabel_;

    // --- Right Column ---
    juce::ComboBox mode_; // Standard / Broadcast / Isolation

    // --- Visualization ---
    juce::Label lIn_, lOut_;
    juce::AudioVisualiserComponent inMeter_{1};  // Oscilloscope (Input)
    juce::AudioVisualiserComponent outMeter_{1}; // Oscilloscope (Output)

    // Buffer to tap the audio signal *before* processing for the Input Meter
    juce::AudioBuffer<float> preTap_;

    // --- Styling ---
    std::unique_ptr<juce::LookAndFeel> lnf_; // Holds the Custom Theme
    juce::TooltipWindow tooltipWindow;       // Handles hover text popups

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};