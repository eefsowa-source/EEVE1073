#include "Ee1073LookAndFeel.h"

namespace
{
constexpr auto kPanelDark = 0xff1a1d26;
constexpr auto kCream = 0xffe8dfc8;
constexpr auto kCreamShadow = 0xffb3a880;
constexpr auto kNeedleRed = 0xffc0392b;
constexpr auto kTrackDim = 0xff3a4156;
}

Ee1073LookAndFeel::Ee1073LookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xffe8dfc8));
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::ComboBox::backgroundColourId, juce::Colour (kPanelDark));
    setColour (juce::ComboBox::textColourId, juce::Colour (kCream));
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0xff4a5270));
    setColour (juce::Label::textColourId, juce::Colour (0xffb8c4dd));
    setColour (juce::ToggleButton::textColourId, juce::Colour (0xffb8c4dd));
    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (kPanelDark));
    setColour (juce::PopupMenu::textColourId, juce::Colour (kCream));
    setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (kNeedleRed));
}

void Ee1073LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                           juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    const auto accent = slider.findColour (juce::Slider::rotarySliderFillColourId);

    const auto wellRadius = radius * 0.985f;
    g.setColour (juce::Colours::black.withAlpha (0.48f));
    g.fillEllipse (centre.x - wellRadius, centre.y - wellRadius + 2.5f,
                   wellRadius * 2.0f, wellRadius * 2.0f);
    juce::ColourGradient wellGradient (juce::Colour (0xff5d6474), centre.x - wellRadius,
                                       centre.y - wellRadius, juce::Colour (0xff171a22),
                                       centre.x + wellRadius, centre.y + wellRadius, true);
    g.setGradientFill (wellGradient);
    g.fillEllipse (centre.x - wellRadius, centre.y - wellRadius,
                   wellRadius * 2.0f, wellRadius * 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.75f));
    g.drawEllipse (centre.x - wellRadius, centre.y - wellRadius,
                   wellRadius * 2.0f, wellRadius * 2.0f, 1.5f);

    // Track arc (dim, full sweep).
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius * 0.88f, radius * 0.88f, 0.0f,
                          rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (0xff10131a));
    g.strokePath (track, juce::PathStrokeType (6.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour (juce::Colour (kTrackDim));
    g.strokePath (track, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Fill arc from the 0-position (proportional 0.5 for bipolar gain
    // params, which is what most EE-1073 knobs are) to the current value.
    const auto zeroProportional = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0)
                                       ? static_cast<float> (slider.valueToProportionOfLength (0.0))
                                       : 0.0f;
    const auto zeroAngle = rotaryStartAngle + zeroProportional * (rotaryEndAngle - rotaryStartAngle);

    juce::Path fill;
    fill.addCentredArc (centre.x, centre.y, radius * 0.88f, radius * 0.88f, 0.0f,
                         juce::jmin (zeroAngle, angle), juce::jmax (zeroAngle, angle), true);
    g.setColour (accent);
    g.strokePath (fill, juce::PathStrokeType (3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knob cap: radial gradient giving a domed metal-cream look, tinted
    // slightly by the accent colour (used for the red Input / blue HPF caps).
    // Sized larger/chunkier relative to the control bounds than a typical
    // knob for a bolder, thicker look.
    const auto capRadius = radius * 0.70f;
    const auto capBase = juce::Colour (kCream).interpolatedWith (accent, 0.12f);
    const auto capShadow = juce::Colour (kCreamShadow).interpolatedWith (accent, 0.18f);
    g.setColour (juce::Colours::black.withAlpha (0.48f));
    g.fillEllipse (centre.x - capRadius, centre.y - capRadius + 3.0f,
                   capRadius * 2.0f, capRadius * 2.0f);
    juce::ColourGradient capGradient (capBase.brighter (0.15f), centre.x - capRadius * 0.4f,
                                       centre.y - capRadius * 0.5f, capShadow,
                                       centre.x, centre.y + capRadius, true);
    g.setGradientFill (capGradient);
    g.fillEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);

    g.setColour (juce::Colour (0xff2a2f3d));
    g.drawEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.2f);
    g.setColour (juce::Colours::white.withAlpha (0.24f));
    juce::Path capHighlight;
    capHighlight.addArc (centre.x - capRadius + 1.0f, centre.y - capRadius + 1.0f,
                         capRadius * 2.0f - 2.0f, capRadius * 2.0f - 2.0f,
                         rotaryStartAngle + 0.2f, rotaryStartAngle + 1.45f, true);
    g.strokePath (capHighlight, juce::PathStrokeType (1.8f));

    // Pointer.
    juce::Path pointer;
    const float pointerLength = capRadius * 0.82f;
    const float pointerThickness = juce::jmax (3.0f, capRadius * 0.12f);
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness,
                                  pointerLength * 0.6f, pointerThickness * 0.5f);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.translated (1.5f, 2.0f)));
    g.setColour (juce::Colour (kNeedleRed));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre));
    g.setColour (juce::Colour (0xfff2c8b9).withAlpha (0.7f));
    g.fillEllipse (centre.x - 2.0f, centre.y - 2.0f, 4.0f, 4.0f);

    // Small tick marks at min/centre/max.
    g.setColour (juce::Colour (0xffaeb5c3));
    for (int index = 0; index <= 10; ++index)
    {
        const auto t = static_cast<float> (index) / 10.0f;
        const auto tickAngle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
        const auto inner = (index % 5 == 0 ? radius * 0.91f : radius * 0.94f);
        const auto p1 = centre.getPointOnCircumference (inner, tickAngle);
        const auto p2 = centre.getPointOnCircumference (radius * 0.98f, tickAngle);
        g.drawLine ({ p1, p2 }, index % 5 == 0 ? 1.5f : 0.8f);
    }
}

void Ee1073LookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float minSliderPos, float maxSliderPos,
                                           const juce::Slider::SliderStyle style, juce::Slider& slider)
{
    juce::ignoreUnused (minSliderPos, maxSliderPos, style);

    const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat();
    const auto trackX = bounds.getCentreX();

    // Slot / track groove.
    const float trackWidth = 8.0f;
    juce::Rectangle<float> track (trackX - trackWidth * 0.5f, bounds.getY() + 6.0f, trackWidth,
                                   bounds.getHeight() - 12.0f);
    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.fillRoundedRectangle (track.translated (1.5f, 2.0f), trackWidth * 0.5f);
    juce::ColourGradient railGradient (juce::Colour (0xff697286), track.getX(), track.getY(),
                                       juce::Colour (0xff1b1e27), track.getRight(), track.getBottom(), false);
    g.setGradientFill (railGradient);
    g.fillRoundedRectangle (track, trackWidth * 0.5f);
    g.setColour (juce::Colour (0xff12151b));
    g.drawRoundedRectangle (track, trackWidth * 0.5f, 1.0f);

    // Fill from the slider's 0 dB reference (proportional position for
    // value 0) down/up to the current fader position, so the track lights
    // up above or below unity like a channel-strip fader.
    const auto accent = slider.findColour (juce::Slider::thumbColourId);
    const bool hasZero = slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0;
    const float zeroY = hasZero
                             ? static_cast<float> (slider.getPositionOfValue (0.0))
                             : bounds.getBottom() - 6.0f;
    juce::Rectangle<float> fillRect (trackX - trackWidth * 0.5f, juce::jmin (zeroY, sliderPos), trackWidth,
                                      std::abs (sliderPos - zeroY));
    g.setColour (accent.withAlpha (0.9f));
    g.fillRoundedRectangle (fillRect, trackWidth * 0.5f);

    // Fader cap: a wide metal block with horizontal grip ridges, centred on
    // sliderPos.
    const float capWidth = juce::jmin (bounds.getWidth() - 4.0f, 42.0f);
    const float capHeight = 26.0f;
    juce::Rectangle<float> cap (trackX - capWidth * 0.5f, sliderPos - capHeight * 0.5f, capWidth, capHeight);

    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillRoundedRectangle (cap.translated (0.0f, 2.0f), 3.0f);

    juce::ColourGradient capGradient (juce::Colour (0xff5a6178), cap.getX(), cap.getY(),
                                       juce::Colour (0xff23262f), cap.getX(), cap.getBottom(), false);
    g.setGradientFill (capGradient);
    g.fillRoundedRectangle (cap, 3.0f);

    g.setColour (juce::Colour (0xff0f1116));
    g.drawRoundedRectangle (cap, 3.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.25f));
    g.drawHorizontalLine (static_cast<int> (cap.getY() + 2.0f), cap.getX() + 4.0f, cap.getRight() - 4.0f);

    g.setColour (juce::Colours::white.withAlpha (0.18f));
    for (float ridgeY = cap.getY() + 5.0f; ridgeY < cap.getBottom() - 3.0f; ridgeY += 4.0f)
        g.drawHorizontalLine (static_cast<int> (ridgeY), cap.getX() + 4.0f, cap.getRight() - 4.0f);

    // Centre indicator line on the cap, matching the accent colour.
    g.setColour (accent);
    g.fillRect (juce::Rectangle<float> (cap.getX() + 3.0f, cap.getCentreY() - 1.0f, cap.getWidth() - 6.0f, 2.0f));
}

void Ee1073LookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                           bool shouldDrawButtonAsHighlighted, bool)
{
    const bool isOn = button.getToggleState();

    if (button.getProperties().contains ("pushbutton"))
    {
        // Small hardware-style cream pushbutton (EQL / PHASE), lit when
        // engaged -- matches the printed cream buttons on real 1073-style
        // strips rather than a sliding rocker.
        auto bounds = button.getLocalBounds().toFloat().reduced (1.0f);
        const auto litColour = juce::Colour (0xffe8dfc8);
        const auto dimColour = juce::Colour (0xff5a5748);

        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillRoundedRectangle (bounds.translated (0.0f, 2.0f), 3.0f);
        juce::ColourGradient grad (isOn ? litColour.brighter (0.1f) : dimColour.darker (0.1f),
                                    bounds.getX(), bounds.getY(),
                                    isOn ? litColour.darker (0.15f) : dimColour.darker (0.3f),
                                    bounds.getX(), bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, 3.0f);

        if (shouldDrawButtonAsHighlighted)
        {
            g.setColour (juce::Colours::white.withAlpha (0.1f));
            g.fillRoundedRectangle (bounds, 3.0f);
        }

        g.setColour (juce::Colours::black.withAlpha (0.65f));
        g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

        g.setColour (isOn ? juce::Colour (0xff2a2f3d) : juce::Colour (0xff8a8570));
        g.setFont (juce::FontOptions (10.0f, juce::Font::bold));
        g.drawFittedText (button.getButtonText(), bounds.toNearestInt(), juce::Justification::centred, 1);
        return;
    }

    // Default: sliding rocker switch (POWER).
    auto bounds = button.getLocalBounds().toFloat();
    const auto switchWidth = juce::jmin (bounds.getWidth() * 0.4f, 34.0f);
    auto switchBounds = bounds.removeFromLeft (switchWidth).reduced (2.0f);

    const auto onColour = juce::Colour (0xff4caf7d);
    const auto offColour = juce::Colour (0xff3a4156);

    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillRoundedRectangle (switchBounds.translated (0.0f, 2.0f), switchBounds.getHeight() * 0.5f);
    g.setColour (isOn ? onColour : offColour);
    g.fillRoundedRectangle (switchBounds, switchBounds.getHeight() * 0.5f);
    if (shouldDrawButtonAsHighlighted)
    {
        g.setColour (juce::Colours::white.withAlpha (0.08f));
        g.fillRoundedRectangle (switchBounds, switchBounds.getHeight() * 0.5f);
    }

    const auto knobDiameter = switchBounds.getHeight() - 4.0f;
    const auto knobX = isOn ? switchBounds.getRight() - knobDiameter - 2.0f : switchBounds.getX() + 2.0f;
    g.setColour (juce::Colour (0xffe8dfc8));
    g.fillEllipse (knobX, switchBounds.getY() + 2.0f, knobDiameter, knobDiameter);
    g.setColour (juce::Colours::white.withAlpha (0.3f));
    g.drawEllipse (knobX + 1.0f, switchBounds.getY() + 3.0f, knobDiameter - 2.0f, knobDiameter - 2.0f, 1.0f);

    g.setColour (button.findColour (juce::ToggleButton::textColourId));
    g.setFont (juce::FontOptions (12.0f));
    g.drawFittedText (button.getButtonText(), bounds.reduced (4.0f, 0.0f).toNearestInt(),
                       juce::Justification::centredLeft, 1);
}

void Ee1073LookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                       int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (1.0f);

    g.setColour (juce::Colours::black.withAlpha (0.4f));
    g.fillRoundedRectangle (bounds.translated (0.0f, 2.0f), 3.0f);
    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawHorizontalLine (static_cast<int> (bounds.getY() + 1.0f), bounds.getX() + 3.0f, bounds.getRight() - 3.0f);

    const auto arrowZone = bounds.removeFromRight (18.0f);
    juce::Path arrow;
    arrow.addTriangle (arrowZone.getCentreX() - 4.0f, arrowZone.getCentreY() - 2.5f,
                        arrowZone.getCentreX() + 4.0f, arrowZone.getCentreY() - 2.5f,
                        arrowZone.getCentreX(), arrowZone.getCentreY() + 3.5f);
    g.setColour (juce::Colour (0xffb8c4dd));
    g.fillPath (arrow);
}

juce::Font Ee1073LookAndFeel::getComboBoxFont (juce::ComboBox&)
{
    return juce::FontOptions (12.0f);
}

juce::Font Ee1073LookAndFeel::getLabelFont (juce::Label&)
{
    return juce::FontOptions (12.0f);
}

void Ee1073LookAndFeel::paintBrushedPanel (juce::Graphics& g, juce::Rectangle<int> bounds,
                                            juce::Colour top, juce::Colour bottom, bool withRivets)
{
    juce::ColourGradient panelGradient (top, (float) bounds.getX(), (float) bounds.getY(),
                                         bottom, (float) bounds.getX(), (float) bounds.getBottom(), false);
    g.setGradientFill (panelGradient);
    g.fillRect (bounds);

    g.setColour (juce::Colours::black.withAlpha (0.36f));
    g.drawRect (bounds.reduced (1), 2.0f);
    g.setColour (juce::Colours::white.withAlpha (0.18f));
    g.drawHorizontalLine (bounds.getY() + 1, (float) bounds.getX() + 3.0f, (float) bounds.getRight() - 3.0f);
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.drawHorizontalLine (bounds.getBottom() - 2, (float) bounds.getX() + 3.0f, (float) bounds.getRight() - 3.0f);

    // Faint brushed-metal streaks.
    g.setColour (juce::Colours::white.withAlpha (0.02f));
    for (int yy = bounds.getY(); yy < bounds.getBottom(); yy += 3)
        g.drawHorizontalLine (yy, (float) bounds.getX(), (float) bounds.getRight());

    if (! withRivets)
        return;

    const float rivetInset = 10.0f;
    const float rivetRadius = 3.0f;
    const juce::Point<float> corners[] = {
        { bounds.getX() + rivetInset, bounds.getY() + rivetInset },
        { bounds.getRight() - rivetInset, bounds.getY() + rivetInset },
        { bounds.getX() + rivetInset, bounds.getBottom() - rivetInset },
        { bounds.getRight() - rivetInset, bounds.getBottom() - rivetInset },
    };
    for (auto c : corners)
    {
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillEllipse (c.x - rivetRadius, c.y - rivetRadius + 1.2f, rivetRadius * 2.0f, rivetRadius * 2.0f);
        juce::ColourGradient rivetGradient (juce::Colour (0xffd9dee5), c.x - rivetRadius,
                                             c.y - rivetRadius, juce::Colour (0xff4b5364),
                                             c.x + rivetRadius, c.y + rivetRadius, true);
        g.setGradientFill (rivetGradient);
        g.fillEllipse (c.x - rivetRadius, c.y - rivetRadius, rivetRadius * 2.0f, rivetRadius * 2.0f);
    }
}

void Ee1073LookAndFeel::paintWoodFrame (juce::Graphics& g, juce::Rectangle<int> outerBounds,
                                         juce::Rectangle<int> innerBounds)
{
    juce::Path frame;
    frame.addRectangle (outerBounds.toFloat());
    juce::Path hole;
    hole.addRectangle (innerBounds.toFloat());
    frame.setUsingNonZeroWinding (false);
    frame.addPath (hole);

    juce::ColourGradient woodGradient (juce::Colour (0xffb5793a), (float) outerBounds.getX(),
                                        (float) outerBounds.getY(), juce::Colour (0xff8a5a28),
                                        (float) outerBounds.getRight(), (float) outerBounds.getBottom(), false);
    g.saveState();
    g.reduceClipRegion (frame);
    g.setGradientFill (woodGradient);
    g.fillRect (outerBounds);

    // Procedural vertical grain: deterministic wavy lines, varying shade,
    // no bitmap or randomness so it renders identically every time.
    for (int gx = outerBounds.getX(); gx < outerBounds.getRight(); gx += 3)
    {
        const float wobble = 6.0f * std::sin (static_cast<float> (gx) * 0.045f)
                            + 3.0f * std::sin (static_cast<float> (gx) * 0.13f + 1.7f);
        const float shade = 0.5f + 0.5f * std::sin (static_cast<float> (gx) * 0.08f);
        g.setColour (juce::Colours::black.withAlpha (0.05f + 0.05f * shade));
        g.drawLine ((float) gx, (float) outerBounds.getY(), (float) gx + wobble,
                    (float) outerBounds.getBottom(), 1.0f);
    }

    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawRect (innerBounds.expanded (2), 3);
    g.setColour (juce::Colours::white.withAlpha (0.22f));
    g.drawLine ((float) outerBounds.getX() + 2.0f, (float) outerBounds.getY() + 1.0f,
                (float) outerBounds.getRight() - 2.0f, (float) outerBounds.getY() + 1.0f, 2.0f);
    g.setColour (juce::Colours::black.withAlpha (0.45f));
    g.drawLine ((float) outerBounds.getX() + 2.0f, (float) outerBounds.getBottom() - 1.0f,
                (float) outerBounds.getRight() - 2.0f, (float) outerBounds.getBottom() - 1.0f, 2.0f);

    g.restoreState();
}
