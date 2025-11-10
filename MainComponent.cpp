#include "MainComponent.hpp"

namespace
{
  void styleSlider(juce::Slider &s, double mn, double mx, double init, double step = 0.0)
  {
    s.setRange(mn, mx, step);
    s.setValue(init);
    s.setSliderStyle(juce::Slider::LinearHorizontal);
    s.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
  }
}

MainComponent::MainComponent()
{
  setSize(1100, 420);
  setAudioChannels(2, 2); // in: 2 (primary/ref), out: 2

  // Labels
  lMu_.setText("Learning rate", juce::dontSendNotification);
  addAndMakeVisible(lMu_);
  lL_.setText("Filter length", juce::dontSendNotification);
  addAndMakeVisible(lL_);
  lN_.setText("FFT size", juce::dontSendNotification);
  addAndMakeVisible(lN_);
  lWin_.setText("Window type", juce::dontSendNotification);
  addAndMakeVisible(lWin_);
  lPost_.setText("Noise reduction", juce::dontSendNotification);
  addAndMakeVisible(lPost_);

  // Sliders/boxes
  styleSlider(mu_, 0.01, 0.40, 0.20, 0.001);
  addAndMakeVisible(mu_);
  styleSlider(L_, 64, 2048, 256, 1);
  addAndMakeVisible(L_);

  Nbox_.addItem("512", 512);
  Nbox_.addItem("1024", 1024);
  Nbox_.addItem("2048", 2048);
  Nbox_.setSelectedId(1024);
  addAndMakeVisible(Nbox_);

  winBox_.addItem("Hann", 1);
  winBox_.addItem("Hamming", 2);
  winBox_.addItem("Blackman", 3);
  winBox_.setSelectedId(1);
  addAndMakeVisible(winBox_);

  styleSlider(postStrength_, 0.0, 1.0, 0.50, 0.01);
  addAndMakeVisible(postStrength_);

  // Toggles
  addAndMakeVisible(freeze_);
  addAndMakeVisible(enablePost_);
  enablePost_.setToggleState(true, juce::dontSendNotification);

  // Tooltips
  mu_.setTooltip("How fast the canceller learns. Higher = faster, risk of warble.");
  L_.setTooltip("Number of FIR taps. Longer models longer paths, uses more CPU.");
  Nbox_.setTooltip("FFT size for spectral cleaner. Larger = more latency.");
  winBox_.setTooltip("STFT window for analysis/synthesis.");
  postStrength_.setTooltip("Amount of extra spectral noise reduction.");
  freeze_.setTooltip("Stop learning to avoid canceling speech.");
  enablePost_.setTooltip("Enable the frequency-domain cleaner after cancellation.");
  deviceBtn_.setTooltip("Pick input/output devices and channel mapping.");

  // Live output visualiser
  outVis_.setBufferSize(256);
  outVis_.setSamplesPerBlock(128);
  outVis_.setOpaque(false);  
  outVis_.setColours(juce::Colours::transparentBlack, juce::Colours::skyblue);
  addAndMakeVisible(outVis_);

  // Device selector dialog
  deviceBtn_.onClick = [this]
  {
    auto* dm = &deviceManager; 
    auto *comp = new juce::AudioDeviceSelectorComponent(*dm,

                                                        0, 2, // min,max inputs
                                                        0, 2, // min,max outputs
                                                        true, true, true, false);
    comp->setSize(640, 380);
    juce::DialogWindow::LaunchOptions o;
    o.content.setOwned(comp);
    o.dialogTitle = "Audio devices";
    o.useNativeTitleBar = true;
    o.resizable = true;
    o.componentToCentreAround = this; 
    o.launchAsync(); 
  };
  addAndMakeVisible(deviceBtn_);

  // Wiring
  mu_.onValueChange = [this]
  { engine_.setMu(mu_.getValue()); };
  L_.onValueChange = [this]
  { engine_.setFilterLength((int)L_.getValue()); };
  freeze_.onClick = [this]
  { engine_.setFrozen(freeze_.getToggleState()); };
  enablePost_.onClick = [this]
  { engine_.enablePost(enablePost_.getToggleState()); };
  postStrength_.onValueChange = [this]
  { engine_.setPostStrength(postStrength_.getValue()); };
  Nbox_.onChange = [this]
  { engine_.setFFTSize(Nbox_.getSelectedId()); };
  winBox_.onChange = [this]
  { engine_.setWindowType(winBox_.getSelectedId()); };

  startTimerHz(20);
}

MainComponent::~MainComponent() { shutdownAudio(); }

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
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

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &info)
{
  engine_.process(*info.buffer, info.startSample, info.numSamples);
  // push post-processed audio into the visualiser
  outVis_.pushBuffer(info.buffer->getArrayOfReadPointers(),
                     juce::jmin(2, info.buffer->getNumChannels()),
                     info.numSamples);
}

void MainComponent::resized()
{
  auto r = getLocalBounds().reduced(12);

  // top bar with device button
  auto top = r.removeFromTop(28);
  deviceBtn_.setBounds(top.removeFromRight(160));

  auto row = [&](int h)
  { auto rr = r.removeFromTop(h); r.removeFromTop(8); return rr; };

  auto rr1 = row(28);
  lMu_.setBounds(rr1.removeFromLeft(140));
  mu_.setBounds(rr1);
  auto rr2 = row(28);
  lL_.setBounds(rr2.removeFromLeft(140));
  L_.setBounds(rr2);

  auto rr3 = row(28);
  lN_.setBounds(rr3.removeFromLeft(140));
  Nbox_.setBounds(rr3.removeFromLeft(120));
  lWin_.setBounds(rr3.removeFromLeft(120));
  winBox_.setBounds(rr3.removeFromLeft(140));

  auto rr4 = row(28);
  lPost_.setBounds(rr4.removeFromLeft(140));
  postStrength_.setBounds(rr4.removeFromLeft(360));
  freeze_.setBounds(rr4.removeFromLeft(160));
  enablePost_.setBounds(rr4.removeFromLeft(200));

  // meters/scope occupies the rest
  outVis_.setBounds(r.removeFromTop(160));
}

void MainComponent::timerCallback() { /* reserved for future meters/plots */ }
