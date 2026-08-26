#pragma once

#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "Ee1073LookAndFeel.h"

// Vertical rack-strip layout: a wood-framed module with a dark charcoal EQ
// panel (Input / HPF / Low / Mid / High knobs, stacked) on the left and a
// brushed-steel panel with a vertical Output fader and Power switch on the
// right -- following the classic 1073-strip proportions rather than the
// earlier horizontal six-column layout.
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

    juce::Slider inputSlider, hpfSlider, lowGainSlider, midGainSlider, highGainSlider, outputFader;
    juce::ComboBox lowFreqBox, midFreqBox;
    juce::ToggleButton eqOnButton { "EQL" }, phaseButton { "PHASE" }, powerButton { "POWER" };

    juce::Label inputLabel, hpfLabel, lowLabel, midLabel, highLabel, outputLabel;
    juce::Label hpfValueLabel, midQLabel, nameplateLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment, hpfAttachment, lowGainAttachment,
        midGainAttachment, highGainAttachment, outputAttachment;
    std::unique_ptr<ComboAttachment> lowFreqAttachment, midFreqAttachment;
    std::unique_ptr<ButtonAttachment> eqOnAttachment, phaseAttachment, powerAttachment;

    // Panel bounds computed in resized(), reused in paint() for the wood
    // frame cut-out and the dark/silver panel split line.
    juce::Rectangle<int> panelBounds, leftPanelBounds, rightPanelBounds;

    static constexpr juce::uint32 accentInput = 0xffc0392b;  // red, mic gain
    static constexpr juce::uint32 accentHpf = 0xff3f6fb3;    // blue, HPF
    static constexpr juce::uint32 accentLow = 0xff6fbf6a;
    static constexpr juce::uint32 accentMid = 0xffd9a441;
    static constexpr juce::uint32 accentHigh = 0xffd9564f;
    static constexpr juce::uint32 accentOutput = 0xff9b7fd4;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ee1073AudioProcessorEditor)
};
