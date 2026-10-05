#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Parameter is referenced by ID only; the APVTS parameter itself is owned
// and declared by PLU-45 (framework engineer). This editor builds and runs
// standalone before that parameter exists, and binds automatically once it
// does, so the two branches can merge in either order.
class PluginFactorySkeletonAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit PluginFactorySkeletonAudioProcessorEditor (PluginFactorySkeletonAudioProcessor& p);
    ~PluginFactorySkeletonAudioProcessorEditor() override = default;

    void resized() override;

private:
    PluginFactorySkeletonAudioProcessor& processor;

    juce::Label driveLabel;
    juce::Slider driveKnob;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginFactorySkeletonAudioProcessorEditor)
};
