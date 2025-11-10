//
// Created by SBNut on 2025-11-04.
//

#ifndef MAINCOMPONENT_HPP
#define MAINCOMPONENT_HPP



#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "AudioEngine.h"


class MainComponent : public juce::AudioAppComponent,
private juce::Timer {
public:
MainComponent();
~MainComponent() override;


void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
void releaseResources() override;


void resized() override;


private:
void timerCallback() override;


AudioEngine engine_;


juce::Slider mu_, L_, postStrength_;
juce::ToggleButton freeze_{"Freeze adaptation"}, enablePost_{"Enable post-filter"};
juce::ComboBox Nbox_, winBox_;
juce::Label lMu_, lL_, lN_, lWin_, lPost_;


JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};


#endif //MAINCOMPONENT_HPP
