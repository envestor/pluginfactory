#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

// Minimal, deliberate dark theme for the rotary drive knob. Owned here for
// now; promote to the shared UI kit once a second plugin needs it.
class DriveLookAndFeel : public juce::LookAndFeel_V4
{
public:
    DriveLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override;
};

class PluginFactorySkeletonAudioProcessorEditor : public juce::AudioProcessorEditor,
                                                   private juce::Slider::Listener
{
public:
    explicit PluginFactorySkeletonAudioProcessorEditor (PluginFactorySkeletonAudioProcessor& p);
    ~PluginFactorySkeletonAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void sliderValueChanged (juce::Slider*) override;
    void updateValueLabel();

    PluginFactorySkeletonAudioProcessor& processor;

    DriveLookAndFeel lookAndFeel;

    juce::Label pluginNameLabel;
    juce::Label paramNameLabel;
    juce::Slider driveKnob;
    juce::Label valueLabel;

    // Bound to the "drive" parameter by ID only. Deliberately tolerant of
    // that parameter not existing yet (PLU-45 / M1-A owns adding it), so
    // this editor builds and opens standalone before that lands, and wires
    // up automatically once it does.
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginFactorySkeletonAudioProcessorEditor)
};
