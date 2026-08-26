#pragma once

#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "Ee1073LookAndFeel.h"

class Ee1073AudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    explicit Ee1073AudioProcessorEditor (Ee1073AudioProcessor&);
    ~Ee1073AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    Ee1073AudioProcessor& processor;
    Ee1073LookAndFeel lookAndFeel;

    juce::Slider inputSlider, lowGainSlider, midGainSlider, highGainSlider, outputSlider;
    juce::ComboBox hpfFreqBox, lowFreqBox, midFreqBox;
    juce::ToggleButton hpfOnButton { "HPF In" }, eqOnButton { "EQ In" };

    juce::Label inputLabel, lowLabel, midLabel, highLabel, outputLabel;
    juce::Label sectionInput, sectionHpf, sectionLow, sectionMid, sectionHigh, sectionOutput;
    juce::Label midQLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment, lowGainAttachment, midGainAttachment,
        highGainAttachment, outputAttachment;
    std::unique_ptr<ComboAttachment> hpfFreqAttachment, lowFreqAttachment, midFreqAttachment;
    std::unique_ptr<ButtonAttachment> hpfOnAttachment, eqOnAttachment;

    // Column x-positions (left edges), computed in resized() and reused in
    // paint() to draw section divider lines.
    std::array<float, 5> dividerX {};

    // Per-section accent colours, evoking a console module's colour-coded
    // legending (input/HPF/low/mid/high/output). Applied to each knob's
    // fill-arc colour and used to tint the corresponding section header.
    static constexpr juce::uint32 accentInput = 0xff5b9bd5;
    static constexpr juce::uint32 accentHpf = 0xff4dbdb0;
    static constexpr juce::uint32 accentLow = 0xff6fbf6a;
    static constexpr juce::uint32 accentMid = 0xffd9a441;
    static constexpr juce::uint32 accentHigh = 0xffd9564f;
    static constexpr juce::uint32 accentOutput = 0xff9b7fd4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ee1073AudioProcessorEditor)
};
