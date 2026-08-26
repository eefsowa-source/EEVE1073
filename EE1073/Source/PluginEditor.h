#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class Ee1073AudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit Ee1073AudioProcessorEditor (Ee1073AudioProcessor&);
    ~Ee1073AudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    Ee1073AudioProcessor& processor;

    juce::Slider inputSlider, lowGainSlider, midGainSlider, highGainSlider, outputSlider;
    juce::ComboBox hpfFreqBox, lowFreqBox, midFreqBox;
    juce::ToggleButton hpfOnButton { "HPF" }, eqOnButton { "EQ In" };

    juce::Label inputLabel, lowLabel, midLabel, highLabel, outputLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment, lowGainAttachment, midGainAttachment,
        highGainAttachment, outputAttachment;
    std::unique_ptr<ComboAttachment> hpfFreqAttachment, lowFreqAttachment, midFreqAttachment;
    std::unique_ptr<ButtonAttachment> hpfOnAttachment, eqOnAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ee1073AudioProcessorEditor)
};
