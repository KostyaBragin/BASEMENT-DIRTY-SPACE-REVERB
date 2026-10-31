#pragma once
#include <JuceHeader.h>

class BasementAudioProcessor : public juce::AudioProcessor
{
public:
    using APVTS = juce::AudioProcessorValueTreeState;

    BasementAudioProcessor();
    ~BasementAudioProcessor() override = default;

    void prepareToPlay(double, int) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "BASEMENT"; }

    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 5; }
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    APVTS parameters;
    void loadPreset(int);

private:
    juce::dsp::Reverb reverb;
    juce::IIRFilter lowCutFilters[2];
    juce::IIRFilter highCutFilters[2];
    float lastLowCutHz = -1.0f;
    float lastHighCutHz = -1.0f;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> preDelayLine;
    juce::dsp::Reverb::Parameters reverbParameters;
    juce::AudioBuffer<float> dryBuffer;
    int currentProgram = 0;
    static APVTS::ParameterLayout layout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BasementAudioProcessor)
};
