#include "PluginEditor.h"

PluginFactorySkeletonAudioProcessorEditor::PluginFactorySkeletonAudioProcessorEditor (PluginFactorySkeletonAudioProcessor& p)
    : juce::GenericAudioProcessorEditor (p)
{
    setSize (300, 120);
}
