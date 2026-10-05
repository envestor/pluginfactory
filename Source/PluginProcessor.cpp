#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginFactorySkeletonAudioProcessor::PluginFactorySkeletonAudioProcessor()
    : AudioProcessor (BusesProperties()
                           .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                           .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    gainParam = apvts.getRawParameterValue (ParamIDs::gainDb);
    driveParam = apvts.getRawParameterValue (ParamIDs::drive);
}

juce::AudioProcessorValueTreeState::ParameterLayout PluginFactorySkeletonAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::gainDb, 1 },
        "Gain",
        juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("dB")));

    // ID `drive`, permanent once released. 0-100%, linear, default 0 (clean,
    // unity-gain path). See PLU-45.
    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::drive, 1 },
        "Drive",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f),
        0.0f,
        juce::AudioParameterFloatAttributes().withLabel ("%")));

    return { params.begin(), params.end() };
}

// Simple linear gain compensation for drive. SaturationStage (PLU-33) has no
// built-in loudness compensation of its own - setOutputTrimDb() is a static
// post-shaper trim, not something that tracks drive - so we add one here:
// as drive goes from 0 to 100%, trim down by up to kMaxDriveCompensationDb.
// This is a simple, ear-tuned compensation, not a loudness-matching
// algorithm; it only needs to keep drive from getting much louder.
static constexpr float kMaxDriveCompensationDb = -6.0f;

void PluginFactorySkeletonAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    smoothedGain.reset (sampleRate, 0.02);
    smoothedGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (gainParam->load()));

    for (auto& stage : saturationStages)
    {
        stage.prepare (sampleRate);
        stage.setMix (1.0f);
    }
}

bool PluginFactorySkeletonAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mono = juce::AudioChannelSet::mono();
    const auto stereo = juce::AudioChannelSet::stereo();

    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != mono && mainOut != stereo)
        return false;

    return mainIn == mainOut;
}

void PluginFactorySkeletonAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto numChannels = buffer.getNumChannels();
    const auto numSamples = buffer.getNumSamples();

    for (auto ch = getTotalNumInputChannels(); ch < numChannels; ++ch)
        buffer.clear (ch, 0, numSamples);

    smoothedGain.setTargetValue (juce::Decibels::decibelsToGain (gainParam->load()));

    // normalizedDrive in [0, 1]; SaturationStage smooths this internally
    // (Sec 2.5.2 one-pole, ~5 ms) so a fast knob sweep can't click or zipper.
    const auto normalizedDrive = driveParam->load() / 100.0f;
    const auto driveCompensationDb = normalizedDrive * kMaxDriveCompensationDb;

    for (int channel = 0; channel < numChannels && channel < maxChannels; ++channel)
    {
        saturationStages[channel].setDrive (normalizedDrive);
        saturationStages[channel].setOutputTrimDb (driveCompensationDb);
    }

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto gain = smoothedGain.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
        {
            auto* data = buffer.getWritePointer (channel);
            auto value = data[sample] * gain;

            if (channel < maxChannels)
                value = saturationStages[channel].processSample (value);

            data[sample] = value;
        }
    }
}

juce::AudioProcessorEditor* PluginFactorySkeletonAudioProcessor::createEditor()
{
    return new PluginFactorySkeletonAudioProcessorEditor (*this);
}

void PluginFactorySkeletonAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
    }
}

void PluginFactorySkeletonAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PluginFactorySkeletonAudioProcessor();
}
