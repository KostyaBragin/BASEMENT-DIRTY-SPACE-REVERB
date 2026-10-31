#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
class BasementAudioProcessorEditor:public juce::AudioProcessorEditor{
public: explicit BasementAudioProcessorEditor(BasementAudioProcessor&); void paint(juce::Graphics&)override; void resized()override;
private: BasementAudioProcessor& proc; juce::Image backgroundImage; std::array<std::unique_ptr<juce::Slider>,9> knobs;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,9> links; juce::ComboBox preset;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasementAudioProcessorEditor)
};
