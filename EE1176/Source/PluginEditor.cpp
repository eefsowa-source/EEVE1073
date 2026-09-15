#include "PluginEditor.h"
#include <array>

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

    const auto accent = juce::Colour (accentMint);
    setupKnob (inputSlider, inputLabel, "INPUT", accent, 12.0f, *this);
    setupKnob (outputSlider, outputLabel, "OUTPUT", accent, 12.0f, *this);
    setupKnob (attackSlider, attackLabel, "ATTACK", accent, 10.0f, *this);
    setupKnob (releaseSlider, releaseLabel, "RELEASE", accent, 10.0f, *this);
    ratioLabel.setText ("COMP RATIO", juce::dontSendNotification);
    ratioLabel.setJustificationType (juce::Justification::centred);
    ratioLabel.setFont (juce::FontOptions (10.0f, juce::Font::bold));
    addAndMakeVisible (ratioLabel);

    const char* ratioNames[] = { "4:1", "8:1", "12:1", "20:1", "ALL" };
    for (size_t i = 0; i < ratioButtons.size(); ++i)
    {
        auto& button = ratioButtons[i];
        button.setButtonText (ratioNames[i]);
        button.setClickingTogglesState (false);
        button.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff26343d));
        button.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff79d8b1));
        button.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffd8e4e7));
        button.setColour (juce::TextButton::textColourOnId, juce::Colour (0xff10231d));
        button.setRadioGroupId (1176);
        button.onClick = [this, i]
        {
            if (auto* parameter = processor.apvts.getParameter ("ratio"))
                parameter->setValueNotifyingHost (static_cast<float> (i) / 4.0f);
        };
        addAndMakeVisible (button);
    }

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
    powerAttachment = std::make_unique<ButtonAttachment> (apvts, "power", powerButton);

    setSize (820, 300);
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

    const bool shiftDown = juce::ModifierKeys::getCurrentModifiers().isShiftDown();
    if (shiftDown && ! shiftAllActive)
    {
        ratioBeforeShift = static_cast<int> (processor.apvts.getRawParameterValue ("ratio")->load() + 0.5f);
        if (auto* parameter = processor.apvts.getParameter ("ratio"))
            parameter->setValueNotifyingHost (1.0f);
        shiftAllActive = true;
    }
    else if (! shiftDown && shiftAllActive)
    {
        if (auto* parameter = processor.apvts.getParameter ("ratio"))
            parameter->setValueNotifyingHost (static_cast<float> (juce::jlimit (0, 4, ratioBeforeShift)) / 4.0f);
        shiftAllActive = false;
    }

    const auto ratio = shiftAllActive ? 4 : static_cast<int> (processor.apvts.getRawParameterValue ("ratio")->load() + 0.5f);
    for (size_t i = 0; i < ratioButtons.size(); ++i)
        ratioButtons[i].setToggleState (static_cast<int> (i) == ratio, juce::dontSendNotification);
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
    auto area = getLocalBounds().reduced (frameThickness).reduced (18, 14);

    nameplateLabel.setBounds (area.removeFromTop (26));
    area.removeFromTop (10);

    // Match the classic front panel: input/output on the left, timing in the
    // centre, ratio buttons and the GR meter on the right.
    auto controls = area;
    auto meterCol = controls.removeFromRight (190);
    meterCaptionLabel.setBounds (meterCol.removeFromBottom (18));
    meterBounds = meterCol.reduced (8, 4);
    controls.removeFromRight (16);

    auto layoutKnob = [] (juce::Rectangle<int> col, juce::Label& label, juce::Slider& knob, int knobSize)
    {
        label.setBounds (col.removeFromTop (14));
        knob.setBounds (col.removeFromTop (knobSize).withSizeKeepingCentre (knobSize, knobSize));
    };

    auto topRow = controls.removeFromTop (150);
    layoutKnob (topRow.removeFromLeft (155), inputLabel, inputSlider, 112);
    layoutKnob (topRow.removeFromLeft (155), outputLabel, outputSlider, 112);

    controls.removeFromTop (8);
    auto timing = controls.removeFromLeft (130);
    layoutKnob (timing.removeFromTop (72), attackLabel, attackSlider, 58);
    layoutKnob (timing.removeFromTop (72), releaseLabel, releaseSlider, 58);

    auto ratioBay = controls.removeFromLeft (110);
    ratioLabel.setBounds (ratioBay.removeFromTop (18));
    auto ratioColumn = ratioBay.reduced (8, 2);
    const auto buttonHeight = ratioColumn.getHeight() / static_cast<int> (ratioButtons.size());
    for (auto& button : ratioButtons)
        button.setBounds (ratioColumn.removeFromTop (buttonHeight).reduced (2, 1));

    powerButton.setBounds (getLocalBounds().getRight() - frameThickness - 94,
                           getLocalBounds().getBottom() - frameThickness - 26, 84, 22);
}
