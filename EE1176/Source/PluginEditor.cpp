#include "PluginEditor.h"

namespace
{
constexpr int frameThickness = 12;

void setupKnob (juce::Slider& s, juce::Label& l, const juce::String& name, juce::Colour accent,
                 juce::Component& parent)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 16);
    s.setColour (juce::Slider::rotarySliderFillColourId, accent);
    parent.addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    parent.addAndMakeVisible (l);
}
}

Ee1176AudioProcessorEditor::Ee1176AudioProcessorEditor (Ee1176AudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);

    const auto steel = juce::Colour (accentSteel);
    setupKnob (inputSlider, inputLabel, "INPUT", steel, *this);
    setupKnob (attackSlider, attackLabel, "ATTACK", steel, *this);
    setupKnob (releaseSlider, releaseLabel, "RELEASE", steel, *this);
    setupKnob (outputSlider, outputLabel, "OUTPUT", steel, *this);

    ratioLabel.setText ("RATIO", juce::dontSendNotification);
    ratioLabel.setJustificationType (juce::Justification::centred);
    ratioLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    addAndMakeVisible (ratioLabel);

    for (auto& button : ratioButtons)
    {
        button.getProperties().set ("ratiobutton", true);
        button.setClickingTogglesState (true);
        button.onClick = [this] { ratioButtonClicked(); };
        addAndMakeVisible (button);
    }

    nameplateLabel.setText ("EE • 1176", juce::dontSendNotification);
    nameplateLabel.setJustificationType (juce::Justification::centredLeft);
    nameplateLabel.setFont (juce::FontOptions (14.0f, juce::Font::bold));
    nameplateLabel.setColour (juce::Label::textColourId, juce::Colour (0xffd8dde6));
    addAndMakeVisible (nameplateLabel);

    auto& apvts = processor.apvts;
    inputAttachment = std::make_unique<SliderAttachment> (apvts, "input", inputSlider);
    attackAttachment = std::make_unique<SliderAttachment> (apvts, "attack", attackSlider);
    releaseAttachment = std::make_unique<SliderAttachment> (apvts, "release", releaseSlider);
    outputAttachment = std::make_unique<SliderAttachment> (apvts, "output", outputSlider);

    syncRatioButtonsFromParameter();

    setSize (640, 260);
    startTimerHz (30);
}

Ee1176AudioProcessorEditor::~Ee1176AudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void Ee1176AudioProcessorEditor::ratioButtonClicked()
{
    if (updatingRatioButtonsFromHost)
        return;

    const bool b0 = ratioButtons[0].getToggleState();
    const bool b1 = ratioButtons[1].getToggleState();
    const bool b2 = ratioButtons[2].getToggleState();
    const bool b3 = ratioButtons[3].getToggleState();
    const int count = (b0 ? 1 : 0) + (b1 ? 1 : 0) + (b2 ? 1 : 0) + (b3 ? 1 : 0);

    int index = -1;
    if (count == 4)
    {
        index = 4; // "All (British)" -- the real hardware's all-buttons-in trick
    }
    else if (count == 1)
    {
        index = b0 ? 0 : b1 ? 1 : b2 ? 2 : 3;
    }
    else if (count >= 2)
    {
        // Real hardware only fully defines the single-ratio and
        // all-four-in states; for 2-3 buttons pressed at once (not a
        // documented mode) this DSP core has no dedicated behavior, so
        // approximate with the highest-numbered ratio engaged.
        index = b3 ? 3 : b2 ? 2 : 1;
    }
    else
    {
        // count == 0: real hardware always has exactly one (or all four)
        // buttons latched -- don't let the user release every button.
        // Re-press whichever ratio is currently selected.
        syncRatioButtonsFromParameter();
        return;
    }

    if (auto* param = processor.apvts.getParameter ("ratio"))
        param->setValueNotifyingHost (param->convertTo0to1 (static_cast<float> (index)));

    syncRatioButtonsFromParameter();
}

void Ee1176AudioProcessorEditor::syncRatioButtonsFromParameter()
{
    const int index = static_cast<int> (processor.apvts.getRawParameterValue ("ratio")->load());

    updatingRatioButtonsFromHost = true;
    if (index == 4)
    {
        for (auto& button : ratioButtons)
            button.setToggleState (true, juce::dontSendNotification);
    }
    else
    {
        for (int i = 0; i < (int) ratioButtons.size(); ++i)
            ratioButtons[(size_t) i].setToggleState (i == index, juce::dontSendNotification);
    }
    updatingRatioButtonsFromHost = false;
}

void Ee1176AudioProcessorEditor::timerCallback()
{
    // Stay in sync if the ratio parameter changes from outside the UI
    // (host automation, preset recall, undo).
    const int index = static_cast<int> (processor.apvts.getRawParameterValue ("ratio")->load());
    const bool allFourShouldBeOn = index == 4;
    bool matches = true;
    for (int i = 0; i < (int) ratioButtons.size(); ++i)
    {
        const bool expected = allFourShouldBeOn || i == index;
        if (ratioButtons[(size_t) i].getToggleState() != expected)
        {
            matches = false;
            break;
        }
    }
    if (! matches)
        syncRatioButtonsFromParameter();

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
    g.setColour (juce::Colour (0xff6a6e78));
    g.setFont (juce::FontOptions (9.0f));
    g.drawFittedText ("GR", meterBounds.withY (meterBounds.getBottom() + 2).withHeight (12),
                       juce::Justification::centred, 1);
}

void Ee1176AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (frameThickness).reduced (16, 10);

    nameplateLabel.setBounds (area.removeFromBottom (20));
    area.removeFromBottom (4);

    const int knobSize = 72;
    const int knobWidth = 96;

    auto layoutKnob = [&] (juce::Rectangle<int> col, juce::Label& label, juce::Slider& knob)
    {
        label.setBounds (col.removeFromTop (16));
        knob.setBounds (col.removeFromTop (knobSize).withSizeKeepingCentre (knobSize, knobSize + 14));
    };

    auto inputCol = area.removeFromLeft (knobWidth);
    layoutKnob (inputCol, inputLabel, inputSlider);

    auto ratioCol = area.removeFromLeft (200);
    ratioLabel.setBounds (ratioCol.removeFromTop (16));
    ratioCol.removeFromTop (10);
    auto buttonRow = ratioCol.removeFromTop (28);
    const int buttonWidth = buttonRow.getWidth() / 4;
    for (auto& button : ratioButtons)
        button.setBounds (buttonRow.removeFromLeft (buttonWidth).reduced (4, 0));

    ratioCol.removeFromTop (14);
    meterBounds = ratioCol.removeFromTop (18).reduced (4, 0);

    auto attackCol = area.removeFromLeft (knobWidth);
    layoutKnob (attackCol, attackLabel, attackSlider);

    auto releaseCol = area.removeFromLeft (knobWidth);
    layoutKnob (releaseCol, releaseLabel, releaseSlider);

    auto outputCol = area.removeFromLeft (knobWidth);
    layoutKnob (outputCol, outputLabel, outputSlider);
}
