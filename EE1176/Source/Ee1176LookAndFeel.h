#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Custom LookAndFeel for EE-1176, styled after the Universal Audio 1176
// Rack Mount Limiting Amplifier's blue-grey faceplate: brushed blue-grey
// anodized panel with rack-ear screw holes, knurled aluminum knobs
// (pointer + bipolar fill arc) for INPUT/OUTPUT plus the small COMP
// RATIO/VERNIER/ATTACK/VERNIER/RELEASE trims, a needle-style analog VU
// meter reading gain reduction, and a small rocker POWER switch. Entirely
// vector-drawn (JUCE Graphics gradients/paths) -- no bitmap assets.
class Ee1176LookAndFeel : public juce::LookAndFeel_V4
{
public:
    Ee1176LookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle,
                            float rotaryEndAngle, juce::Slider&) override;

    // Small rocker switch, used for POWER.
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont (juce::Label&) override;

    // Paints the blue-grey anodized rack panel with brushed streaks and
    // rack-ear screw holes near the left/right edges.
    static void paintRackPanel (juce::Graphics&, juce::Rectangle<int> bounds);

    // Paints an analog needle-style VU meter (cream face, black scale arc,
    // red overload zone, swinging needle) reading gainReductionDb against
    // a 0..-maxRangeDb scale -- 0 dB at the right (needle up), -maxRangeDb
    // at the left, matching the 1176's meter convention when switched to
    // GR mode.
    static void paintGainReductionMeter (juce::Graphics&, juce::Rectangle<int> bounds,
                                          float gainReductionDb, float maxRangeDb);
};
