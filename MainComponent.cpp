//
// Created by SBNut on 2025-11-04.
//

#include "MainComponent.h"


namespace {
void styleSlider(juce::Slider& s, double mn, double mx, double init, double step=0.0) {
s.setRange(mn, mx, step);
s.setValue(init);
s.setSliderStyle(juce::Slider::LinearHorizontal);
s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
}
}


MainComponent::MainComponent() {
setSize(980, 360);
setAudioChannels(2, 2); // 2 inputs (primary/ref), 2 outputs


lMu_.setText("mu (step)", juce::dontSendNotification); addAndMakeVisible(lMu_);
styleSlider(mu_, 0.0, 1.0, 0.2, 0.001); addAndMakeVisible(mu_);


lL_.setText("L (taps)", juce::dontSendNotification); addAndMakeVisible(lL_);
styleSlider(L_, 64, 2048, 256, 1); addAndMakeVisible(L_);


lN_.setText("FFT N", juce::dontSendNotification); addAndMakeVisible(lN_);
Nbox_.addItem("512", 512); Nbox_.addItem("1024", 1024); Nbox_.addItem("2048", 2048);
Nbox_.setSelectedId(1024); addAndMakeVisible(Nbox_);


lWin_.setText("Window", juce::dontSendNotification); addAndMakeVisible(lWin_);
winBox_.addItem("Hann", 1); winBox_.addItem("Hamming", 2); winBox_.addItem("Blackman", 3);
winBox_.setSelectedId(1); addAndMakeVisible(winBox_);


lPost_.setText("Post strength", juce::dontSendNotification); addAndMakeVisible(lPost_);
styleSlider(postStrength_, 0.0, 1.0, 0.5, 0.01); addAndMakeVisible(postStrength_);


addAndMakeVisible(freeze_);
addAndMakeVisible(enablePost_);
enablePost_.setToggleState(true, juce::dontSendNotification);


mu_.onValueChange = [this]{ engine_.setMu(mu_.getValue()); };
L_.onValueChange = [this]{ engine_.setFilterLength((int)L_.getValue()); };
freeze_.onClick = [this]{ engine_.setFrozen(freeze_.getToggleState()); };
enablePost_.onClick = [this]{ engine_.enablePost(enablePost_.getToggleState()); };
postStrength_.onValueChange = [this]{ engine_.setPostStrength(postStrength_.getValue()); };
Nbox_.onChange = [this]{ engine_.setFFTSize(Nbox_.getSelectedId()); };
winBox_.onChange = [this]{ engine_.setWindowType(winBox_.getSelectedId()); };


startTimerHz(20);
}


MainComponent::~MainComponent(){ shutdownAudio(); }


void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate){
engine_.prepare(sampleRate, samplesPerBlockExpected);
engine_.setMu(mu_.getValue());
engine_.setFilterLength((int)L_.getValue());
engine_.setFFTSize(Nbox_.getSelectedId());
engine_.setWindowType(winBox_.getSelectedId());
engine_.setPostStrength(postStrength_.getValue());
engine_.enablePost(enablePost_.getToggleState());
engine_.setFrozen(freeze_.getToggleState());
}


void MainComponent::releaseResources() {}


void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& info){
engine_.process(*info.buffer, info.startSample, info.numSamples);
}


void MainComponent::resized(){
auto r = getLocalBounds().reduced(12);
auto row = [&](int h){ auto rr=r.removeFromTop(h); r.removeFromTop(8); return rr; };


auto rr1=row(28); lMu_.setBounds(rr1.removeFromLeft(120)); mu_.setBounds(rr1);
auto rr2=row(28); lL_.setBounds(rr2.removeFromLeft(120)); L_.setBounds(rr2);
auto rr3=row(28); lN_.setBounds(rr3.removeFromLeft(120)); Nbox_.setBounds(rr3.removeFromLeft(140));
lWin_.setBounds(rr3.removeFromLeft(80)); winBox_.setBounds(rr3.removeFromLeft(140));
auto rr4=row(28); lPost_.setBounds(rr4.removeFromLeft(120)); postStrength_.setBounds(rr4.removeFromLeft(360));
freeze_.setBounds(rr4.removeFromLeft(180)); enablePost_.setBounds(rr4.removeFromLeft(180));
}


void MainComponent::timerCallback(){ /* reserved for meters/plots */ }