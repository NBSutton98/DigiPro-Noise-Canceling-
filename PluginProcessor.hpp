/**
 * @file PluginProcessor.h
 * @brief Defines the core audio processor class, NCliteAudioProcessor.
 * * This class is the central "Model" in the MVC pattern for the plugin,
 * responsible for handling audio I/O, hosting the DSP engine, and managing
 * the plugin's state within a host application (DAW).
 */
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "AudioEngine.hpp"

/**
 * @class NCliteAudioProcessor
 * @brief The main AudioProcessor class for the NClite Noise Reducer plugin.
 * * Inherits from juce::AudioProcessor, providing the necessary interface
 * for the host application (DAW) to interact with the plugin's audio and state.
 */
class NCliteAudioProcessor : public juce::AudioProcessor
{
public:
    /** Constructor. Initializes the AudioProcessor and the internal AudioEngine. */
    NCliteAudioProcessor();

    /** Destructor. */
    ~NCliteAudioProcessor() override;

    /** * @brief Prepares the processor to begin playing audio.
     * * Called before processing starts, typically when the host transport starts or
     * when the audio settings change. It prepares the internal DSP engine.
     * * @param sampleRate The current sample rate of the audio context.
     * @param samplesPerBlock The maximum number of samples in an audio block.
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    /** * @brief Releases any resources used by the processor.
     * * Called when the host stops playback or when the plugin is closed.
     * It resets the internal DSP engine state.
     */
    void releaseResources() override;

    /** * @brief The core audio processing method.
     * * This function is called repeatedly by the host to process blocks of audio.
     * It routes the audio data to the internal AudioEngine for noise reduction.
     * * @param buffer The audio buffer containing input and output data.
     * @param midiMessages Any incoming MIDI messages (unused in this effect).
     */
    void processBlock(juce::AudioBuffer<float> &buffer, juce::MidiBuffer &midiMessages) override;

    /** * @brief Creates the editor (GUI) for the processor.
     * @return A pointer to the newly created MainComponent editor instance.
     */
    juce::AudioProcessorEditor *createEditor() override;

    /** * @brief Checks if the processor has a GUI editor.
     * @return Always returns true.
     */
    bool hasEditor() const override;

    /** @brief Returns the plugin's official name ("NClite"). */
    const juce::String getName() const override;

    /** @brief Checks if the plugin accepts MIDI input (returns false). */
    bool acceptsMidi() const override;

    /** @brief Checks if the plugin produces MIDI output (returns false). */
    bool producesMidi() const override;

    /** @brief Checks if the plugin is a MIDI effect (returns false). */
    bool isMidiEffect() const override;

    /** @brief Returns the plugin's tail length (0.0 seconds). */
    double getTailLengthSeconds() const override;

    /** @brief Returns the number of programs/presets (1: Default). */
    int getNumPrograms() override;

    /** @brief Returns the currently active program index (0). */
    int getCurrentProgram() override;

    /** * @brief Sets the current program/preset (implementation is empty as no presets are used).
     * @param index The index of the program to switch to.
     */
    void setCurrentProgram(int index) override;

    /** * @brief Gets the name of the program at a given index.
     * @param index The index of the program.
     * @return Always returns "Default".
     */
    const juce::String getProgramName(int index) override;

    /** * @brief Changes the name of the program (implementation is empty).
     * @param index The index of the program.
     * @param newName The new name for the program.
     */
    void changeProgramName(int index, const juce::String &newName) override;

    /** * @brief Saves the plugin's current state (e.g., knob positions) to a memory block.
     * @param destData The memory block where the state should be written.
     */
    void getStateInformation(juce::MemoryBlock &destData) override;

    /** * @brief Restores the plugin's state from a memory block.
     * @param data A pointer to the raw data containing the saved state.
     * @param sizeInBytes The size of the memory block in bytes.
     */
    void setStateInformation(const void *data, int sizeInBytes) override;

    /** * @brief The core DSP engine that performs the noise cancellation.
     * * This member is public so the GUI (`MainComponent`) can directly control
     * its parameters (e.g., setStrength, setBypass).
     */
    AudioEngine engine;

private:
    /** Prevents the class from being copied or moved. */
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NCliteAudioProcessor)
};