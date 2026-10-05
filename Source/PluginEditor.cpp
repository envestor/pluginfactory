#include "PluginEditor.h"

namespace
{
    constexpr const char* kDriveParamID = "drive";
}

PluginFactorySkeletonAudioProcessorEditor::PluginFactorySkeletonAudioProcessorEditor (PluginFactorySkeletonAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    driveLabel.setText ("Drive", juce::dontSendNotification);
    driveLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (driveLabel);

    driveKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    driveKnob.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 20);
    driveKnob.setRange (0.0, 100.0, 0.1);
    driveKnob.setTextValueSuffix (" %");
    driveKnob.setDoubleClickReturnValue (true, 0.0);
    driveKnob.setValue (0.0, juce::dontSendNotification);
    addAndMakeVisible (driveKnob);

    if (processor.apvts.getParameter (kDriveParamID) != nullptr)
    {
        driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, kDriveParamID, driveKnob);
    }

    setSize (260, 220);
}

void PluginFactorySkeletonAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (20);
    driveLabel.setBounds (bounds.removeFromTop (24));
    bounds.removeFromTop (8);
    driveKnob.setBounds (bounds);
}
