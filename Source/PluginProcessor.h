#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// Parameter IDs (shared with the editor)
namespace ParamIDs
{
    inline constexpr auto drive  = "drive";
    inline constexpr auto tone   = "tone";
    inline constexpr auto mix    = "mix";
    inline constexpr auto output = "output";
}

class WarmDriveProcessor final : public juce::AudioProcessor
{
public:
    WarmDriveProcessor();
    ~WarmDriveProcessor() override = default;

    // --- audio ---
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    // --- editor ---
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    // --- info ---
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    // --- programs (unused) ---
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    // --- save / load settings in the song ---
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateParameters();

    // Cached parameter pointers (fast, real-time safe reads)
    std::atomic<float>* driveParam  = nullptr;
    std::atomic<float>* toneParam   = nullptr;
    std::atomic<float>* mixParam    = nullptr;
    std::atomic<float>* outputParam = nullptr;

    // DSP building blocks
    // 2x oversampling keeps the distortion from creating harsh "aliasing" tones.
    juce::dsp::Oversampling<float> oversampling { 2, 1,
        juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false };

    juce::dsp::Gain<float> driveGain;       // runs at the oversampled rate
    juce::dsp::StateVariableTPTFilter<float> toneFilter;
    juce::dsp::Gain<float> makeupGain;      // automatic level compensation
    juce::dsp::DryWetMixer<float> dryWet { 64 };   // 64 = max latency it can line up
    juce::dsp::Gain<float> outputGain;

    double currentSampleRate = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WarmDriveProcessor)
};
