#include "PluginEditor.h"
#include <algorithm>
#include <cmath>

namespace
{
constexpr int headerHeight = 54;
constexpr int sectionLabelHeight = 20;

void setupSlider (juce::Slider& s, juce::Label& l, const juce::String& name, juce::Component& parent)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 18);
    parent.addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::FontOptions (12.0f));
    l.attachToComponent (&s, false);
    parent.addAndMakeVisible (l);
}

void setupSectionHeader (juce::Label& l, const juce::String& text, juce::Component& parent)
{
    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setFont (juce::FontOptions (13.0f, juce::Font::bold));
    l.setColour (juce::Label::textColourId, juce::Colour (0xffb8c4dd));
    parent.addAndMakeVisible (l);
}

// Mirrors the interactive mid-band Q formula in
// EE1073/Shared/Ee1073ChannelStripCore.h (ChannelStripCore::updateFilters)
// purely for the on-screen readout -- kept in sync manually since the UI
// has no direct access to the DSP core's internal state.
float approximateMidQ (float midGainDb)
{
    const float gainMagnitude = std::abs (midGainDb);
    return std::clamp (1.4f / (1.0f + gainMagnitude * 0.09f), 0.35f, 1.4f);
}
}

Ee1073AudioProcessorEditor::Ee1073AudioProcessorEditor (Ee1073AudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setupSectionHeader (sectionInput, "INPUT", *this);
    setupSectionHeader (sectionHpf, "HPF", *this);
    setupSectionHeader (sectionLow, "LOW SHELF", *this);
    setupSectionHeader (sectionMid, "MID (PARAMETRIC)", *this);
    setupSectionHeader (sectionHigh, "HIGH SHELF 12k", *this);
    setupSectionHeader (sectionOutput, "OUTPUT", *this);

    setupSlider (inputSlider, inputLabel, "Gain", *this);
    setupSlider (lowGainSlider, lowLabel, "Gain", *this);
    setupSlider (midGainSlider, midLabel, "Gain", *this);
    setupSlider (highGainSlider, highLabel, "Gain", *this);
    setupSlider (outputSlider, outputLabel, "Gain", *this);

    hpfFreqBox.addItemList (Ee1073AudioProcessor::hpfFreqChoices(), 1);
    lowFreqBox.addItemList (Ee1073AudioProcessor::lowShelfFreqChoices(), 1);
    midFreqBox.addItemList (Ee1073AudioProcessor::midFreqChoices(), 1);
    addAndMakeVisible (hpfFreqBox);
    addAndMakeVisible (lowFreqBox);
    addAndMakeVisible (midFreqBox);

    addAndMakeVisible (hpfOnButton);
    addAndMakeVisible (eqOnButton);
    eqOnButton.setColour (juce::ToggleButton::textColourId, juce::Colour (0xffb8c4dd));

    midQLabel.setJustificationType (juce::Justification::centred);
    midQLabel.setFont (juce::FontOptions (11.0f));
    midQLabel.setColour (juce::Label::textColourId, juce::Colour (0xff7d8bab));
    addAndMakeVisible (midQLabel);

    auto& apvts = processor.apvts;
    inputAttachment = std::make_unique<SliderAttachment> (apvts, "input", inputSlider);
    lowGainAttachment = std::make_unique<SliderAttachment> (apvts, "lowGain", lowGainSlider);
    midGainAttachment = std::make_unique<SliderAttachment> (apvts, "midGain", midGainSlider);
    highGainAttachment = std::make_unique<SliderAttachment> (apvts, "highGain", highGainSlider);
    outputAttachment = std::make_unique<SliderAttachment> (apvts, "output", outputSlider);

    hpfFreqAttachment = std::make_unique<ComboAttachment> (apvts, "hpfFreq", hpfFreqBox);
    lowFreqAttachment = std::make_unique<ComboAttachment> (apvts, "lowFreq", lowFreqBox);
    midFreqAttachment = std::make_unique<ComboAttachment> (apvts, "midFreq", midFreqBox);

    hpfOnAttachment = std::make_unique<ButtonAttachment> (apvts, "hpfOn", hpfOnButton);
    eqOnAttachment = std::make_unique<ButtonAttachment> (apvts, "eqOn", eqOnButton);

    setSize (720, 300);
    startTimerHz (15);
    timerCallback();
}

Ee1073AudioProcessorEditor::~Ee1073AudioProcessorEditor()
{
    stopTimer();
}

void Ee1073AudioProcessorEditor::timerCallback()
{
    const float midGainDb = static_cast<float> (midGainSlider.getValue());
    midQLabel.setText ("Q ≈ " + juce::String (approximateMidQ (midGainDb), 2), juce::dontSendNotification);
}

void Ee1073AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1c202c));

    auto headerArea = getLocalBounds().removeFromTop (headerHeight);
    g.setColour (juce::Colour (0xff262c3d));
    g.fillRect (headerArea);

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawFittedText ("EE-1073", headerArea.reduced (16, 0), juce::Justification::centredLeft, 1);
    g.setFont (juce::FontOptions (12.0f));
    g.setColour (juce::Colour (0xff8a93ab));
    g.drawFittedText ("Neve 1073-style mic preamp + EQ", headerArea.reduced (16, 0),
                       juce::Justification::bottomLeft, 1);

    g.setColour (juce::Colour (0xff353d54));
    for (auto x : dividerX)
        g.drawVerticalLine (static_cast<int> (x), static_cast<float> (headerHeight) + 8.0f,
                             static_cast<float> (getHeight()) - 8.0f);
}

void Ee1073AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().withTrimmedTop (headerHeight).reduced (12);

    // EQ bypass toggle lives in the header, right-aligned.
    eqOnButton.setBounds (getWidth() - 90, 16, 80, 22);

    const int numColumns = 6;
    const int columnWidth = area.getWidth() / numColumns;

    auto takeColumn = [&] { return area.removeFromLeft (columnWidth); };

    auto inputCol = takeColumn();
    auto hpfCol = takeColumn();
    auto lowCol = takeColumn();
    auto midCol = takeColumn();
    auto highCol = takeColumn();
    auto outputCol = takeColumn();

    for (size_t i = 0; i < dividerX.size(); ++i)
        dividerX[i] = static_cast<float> (inputCol.getX() + columnWidth * static_cast<int> (i + 1));

    auto layoutColumn = [] (juce::Rectangle<int> col, juce::Label& header)
    {
        header.setBounds (col.removeFromTop (sectionLabelHeight));
        col.removeFromTop (28); // space for the knob's attached label above it
        return col;
    };

    { // INPUT
        auto col = layoutColumn (inputCol, sectionInput);
        inputSlider.setBounds (col.removeFromTop (90).reduced (10, 0));
    }
    { // HPF
        auto col = layoutColumn (hpfCol, sectionHpf);
        hpfOnButton.setBounds (col.removeFromTop (24).reduced (8, 0));
        col.removeFromTop (8);
        hpfFreqBox.setBounds (col.removeFromTop (24).reduced (8, 0));
    }
    { // LOW SHELF
        auto col = layoutColumn (lowCol, sectionLow);
        lowFreqBox.setBounds (col.removeFromTop (24).reduced (8, 0));
        col.removeFromTop (6);
        lowGainSlider.setBounds (col.removeFromTop (70).reduced (10, 0));
    }
    { // MID
        auto col = layoutColumn (midCol, sectionMid);
        midFreqBox.setBounds (col.removeFromTop (24).reduced (8, 0));
        col.removeFromTop (6);
        midGainSlider.setBounds (col.removeFromTop (70).reduced (10, 0));
        midQLabel.setBounds (col.removeFromTop (16));
    }
    { // HIGH SHELF
        auto col = layoutColumn (highCol, sectionHigh);
        col.removeFromTop (30); // no frequency selector -- fixed at 12kHz
        highGainSlider.setBounds (col.removeFromTop (90).reduced (10, 0));
    }
    { // OUTPUT
        auto col = layoutColumn (outputCol, sectionOutput);
        outputSlider.setBounds (col.removeFromTop (90).reduced (10, 0));
    }
}
