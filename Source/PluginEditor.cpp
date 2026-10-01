#include "PluginEditor.h"

namespace Colours
{
    const juce::Colour background { 0xff1e1a17 };   // dark warm brown
    const juce::Colour panel      { 0xff2a2420 };
    const juce::Colour accent     { 0xffe0893a };   // amber
    const juce::Colour text       { 0xfff2e6d9 };
    const juce::Colour dimText    { 0xffa8998a };
}

//==============================================================================
LabelledKnob::LabelledKnob (juce::AudioProcessorValueTreeState& state, const juce::String& paramID,
                            const juce::String& caption, const juce::String& unitSuffix)
    : attachment (state, paramID, slider)
{
    slider.setTextValueSuffix (unitSuffix);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    slider.setColour (juce::Slider::textBoxTextColourId, Colours::dimText);
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 72, 20);
    slider.setDoubleClickReturnValue (true, (double) state.getParameter (paramID)->convertFrom0to1 (
                                                state.getParameter (paramID)->getDefaultValue()));
    addAndMakeVisible (slider);

    label.setText (caption, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setFont (juce::FontOptions (15.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, Colours::text);
    addAndMakeVisible (label);
}

void LabelledKnob::resized()
{
    auto area = getLocalBounds();
    label.setBounds (area.removeFromTop (22));
    slider.setBounds (area);
}

//==============================================================================
WarmDriveEditor::WarmDriveEditor (WarmDriveProcessor& p)
    : AudioProcessorEditor (&p),
      drive  (p.apvts, ParamIDs::drive,  "DRIVE",  " dB"),
      tone   (p.apvts, ParamIDs::tone,   "TONE",   " %"),
      mix    (p.apvts, ParamIDs::mix,    "MIX",    " %"),
      output (p.apvts, ParamIDs::output, "OUTPUT", " dB")
{
    lookAndFeel.setColour (juce::Slider::rotarySliderFillColourId,    Colours::accent);
    lookAndFeel.setColour (juce::Slider::rotarySliderOutlineColourId, Colours::panel.brighter (0.25f));
    lookAndFeel.setColour (juce::Slider::thumbColourId,               Colours::text);
    lookAndFeel.setColour (juce::Slider::textBoxTextColourId,         Colours::dimText);
    lookAndFeel.setColour (juce::Slider::textBoxOutlineColourId,      juce::Colours::transparentBlack);
    lookAndFeel.setColour (juce::Slider::textBoxBackgroundColourId,   juce::Colours::transparentBlack);
    setLookAndFeel (&lookAndFeel);

    for (auto* knob : { &drive, &tone, &mix, &output })
        addAndMakeVisible (knob);

    setSize (520, 250);
}

WarmDriveEditor::~WarmDriveEditor()
{
    setLookAndFeel (nullptr);
}

void WarmDriveEditor::paint (juce::Graphics& g)
{
    g.fillAll (Colours::background);

    auto area = getLocalBounds().reduced (12);
    auto header = area.removeFromTop (40);

    g.setColour (Colours::accent);
    g.setFont (juce::FontOptions (24.0f, juce::Font::bold));
    g.drawText ("WARM DRIVE", header, juce::Justification::centredLeft);

    g.setColour (Colours::dimText);
    g.setFont (juce::FontOptions (13.0f));
    g.drawText ("saturation", header, juce::Justification::centredRight);

    g.setColour (Colours::panel);
    g.fillRoundedRectangle (area.toFloat(), 10.0f);
}

void WarmDriveEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    area.removeFromTop (40);
    area = area.reduced (10);

    const auto knobWidth = area.getWidth() / 4;
    for (auto* knob : { &drive, &tone, &mix, &output })
        knob->setBounds (area.removeFromLeft (knobWidth).reduced (4));
}
