#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "Ee1176LookAndFeel.h"

// Rack-strip layout modeled on the Universal Audio 1176 Rack Mount Limiting
// Amplifier's faceplate, left to right: INPUT, COMP RATIO knob (with a
// VERNIER fine-trim beneath it), an analog needle VU meter reading gain
// reduction, ATTACK (with its own VERNIER beneath it), OUTPUT, RELEASE,
// and a small POWER rocker switch.
class Ee1176AudioProcessorEditor : public juce::AudioProcessorEditor,
                                    private juce::Timer
{
public:
    explicit Ee1176AudioProcessorEditor (Ee1176AudioProcessor&);
    ~Ee1176AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    Ee1176AudioProcessor& processor;
    Ee1176LookAndFeel lookAndFeel;

    juce::Slider inputSlider, ratioSlider, ratioVernierSlider, attackSlider, attackVernierSlider,
        outputSlider, releaseSlider;
    juce::ToggleButton powerButton { "POWER" };

    juce::Label inputLabel, ratioLabel, ratioVernierLabel, attackLabel, attackVernierLabel,
        outputLabel, releaseLabel, nameplateLabel, meterCaptionLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment, ratioAttachment, ratioVernierAttachment,
        attackAttachment, attackVernierAttachment, outputAttachment, releaseAttachment;
    std::unique_ptr<ButtonAttachment> powerAttachment;

    juce::Rectangle<int> meterBounds;

    static constexpr juce::uint32 accentSteel = 0xffb7bcc8;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ee1176AudioProcessorEditor)
};
