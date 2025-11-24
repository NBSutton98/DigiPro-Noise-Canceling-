#include "PluginProcessor.hpp"
#include "MainComponent.hpp"

/**
 * Constructs the audio processor and configures the input/output buses.
 *
 * For this plugin, a stereo input and stereo output bus are created.
 */
NCliteAudioProcessor::NCliteAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
                         .withInput("Input", juce::AudioChannelSet::stereo(), true)
                         .withOutput("Output", juce::AudioChannelSet::stereo(), true))
#endif
{
}

/**
 * Destructor for the audio processor.
 *
 * Currently relies on default destruction behaviour.
 */
NCliteAudioProcessor::~NCliteAudioProcessor() {}

/**
 * Returns the name of the processor as it will appear in the host.
 *
 * @return The processor name string.
 */
const juce::String NCliteAudioProcessor::getName() const { return "NClite"; }

/**
 * Indicates whether this processor can accept MIDI input.
 *
 * @return Always false; this plugin does not use MIDI.
 */
bool NCliteAudioProcessor::acceptsMidi() const { return false; }

/**
 * Indicates whether this processor can produce MIDI output.
 *
 * @return Always false; this plugin does not generate MIDI.
 */
bool NCliteAudioProcessor::producesMidi() const { return false; }

/**
 * Indicates whether this processor should be treated as a pure MIDI effect.
 *
 * @return Always false; this is an audio effect, not a MIDI effect.
 */
bool NCliteAudioProcessor::isMidiEffect() const { return false; }

/**
 * Returns the tail length of the processor in seconds.
 *
 * This is used by the host for latency and tail-handling decisions.
 *
 * @return The tail length in seconds (0.0 for no tail).
 */
double NCliteAudioProcessor::getTailLengthSeconds() const { return 0.0; }

/**
 * Returns the number of programs (presets) supported by this processor.
 *
 * @return The number of available programs (currently 1).
 */
int NCliteAudioProcessor::getNumPrograms() { return 1; }

/**
 * Returns the index of the currently active program.
 *
 * @return Always 0, since only one program exists.
 */
int NCliteAudioProcessor::getCurrentProgram() { return 0; }

/**
 * Sets the active program by index.
 *
 * Currently a no-op because only one program exists.
 *
 * @param index The index of the program to activate.
 */
void NCliteAudioProcessor::setCurrentProgram(int index) {}

/**
 * Returns the name of the program at the given index.
 *
 * @param index The index of the requested program.
 * @return The name of the program (currently "Default" for any index).
 */
const juce::String NCliteAudioProcessor::getProgramName(int index) { return "Default"; }

/**
 * Changes the name of the program at the given index.
 *
 * Currently a no-op as program naming is not implemented.
 *
 * @param index   The index of the program to rename.
 * @param newName The new name to assign to the program.
 */
void NCliteAudioProcessor::changeProgramName(int index, const juce::String &newName) {}

/**
 * Prepares the processor for playback.
 *
 * This is called by the host before audio processing starts, and can be used
 * to allocate buffers and initialise internal DSP structures.
 *
 * @param sampleRate      The current audio sample rate.
 * @param samplesPerBlock The maximum expected block size.
 */
void NCliteAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    engine.prepare(sampleRate, samplesPerBlock);
}

/**
 * Releases any resources that were allocated in prepareToPlay().
 *
 * Called when the host stops playback or the processor is being destroyed.
 */
void NCliteAudioProcessor::releaseResources()
{
    engine.resetAll();
}

/**
 * Main audio processing callback.
 *
 * The host provides an audio buffer and a MIDI buffer. Audio is passed
 * through the internal DSP engine, and input/output levels are pushed
 * to the editor (if it is currently open) for metering/visualisation.
 *
 * @param buffer       The audio buffer to read from and write to.
 * @param midiMessages The MIDI messages for this block (unused here).
 */
void NCliteAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    // 1. Handle Input Visualization (Push to GUI if open)
    if (auto *editor = dynamic_cast<MainComponent *>(getActiveEditor()))
    {
        editor->pushInputToMeters(buffer);
    }

    // 2. DSP Processing
    // Passes audio from Ableton -> Your Engine -> Back to Buffer
    engine.process(buffer, 0, buffer.getNumSamples());

    // 3. Handle Output Visualization
    if (auto *editor = dynamic_cast<MainComponent *>(getActiveEditor()))
    {
        editor->pushOutputToMeters(buffer);
    }
}

/**
 * Indicates whether this processor provides a custom editor component.
 *
 * @return true, since a MainComponent editor is implemented.
 */
bool NCliteAudioProcessor::hasEditor() const { return true; }

/**
 * Creates the editor component for this processor.
 *
 * The host will call this when it needs to display the plugin's UI.
 *
 * @return A new instance of the MainComponent editor.
 */
juce::AudioProcessorEditor *NCliteAudioProcessor::createEditor()
{
    return new MainComponent(*this);
}

/**
 * Serialises the state of the processor into a binary block.
 *
 * This is where parameter/state data should be written for session recall.
 * Currently unimplemented.
 *
 * @param destData The destination memory block to fill with state data.
 */
void NCliteAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {}

/**
 * Restores the processor state from a binary block.
 *
 * This is called when a session is reloaded and should restore any state
 * that was written in getStateInformation().
 * Currently unimplemented.
 *
 * @param data        Pointer to the binary state data.
 * @param sizeInBytes Size of the state data in bytes.
 */
void NCliteAudioProcessor::setStateInformation(const void *data, int sizeInBytes) {}

/**
 * Factory function used by the host to create a new instance of the plugin.
 *
 * This is the entry point JUCE uses to construct the AudioProcessor.
 *
 * @return A pointer to a newly allocated NCliteAudioProcessor instance.
 */
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new NCliteAudioProcessor();
}
