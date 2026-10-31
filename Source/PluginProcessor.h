#pragma once
#include <JuceHeader.h>
class BasementAudioProcessor : public juce::AudioProcessor {
public:
 using APVTS=juce::AudioProcessorValueTreeState;
 BasementAudioProcessor(); ~BasementAudioProcessor() override=default;
 void prepareToPlay(double,int) override; void releaseResources() override;
 void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
 juce::AudioProcessorEditor* createEditor() override; bool hasEditor() const override{return true;}
 const juce::String getName() const override{return "BASEMENT";}
 bool acceptsMidi() const override{return false;} bool producesMidi() const override{return false;}
 bool isMidiEffect() const override{return false;} double getTailLengthSeconds() const override{return 15.0;}
 int getNumPrograms() override{return 5;} int getCurrentProgram() override{return currentProgram;}
 void setCurrentProgram(int) override; const juce::String getProgramName(int) override;
 void changeProgramName(int,const juce::String&) override{}
 void getStateInformation(juce::MemoryBlock&) override; void setStateInformation(const void*,int) override;
 APVTS parameters; void loadPreset(int);
private:
 juce::dsp::Reverb reverb; juce::dsp::Reverb::Parameters rp;
 juce::dsp::DelayLine<float,juce::dsp::DelayLineInterpolationTypes::Linear> predelay{48000};
 juce::dsp::IIR::Filter<float> lowL,lowR,highL,highR;
 juce::AudioBuffer<float> dry; juce::Random rng; double sr=44100; int currentProgram=0;
 static APVTS::ParameterLayout layout();
 JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasementAudioProcessor)
};
