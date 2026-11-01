#include "PluginProcessor.h"
#include "PluginEditor.h"

static juce::AudioProcessorValueTreeState::ParameterLayout makeParams()
{
    juce::AudioProcessorValueTreeState::ParameterLayout p;
    auto add = [&](const char* id, const char* name, float lo, float hi, float def)
    {
        p.add(std::make_unique<juce::AudioParameterFloat>(
            id, name, juce::NormalisableRange<float>(lo, hi, 0.01f), def));
    };
    add("glue", "GLUE", 0, 100, 35);
    add("focus", "FOCUS", 0, 100, 55);
    add("space", "SPACE", 0, 100, 20);
    add("time", "TIME", 0, 100, 45);
    add("mix", "MIX", 0, 100, 100);
    add("output", "OUTPUT", -12, 12, 0);
    return p;
}

SMProductionGlueAudioProcessor::SMProductionGlueAudioProcessor()
    : AudioProcessor(BusesProperties()
        .withInput("Vocal", juce::AudioChannelSet::stereo(), true)
        .withInput("Instrumental Sidechain", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      state(*this, nullptr, "PARAMETERS", makeParams())
{
}

bool SMProductionGlueAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto main = layouts.getMainInputChannelSet();
    const auto output = layouts.getMainOutputChannelSet();
    return (main == juce::AudioChannelSet::mono() || main == juce::AudioChannelSet::stereo())
        && output == main;
}

void SMProductionGlueAudioProcessor::prepareToPlay(double sampleRate, int)
{
    sr = sampleRate;
    const float centres[6] = { 140.0f, 350.0f, 800.0f, 1800.0f, 4200.0f, 9000.0f };
    for (int i = 0; i < 6; ++i)
    {
        auto coeff = juce::IIRCoefficients::makeBandPass(sr, centres[i], 0.8f);
        auto& band = bands[(size_t)i];
        band.mainL.setCoefficients(coeff); band.mainR.setCoefficients(coeff);
        band.scL.setCoefficients(coeff); band.scR.setCoefficients(coeff);
        band.mainEnv = band.scEnv = 0.0f;
        band.gain = 1.0f;
    }
}

void SMProductionGlueAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto mainInput = getBusBuffer(buffer, true, 0);
    auto outputBus = getBusBuffer(buffer, false, 0);
    auto sidechain = getBusBuffer(buffer, true, 1);
    const int samples = outputBus.getNumSamples();
    const int channels = outputBus.getNumChannels();
    const int sidechainChannels = sidechain.getNumChannels();

    const float amount = state.getRawParameterValue("glue")->load() * 0.01f;
    const float focus = state.getRawParameterValue("focus")->load() * 0.01f;
    const float space = state.getRawParameterValue("space")->load() * 0.01f;
    const float time = state.getRawParameterValue("time")->load() * 0.01f;
    const float wet = state.getRawParameterValue("mix")->load() * 0.01f;
    const float outputGain = juce::Decibels::decibelsToGain(state.getRawParameterValue("output")->load());

    const float attack = std::exp(-1.0f / ((0.008f + time * 0.045f) * (float)sr));
    const float release = std::exp(-1.0f / ((0.12f + time * 0.55f) * (float)sr));
    const float gainSmoothing = std::exp(-1.0f / (0.025f * (float)sr));

    for (int i = 0; i < samples; ++i)
    {
        float sc[2] = { 0.0f, 0.0f };
        if (sidechainChannels > 0) sc[0] = sidechain.getSample(0, i);
        if (sidechainChannels > 1) sc[1] = sidechain.getSample(1, i);
        else sc[1] = sc[0];

        const float dry[2] = { mainInput.getNumChannels() > 0 ? mainInput.getSample(0, i) : 0.0f,
            mainInput.getNumChannels() > 1 ? mainInput.getSample(1, i)
                : (mainInput.getNumChannels() > 0 ? mainInput.getSample(0, i) : 0.0f) };
        float processed[2] = { dry[0], dry[1] };

        // Six perceptual detector bands. Their filtered components are mixed back
        // in parallel, so the sidechain shapes spectral regions rather than applying
        // six successive broadband gain reductions.
        for (int k = 0; k < 6; ++k)
        {
            auto& band = bands[(size_t)k];
            float mainBand[2], scBand[2];
            mainBand[0] = band.mainL.processSingleSampleRaw(dry[0]);
            mainBand[1] = band.mainR.processSingleSampleRaw(dry[1]);
            scBand[0] = band.scL.processSingleSampleRaw(sc[0]);
            scBand[1] = band.scR.processSingleSampleRaw(sc[1]);

            const float mainLevel = std::sqrt(0.5f * (mainBand[0] * mainBand[0]
                + mainBand[1] * mainBand[1]) + 1.0e-12f);
            const float scLevel = std::sqrt(0.5f * (scBand[0] * scBand[0]
                + scBand[1] * scBand[1]) + 1.0e-12f);
            const float mainCoeff = mainLevel > band.mainEnv ? attack : release;
            const float scCoeff = scLevel > band.scEnv ? attack : release;
            band.mainEnv = mainCoeff * band.mainEnv + (1.0f - mainCoeff) * mainLevel;
            band.scEnv = scCoeff * band.scEnv + (1.0f - scCoeff) * scLevel;

            const float dominance = band.scEnv / (band.scEnv + band.mainEnv + 1.0e-5f);
            const float target = 1.0f - amount * focus * 0.20f * dominance;
            band.gain = gainSmoothing * band.gain + (1.0f - gainSmoothing) * target;
            const float bandGain = 1.0f + (band.gain - 1.0f) * (0.35f + 0.65f * focus);
            processed[0] += (bandGain - 1.0f) * mainBand[0];
            processed[1] += (bandGain - 1.0f) * mainBand[1];
        }

        const float mid = 0.5f * (processed[0] + processed[1]);
        const float side = 0.5f * (processed[0] - processed[1]);
        const float width = 1.0f - space * amount * 0.10f;
        processed[0] = mid + side * width;
        processed[1] = mid - side * width;

        for (int ch = 0; ch < channels; ++ch)
        {
            const float wetSample = ch == 0 ? processed[0] : processed[1];
            outputBus.setSample(ch, i, (dry[ch] * (1.0f - wet) + wetSample * wet) * outputGain);
        }
    }
}

juce::AudioProcessorEditor* SMProductionGlueAudioProcessor::createEditor()
{
    return new SMProductionGlueAudioProcessorEditor(*this);
}

void SMProductionGlueAudioProcessor::getStateInformation(juce::MemoryBlock& destination)
{
    auto copy = state.copyState();
    std::unique_ptr<juce::XmlElement> xml(copy.createXml());
    copyXmlToBinary(*xml, destination);
}

void SMProductionGlueAudioProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(state.state.getType()))
        state.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SMProductionGlueAudioProcessor();
}
