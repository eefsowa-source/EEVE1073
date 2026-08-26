#include "PluginEditor.h"

namespace
{
constexpr int frameThickness = 12;

void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name, juce::Colour accent,
                 float fontSize, juce::Component& parent)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    s.setColour (juce::Slider::rotarySliderFillColourId, accent);
    parent.addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::FontOptions (fontSize, juce::Font::bold));
    parent.addAndMakeVisible (l);
}
}

Ee1176AudioProcessorEditor::Ee1176AudioProcessorEditor (Ee1176AudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);

    const auto steel = juce::Colour (accentSteel);
    setupKnob (inputSlider, inputLabel, "INPUT", steel, 12.0f, *this);
    setupKnob (ratioSlider, ratioLabel, "COMP RATIO", steel, 10.0f, *this);
    setupKnob (ratioVernierSlider, ratioVernierLabel, "VERNIER", steel, 8.5f, *this);
    setupKnob (attackSlider, attackLabel, "ATTACK", steel, 10.0f, *this);
    setupKnob (attackVernierSlider, attackVernierLabel, "VERNIER", steel, 8.5f, *this);
    setupKnob (outputSlider, outputLabel, "OUTPUT", steel, 12.0f, *this);
    setupKnob (releaseSlider, releaseLabel, "RELEASE", steel, 10.0f, *this);

    addAndMakeVisible (powerButton);

    meterCaptionLabel.setText ("GAIN REDUCTION", juce::dontSendNotification);
    meterCaptionLabel.setJustificationType (juce::Justification::centred);
    meterCaptionLabel.setFont (juce::FontOptions (9.0f));
    meterCaptionLabel.setColour (juce::Label::textColourId, juce::Colour (0xff9aa0ab));
    addAndMakeVisible (meterCaptionLabel);

    nameplateLabel.setText ("EE • 1176   LIMITING AMPLIFIER", juce::dontSendNotification);
    nameplateLabel.setJustificationType (juce::Justification::centred);
    nameplateLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    nameplateLabel.setColour (juce::Label::textColourId, juce::Colour (0xffd8dde6));
    addAndMakeVisible (nameplateLabel);

    auto& apvts = processor.apvts;
    inputAttachment = std::make_unique<SliderAttachment> (apvts, "input", inputSlider);
    ratioAttachment = std::make_unique<SliderAttachment> (apvts, "ratio", ratioSlider);
    ratioVernierAttachment = std::make_unique<SliderAttachment> (apvts, "ratioTrim", ratioVernierSlider);
    attackAttachment = std::make_unique<SliderAttachment> (apvts, "attack", attackSlider);
    attackVernierAttachment = std::make_unique<SliderAttachment> (apvts, "attackTrim", attackVernierSlider);
    outputAttachment = std::make_unique<SliderAttachment> (apvts, "output", outputSlider);
    releaseAttachment = std::make_unique<SliderAttachment> (apvts, "release", releaseSlider);
    powerAttachment = std::make_unique<ButtonAttachment> (apvts, "power", powerButton);

    setSize (640, 260);
    startTimerHz (30);
}

Ee1176AudioProcessorEditor::~Ee1176AudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void Ee1176AudioProcessorEditor::timerCallback()
{
    repaint (meterBounds);
}

void Ee1176AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0a0a0b));

    auto panelBounds = getLocalBounds().reduced (frameThickness);
    Ee1176LookAndFeel::paintRackPanel (g, panelBounds);

    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawRect (panelBounds, 1);

    Ee1176LookAndFeel::paintGainReductionMeter (g, meterBounds, processor.currentGainReductionDb.load(), 20.0f);
}

void Ee1176AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (frameThickness).reduced (16, 8);

    nameplateLabel.setBounds (area.removeFromBottom (18));
    area.removeFromBottom (4);

    auto layoutKnob = [] (juce::Rectangle<int> col, juce::Label& label, juce::Slider& knob, int knobSize)
    {
        label.setBounds (col.removeFromTop (14));
        knob.setBounds (col.removeFromTop (knobSize).withSizeKeepingCentre (knobSize, knobSize));
    };

    // INPUT (large) -- far left.
    auto inputCol = area.removeFromLeft (100);
    layoutKnob (inputCol, inputLabel, inputSlider, 74);

    // COMP RATIO (small) stacked over its VERNIER trim.
    auto ratioCol = area.removeFromLeft (86);
    layoutKnob (ratioCol.removeFromTop (78), ratioLabel, ratioSlider, 50);
    layoutKnob (ratioCol, ratioVernierLabel, ratioVernierSlider, 40);

    // OUTPUT (large) -- far right, laid out now so the meter gets whatever
    // is left in the middle.
    auto outputCol = area.removeFromRight (100);
    layoutKnob (outputCol, outputLabel, outputSlider, 74);

    // RELEASE (small) -- just left of Output.
    auto releaseCol = area.removeFromRight (78);
    releaseCol.removeFromTop (26); // vertically centre-ish, no vernier beneath it
    layoutKnob (releaseCol, releaseLabel, releaseSlider, 54);

    // ATTACK (small) stacked over its VERNIER trim -- just right of the meter.
    auto attackCol = area.removeFromRight (86);
    layoutKnob (attackCol.removeFromTop (78), attackLabel, attackSlider, 50);
    layoutKnob (attackCol, attackVernierLabel, attackVernierSlider, 40);

    // Everything remaining in the middle is the VU meter.
    auto meterCol = area;
    meterCaptionLabel.setBounds (meterCol.removeFromBottom (14));
    meterBounds = meterCol.reduced (6, 10);

    // POWER switch, tucked under the meter caption at the panel's bottom-right.
    powerButton.setBounds (getWidth() - frameThickness - 90, getHeight() - frameThickness - 30, 80, 20);
}
