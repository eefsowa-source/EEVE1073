#include "PluginEditor.h"

namespace
{
void setupSlider (juce::Slider& s, juce::Label& l, const juce::String& name, juce::Component& parent)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 18);
    parent.addAndMakeVisible (s);

    l.setText (name, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.attachToComponent (&s, false);
    parent.addAndMakeVisible (l);
}
}

Ee1073AudioProcessorEditor::Ee1073AudioProcessorEditor (Ee1073AudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setupSlider (inputSlider, inputLabel, "Input", *this);
    setupSlider (lowGainSlider, lowLabel, "Low", *this);
    setupSlider (midGainSlider, midLabel, "Mid", *this);
    setupSlider (highGainSlider, highLabel, "High (12k)", *this);
    setupSlider (outputSlider, outputLabel, "Output", *this);

    hpfFreqBox.addItemList (Ee1073AudioProcessor::hpfFreqChoices(), 1);
    lowFreqBox.addItemList (Ee1073AudioProcessor::lowShelfFreqChoices(), 1);
    midFreqBox.addItemList (Ee1073AudioProcessor::midFreqChoices(), 1);
    addAndMakeVisible (hpfFreqBox);
    addAndMakeVisible (lowFreqBox);
    addAndMakeVisible (midFreqBox);

    addAndMakeVisible (hpfOnButton);
    addAndMakeVisible (eqOnButton);

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

    setSize (640, 260);
}

void Ee1073AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff23293a));
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawFittedText ("EE-1073", getLocalBounds().removeFromTop (30), juce::Justification::centred, 1);
}

void Ee1073AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().withTrimmedTop (50).reduced (10);

    auto topRow = area.removeFromTop (area.getHeight() / 2);
    const int knobWidth = topRow.getWidth() / 5;
    inputSlider.setBounds (topRow.removeFromLeft (knobWidth).reduced (8));
    lowGainSlider.setBounds (topRow.removeFromLeft (knobWidth).reduced (8));
    midGainSlider.setBounds (topRow.removeFromLeft (knobWidth).reduced (8));
    highGainSlider.setBounds (topRow.removeFromLeft (knobWidth).reduced (8));
    outputSlider.setBounds (topRow.removeFromLeft (knobWidth).reduced (8));

    auto bottomRow = area;
    const int boxWidth = bottomRow.getWidth() / 5;
    hpfOnButton.setBounds (bottomRow.removeFromLeft (boxWidth).reduced (4).withHeight (24));
    hpfFreqBox.setBounds (bottomRow.removeFromLeft (boxWidth).reduced (4).withHeight (24));
    lowFreqBox.setBounds (bottomRow.removeFromLeft (boxWidth).reduced (4).withHeight (24));
    midFreqBox.setBounds (bottomRow.removeFromLeft (boxWidth).reduced (4).withHeight (24));
    eqOnButton.setBounds (bottomRow.removeFromLeft (boxWidth).reduced (4).withHeight (24));
}
