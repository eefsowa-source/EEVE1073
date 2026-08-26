#include "Ee1176LookAndFeel.h"

namespace
{
constexpr auto kPanelBlack = 0xff141416;
constexpr auto kAluminum = 0xffc9cdd4;
constexpr auto kAluminumShadow = 0xff7d838f;
constexpr auto kPointerBlack = 0xff1a1a1c;
constexpr auto kTrackDim = 0xff2e3138;
constexpr auto kRatioRed = 0xffd0392b;
}

Ee1176LookAndFeel::Ee1176LookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffd8dde6));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, juce::Colour (0xffb7bcc8));
    setColour (juce::ToggleButton::textColourId, juce::Colour (0xffb7bcc8));
}

void Ee1176LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                          juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto accent = slider.findColour (juce::Slider::rotarySliderFillColourId);

    // Track arc (dim, full sweep).
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius * 0.94f, radius * 0.94f, 0.0f,
                          rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (kTrackDim));
    g.strokePath (track, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Fill arc from the sweep start (these are unipolar 1176 controls --
    // Input/Output/Attack/Release all read as "more clockwise = more").
    juce::Path fill;
    fill.addCentredArc (centre.x, centre.y, radius * 0.94f, radius * 0.94f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (accent);
    g.strokePath (fill, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knurled aluminum knob cap.
    const auto capRadius = radius * 0.72f;
    juce::ColourGradient capGradient (juce::Colour (kAluminum).brighter (0.2f), centre.x - capRadius * 0.4f,
                                       centre.y - capRadius * 0.5f, juce::Colour (kAluminumShadow),
                                       centre.x, centre.y + capRadius, true);
    g.setGradientFill (capGradient);
    g.fillEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);

    // Knurl marks around the cap rim.
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    for (int i = 0; i < 24; ++i)
    {
        const float knurlAngle = juce::MathConstants<float>::twoPi * (float) i / 24.0f;
        const auto p1 = centre.getPointOnCircumference (capRadius * 0.86f, knurlAngle);
        const auto p2 = centre.getPointOnCircumference (capRadius * 0.98f, knurlAngle);
        g.drawLine ({ p1, p2 }, 1.0f);
    }

    g.setColour (juce::Colour (0xff33363c));
    g.drawEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.2f);

    // Pointer.
    juce::Path pointer;
    const float pointerLength = capRadius * 0.8f;
    const float pointerThickness = 3.0f;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness,
                                  pointerLength * 0.6f, pointerThickness * 0.5f);
    g.setColour (juce::Colour (kPointerBlack));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre));

    // Tick marks at min/mid/max.
    g.setColour (juce::Colour (0xff4a4e57));
    for (float t : { 0.0f, 0.5f, 1.0f })
    {
        const auto tickAngle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
        const auto p1 = centre.getPointOnCircumference (radius * 0.98f, tickAngle);
        const auto p2 = centre.getPointOnCircumference (radius, tickAngle);
        g.drawLine ({ p1, p2 }, 1.5f);
    }
}

void Ee1176LookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                          bool shouldDrawButtonAsHighlighted, bool)
{
    if (! button.getProperties().contains ("ratiobutton"))
    {
        LookAndFeel_V4::drawToggleButton (g, button, shouldDrawButtonAsHighlighted, false);
        return;
    }

    const bool isOn = button.getToggleState();
    auto bounds = button.getLocalBounds().toFloat().reduced (2.0f);

    juce::ColourGradient bg (juce::Colour (0xff26282d), bounds.getX(), bounds.getY(),
                              juce::Colour (0xff121315), bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (bg);
    g.fillRoundedRectangle (bounds, 2.0f);

    if (isOn)
    {
        g.setColour (juce::Colour (kRatioRed).withAlpha (0.9f));
        g.fillRoundedRectangle (bounds.reduced (2.0f), 1.5f);
        g.setColour (juce::Colour (kRatioRed).brighter (0.6f).withAlpha (0.5f));
        g.drawRoundedRectangle (bounds.reduced (2.0f), 1.5f, 1.5f);
    }

    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.fillRoundedRectangle (bounds, 2.0f);
    }

    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawRoundedRectangle (bounds, 2.0f, 1.0f);

    g.setColour (isOn ? juce::Colours::white : juce::Colour (0xff8c909a));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawFittedText (button.getButtonText(), bounds.toNearestInt(), juce::Justification::centred, 1);
}

juce::Font Ee1176LookAndFeel::getLabelFont (juce::Label&)
{
    return juce::FontOptions (11.0f);
}

void Ee1176LookAndFeel::paintRackPanel (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    juce::ColourGradient panelGradient (juce::Colour (0xff1e1f22), (float) bounds.getX(), (float) bounds.getY(),
                                         juce::Colour (kPanelBlack), (float) bounds.getX(),
                                         (float) bounds.getBottom(), false);
    g.setGradientFill (panelGradient);
    g.fillRect (bounds);

    g.setColour (juce::Colours::white.withAlpha (0.015f));
    for (int yy = bounds.getY(); yy < bounds.getBottom(); yy += 3)
        g.drawHorizontalLine (yy, (float) bounds.getX(), (float) bounds.getRight());

    // Rack-ear screw holes, two near each vertical edge.
    const float inset = 14.0f;
    const float holeRadius = 3.5f;
    const float ys[] = { bounds.getY() + inset, (float) bounds.getBottom() - inset };
    const float xs[] = { bounds.getX() + inset, (float) bounds.getRight() - inset };
    for (auto yy : ys)
        for (auto xx : xs)
        {
            g.setColour (juce::Colours::black.withAlpha (0.7f));
            g.fillEllipse (xx - holeRadius, yy - holeRadius, holeRadius * 2.0f, holeRadius * 2.0f);
            g.setColour (juce::Colour (0xff44474e));
            g.drawEllipse (xx - holeRadius, yy - holeRadius, holeRadius * 2.0f, holeRadius * 2.0f, 0.8f);
        }
}

void Ee1176LookAndFeel::paintGainReductionMeter (juce::Graphics& g, juce::Rectangle<int> bounds,
                                                  float gainReductionDb, float maxRangeDb)
{
    auto b = bounds.toFloat();
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.fillRoundedRectangle (b, 2.0f);
    g.setColour (juce::Colour (0xff3a3d44));
    g.drawRoundedRectangle (b, 2.0f, 1.0f);

    const float amount = juce::jlimit (0.0f, 1.0f, -gainReductionDb / juce::jmax (0.01f, maxRangeDb));
    const int numLeds = 12;
    const auto ledArea = b.reduced (2.0f);
    const float ledWidth = ledArea.getWidth() / (float) numLeds;

    for (int i = 0; i < numLeds; ++i)
    {
        const float ledProportion = (float) (i + 1) / (float) numLeds;
        const bool lit = ledProportion <= amount + 1.0e-3f;
        auto ledBounds = ledArea.withX (ledArea.getX() + ledWidth * (float) i).withWidth (ledWidth * 0.8f);

        juce::Colour ledColour = juce::Colour (0xff2a2d33);
        if (lit)
            ledColour = ledProportion > 0.83f ? juce::Colour (0xffe3413a)
                      : ledProportion > 0.5f  ? juce::Colour (0xffe0a83a)
                                               : juce::Colour (0xff5fd97a);

        g.setColour (ledColour);
        g.fillRoundedRectangle (ledBounds, 1.0f);
    }
}
