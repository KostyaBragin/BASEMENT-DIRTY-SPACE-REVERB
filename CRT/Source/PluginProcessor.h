#pragma once
#include <JuceHeader.h>

class SMProductionCRTProcessor final : public juce::AudioProcessor
{
public:
    SMProductionCRTProcessor();
    ~SMProductionCRTProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "SM Production CRT"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;
    static APVTS::ParameterLayout createParameterLayout();

private:
    double sr = 44100.0;
    juce::AudioBuffer<float> delayBuffer;
    int writePos = 0;
    float phase = 0.0f;
    float wowPhase = 0.0f;
    float flutterPhase = 0.0f;
    float crushHoldL = 0.0f, crushHoldR = 0.0f;
    int crushCounter = 0;
    float env = 0.0f;
    float lpStateL = 0.0f, lpStateR = 0.0f;
    juce::Random random;
    float noiseStateL = 0.0f, noiseStateR = 0.0f;

    float get(const char* id) const;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SMProductionCRTProcessor)
};
