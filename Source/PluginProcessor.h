#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "saturation/SaturationStage.h"

// Permanent parameter ID. Never rename or remove once released; only add new ones.
namespace ParamIDs
{
    static const juce::String gainDb { "gainDb" };
    static const juce::String drive { "drive" };
}

class PluginFactorySkeletonAudioProcessor : public juce::AudioProcessor
{
public:
    PluginFactorySkeletonAudioProcessor();
    ~PluginFactorySkeletonAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    using AudioProcessor::processBlock;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::LinearSmoothedValue<float> smoothedGain;
    std::atomic<float>* gainParam = nullptr;
    std::atomic<float>* driveParam = nullptr;

    // One saturation stage per channel so left/right smoothing state never
    // cross-talks. Sized for stereo; processBlock only drives as many as the
    // host gives us.
    static constexpr int maxChannels = 2;
    bdpo::dsp::SaturationStage saturationStages[maxChannels];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginFactorySkeletonAudioProcessor)
};
