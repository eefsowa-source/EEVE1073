#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "../Shared/Ee1073ChannelStripCore.h"

class Ee1073AudioProcessor : public juce::AudioProcessor
{
public:
    Ee1073AudioProcessor();
    ~Ee1073AudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static juce::StringArray hpfFreqChoices();
    static juce::StringArray lowShelfFreqChoices();
    static juce::StringArray midFreqChoices();

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateCoreParameters();

    std::array<ee1073::ChannelStripCore, 2> cores; // up to stereo

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ee1073AudioProcessor)
};
