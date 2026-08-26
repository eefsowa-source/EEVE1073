#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class EeveAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit EeveAudioProcessorEditor (EeveAudioProcessor&);
    ~EeveAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    EeveAudioProcessor& processor;

    juce::Slider inputSlider, outputSlider, attackSlider, releaseSlider;
    juce::ComboBox ratioBox;
    juce::Label inputLabel, outputLabel, attackLabel, releaseLabel, ratioLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::unique_ptr<SliderAttachment> inputAttachment, outputAttachment, attackAttachment, releaseAttachment;
    std::unique_ptr<ComboAttachment> ratioAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EeveAudioProcessorEditor)
};
