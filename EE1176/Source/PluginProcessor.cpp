#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr auto paramInput      = "input";
constexpr auto paramOutput     = "output";
constexpr auto paramAttack     = "attack";
constexpr auto paramRelease    = "release";
constexpr auto paramRatio      = "ratio";
constexpr auto paramRatioTrim  = "ratioTrim";
constexpr auto paramAttackTrim = "attackTrim";
constexpr auto paramPower      = "power";
}

Ee1176AudioProcessor::Ee1176AudioProcessor()
    : juce::AudioProcessor (BusesProperties()
                                 .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                 .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout Ee1176AudioProcessor::createParameterLayout()
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
        paramRatio, "Comp Ratio",
        juce::StringArray { "4:1", "8:1", "12:1", "20:1", "All" }, 0));

    // VERNIER fine-trim knobs (UA 1176 Rack Mount): continuous trim layered
    // on top of the coarse Comp Ratio / Attack controls, centered at 0.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramRatioTrim, "Ratio Vernier", Range (-1.0f, 1.0f, 0.01f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        paramAttackTrim, "Attack Vernier", Range (-0.15f, 0.15f, 0.001f), 0.0f, " ms"));

    params.push_back (std::make_unique<juce::AudioParameterBool> (paramPower, "Power", true));

    return { params.begin(), params.end() };
}

void Ee1176AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const auto numChannels = static_cast<size_t> (juce::jmax (1, getTotalNumOutputChannels()));
    oversampling = std::make_unique<juce::dsp::Oversampling<float>> (
        numChannels, 2 /* stages: 2^2 = 4x */, juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true);
    oversampling->initProcessing (static_cast<size_t> (samplesPerBlock));
    setLatencySamples (static_cast<int> (oversampling->getLatencyInSamples()));

    for (auto& core : cores)
        core.prepare (sampleRate * static_cast<double> (oversampling->getOversamplingFactor()));
    updateCoreParameters();
}

bool Ee1176AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return (in == mono || in == stereo) && in == out;
}

void Ee1176AudioProcessor::updateCoreParameters()
{
    ee1176::Parameters p;
    p.inputGainDb = apvts.getRawParameterValue (paramInput)->load();
    p.outputGainDb = apvts.getRawParameterValue (paramOutput)->load();
    p.attackMs = apvts.getRawParameterValue (paramAttack)->load();
    p.releaseMs = apvts.getRawParameterValue (paramRelease)->load();
    p.ratio = static_cast<ee1176::Ratio> (
        static_cast<int> (apvts.getRawParameterValue (paramRatio)->load()));
    p.ratioTrim = apvts.getRawParameterValue (paramRatioTrim)->load();
    p.attackTrimMs = apvts.getRawParameterValue (paramAttackTrim)->load();

    for (auto& core : cores)
        core.setParameters (p);
}

void Ee1176AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // POWER switch: hard-bypass the whole unit, mirroring the real
    // hardware's power switch turning it off (signal passes through).
    if (apvts.getRawParameterValue (paramPower)->load() <= 0.5f)
        return;

    updateCoreParameters();

    juce::dsp::AudioBlock<float> block (buffer);
    auto oversampledBlock = oversampling->processSamplesUp (block);

    const auto numChannels = std::min ((size_t) cores.size(), oversampledBlock.getNumChannels());
    float deepestGrDb = 0.0f;
    for (size_t ch = 0; ch < numChannels; ++ch)
    {
        auto* data = oversampledBlock.getChannelPointer (ch);
        for (size_t i = 0; i < oversampledBlock.getNumSamples(); ++i)
            data[i] = cores[ch].processSample (data[i]);
        deepestGrDb = std::min (deepestGrDb, cores[ch].getGainReductionDb());
    }
    currentGainReductionDb.store (deepestGrDb, std::memory_order_relaxed);

    oversampling->processSamplesDown (block);
}

juce::AudioProcessorEditor* Ee1176AudioProcessor::createEditor()
{
    return new Ee1176AudioProcessorEditor (*this);
}

void Ee1176AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
}

void Ee1176AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Ee1176AudioProcessor();
}
