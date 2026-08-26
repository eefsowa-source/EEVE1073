#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Custom LookAndFeel for EE-1176, styled after the classic UREI/UA 1176
// FET compressor's black rack faceplate: brushed-black anodized panel with
// rack-ear screw holes, knurled aluminum knobs (pointer + bipolar fill
// arc), and square red-lit pushbuttons for the four ratio switches (the
// hallmark "all-buttons-in" trick is pressing all four at once). Entirely
// vector-drawn (JUCE Graphics gradients/paths) -- no bitmap assets.
class Ee1176LookAndFeel : public juce::LookAndFeel_V4
{
public:
    Ee1176LookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                            float sliderPosProportional, float rotaryStartAngle,
                            float rotaryEndAngle, juce::Slider&) override;

    // Square ratio pushbutton, lit red when engaged. `button` must have
    // properties.set("ratiobutton", true) for this style; other toggle
    // buttons fall back to LookAndFeel_V4's default.
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&,
                            bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    juce::Font getLabelFont (juce::Label&) override;

    // Paints the black anodized rack panel with brushed streaks and rack-ear
    // screw holes near the left/right edges.
    static void paintRackPanel (juce::Graphics&, juce::Rectangle<int> bounds);

    // Paints a small gain-reduction VU-style meter: a horizontal LED-style
    // bar that fills from the right as gainReductionDb goes more negative,
    // scaled against `maxRangeDb` (a positive number, e.g. 20 for a
    // 0..-20 dB meter range).
    static void paintGainReductionMeter (juce::Graphics&, juce::Rectangle<int> bounds,
                                          float gainReductionDb, float maxRangeDb);
};
