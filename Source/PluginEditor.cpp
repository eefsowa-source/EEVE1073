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

EeveAudioProcessorEditor::EeveAudioProcessorEditor (EeveAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setupSlider (inputSlider, inputLabel, "Input", *this);
    setupSlider (outputSlider, outputLabel, "Output", *this);
    setupSlider (attackSlider, attackLabel, "Attack", *this);
    setupSlider (releaseSlider, releaseLabel, "Release", *this);

    ratioBox.addItemList ({ "4:1", "8:1", "12:1", "20:1", "All (British)" }, 1);
    addAndMakeVisible (ratioBox);
    ratioLabel.setText ("Ratio", juce::dontSendNotification);
    ratioLabel.setJustificationType (juce::Justification::centred);
    ratioLabel.attachToComponent (&ratioBox, false);
    addAndMakeVisible (ratioLabel);

    auto& apvts = processor.apvts;
    inputAttachment = std::make_unique<SliderAttachment> (apvts, "input", inputSlider);
    outputAttachment = std::make_unique<SliderAttachment> (apvts, "output", outputSlider);
    attackAttachment = std::make_unique<SliderAttachment> (apvts, "attack", attackSlider);
    releaseAttachment = std::make_unique<SliderAttachment> (apvts, "release", releaseSlider);
    ratioAttachment = std::make_unique<ComboAttachment> (apvts, "ratio", ratioBox);

    setSize (500, 220);
}

void EeveAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff2b2b2b));
    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawFittedText ("EEVE1073", getLocalBounds().removeFromTop (30), juce::Justification::centred, 1);
}

void EeveAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().withTrimmedTop (50).reduced (10);
    const int knobWidth = area.getWidth() / 5;

    inputSlider.setBounds (area.removeFromLeft (knobWidth).reduced (8));
    outputSlider.setBounds (area.removeFromLeft (knobWidth).reduced (8));
    attackSlider.setBounds (area.removeFromLeft (knobWidth).reduced (8));
    releaseSlider.setBounds (area.removeFromLeft (knobWidth).reduced (8));
    ratioBox.setBounds (area.removeFromLeft (knobWidth).reduced (8).withHeight (24).withY (area.getCentreY()));
}
