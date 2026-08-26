#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Custom LookAndFeel for EE-1073, styled after a classic rack-strip EQ
// module in a wood-side chassis: a dark charcoal EQ panel and a lighter
// brushed-steel fader panel, cream-capped rotary knobs (pointer + bipolar
// fill arc), a metal-capped vertical fader, hardware-style rocker toggle
// switches, and LCD-like combo boxes. Entirely vector-drawn (JUCE Graphics
// gradients/paths) -- no bitmap assets, so it stays self-contained and
// resolution-independent.
class Ee1073LookAndFeel : public juce::LookAndFeel_V4
{
public:
    Ee1073LookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle,
                            float rotaryEndAngle, juce::Slider&) override;

    void drawLinearSlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPos, float minSliderPos, float maxSliderPos,
                            const juce::Slider::SliderStyle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                        int buttonX, int buttonY, int buttonW, int buttonH,
                        juce::ComboBox&) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getLabelFont (juce::Label&) override;

    // Paints a brushed-metal panel between two given colours (top -> bottom
    // gradient), with faint horizontal streaks. `withRivets` adds four
    // corner rivets, matching the dark EQ-side panel on real hardware.
    static void paintBrushedPanel (juce::Graphics&, juce::Rectangle<int> bounds,
                                    juce::Colour top, juce::Colour bottom, bool withRivets);

    // Paints a procedural wood-grain frame (no bitmap) around `outerBounds`,
    // leaving `innerBounds` (the metal panel area) untouched.
    static void paintWoodFrame (juce::Graphics&, juce::Rectangle<int> outerBounds,
                                 juce::Rectangle<int> innerBounds);
};
