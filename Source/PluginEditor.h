#pragma once
#include <JuceHeader.h>

class BasementAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit BasementAudioProcessorEditor(juce::AudioProcessor&);
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    juce::Label title;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasementAudioProcessorEditor)
};