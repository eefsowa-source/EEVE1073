#include "PluginEditor.h"
#include <algorithm>
#include <cmath>

namespace
{
constexpr int frameThickness = 18;

void setupKnob (juce::Slider& s, juce::Colour accent, juce::Component& parent)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    s.setColour (juce::Slider::rotarySliderFillColourId, accent);
    parent.addAndMakeVisible (s);
}

void setupHeaderLabel (juce::Label& l, const juce::String& text, juce::Colour accent, juce::Component& parent)
{
    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    l.setColour (juce::Label::textColourId, accent.brighter (0.35f));
    parent.addAndMakeVisible (l);
}

void setupValueLabel (juce::Label& l, juce::Colour colour, juce::Component& parent)
{
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::FontOptions (10.0f));
    l.setColour (juce::Label::textColourId, colour);
    parent.addAndMakeVisible (l);
}

// Mirrors the interactive mid-band Q formula in
// EE1073/Shared/Ee1073ChannelStripCore.h (ChannelStripCore::updateFilters)
// purely for the on-screen readout -- kept in sync manually since the UI
// has no direct access to the DSP core's internal state.
float approximateMidQ (float midGainDb)
{
    const float gainMagnitude = std::abs (midGainDb);
    return std::clamp (1.4f / (1.0f + gainMagnitude * 0.1f), 0.35f, 1.4f);
}
}

Ee1073AudioProcessorEditor::Ee1073AudioProcessorEditor (Ee1073AudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);

    setupHeaderLabel (inputLabel, "INPUT", juce::Colour (accentInput), *this);
    setupHeaderLabel (highLabel, "HIGH 12k", juce::Colour (accentHigh), *this);
    setupHeaderLabel (midLabel, "MID", juce::Colour (accentMid), *this);
    setupHeaderLabel (lowLabel, "LOW", juce::Colour (accentLow), *this);
    setupHeaderLabel (hpfLabel, "HPF", juce::Colour (accentHpf), *this);
    setupHeaderLabel (outputLabel, "OUTPUT", juce::Colour (accentOutput), *this);
    // Right (silver) panel is light -- override the default light-on-dark
    // label/button text colours set up for the dark left panel.
    outputLabel.setColour (juce::Label::textColourId, juce::Colour (0xff2b2f3a));

    setupKnob (inputSlider, juce::Colour (accentInput), *this);
    setupKnob (highGainSlider, juce::Colour (accentHigh), *this);
    setupKnob (midGainSlider, juce::Colour (accentMid), *this);
    setupKnob (lowGainSlider, juce::Colour (accentLow), *this);
    setupKnob (hpfSlider, juce::Colour (accentHpf), *this);

    outputFader.setSliderStyle (juce::Slider::LinearVertical);
    outputFader.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    outputFader.setColour (juce::Slider::thumbColourId, juce::Colour (accentOutput));
    addAndMakeVisible (outputFader);

    lowFreqBox.addItemList (Ee1073AudioProcessor::lowShelfFreqChoices(), 1);
    midFreqBox.addItemList (Ee1073AudioProcessor::midFreqChoices(), 1);
    addAndMakeVisible (lowFreqBox);
    addAndMakeVisible (midFreqBox);

    setupValueLabel (hpfValueLabel, juce::Colour (accentHpf).brighter (0.4f), *this);
    setupValueLabel (midQLabel, juce::Colour (accentMid).withAlpha (0.85f), *this);

    nameplateLabel.setText ("EE • 1073", juce::dontSendNotification);
    nameplateLabel.setJustificationType (juce::Justification::centredLeft);
    nameplateLabel.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    nameplateLabel.setColour (juce::Label::textColourId, juce::Colour (0xffcdd6ea));
    addAndMakeVisible (nameplateLabel);

    for (auto* button : { &eqOnButton, &phaseButton })
    {
        button->getProperties().set ("pushbutton", true);
        addAndMakeVisible (*button);
    }
    powerButton.getProperties().set ("pushbutton", false);
    powerButton.setColour (juce::ToggleButton::textColourId, juce::Colour (0xff2b2f3a));
    addAndMakeVisible (powerButton);

    auto& apvts = processor.apvts;
    inputAttachment = std::make_unique<SliderAttachment> (apvts, "input", inputSlider);
    hpfAttachment = std::make_unique<SliderAttachment> (apvts, "hpfFreq", hpfSlider);
    lowGainAttachment = std::make_unique<SliderAttachment> (apvts, "lowGain", lowGainSlider);
    midGainAttachment = std::make_unique<SliderAttachment> (apvts, "midGain", midGainSlider);
    highGainAttachment = std::make_unique<SliderAttachment> (apvts, "highGain", highGainSlider);
    outputAttachment = std::make_unique<SliderAttachment> (apvts, "output", outputFader);

    lowFreqAttachment = std::make_unique<ComboAttachment> (apvts, "lowFreq", lowFreqBox);
    midFreqAttachment = std::make_unique<ComboAttachment> (apvts, "midFreq", midFreqBox);

    eqOnAttachment = std::make_unique<ButtonAttachment> (apvts, "eqOn", eqOnButton);
    phaseAttachment = std::make_unique<ButtonAttachment> (apvts, "phaseInvert", phaseButton);
    powerAttachment = std::make_unique<ButtonAttachment> (apvts, "power", powerButton);

    setSize (340, 680);
    startTimerHz (15);
    timerCallback();
}

Ee1073AudioProcessorEditor::~Ee1073AudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel (nullptr);
}

void Ee1073AudioProcessorEditor::timerCallback()
{
    const float midGainDb = static_cast<float> (midGainSlider.getValue());
    midQLabel.setText ("Q ≈ " + juce::String (approximateMidQ (midGainDb), 2), juce::dontSendNotification);

    const auto hpfChoices = Ee1073AudioProcessor::hpfFreqChoices();
    const int hpfIndex = juce::jlimit (0, hpfChoices.size() - 1,
                                        static_cast<int> (std::round (hpfSlider.getValue())));
    hpfValueLabel.setText (hpfChoices[hpfIndex], juce::dontSendNotification);
}

void Ee1073AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff0d0e12));

    Ee1073LookAndFeel::paintWoodFrame (g, getLocalBounds(), panelBounds);

    Ee1073LookAndFeel::paintBrushedPanel (g, leftPanelBounds, juce::Colour (0xff262c3d),
                                          juce::Colour (0xff181c27), true);
    Ee1073LookAndFeel::paintBrushedPanel (g, rightPanelBounds, juce::Colour (0xffb9bfc9),
                                          juce::Colour (0xff8b909c), true);

    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.drawVerticalLine (leftPanelBounds.getRight(), (float) panelBounds.getY(), (float) panelBounds.getBottom());

    // Brand roundel at the top of the silver (fader) panel.
    const auto logoBounds = juce::Rectangle<float> (0, 0, 40.0f, 40.0f)
                                 .withCentre ({ (float) rightPanelBounds.getCentreX(),
                                                (float) rightPanelBounds.getY() + 32.0f });
    g.setColour (juce::Colour (0xff2b2f3a));
    g.fillEllipse (logoBounds);
    g.setColour (juce::Colour (0xffd8dde6));
    g.drawEllipse (logoBounds, 1.5f);
    g.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    g.drawFittedText ("EE", logoBounds.toNearestInt(), juce::Justification::centred, 1);

    // dB tick scale flanking the output fader, drawn from the fader's own
    // value-to-position mapping so it always lines up with the cap.
    if (outputFader.getHeight() > 0)
    {
        g.setFont (juce::FontOptions (9.0f));
        g.setColour (juce::Colour (0xff2b2f3a));
        for (double db : { 20.0, 10.0, 0.0, -10.0, -20.0, -30.0 })
        {
            // getPositionOfValue() returns a position in the slider's own
            // local coordinates, so offset by the slider's position within
            // the editor to draw in the editor's paint() coordinate space.
            const int yy = outputFader.getY() + static_cast<int> (outputFader.getPositionOfValue (db));
            const auto text = (db > 0 ? "+" : "") + juce::String (db, 0);
            g.drawFittedText (text, outputFader.getX() - 30, yy - 7, 26, 14, juce::Justification::centredRight, 1);
            g.drawFittedText (text, outputFader.getRight() + 4, yy - 7, 26, 14, juce::Justification::centredLeft, 1);
            g.drawHorizontalLine (yy, (float) outputFader.getX() - 4.0f, (float) outputFader.getX());
        }
    }

    // LED next to EQL/PHASE row, lit when EQ is engaged.
    const auto ledBounds = juce::Rectangle<float> (0, 0, 7.0f, 7.0f)
                                .withCentre ({ (float) eqOnButton.getBounds().getRight() + 12.0f,
                                               (float) eqOnButton.getBounds().getCentreY() });
    g.setColour (eqOnButton.getToggleState() ? juce::Colour (0xff5fd97a) : juce::Colour (0xff2a2f3a));
    g.fillEllipse (ledBounds);
}

void Ee1073AudioProcessorEditor::resized()
{
    panelBounds = getLocalBounds().reduced (frameThickness);

    const int leftWidth = juce::roundToInt (panelBounds.getWidth() * 0.62f);
    leftPanelBounds = panelBounds.withWidth (leftWidth);
    rightPanelBounds = panelBounds.withTrimmedLeft (leftWidth);

    // ---- Left (dark EQ) panel ----
    auto left = leftPanelBounds.reduced (14, 10);

    auto layoutKnobRow = [&] (juce::Rectangle<int> row, juce::Label& header, juce::Slider& knob, int knobSize)
    {
        header.setBounds (row.removeFromTop (14));
        knob.setBounds (row.removeFromTop (knobSize).withSizeKeepingCentre (knobSize, knobSize));
        return row; // whatever remains, for a freq combo / value label
    };

    auto inputRow = left.removeFromTop (92);
    auto rest1 = layoutKnobRow (inputRow, inputLabel, inputSlider, 62);
    juce::ignoreUnused (rest1);

    auto highRow = left.removeFromTop (86);
    layoutKnobRow (highRow, highLabel, highGainSlider, 56);

    auto midRow = left.removeFromTop (100);
    auto midRest = layoutKnobRow (midRow, midLabel, midGainSlider, 56);
    midFreqBox.setBounds (midRest.removeFromTop (18).reduced (6, 0));
    midQLabel.setBounds (midRest.removeFromTop (14));

    auto lowRow = left.removeFromTop (100);
    auto lowRest = layoutKnobRow (lowRow, lowLabel, lowGainSlider, 56);
    lowFreqBox.setBounds (lowRest.removeFromTop (18).reduced (6, 0));

    auto hpfRow = left.removeFromTop (92);
    auto hpfRest = layoutKnobRow (hpfRow, hpfLabel, hpfSlider, 56);
    hpfValueLabel.setBounds (hpfRest.removeFromTop (14));

    auto buttonsRow = left; // remaining space
    auto buttonsTop = buttonsRow.removeFromTop (24);
    eqOnButton.setBounds (buttonsTop.removeFromLeft (60));
    buttonsTop.removeFromLeft (20); // leave room for the LED drawn in paint()
    phaseButton.setBounds (buttonsTop.removeFromLeft (60));
    nameplateLabel.setBounds (buttonsRow.removeFromBottom (20));

    // ---- Right (silver fader) panel ----
    auto right = rightPanelBounds.reduced (0, 10);
    right.removeFromTop (54); // logo roundel, drawn in paint()
    powerButton.setBounds (right.removeFromBottom (26).withSizeKeepingCentre (70, 22));
    outputLabel.setBounds (right.removeFromBottom (16));
    outputFader.setBounds (right.reduced (46, 6));
}
