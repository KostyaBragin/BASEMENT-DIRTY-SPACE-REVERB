#include "PluginProcessor.h"
#include "PluginEditor.h"

BasementAudioProcessor::BasementAudioProcessor()
    : juce::AudioProcessor(juce::AudioProcessor::BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true))
{
}

void BasementAudioProcessor::prepareToPlay(double, int) {}
void BasementAudioProcessor::releaseResources() {}

void BasementAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());
}

juce::AudioProcessorEditor* BasementAudioProcessor::createEditor()
{
    return new BasementAudioProcessorEditor(*this);
}

void BasementAudioProcessor::getStateInformation(juce::MemoryBlock&) {}
void BasementAudioProcessor::setStateInformation(const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasementAudioProcessor();
}