#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <array>
#include <memory>

class BasementAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit BasementAudioProcessorEditor(BasementAudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    BasementAudioProcessor& audioProcessor;

    juce::Label title;
    juce::Label presetLabel;
    juce::ComboBox presetSelector;
    juce::ToggleButton freezeButton;

    std::array<juce::Slider, 9> sliders;
    std::array<juce::Label, 9> labels;
    std::array<std::unique_ptr<BasementAudioProcessor::APVTS::SliderAttachment>, 9> sliderAttachments;
    std::unique_ptr<BasementAudioProcessor::APVTS::ButtonAttachment> freezeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasementAudioProcessorEditor)
};
