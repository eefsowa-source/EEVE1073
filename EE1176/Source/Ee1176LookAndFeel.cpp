#include "Ee1176LookAndFeel.h"

namespace
{
constexpr auto kPanelSlate = 0xff474f5c;   // blue-grey anodized panel, top
constexpr auto kPanelSlateDark = 0xff2e333d; // panel, bottom
constexpr auto kAluminum = 0xffc9cdd4;
constexpr auto kAluminumShadow = 0xff7d838f;
constexpr auto kPointerBlack = 0xff1a1a1c;
constexpr auto kTrackDim = 0xff5a616f;
}

Ee1176LookAndFeel::Ee1176LookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe4e7ec));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, juce::Colour (0xffd3d7de));
    setColour (juce::ToggleButton::textColourId, juce::Colour (0xffd3d7de));
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
    g.strokePath (track, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Fill arc from the sweep start (these are unipolar 1176 controls --
    // Input/Output/Attack/Release/Comp Ratio/Verniers all read as "more
    // clockwise = more").
    juce::Path fill;
    fill.addCentredArc (centre.x, centre.y, radius * 0.94f, radius * 0.94f, 0.0f, rotaryStartAngle, angle, true);
    g.setColour (accent);
    g.strokePath (fill, juce::PathStrokeType (2.2f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knurled aluminum knob cap.
    const auto capRadius = radius * 0.72f;
    juce::ColourGradient capGradient (juce::Colour (kAluminum).brighter (0.2f), centre.x - capRadius * 0.4f,
                                       centre.y - capRadius * 0.5f, juce::Colour (kAluminumShadow),
                                       centre.x, centre.y + capRadius, true);
    g.setGradientFill (capGradient);
    g.fillEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);

    // Knurl marks around the cap rim.
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    const int numKnurls = capRadius > 16.0f ? 24 : 14;
    for (int i = 0; i < numKnurls; ++i)
    {
        const float knurlAngle = juce::MathConstants<float>::twoPi * (float) i / (float) numKnurls;
        const auto p1 = centre.getPointOnCircumference (capRadius * 0.86f, knurlAngle);
        const auto p2 = centre.getPointOnCircumference (capRadius * 0.98f, knurlAngle);
        g.drawLine ({ p1, p2 }, 1.0f);
    }

    g.setColour (juce::Colour (0xff33363c));
    g.drawEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.2f);

    // Pointer.
    juce::Path pointer;
    const float pointerLength = capRadius * 0.8f;
    const float pointerThickness = juce::jmax (2.0f, capRadius * 0.09f);
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness,
                                  pointerLength * 0.6f, pointerThickness * 0.5f);
    g.setColour (juce::Colour (kPointerBlack));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre));

    // Tick marks at min/mid/max.
    g.setColour (juce::Colour (0xff2b2e34));
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
    // Small hardware-style rocker switch, used for POWER.
    const bool isOn = button.getToggleState();
    auto bounds = button.getLocalBounds().toFloat();
    const auto switchWidth = juce::jmin (bounds.getWidth() * 0.5f, 30.0f);
    auto switchBounds = bounds.removeFromLeft (switchWidth).reduced (2.0f);

    const auto onColour = juce::Colour (0xff5fd97a);
    const auto offColour = juce::Colour (0xff23262c);

    g.setColour (isOn ? onColour : offColour);
    g.fillRoundedRectangle (switchBounds, switchBounds.getHeight() * 0.5f);
    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour (juce::Colours::white.withAlpha (0.1f));
        g.fillRoundedRectangle (switchBounds, switchBounds.getHeight() * 0.5f);
    }

    const auto knobDiameter = switchBounds.getHeight() - 4.0f;
    const auto knobX = isOn ? switchBounds.getRight() - knobDiameter - 2.0f : switchBounds.getX() + 2.0f;
    g.setColour (juce::Colour (0xffe8ebf0));
    g.fillEllipse (knobX, switchBounds.getY() + 2.0f, knobDiameter, knobDiameter);

    g.setColour (button.findColour (juce::ToggleButton::textColourId));
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawFittedText (button.getButtonText(), bounds.reduced (4.0f, 0.0f).toNearestInt(),
                       juce::Justification::centredLeft, 1);
}

juce::Font Ee1176LookAndFeel::getLabelFont (juce::Label&)
{
    return juce::FontOptions (11.0f);
}

void Ee1176LookAndFeel::paintRackPanel (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    juce::ColourGradient panelGradient (juce::Colour (kPanelSlate), (float) bounds.getX(), (float) bounds.getY(),
                                         juce::Colour (kPanelSlateDark), (float) bounds.getX(),
                                         (float) bounds.getBottom(), false);
    g.setGradientFill (panelGradient);
    g.fillRect (bounds);

    g.setColour (juce::Colours::white.withAlpha (0.02f));
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
            g.setColour (juce::Colours::black.withAlpha (0.6f));
            g.fillEllipse (xx - holeRadius, yy - holeRadius, holeRadius * 2.0f, holeRadius * 2.0f);
            g.setColour (juce::Colour (0xff1c1e22));
            g.drawEllipse (xx - holeRadius, yy - holeRadius, holeRadius * 2.0f, holeRadius * 2.0f, 0.8f);
        }
}

void Ee1176LookAndFeel::paintGainReductionMeter (juce::Graphics& g, juce::Rectangle<int> bounds,
                                                  float gainReductionDb, float maxRangeDb)
{
    auto b = bounds.toFloat();

    // Aged cream meter face.
    juce::ColourGradient faceGradient (juce::Colour (0xfff1e6c8), b.getX(), b.getY(),
                                        juce::Colour (0xffd8c99a), b.getX(), b.getBottom(), false);
    g.setGradientFill (faceGradient);
    g.fillRoundedRectangle (b, 3.0f);
    g.setColour (juce::Colour (0xff1a1a1c));
    g.drawRoundedRectangle (b, 3.0f, 2.0f);

    g.saveState();
    g.reduceClipRegion (b.toNearestInt());

    // Needle pivots below the visible face; the scale sweeps through the
    // upper portion, 0 dB (no reduction) at the right, -maxRangeDb at the
    // left -- matching the 1176's GR meter convention.
    const auto pivot = juce::Point<float> (b.getCentreX(), b.getBottom() + b.getHeight() * 0.35f);
    const float needleLength = b.getHeight() * 1.05f;
    const float minAngle = juce::MathConstants<float>::pi * -0.72f; // left, max GR
    const float maxAngle = juce::MathConstants<float>::pi * -0.28f; // right, 0 dB

    // Scale arc + tick marks at 0, -5, -10, -15, -20 (or scaled to maxRangeDb).
    g.setColour (juce::Colour (0xff2a2620));
    const int numTicks = 5;
    for (int i = 0; i < numTicks; ++i)
    {
        const float t = (float) i / (float) (numTicks - 1); // 0 = 0dB (right), 1 = -max (left)
        const float tickAngle = maxAngle + (minAngle - maxAngle) * t;
        const auto p1 = pivot.getPointOnCircumference (needleLength * 0.86f, tickAngle);
        const auto p2 = pivot.getPointOnCircumference (needleLength * 0.98f, tickAngle);
        g.drawLine ({ p1, p2 }, i == 0 ? 2.0f : 1.2f);
    }

    // Red zone for heavy gain reduction (last ~20% of scale).
    juce::Path redZone;
    const float redStartAngle = maxAngle + (minAngle - maxAngle) * 0.8f;
    redZone.addCentredArc (pivot.x, pivot.y, needleLength * 0.92f, needleLength * 0.92f, 0.0f,
                            redStartAngle, minAngle, true);
    g.setColour (juce::Colour (0xffb0392b));
    g.strokePath (redZone, juce::PathStrokeType (2.0f));

    // Needle.
    const float amount = juce::jlimit (0.0f, 1.0f, -gainReductionDb / juce::jmax (0.01f, maxRangeDb));
    const float needleAngle = maxAngle + (minAngle - maxAngle) * amount;

    juce::Path needle;
    needle.startNewSubPath (pivot);
    needle.lineTo (pivot.getPointOnCircumference (needleLength, needleAngle));
    g.setColour (juce::Colour (0xff1a1a1c));
    g.strokePath (needle, juce::PathStrokeType (1.6f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    g.setColour (juce::Colour (0xff1a1a1c));
    g.fillEllipse (pivot.x - 2.5f, pivot.y - 2.5f, 5.0f, 5.0f);

    g.restoreState();
}
