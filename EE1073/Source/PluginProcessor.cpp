#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr auto paramInput      = "input";
constexpr auto paramHpfOn      = "hpfOn";
constexpr auto paramHpfFreq    = "hpfFreq";
constexpr auto paramLowFreq    = "lowFreq";
constexpr auto paramLowGain    = "lowGain";
constexpr auto paramMidFreq    = "midFreq";
constexpr auto paramMidGain    = "midGain";
constexpr auto paramHighGain   = "highGain";
constexpr auto paramEqOn       = "eqOn";
constexpr auto paramOutput     = "output";

// Switch-position frequencies matching the real 1073's stepped controls
// (see project blueprint).
const float hpfFreqs[]      = { 50.0f, 80.0f, 160.0f, 300.0f };
const float lowShelfFreqs[] = { 35.0f, 60.0f, 110.0f, 220.0f };
const float midFreqs[]      = { 360.0f, 700.0f, 1600.0f, 3200.0f, 4800.0f, 7200.0f };
}

juce::StringArray Ee1073AudioProcessor::hpfFreqChoices() { return { "50 Hz", "80 Hz", "160 Hz", "300 Hz" }; }
juce::StringArray Ee1073AudioProcessor::lowShelfFreqChoices() { return { "35 Hz", "60 Hz", "110 Hz", "220 Hz" }; }
juce::StringArray Ee1073AudioProcessor::midFreqChoices() { return { "360 Hz", "700 Hz", "1.6 kHz", "3.2 kHz", "4.8 kHz", "7.2 kHz" }; }

Ee1073AudioProcessor::Ee1073AudioProcessor()
    : juce::AudioProcessor (BusesProperties()
                                 .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                 .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Ee1073AudioProcessor::createParameterLayout()
{
    using Range = juce::NormalisableRange<float>;
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramInput, "Input", Range (-20.0f, 30.0f, 0.1f), 0.0f, " dB"));

    params.push_back (std::make_unique<juce::AudioParameterBool> (paramHpfOn, "HPF On", false));
    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        paramHpfFreq, "HPF Freq", hpfFreqChoices(), 1));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        paramLowFreq, "Low Freq", lowShelfFreqChoices(), 1));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramLowGain, "Low Gain", Range (-16.0f, 16.0f, 0.1f), 0.0f, " dB"));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        paramMidFreq, "Mid Freq", midFreqChoices(), 2));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramMidGain, "Mid Gain", Range (-18.0f, 18.0f, 0.1f), 0.0f, " dB"));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramHighGain, "High Gain (12kHz)", Range (-16.0f, 16.0f, 0.1f), 0.0f, " dB"));

    params.push_back (std::make_unique<juce::AudioParameterBool> (paramEqOn, "EQ In", true));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramOutput, "Output", Range (-30.0f, 20.0f, 0.1f), 0.0f, " dB"));

    return { params.begin(), params.end() };
}

void Ee1073AudioProcessor::prepareToPlay (double sampleRate, int)
{
    for (auto& core : cores)
        core.prepare (sampleRate);
    updateCoreParameters();
}

bool Ee1073AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return (in == mono || in == stereo) && in == out;
}

void Ee1073AudioProcessor::updateCoreParameters()
{
    ee1073::Parameters p;
    p.inputGainDb = apvts.getRawParameterValue (paramInput)->load();
    p.hpfEnabled = apvts.getRawParameterValue (paramHpfOn)->load() > 0.5f;
    p.hpfFreqHz = hpfFreqs[static_cast<int> (apvts.getRawParameterValue (paramHpfFreq)->load())];
    p.lowShelfFreqHz = lowShelfFreqs[static_cast<int> (apvts.getRawParameterValue (paramLowFreq)->load())];
    p.lowShelfGainDb = apvts.getRawParameterValue (paramLowGain)->load();
    p.midFreqHz = midFreqs[static_cast<int> (apvts.getRawParameterValue (paramMidFreq)->load())];
    p.midGainDb = apvts.getRawParameterValue (paramMidGain)->load();
    p.highShelfGainDb = apvts.getRawParameterValue (paramHighGain)->load();
    p.eqEnabled = apvts.getRawParameterValue (paramEqOn)->load() > 0.5f;
    p.outputGainDb = apvts.getRawParameterValue (paramOutput)->load();

    for (auto& core : cores)
        core.setParameters (p);
}

void Ee1073AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
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

juce::AudioProcessorEditor* Ee1073AudioProcessor::createEditor()
{
    return new Ee1073AudioProcessorEditor (*this);
}

void Ee1073AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
}

void Ee1073AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ee1073AudioProcessor();
}
