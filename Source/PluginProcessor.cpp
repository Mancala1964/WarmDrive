#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
WarmDriveProcessor::WarmDriveProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    driveParam  = apvts.getRawParameterValue (ParamIDs::drive);
    toneParam   = apvts.getRawParameterValue (ParamIDs::tone);
    mixParam    = apvts.getRawParameterValue (ParamIDs::mix);
    outputParam = apvts.getRawParameterValue (ParamIDs::output);
}

juce::AudioProcessorValueTreeState::ParameterLayout WarmDriveProcessor::createParameterLayout()
{
    using namespace juce;
    AudioProcessorValueTreeState::ParameterLayout layout;

    auto dB = AudioParameterFloatAttributes().withLabel ("dB");
    auto pct = AudioParameterFloatAttributes().withLabel ("%");

    // Drive: how hard we push the signal into the saturator (0 to +36 dB)
    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::drive, 1 }, "Drive",
        NormalisableRange<float> (0.0f, 36.0f, 0.1f), 12.0f, dB));

    // Tone: 0% = dark, 100% = bright
    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::tone, 1 }, "Tone",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 70.0f, pct));

    // Mix: 0% = original sound only, 100% = fully saturated
    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::mix, 1 }, "Mix",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, pct));

    // Output volume
    layout.add (std::make_unique<AudioParameterFloat> (
        ParameterID { ParamIDs::output, 1 }, "Output",
        NormalisableRange<float> (-24.0f, 12.0f, 0.1f), 0.0f, dB));

    return layout;
}

//==============================================================================
void WarmDriveProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    const auto numChannels = (juce::uint32) juce::jmax (getTotalNumInputChannels(),
                                                        getTotalNumOutputChannels());

    const juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, numChannels };

    oversampling.reset();
    oversampling.initProcessing ((size_t) samplesPerBlock);

    const juce::dsp::ProcessSpec osSpec { sampleRate * (double) oversampling.getOversamplingFactor(),
                                          (juce::uint32) samplesPerBlock * (juce::uint32) oversampling.getOversamplingFactor(),
                                          numChannels };

    driveGain.prepare (osSpec);
    driveGain.setRampDurationSeconds (0.05);

    toneFilter.prepare (spec);
    toneFilter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    toneFilter.setResonance (1.0f / juce::MathConstants<float>::sqrt2);

    makeupGain.prepare (spec);
    makeupGain.setRampDurationSeconds (0.05);

    outputGain.prepare (spec);
    outputGain.setRampDurationSeconds (0.05);

    dryWet.prepare (spec);
    dryWet.setMixingRule (juce::dsp::DryWetMixingRule::balanced);

    // Oversampling filters delay the sound slightly; tell the mixer and the host.
    const auto latency = oversampling.getLatencyInSamples();
    dryWet.setWetLatency (latency);
    setLatencySamples (juce::roundToInt (latency));

    updateParameters();

    driveGain.reset();
    toneFilter.reset();
    makeupGain.reset();
    outputGain.reset();
    dryWet.reset();
}

void WarmDriveProcessor::releaseResources()
{
    oversampling.reset();
}

bool WarmDriveProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();

    // Mono or stereo, same in and out.
    if (out != juce::AudioChannelSet::mono() && out != juce::AudioChannelSet::stereo())
        return false;

    return out == layouts.getMainInputChannelSet();
}

//==============================================================================
void WarmDriveProcessor::updateParameters()
{
    const auto driveDb = driveParam->load();
    driveGain.setGainDecibels (driveDb);

    // Automatic level compensation, so turning Drive up changes the *character*
    // more than the volume. We match the level of a typical signal (-10 dBFS).
    constexpr float refLevel = 0.316f;
    const auto driveLin = juce::Decibels::decibelsToGain (driveDb);
    makeupGain.setGainLinear (refLevel / std::tanh (driveLin * refLevel));

    outputGain.setGainDecibels (outputParam->load());

    dryWet.setWetMixProportion (mixParam->load() / 100.0f);

    // Tone 0..100% -> low-pass cutoff from 1 kHz to 20 kHz (on a musical, log scale)
    const auto t = toneParam->load() / 100.0f;
    auto cutoff = 1000.0f * std::pow (20.0f, t);
    cutoff = juce::jmin (cutoff, (float) currentSampleRate * 0.45f);
    toneFilter.setCutoffFrequency (cutoff);
}

void WarmDriveProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Clear any extra output channels that have no input.
    for (auto ch = getTotalNumInputChannels(); ch < getTotalNumOutputChannels(); ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    updateParameters();

    juce::dsp::AudioBlock<float> block (buffer);
    block = block.getSubsetChannelBlock (0, (size_t) getTotalNumInputChannels());

    // 1) Remember the clean signal for the Mix knob
    dryWet.pushDrySamples (block);

    // 2) Go up to 2x sample rate
    auto osBlock = oversampling.processSamplesUp (block);

    // 3) Push the level up (Drive) ...
    driveGain.process (juce::dsp::ProcessContextReplacing<float> (osBlock));

    // 4) ... and soft-clip it. tanh() rounds off the peaks smoothly = "warm" saturation.
    for (size_t ch = 0; ch < osBlock.getNumChannels(); ++ch)
    {
        auto* data = osBlock.getChannelPointer (ch);
        for (size_t i = 0; i < osBlock.getNumSamples(); ++i)
            data[i] = std::tanh (data[i]);
    }

    // 5) Back down to the normal sample rate
    oversampling.processSamplesDown (block);

    // 6) Tone control and level compensation (on the saturated signal only)
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    toneFilter.process (ctx);
    makeupGain.process (ctx);

    // 7) Blend clean + saturated, then final output volume
    dryWet.mixWetSamples (block);
    outputGain.process (ctx);
}

//==============================================================================
juce::AudioProcessorEditor* WarmDriveProcessor::createEditor()
{
    return new WarmDriveEditor (*this);
}

void WarmDriveProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, destData);
}

void WarmDriveProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

//==============================================================================
// This creates new instances of the plugin.
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new WarmDriveProcessor();
}
