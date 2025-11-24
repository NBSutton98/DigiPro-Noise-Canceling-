/**
 * @file MainComponent.cpp
 * @brief Implementation of the UI and Custom LookAndFeel.
 */

#include "MainComponent.hpp"

// ==============================================================================
// Custom Theme (Look And Feel)
// ==============================================================================
namespace Styling
{
  const juce::Colour bgDark = juce::Colour(0xFF161B22);    // Github Dark Dimmed
  const juce::Colour panelDark = juce::Colour(0xFF21262D); // Panel Background
  const juce::Colour accent = juce::Colour(0xFF58A6FF);    // Electric Blue
  const juce::Colour textBright = juce::Colour(0xFFF0F6FC);
  const juce::Colour textDim = juce::Colour(0xFF8B949E);

  /**
   * @class ModernLNF
   * @brief Overrides default JUCE drawing methods to create a flat, modern aesthetic.
   */
  class ModernLNF : public juce::LookAndFeel_V4
  {
  public:
    ModernLNF()
    {
      // Set global color palette
      setColour(juce::ResizableWindow::backgroundColourId, bgDark);
      setColour(juce::Label::textColourId, textBright);
      setColour(juce::TextButton::buttonColourId, panelDark);
      setColour(juce::TextButton::textColourOffId, textBright);
      setColour(juce::ComboBox::backgroundColourId, panelDark);
      setColour(juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);

      // Tooltip Styling
      setColour(juce::TooltipWindow::backgroundColourId, juce::Colour(0xFF111111));
      setColour(juce::TooltipWindow::textColourId, textBright);
      setColour(juce::TooltipWindow::outlineColourId, accent);
    }

    /** Draws the custom "Power Gauge" Fader for the Mic Boost. */
    void drawLinearSlider(juce::Graphics &g, int x, int y, int width, int height,
                          float sliderPos, float minSliderPos, float maxSliderPos,
                          const juce::Slider::SliderStyle style, juce::Slider &slider) override
    {
      juce::ignoreUnused(minSliderPos, maxSliderPos, style, slider);

      // 1. Draw Track (Recessed Slot)
      auto trackHeight = 6.0f;
      auto trackY = y + (height - trackHeight) * 0.5f;

      g.setColour(juce::Colours::black.withAlpha(0.5f));
      g.fillRoundedRectangle((float)x, trackY, (float)width, trackHeight, trackHeight / 2.0f);

      // 2. Draw Fill (Gradient from Dark Blue to Bright Cyan)
      float fillWidth = sliderPos - (float)x;
      if (fillWidth > 0)
      {
        g.setColour(accent.darker(0.1f));
        g.fillRoundedRectangle((float)x, trackY, fillWidth, trackHeight, trackHeight / 2.0f);
      }

      // 3. Draw Thumb (White Handle) with Glow
      float thumbSize = 16.0f;
      g.setColour(juce::Colours::white);
      g.fillEllipse(sliderPos - thumbSize * 0.5f, y + (height - thumbSize) * 0.5f, thumbSize, thumbSize);

      g.setColour(accent.withAlpha(0.4f)); // Glow ring
      g.drawEllipse(sliderPos - thumbSize * 0.5f, y + (height - thumbSize) * 0.5f, thumbSize, thumbSize, 2.0f);
    }

    /** Draws the iOS-style Pill Toggles. */
    void drawToggleButton(juce::Graphics &g, juce::ToggleButton &button,
                          bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
      juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
      auto bounds = button.getLocalBounds().toFloat().reduced(2);
      bool on = button.getToggleState();

      // Draw Text Label
      g.setFont(juce::Font(14.0f));
      g.setColour(textBright);
      g.drawText(button.getButtonText(), bounds.removeFromRight(bounds.getWidth() - 38).toNearestInt(), juce::Justification::centredLeft);

      // Draw Switch Body
      auto toggleRect = bounds.removeFromLeft(34).reduced(0, 6);
      g.setColour(on ? accent : juce::Colours::darkgrey);
      g.fillRoundedRectangle(toggleRect, toggleRect.getHeight() / 2.0f);

      // Draw Switch Thumb
      float thumbSize = toggleRect.getHeight() - 4;
      float thumbX = on ? (toggleRect.getRight() - thumbSize - 2) : (toggleRect.getX() + 2);
      g.setColour(juce::Colours::white);
      g.fillEllipse(thumbX, toggleRect.getY() + 2, thumbSize, thumbSize);
    }

    /** Draws the modern "Arc" style rotary knob. */
    void drawRotarySlider(juce::Graphics &g, int x, int y, int width, int height, float sliderPos,
                          const float rotaryStartAngle, const float rotaryEndAngle, juce::Slider &slider) override
    {
      juce::ignoreUnused(slider);
      auto radius = (float)juce::jmin(width / 2, height / 2) - 4.0f;
      auto centreX = (float)x + (float)width * 0.5f;
      auto centreY = (float)y + (float)height * 0.5f;
      auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

      // Background Arc
      juce::Path bgArc;
      bgArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, rotaryEndAngle, true);
      g.setColour(juce::Colours::black.withAlpha(0.4f));
      g.strokePath(bgArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

      // Value Arc (Active)
      juce::Path valArc;
      valArc.addCentredArc(centreX, centreY, radius, radius, 0.0f, rotaryStartAngle, angle, true);
      g.setColour(accent);
      g.strokePath(valArc, juce::PathStrokeType(6.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    /** Draws flat, rounded buttons. */
    void drawButtonBackground(juce::Graphics &g, juce::Button &button, const juce::Colour &backgroundColour,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override
    {
      auto cornerSize = 6.0f;
      auto bounds = button.getLocalBounds().toFloat().reduced(0.5f, 0.5f);
      auto baseColour = backgroundColour;
      if (shouldDrawButtonAsDown)
        baseColour = baseColour.darker(0.2f);
      else if (shouldDrawButtonAsHighlighted)
        baseColour = baseColour.brighter(0.1f);
      g.setColour(baseColour);
      g.fillRoundedRectangle(bounds, cornerSize);
      g.setColour(juce::Colours::white.withAlpha(0.1f));
      g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    }
  };
}

// ==============================================================================
// Main Component Implementation
// ==============================================================================

MainComponent::MainComponent()
{
  // Apply Custom Theme
  lnf_ = std::make_unique<Styling::ModernLNF>();
  setLookAndFeel(lnf_.get());

  setSize(900, 600);
  setAudioChannels(2, 2); // Request Input/Output

  // --- Initialize Header ---
  toggleOn_.setButtonText("Processing Active");
  toggleOn_.setToggleState(true, juce::dontSendNotification);
  toggleOn_.setTooltip("Master Bypass.");
  addAndMakeVisible(toggleOn_);

  deviceBtn_.setButtonText("Audio Settings");
  addAndMakeVisible(deviceBtn_);
  resetBtn_.setButtonText("Reset Defaults");
  addAndMakeVisible(resetBtn_);

  // --- Initialize Hero Section ---
  cleanBtn_.setButtonText("Analyze & Clean Mic");
  cleanBtn_.setColour(juce::TextButton::buttonColourId, Styling::accent);
  cleanBtn_.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
  cleanBtn_.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
  addAndMakeVisible(cleanBtn_);

  pillAtten_.setText("Ready", juce::dontSendNotification);
  pillAtten_.setColour(juce::Label::backgroundColourId, Styling::panelDark);
  pillAtten_.setColour(juce::Label::textColourId, Styling::accent);
  pillAtten_.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(pillAtten_);

  // --- Initialize Meters ---
  inMeter_.setColours(juce::Colours::black, Styling::accent.darker(0.4f));
  outMeter_.setColours(juce::Colours::black, Styling::accent);
  inMeter_.setBufferSize(256);
  inMeter_.setSamplesPerBlock(128);
  outMeter_.setBufferSize(256);
  outMeter_.setSamplesPerBlock(128);
  addAndMakeVisible(inMeter_);
  addAndMakeVisible(outMeter_);

  lIn_.setText("INPUT SIGNAL", juce::dontSendNotification);
  lIn_.setFont(juce::Font(10.0f, juce::Font::bold));
  lIn_.setColour(juce::Label::textColourId, Styling::textDim);
  lIn_.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(lIn_);
  lOut_.setText("CLEAN OUTPUT", juce::dontSendNotification);
  lOut_.setFont(juce::Font(10.0f, juce::Font::bold));
  lOut_.setColour(juce::Label::textColourId, Styling::textDim);
  lOut_.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(lOut_);

  // --- Initialize Main Controls ---
  strength_.setRange(0.0, 100.0, 1.0);
  strength_.setValue(50.0);
  strength_.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
  strength_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  strength_.setTooltip("Master Noise Reduction Amount.");
  addAndMakeVisible(strength_);

  lStrength_.setText("50%", juce::dontSendNotification);
  lStrength_.setJustificationType(juce::Justification::centred);
  lStrength_.setFont(juce::Font(24.0f, juce::Font::bold));
  lStrength_.setColour(juce::Label::textColourId, Styling::accent);
  addAndMakeVisible(lStrength_);

  strengthLabel_.reset(new juce::Label("sLab", "NOISE REDUCTION"));
  strengthLabel_->setFont(juce::Font(11.0f));
  strengthLabel_->setJustificationType(juce::Justification::centred);
  strengthLabel_->setColour(juce::Label::textColourId, Styling::textDim);
  addAndMakeVisible(strengthLabel_.get());

  // --- Initialize Side Controls ---
  voiceProtect_.setToggleState(true, juce::dontSendNotification);
  voiceProtect_.setTooltip("Prevents removing human speech.");
  addAndMakeVisible(voiceProtect_);

  humFix_.setToggleState(true, juce::dontSendNotification);
  humFix_.setTooltip("Removes 60Hz ground loop hum.");
  addAndMakeVisible(humFix_);

  deltaBtn_.setTooltip("Listen to the removed noise only.");
  addAndMakeVisible(deltaBtn_);

  // --- Initialize Mic Boost ---
  boostToggle_.setToggleState(false, juce::dontSendNotification);
  boostToggle_.setTooltip("Enables digital Pre-Amp gain.");
  addAndMakeVisible(boostToggle_);

  boostSlider_.setRange(0.0, 300.0, 1.0);
  boostSlider_.setValue(100.0);
  boostSlider_.setSliderStyle(juce::Slider::LinearHorizontal); // Custom Fader
  boostSlider_.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
  boostSlider_.setTooltip("Gain amount (0% to 300%)");
  addAndMakeVisible(boostSlider_);

  boostLabel_.setText("+100%", juce::dontSendNotification);
  boostLabel_.setFont(juce::Font(14.0f, juce::Font::bold));
  boostLabel_.setColour(juce::Label::textColourId, Styling::accent);
  boostLabel_.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(boostLabel_);

  // --- Initialize Modes ---
  mode_.addItem("Standard Clean", 1);
  mode_.addItem("Broadcast (Rich)", 2);
  mode_.addItem("Vocal Isolation", 3);
  mode_.setSelectedId(1);
  mode_.setTooltip("Select the DSP profile.");
  addAndMakeVisible(mode_);

  lStatus_.setText("System Ready.", juce::dontSendNotification);
  lStatus_.setColour(juce::Label::textColourId, Styling::textDim);
  lStatus_.setJustificationType(juce::Justification::centred);
  addAndMakeVisible(lStatus_);

  setupCallbacks();
  startTimerHz(30); // Start UI Update loop
}

MainComponent::~MainComponent()
{
  setLookAndFeel(nullptr);
  shutdownAudio();
}

// ==============================================================================
// Layout Engine
// ==============================================================================
void MainComponent::resized()
{
  auto area = getLocalBounds();
  auto header = area.removeFromTop(60).reduced(15, 10);
  toggleOn_.setBounds(header.removeFromLeft(160));
  deviceBtn_.setBounds(header.removeFromRight(110));
  header.removeFromRight(10);
  resetBtn_.setBounds(header.removeFromRight(110));

  auto footer = area.removeFromBottom(30);
  lStatus_.setBounds(footer);

  area.reduce(20, 10);
  auto controls = area.removeFromBottom(140);
  auto cLeft = controls.removeFromLeft(controls.getWidth() / 3);
  auto cRight = controls.removeFromRight(controls.getWidth() / 2);
  auto cCenter = controls;

  // --- Left Column Layout ---
  cLeft.reduce(10, 5);
  voiceProtect_.setBounds(cLeft.removeFromTop(25));
  humFix_.setBounds(cLeft.removeFromTop(25));
  deltaBtn_.setBounds(cLeft.removeFromTop(25));
  cLeft.removeFromTop(10); // Spacer

  // Mic Boost Stack
  boostToggle_.setBounds(cLeft.removeFromTop(25));
  boostSlider_.setBounds(cLeft.removeFromTop(25));
  boostLabel_.setBounds(cLeft.removeFromTop(20));

  // --- Right Column Layout ---
  cRight.reduce(20, 20);
  cleanBtn_.setBounds(cRight.removeFromTop(40));
  cRight.removeFromTop(10);
  mode_.setBounds(cRight.removeFromTop(30));

  // --- Center Column Layout ---
  cCenter.reduce(10, 0);
  lStrength_.setBounds(cCenter.removeFromTop(30));
  strength_.setBounds(cCenter.removeFromTop(80));
  if (strengthLabel_)
    strengthLabel_->setBounds(cCenter);

  // --- Meter Layout ---
  area.removeFromBottom(20);
  auto badgeRow = area.removeFromTop(40);
  pillAtten_.setBounds(badgeRow.getCentreX() - 60, badgeRow.getCentreY() - 14, 120, 28);
  pillAtten_.toFront(true);
  int gap = 12;
  int panelWidth = (area.getWidth() - gap) / 2;
  auto inRect = area.removeFromLeft(panelWidth);
  auto outRect = area;
  lIn_.setBounds(inRect.removeFromTop(20));
  inMeter_.setBounds(inRect);
  lOut_.setBounds(outRect.removeFromTop(20));
  outMeter_.setBounds(outRect);
}

void MainComponent::paint(juce::Graphics &g)
{
  g.fillAll(Styling::bgDark);
  auto area = getLocalBounds();

  // Draw Control Deck Background
  auto footerHeight = 30 + 140 + 20;
  auto controlBg = area.removeFromBottom(footerHeight);

  // FIX: Use float casts for warning-free drawing
  juce::ColourGradient grad(Styling::bgDark, 0.0f, (float)controlBg.getY(), Styling::panelDark, 0.0f, (float)controlBg.getBottom(), false);
  g.setGradientFill(grad);
  g.fillRect(controlBg);

  g.setColour(juce::Colours::white.withAlpha(0.05f));
  g.drawHorizontalLine(controlBg.getY(), 0.0f, (float)getWidth());
}

// ==============================================================================
// Audio Processing
// ==============================================================================
void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
  engine_.prepare(sampleRate, samplesPerBlockExpected);

  // Sync UI state to Engine
  engine_.setBypass(!toggleOn_.getToggleState());
  engine_.setStrength(strength_.getValue() / 100.0);
  engine_.setVoiceProtect(voiceProtect_.getToggleState());
  engine_.setHumFix(humFix_.getToggleState());
  engine_.setListenDelta(deltaBtn_.getToggleState());
  engine_.setMicBoost(boostToggle_.getToggleState(), (float)boostSlider_.getValue());
  engine_.setOperationMode(mode_.getSelectedId() - 1);

  preTap_.setSize(1, samplesPerBlockExpected);
}

void MainComponent::releaseResources() {}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &info)
{
  auto *buf = info.buffer;
  const int n = info.numSamples;
  const int s = info.startSample;

  // 1. Tap Input for Metering
  if (buf->getNumChannels() > 0 && preTap_.getNumSamples() >= n)
  {
    preTap_.copyFrom(0, 0, *buf, 0, s, n);
    const float *chans[1]{preTap_.getReadPointer(0)};
    inMeter_.pushBuffer(chans, 1, n);
  }

  // 2. Process Audio
  engine_.process(*buf, s, n);

  // 3. Tap Output for Metering
  if (buf->getNumChannels() > 0)
  {
    const float *chans[1]{buf->getReadPointer(0, s)};
    outMeter_.pushBuffer(chans, 1, n);
  }
}

// ==============================================================================
// Interaction Logic
// ==============================================================================
void MainComponent::setupCallbacks()
{
  toggleOn_.onClick = [this]
  {
    bool on = toggleOn_.getToggleState();
    engine_.setBypass(!on);
    lStatus_.setText(on ? "System Active" : "System Bypassed", juce::dontSendNotification);
  };
  cleanBtn_.onClick = [this]
  { startAutoSetup(); };

  strength_.onValueChange = [this]
  {
    double val = strength_.getValue();
    engine_.setStrength(val / 100.0);
    lStrength_.setText(juce::String(juce::roundToInt(val)) + "%", juce::dontSendNotification);
  };

  voiceProtect_.onClick = [this]
  { engine_.setVoiceProtect(voiceProtect_.getToggleState()); };
  humFix_.onClick = [this]
  { engine_.setHumFix(humFix_.getToggleState()); };

  deltaBtn_.onClick = [this]
  {
    bool delta = deltaBtn_.getToggleState();
    engine_.setListenDelta(delta);
    if (delta)
      lStatus_.setText("Listening to Removed Noise Only.", juce::dontSendNotification);
    else
      lStatus_.setText("System Ready.", juce::dontSendNotification);
  };

  // Mic Boost Handler
  auto updateBoost = [this]
  {
    engine_.setMicBoost(boostToggle_.getToggleState(), (float)boostSlider_.getValue());
    boostLabel_.setText("+" + juce::String(juce::roundToInt(boostSlider_.getValue())) + "%", juce::dontSendNotification);
  };
  boostToggle_.onClick = updateBoost;
  boostSlider_.onValueChange = updateBoost;

  mode_.onChange = [this]
  {
    engine_.setOperationMode(mode_.getSelectedId() - 1);
  };

  resetBtn_.onClick = [this]
  {
    // Force UI updates
    strength_.setValue(50.0, juce::sendNotification);
    toggleOn_.setToggleState(true, juce::dontSendNotification);
    voiceProtect_.setToggleState(true, juce::sendNotification);
    mode_.setSelectedId(1, juce::sendNotification);
    deltaBtn_.setToggleState(false, juce::sendNotification);
    boostToggle_.setToggleState(false, juce::sendNotification);
    boostSlider_.setValue(100.0, juce::dontSendNotification);

    engine_.resetAll();
    engine_.setBypass(false);
    lStatus_.setText("Defaults Restored.", juce::dontSendNotification);
  };

  deviceBtn_.onClick = [this]
  {
    auto *dm = &deviceManager;
    auto *comp = new juce::AudioDeviceSelectorComponent(*dm, 0, 2, 0, 2, true, true, true, false);
    comp->setSize(640, 380);
    juce::DialogWindow::LaunchOptions o;
    o.content.setOwned(comp);
    o.dialogTitle = "Audio Settings";
    o.launchAsync();
  };
}

void MainComponent::timerCallback()
{
  // Update text
  double db = engine_.getAttenuationDb();
  juce::String txt = juce::String(juce::roundToInt(db)) + " dB";
  pillAtten_.setText(txt, juce::dontSendNotification);

  // Redline Logic
  float peak = engine_.getOutputLevel();
  if (peak > 0.95f)
  {
    lOut_.setColour(juce::Label::textColourId, juce::Colours::red);
    lOut_.setText("CLIPPING!", juce::dontSendNotification);
  }
  else
  {
    lOut_.setColour(juce::Label::textColourId, Styling::textDim);
    lOut_.setText("CLEAN OUTPUT", juce::dontSendNotification);
  }

  // AI Animation
  if (state_ == CleanState::Listening)
  {
    static int dots = 0;
    dots = (dots + 1) % 40;
    juce::String s = "Analyzing Environment";
    if (dots > 10)
      s += ".";
    if (dots > 20)
      s += ".";
    if (dots > 30)
      s += ".";
    lStatus_.setText(s, juce::dontSendNotification);
  }
}

void MainComponent::startAutoSetup()
{
  if (state_ != CleanState::Idle && state_ != CleanState::Ready)
    return;
  state_ = CleanState::Listening;
  cleanBtn_.setEnabled(false);
  toggleOn_.setToggleState(true, juce::dontSendNotification);
  engine_.setBypass(false);
  engine_.autoSetupBegin();
  juce::Component::SafePointer<MainComponent> safe(this);
  juce::Timer::callAfterDelay(3000, [safe]
                              { if (safe != nullptr) safe->finishAutoSetup(); });
}

void MainComponent::finishAutoSetup()
{
  if (state_ != CleanState::Listening)
    return;
  engine_.autoSetupEnd();
  float noiseFloor = engine_.getDetectedNoiseFloor();
  float db = juce::Decibels::gainToDecibels(noiseFloor);
  float recommendedStrength = 50.0f;
  juce::String statusMsg = "Setup complete. ";
  if (db > -50.0f)
  {
    recommendedStrength = 75.0f;
    statusMsg += "High background noise detected.";
  }
  else if (db > -70.0f)
  {
    recommendedStrength = 40.0f;
    statusMsg += "Moderate background noise detected.";
  }
  else
  {
    recommendedStrength = 20.0f;
    statusMsg += "Room is quiet.";
  }
  strength_.setValue(recommendedStrength, juce::sendNotification);
  lStatus_.setText(statusMsg, juce::dontSendNotification);
  state_ = CleanState::Ready;
  cleanBtn_.setEnabled(true);
}