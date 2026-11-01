#pragma once
#include "PluginProcessor.h"
class SMProductionGlueAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
 explicit SMProductionGlueAudioProcessorEditor(SMProductionGlueAudioProcessor&);
 void paint(juce::Graphics&) override; void resized() override;
private:
 SMProductionGlueAudioProcessor& proc;
 struct Knob:juce::Slider{Knob(){setSliderStyle(SliderStyle::RotaryHorizontalVerticalDrag);setTextBoxStyle(TextBoxBelow,false,64,18);setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffa67cff));setColour(juce::Slider::thumbColourId,juce::Colour(0xfff0eaff));setColour(juce::Slider::textBoxTextColourId,juce::Colours::white);}};
 std::array<Knob,6> knobs; std::array<juce::Label,6> labels;
 std::array<std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>,6> links;
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SMProductionGlueAudioProcessorEditor)
};