#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include "AudioEngine.hpp"

class MainComponent : public juce::AudioAppComponent, private juce::Timer {
public:
  MainComponent();
  ~MainComponent() override;

  void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
  void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
  void releaseResources() override;
  void resized() override;

private:
  void timerCallback() override;

  // DSP engine
  AudioEngine engine_;

  // Controls
  juce::Slider mu_, L_, postStrength_;
  juce::ToggleButton freeze_{"Freeze learning"}, enablePost_{"Enable spectral cleaner"};
  juce::ComboBox Nbox_, winBox_;
  juce::Label lMu_, lL_, lN_, lWin_, lPost_;

  // Live feedback + devices
  juce::AudioVisualiserComponent outVis_{2};   // stereo output meter/scope
  juce::TextButton deviceBtn_{"Audio devices"};

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
