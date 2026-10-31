#include "PluginEditor.h"

BasementAudioProcessorEditor::BasementAudioProcessorEditor(juce::AudioProcessor& processor)
    : juce::AudioProcessorEditor(&processor)
{
    setSize(420, 180);
    title.setText("BASEMENT - VST3 diagnostic build", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(title);
}

void BasementAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff202020));
}

void BasementAudioProcessorEditor::resized()
{
    title.setBounds(getLocalBounds().reduced(12));
}