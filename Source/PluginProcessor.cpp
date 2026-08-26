#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr auto paramInput   = "input";
constexpr auto paramOutput  = "output";
constexpr auto paramAttack  = "attack";
constexpr auto paramRelease = "release";
constexpr auto paramRatio   = "ratio";
}

EeveAudioProcessor::EeveAudioProcessor()
    : juce::AudioProcessor (BusesProperties()
                                 .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                 .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout EeveAudioProcessor::createParameterLayout()
{
    using Range = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramInput, "Input", Range (-20.0f, 40.0f, 0.1f), 0.0f, " dB"));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramOutput, "Output", Range (-40.0f, 20.0f, 0.1f), 0.0f, " dB"));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramAttack, "Attack", Range (0.02f, 0.8f, 0.001f), 0.4f, " ms"));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramRelease, "Release", Range (50.0f, 1100.0f, 1.0f), 300.0f, " ms"));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        paramRatio, "Ratio",
        juce::StringArray { "4:1", "8:1", "12:1", "20:1", "All (British)" }, 0));

    return { params.begin(), params.end() };
}

void EeveAudioProcessor::prepareToPlay (double sampleRate, int)
{
    for (auto& core : cores)
        core.prepare (sampleRate);
    updateCoreParameters();
}

bool EeveAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return (in == mono || in == stereo) && in == out;
}

void EeveAudioProcessor::updateCoreParameters()
{
    eeve::Parameters p;
    p.inputGainDb = apvts.getRawParameterValue (paramInput)->load();
    p.outputGainDb = apvts.getRawParameterValue (paramOutput)->load();
    p.attackMs = apvts.getRawParameterValue (paramAttack)->load();
    p.releaseMs = apvts.getRawParameterValue (paramRelease)->load();
    p.ratio = static_cast<eeve::Ratio> (
        static_cast<int> (apvts.getRawParameterValue (paramRatio)->load()));

    for (auto& core : cores)
        core.setParameters (p);
}

void EeveAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    updateCoreParameters();

    const auto numChannels = std::min (buffer.getNumChannels(), (int) cores.size());
    for (int ch = 0; ch < numChannels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            data[i] = cores[(size_t) ch].processSample (data[i]);
    }
}

juce::AudioProcessorEditor* EeveAudioProcessor::createEditor()
{
    return new EeveAudioProcessorEditor (*this);
}

void EeveAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
}

void EeveAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EeveAudioProcessor();
}
