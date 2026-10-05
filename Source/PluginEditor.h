#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

class PluginFactorySkeletonAudioProcessorEditor : public juce::GenericAudioProcessorEditor
{
public:
    explicit PluginFactorySkeletonAudioProcessorEditor (PluginFactorySkeletonAudioProcessor& p);
    ~PluginFactorySkeletonAudioProcessorEditor() override = default;
};
