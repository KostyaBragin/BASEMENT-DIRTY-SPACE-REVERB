#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class SMProductionCRTEditor final : public juce::AudioProcessorEditor
{
public:
    explicit SMProductionCRTEditor(SMProductionCRTProcessor&);
    ~SMProductionCRTEditor() override = default;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    SMProductionCRTProcessor& processor;
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> labels;
    juce::OwnedArray<juce::AudioProcessorValueTreeState::SliderAttachment> attachments;
    juce::ToggleButton stereo { "STEREO" }, tape { "TAPE WEAR" }, power { "POWER" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stereoAttach, tapeAttach, powerAttach;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SMProductionCRTEditor)
};
