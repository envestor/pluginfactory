#include "PluginEditor.h"

namespace
{
    // Parameter is referenced by ID only; owned and declared by PLU-45 / M1-A.
    constexpr const char* kDriveParamID = "drive";

    const juce::Colour kBackgroundColour { 0xff15161b };
    const juce::Colour kPanelColour      { 0xff1e2027 };
    const juce::Colour kTrackColour      { 0xff34373f };
    const juce::Colour kAccentColour     { 0xffe8793a };
    const juce::Colour kTextColour       { 0xfff0f0f2 };
    const juce::Colour kMutedTextColour  { 0xff9a9ca6 };
}

DriveLookAndFeel::DriveLookAndFeel()
{
    setColour (juce::Label::textColourId, kTextColour);
    setColour (juce::Slider::rotarySliderFillColourId, kAccentColour);
    setColour (juce::Slider::rotarySliderOutlineColourId, kTrackColour);
}

void DriveLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                          float sliderPosProportional, float rotaryStartAngle,
                                          float rotaryEndAngle, juce::Slider&)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (6.0f);
    const auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
    const auto centre = bounds.getCentre();
    const auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    constexpr float trackThickness = 4.0f;
    const auto arcRadius = radius - trackThickness;

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                 rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (kTrackColour);
    g.strokePath (backgroundArc, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                                        juce::PathStrokeType::rounded));

    juce::Path valueArc;
    valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                            rotaryStartAngle, angle, true);
    g.setColour (kAccentColour);
    g.strokePath (valueArc, juce::PathStrokeType (trackThickness, juce::PathStrokeType::curved,
                                                   juce::PathStrokeType::rounded));

    const auto knobRadius = arcRadius - trackThickness * 2.5f;
    g.setColour (kPanelColour);
    g.fillEllipse (centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);
    g.setColour (kTrackColour);
    g.drawEllipse (centre.x - knobRadius, centre.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f, 1.0f);

    juce::Path pointer;
    pointer.startNewSubPath (0.0f, -knobRadius * 0.15f);
    pointer.lineTo (0.0f, -knobRadius * 0.85f);
    pointer.applyTransform (juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
    g.setColour (kAccentColour);
    g.strokePath (pointer, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
}

PluginFactorySkeletonAudioProcessorEditor::PluginFactorySkeletonAudioProcessorEditor (PluginFactorySkeletonAudioProcessor& p)
    : juce::AudioProcessorEditor (&p), processor (p)
{
    setLookAndFeel (&lookAndFeel);

    pluginNameLabel.setText (processor.getName(), juce::dontSendNotification);
    pluginNameLabel.setJustificationType (juce::Justification::centred);
    pluginNameLabel.setFont (juce::Font (juce::FontOptions (19.0f)));
    pluginNameLabel.setColour (juce::Label::textColourId, kTextColour);
    addAndMakeVisible (pluginNameLabel);

    paramNameLabel.setText ("DRIVE", juce::dontSendNotification);
    paramNameLabel.setJustificationType (juce::Justification::centred);
    paramNameLabel.setFont (juce::Font (juce::FontOptions (13.0f)));
    paramNameLabel.setColour (juce::Label::textColourId, kMutedTextColour);
    addAndMakeVisible (paramNameLabel);

    driveKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    driveKnob.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    driveKnob.setRange (0.0, 100.0, 0.1);
    driveKnob.setDoubleClickReturnValue (true, 0.0);
    driveKnob.setValue (0.0, juce::dontSendNotification);
    driveKnob.addListener (this);
    addAndMakeVisible (driveKnob);

    valueLabel.setJustificationType (juce::Justification::centred);
    valueLabel.setFont (juce::Font (juce::FontOptions (15.0f)));
    valueLabel.setColour (juce::Label::textColourId, kTextColour);
    addAndMakeVisible (valueLabel);

    if (processor.apvts.getParameter (kDriveParamID) != nullptr)
    {
        driveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            processor.apvts, kDriveParamID, driveKnob);
    }

    updateValueLabel();
    setSize (280, 368);
}

PluginFactorySkeletonAudioProcessorEditor::~PluginFactorySkeletonAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void PluginFactorySkeletonAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (kBackgroundColour);

    auto panel = getLocalBounds().toFloat().reduced (10.0f);
    g.setColour (kPanelColour);
    g.fillRoundedRectangle (panel, 12.0f);
}

void PluginFactorySkeletonAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (24);

    pluginNameLabel.setBounds (bounds.removeFromTop (28));
    bounds.removeFromTop (8);
    paramNameLabel.setBounds (bounds.removeFromTop (18));
    bounds.removeFromTop (4);

    auto knobArea = bounds.removeFromTop (bounds.getWidth());
    driveKnob.setBounds (knobArea.reduced (8));

    bounds.removeFromTop (4);
    valueLabel.setBounds (bounds.removeFromTop (26));
}

void PluginFactorySkeletonAudioProcessorEditor::sliderValueChanged (juce::Slider*)
{
    updateValueLabel();
}

void PluginFactorySkeletonAudioProcessorEditor::updateValueLabel()
{
    valueLabel.setText (juce::String (driveKnob.getValue(), 1) + " %", juce::dontSendNotification);
}
