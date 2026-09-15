#pragma once

#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "Ee1176LookAndFeel.h"

// Classic 1176LN-inspired layout: a large gain-reduction meter anchors the
// left side, with INPUT/OUTPUT in a prominent top row and ATTACK/RELEASE/
// RATIO grouped below on a blue-grey hardware-style faceplate.
//
// The VERNIER fine-trim knobs from the previous (UA 1176 Rack Mount)
// design aren't part of this faceplate, so they're not exposed here --
// the underlying ratioTrim/attackTrim parameters still exist and default
// to 0 (no trim), reachable via the host's generic parameter list if ever
// needed.
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

    juce::Slider inputSlider, outputSlider, attackSlider, releaseSlider;
    juce::ToggleButton powerButton { "POWER" };
    std::array<juce::TextButton, 5> ratioButtons;
    int ratioBeforeShift = 0;
    bool shiftAllActive = false;

    juce::Label inputLabel, outputLabel, attackLabel, releaseLabel, ratioLabel,
        nameplateLabel, meterCaptionLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::unique_ptr<SliderAttachment> inputAttachment, outputAttachment, attackAttachment,
        releaseAttachment;
    std::unique_ptr<ButtonAttachment> powerAttachment;

    juce::Rectangle<int> meterBounds;

    static constexpr juce::uint32 accentMint = 0xff9fc4dc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ee1176AudioProcessorEditor)
};
