#include "PluginProcessor.h"
#include "PluginEditor.h"

PluginFactorySkeletonAudioProcessor::PluginFactorySkeletonAudioProcessor()
    : AudioProcessor (BusesProperties()
                           .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                           .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    gainParam = apvts.getRawParameterValue (ParamIDs::gainDb);
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

    return { params.begin(), params.end() };
}

void PluginFactorySkeletonAudioProcessor::prepareToPlay (double sampleRate, int /*samplesPerBlock*/)
{
    smoothedGain.reset (sampleRate, 0.02);
    smoothedGain.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (gainParam->load()));
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

    for (int sample = 0; sample < numSamples; ++sample)
    {
        const auto gain = smoothedGain.getNextValue();

        for (int channel = 0; channel < numChannels; ++channel)
            buffer.getWritePointer (channel)[sample] *= gain;
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
