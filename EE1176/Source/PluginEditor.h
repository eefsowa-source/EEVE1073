#pragma once

#include <array>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "Ee1176LookAndFeel.h"

// Rack-strip layout modeled on the classic 1176 faceplate: Input knob,
// four square ratio pushbuttons (pressing all four engages "British mode",
// matching the real hardware's all-buttons-in trick), a gain-reduction LED
// meter, Attack/Release knobs, and Output -- left to right.
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

    // Recomputes the "ratio" choice parameter from the four ratio button
    // states and pushes it to the host. Called from each button's onClick.
    void ratioButtonClicked();

    // Reflects the current "ratio" parameter value onto the four button
    // toggle states, without re-triggering ratioButtonClicked() (used on
    // construction and to stay in sync with host automation / presets).
    void syncRatioButtonsFromParameter();

    Ee1176AudioProcessor& processor;
    Ee1176LookAndFeel lookAndFeel;

    juce::Slider inputSlider, attackSlider, releaseSlider, outputSlider;
    std::array<juce::ToggleButton, 4> ratioButtons {
        juce::ToggleButton { "4:1" }, juce::ToggleButton { "8:1" },
        juce::ToggleButton { "12:1" }, juce::ToggleButton { "20:1" }
    };

    juce::Label inputLabel, attackLabel, releaseLabel, outputLabel, ratioLabel, nameplateLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> inputAttachment, attackAttachment, releaseAttachment, outputAttachment;

    bool updatingRatioButtonsFromHost = false;

    juce::Rectangle<int> meterBounds;

    static constexpr juce::uint32 accentSteel = 0xff9aa0ab;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Ee1176AudioProcessorEditor)
};
