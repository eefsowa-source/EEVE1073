#include "Ee1073LookAndFeel.h"

namespace
{
constexpr auto kPanelDark = 0xff1a1d26;
constexpr auto kPanelLight = 0xff262b3a;
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

    // Track arc (dim, full sweep).
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, radius * 0.92f, radius * 0.92f, 0.0f,
                          rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (juce::Colour (kTrackDim));
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Fill arc from the 0-position (proportional 0.5 for bipolar gain
    // params, which is what all EE-1073 knobs are) to the current value.
    const auto zeroProportional = (slider.getMinimum() < 0.0 && slider.getMaximum() > 0.0)
                                       ? static_cast<float> (slider.valueToProportionOfLength (0.0))
                                       : 0.0f;
    const auto zeroAngle = rotaryStartAngle + zeroProportional * (rotaryEndAngle - rotaryStartAngle);

    juce::Path fill;
    fill.addCentredArc (centre.x, centre.y, radius * 0.92f, radius * 0.92f, 0.0f,
                         juce::jmin (zeroAngle, angle), juce::jmax (zeroAngle, angle), true);
    g.setColour (accent);
    g.strokePath (fill, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Knob cap: radial gradient giving a domed metal-cream look.
    const auto capRadius = radius * 0.68f;
    juce::ColourGradient capGradient (juce::Colour (kCream).brighter (0.15f), centre.x - capRadius * 0.4f,
                                       centre.y - capRadius * 0.5f, juce::Colour (kCreamShadow),
                                       centre.x, centre.y + capRadius, true);
    g.setGradientFill (capGradient);
    g.fillEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f);

    g.setColour (juce::Colour (0xff2a2f3d));
    g.drawEllipse (centre.x - capRadius, centre.y - capRadius, capRadius * 2.0f, capRadius * 2.0f, 1.2f);

    // Pointer.
    juce::Path pointer;
    const float pointerLength = capRadius * 0.82f;
    const float pointerThickness = 3.0f;
    pointer.addRoundedRectangle (-pointerThickness * 0.5f, -pointerLength, pointerThickness,
                                  pointerLength * 0.6f, pointerThickness * 0.5f);
    g.setColour (juce::Colour (kNeedleRed));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre));

    // Small tick marks at min/centre/max.
    g.setColour (juce::Colour (0xff5a6280));
    for (float t : { 0.0f, 0.5f, 1.0f })
    {
        const auto tickAngle = rotaryStartAngle + t * (rotaryEndAngle - rotaryStartAngle);
        const auto p1 = centre.getPointOnCircumference (radius * 0.97f, tickAngle);
        const auto p2 = centre.getPointOnCircumference (radius, tickAngle);
        g.drawLine ({ p1, p2 }, 1.5f);
    }
}

void Ee1073LookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                           bool shouldDrawButtonAsHighlighted, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    const auto switchWidth = juce::jmin (bounds.getWidth() * 0.4f, 34.0f);
    auto switchBounds = bounds.removeFromLeft (switchWidth).reduced (2.0f);

    const bool isOn = button.getToggleState();
    const auto onColour = juce::Colour (0xff4caf7d);
    const auto offColour = juce::Colour (0xff3a4156);

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

    g.setColour (button.findColour (juce::ToggleButton::textColourId));
    g.setFont (juce::FontOptions (12.0f));
    g.drawFittedText (button.getButtonText(), bounds.reduced (4.0f, 0.0f).toNearestInt(),
                       juce::Justification::centredLeft, 1);
}

void Ee1073LookAndFeel::drawComboBox (juce::Graphics& g, int width, int height, bool,
                                       int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (1.0f);

    g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle (bounds, 3.0f);
    g.setColour (box.findColour (juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle (bounds, 3.0f, 1.0f);

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

void Ee1073LookAndFeel::paintPanelBackground (juce::Graphics& g, juce::Rectangle<int> bounds)
{
    juce::ColourGradient panelGradient (juce::Colour (kPanelLight), bounds.getX(), (float) bounds.getY(),
                                         juce::Colour (kPanelDark), bounds.getX(), (float) bounds.getBottom(), false);
    g.setGradientFill (panelGradient);
    g.fillRect (bounds);

    // Faint brushed-metal streaks.
    g.setColour (juce::Colours::white.withAlpha (0.015f));
    for (int yy = bounds.getY(); yy < bounds.getBottom(); yy += 3)
        g.drawHorizontalLine (yy, (float) bounds.getX(), (float) bounds.getRight());

    // Corner rivets.
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
        g.setColour (juce::Colours::black.withAlpha (0.5f));
        g.fillEllipse (c.x - rivetRadius, c.y - rivetRadius + 0.6f, rivetRadius * 2.0f, rivetRadius * 2.0f);
        g.setColour (juce::Colour (0xff6b7390));
        g.fillEllipse (c.x - rivetRadius, c.y - rivetRadius, rivetRadius * 2.0f, rivetRadius * 2.0f);
    }
}
