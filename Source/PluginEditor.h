#pragma once

#include "PluginProcessor.h"

// A round knob with a caption underneath, connected to one parameter.
struct LabelledKnob final : public juce::Component
{
    LabelledKnob (juce::AudioProcessorValueTreeState& state, const juce::String& paramID,
                  const juce::String& caption, const juce::String& unitSuffix);

    void resized() override;

    juce::Slider slider;
    juce::Label label;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

class WarmDriveEditor final : public juce::AudioProcessorEditor
{
public:
    explicit WarmDriveEditor (WarmDriveProcessor&);
    ~WarmDriveEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    juce::LookAndFeel_V4 lookAndFeel;

    LabelledKnob drive, tone, mix, output;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WarmDriveEditor)
};
