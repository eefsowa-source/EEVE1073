#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Custom LookAndFeel for EE-1073, styled after a vintage console EQ module:
// a brushed-metal panel with a cream-capped rotary knob (pointer + fill
// arc), hardware-style rocker toggle switches, and LCD-like combo boxes.
// Entirely vector-drawn (JUCE Graphics gradients/paths) -- no bitmap
// assets, so it stays self-contained and resolution-independent.
class Ee1073LookAndFeel : public juce::LookAndFeel_V4
{
public:
    Ee1073LookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle,
                            float rotaryEndAngle, juce::Slider&) override;

    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawComboBox (juce::Graphics&, int width, int height, bool isButtonDown,
                        int buttonX, int buttonY, int buttonW, int buttonH,
                        juce::ComboBox&) override;

    juce::Font getComboBoxFont (juce::ComboBox&) override;
    juce::Font getLabelFont (juce::Label&) override;

    // Paints the module's brushed-metal panel background, with corner
    // rivets and a top accent strip. Call from the editor's paint(),
    // beneath all child components.
    static void paintPanelBackground (juce::Graphics&, juce::Rectangle<int> bounds);
};
