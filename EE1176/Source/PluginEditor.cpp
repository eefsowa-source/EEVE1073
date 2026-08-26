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

    const auto mint = juce::Colour (accentMint);
    setupKnob (inputSlider, inputLabel, "INPUT", mint, 12.0f, *this);
    setupKnob (outputSlider, outputLabel, "OUTPUT", mint, 12.0f, *this);
    setupKnob (attackSlider, attackLabel, "ATTACK", mint, 10.0f, *this);
    setupKnob (releaseSlider, releaseLabel, "RELEASE", mint, 10.0f, *this);
    setupKnob (ratioSlider, ratioLabel, "RATIO", mint, 10.0f, *this);

    addAndMakeVisible (powerButton);

    meterCaptionLabel.setText ("METER", juce::dontSendNotification);
    meterCaptionLabel.setJustificationType (juce::Justification::centred);
    meterCaptionLabel.setFont (juce::FontOptions (9.0f));
    meterCaptionLabel.setColour (juce::Label::textColourId, juce::Colour (0xff9fe6c9));
    addAndMakeVisible (meterCaptionLabel);

    nameplateLabel.setText ("EE • 1176LN   LIMITING AMPLIFIER", juce::dontSendNotification);
    nameplateLabel.setJustificationType (juce::Justification::centred);
    nameplateLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    nameplateLabel.setColour (juce::Label::textColourId, juce::Colour (0xffdaf7ea));
    addAndMakeVisible (nameplateLabel);

    auto& apvts = processor.apvts;
    inputAttachment = std::make_unique<SliderAttachment> (apvts, "input", inputSlider);
    outputAttachment = std::make_unique<SliderAttachment> (apvts, "output", outputSlider);
    attackAttachment = std::make_unique<SliderAttachment> (apvts, "attack", attackSlider);
    releaseAttachment = std::make_unique<SliderAttachment> (apvts, "release", releaseSlider);
    ratioAttachment = std::make_unique<SliderAttachment> (apvts, "ratio", ratioSlider);
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
    g.fillAll (juce::Colour (0xff081512));

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

    // OUTPUT (large) -- right next to Input.
    auto outputCol = area.removeFromLeft (100);
    layoutKnob (outputCol, outputLabel, outputSlider, 74);

    // ATTACK stacked above RELEASE, one column.
    auto arCol = area.removeFromLeft (86);
    layoutKnob (arCol.removeFromTop (78), attackLabel, attackSlider, 52);
    layoutKnob (arCol, releaseLabel, releaseSlider, 52);

    // RATIO (small) -- just left of the meter.
    auto ratioCol = area.removeFromLeft (86);
    ratioCol.removeFromTop (26); // roughly vertically centred, single knob
    layoutKnob (ratioCol, ratioLabel, ratioSlider, 60);

    // POWER switch -- bottom-right corner.
    powerButton.setBounds (getWidth() - frameThickness - 90, getHeight() - frameThickness - 30, 80, 20);

    // Everything remaining on the right is the VU meter.
    auto meterCol = area;
    meterCaptionLabel.setBounds (meterCol.removeFromBottom (14));
    meterCol.removeFromBottom (24); // leave room above the Power switch
    meterBounds = meterCol.reduced (6, 4);
}
