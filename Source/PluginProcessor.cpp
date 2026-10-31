#include "PluginProcessor.h"
#include "PluginEditor.h"

static std::unique_ptr<juce::RangedAudioParameter> makeFloatParameter(
    const char* id, const char* name, float minValue, float maxValue,
    float defaultValue, const char* unit = "")
{
    return std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { id, 1 }, name,
        juce::NormalisableRange<float>(minValue, maxValue),
        defaultValue, unit);
}

BasementAudioProcessor::APVTS::ParameterLayout BasementAudioProcessor::layout()
{
    APVTS::ParameterLayout p;
    p.add(makeFloatParameter("decay", "DECAY", 0.1f, 12.0f, 2.8f, " s"));
    p.add(makeFloatParameter("predelay", "PRE-DELAY", 0.0f, 200.0f, 24.0f, " ms"));
    p.add(makeFloatParameter("size", "SIZE", 0.0f, 100.0f, 62.0f, "%"));
    p.add(makeFloatParameter("mix", "MIX", 0.0f, 100.0f, 25.0f, "%"));
    p.add(makeFloatParameter("damping", "DAMPING", 0.0f, 100.0f, 58.0f, "%"));
    p.add(makeFloatParameter("width", "WIDTH", 0.0f, 200.0f, 115.0f, "%"));
    p.add(makeFloatParameter("dirt", "DIRT", 0.0f, 100.0f, 32.0f, "%"));
    p.add(makeFloatParameter("lowcut", "LOW CUT", 20.0f, 1000.0f, 140.0f, " Hz"));
    p.add(makeFloatParameter("highcut", "HIGH CUT", 1000.0f, 20000.0f, 8500.0f, " Hz"));
    p.add(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID { "freeze", 1 }, "FREEZE", false));
    return p;
}

BasementAudioProcessor::BasementAudioProcessor()
    : juce::AudioProcessor(juce::AudioProcessor::BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "BASEMENT", layout())
{
}

void BasementAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32>(samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32>(getTotalNumOutputChannels());

    preDelayLine.setMaximumDelayInSamples(
        juce::jmax(1, static_cast<int>(sampleRate * 0.2) + 1));
    preDelayLine.prepare(spec);
    preDelayLine.reset();

    reverb.prepare(spec);
    reverb.reset();
    for (int channel = 0; channel < 2; ++channel)
    {
        lowCutFilters[channel].reset();
        highCutFilters[channel].reset();
    }
    lastLowCutHz = -1.0f;
    lastHighCutHz = -1.0f;
    dryBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
}

void BasementAudioProcessor::releaseResources()
{
    preDelayLine.reset();
    reverb.reset();
    for (int channel = 0; channel < 2; ++channel)
    {
        lowCutFilters[channel].reset();
        highCutFilters[channel].reset();
    }
}

void BasementAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples = buffer.getNumSamples();
    const int numChannels = juce::jmin(buffer.getNumChannels(), getTotalNumInputChannels());

    for (int channel = getTotalNumInputChannels(); channel < getTotalNumOutputChannels(); ++channel)
        buffer.clear(channel, 0, numSamples);

    if (numChannels == 0)
        return;

    if (dryBuffer.getNumChannels() != buffer.getNumChannels()
        || dryBuffer.getNumSamples() < numSamples)
        dryBuffer.setSize(buffer.getNumChannels(), numSamples, false, false, true);

    for (int channel = 0; channel < numChannels; ++channel)
        dryBuffer.copyFrom(channel, 0, buffer, channel, 0, numSamples);

    const float decay = parameters.getRawParameterValue("decay")->load();
    const float preDelayMs = parameters.getRawParameterValue("predelay")->load();
    const float size = parameters.getRawParameterValue("size")->load() * 0.01f;
    const float damping = parameters.getRawParameterValue("damping")->load() * 0.01f;
    const float width = parameters.getRawParameterValue("width")->load() * 0.005f;
    const float mix = parameters.getRawParameterValue("mix")->load() * 0.01f;
    const float lowCutHz = parameters.getRawParameterValue("lowcut")->load();
    const float highCutHz = parameters.getRawParameterValue("highcut")->load();
    const float dirt = parameters.getRawParameterValue("dirt")->load() * 0.01f;
    const bool freeze = parameters.getRawParameterValue("freeze")->load() >= 0.5f;

    preDelayLine.setDelay(preDelayMs * static_cast<float>(getSampleRate()) * 0.001f);

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* wetInput = buffer.getWritePointer(channel);
        for (int sample = 0; sample < numSamples; ++sample)
        {
            preDelayLine.pushSample(channel, wetInput[sample]);
            wetInput[sample] = preDelayLine.popSample(channel);
        }
    }

    reverbParameters.roomSize = juce::jlimit(0.05f, 0.98f,
        0.18f + size * 0.48f + (decay / 12.0f) * 0.30f);
    reverbParameters.damping = juce::jlimit(0.0f, 1.0f, damping);
    reverbParameters.width = juce::jlimit(0.0f, 1.0f, width);
    reverbParameters.wetLevel = 1.0f;
    reverbParameters.dryLevel = 0.0f;
    reverbParameters.freezeMode = freeze ? 1.0f : 0.0f;
    reverb.setParameters(reverbParameters);

    juce::dsp::AudioBlock<float> audioBlock(buffer);
    reverb.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));

    const float safeLowCutHz = juce::jlimit(20.0f,
        static_cast<float>(getSampleRate() * 0.45), lowCutHz);
    const float safeHighCutHz = juce::jlimit(1000.0f,
        static_cast<float>(getSampleRate() * 0.45), highCutHz);

    if (safeLowCutHz != lastLowCutHz)
    {
        const auto coefficients = juce::IIRCoefficients::makeHighPass(getSampleRate(), safeLowCutHz);
        for (int channel = 0; channel < numChannels; ++channel)
            lowCutFilters[channel].setCoefficients(coefficients);
        lastLowCutHz = safeLowCutHz;
    }

    if (safeHighCutHz != lastHighCutHz)
    {
        const auto coefficients = juce::IIRCoefficients::makeLowPass(getSampleRate(), safeHighCutHz);
        for (int channel = 0; channel < numChannels; ++channel)
            highCutFilters[channel].setCoefficients(coefficients);
        lastHighCutHz = safeHighCutHz;
    }

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* wet = buffer.getWritePointer(channel);
        lowCutFilters[channel].processSamples(wet, numSamples);
        highCutFilters[channel].processSamples(wet, numSamples);

        // Gentle, controllable saturation on the reverb signal only.
        if (dirt > 0.0f)
        {
            const float drive = 1.0f + dirt * 4.0f;
            const float blend = dirt * 0.85f;
            const float normalizer = std::tanh(drive);
            for (int sample = 0; sample < numSamples; ++sample)
            {
                const float dryWetSample = wet[sample];
                const float saturated = std::tanh(dryWetSample * drive) / normalizer;
                wet[sample] = dryWetSample + (saturated - dryWetSample) * blend;
            }
        }
    }

    for (int channel = 0; channel < numChannels; ++channel)
    {
        auto* output = buffer.getWritePointer(channel);
        const auto* dry = dryBuffer.getReadPointer(channel);

        for (int sample = 0; sample < numSamples; ++sample)
            output[sample] = dry[sample] * (1.0f - mix) + output[sample] * mix;
    }
}

void BasementAudioProcessor::loadPreset(int index)
{
    struct PresetValues { float decay, predelay, size, mix, damping, width, dirt, lowcut, highcut; };
    static constexpr PresetValues presets[] = {
        { 2.8f, 24.0f, 62.0f, 25.0f, 58.0f, 115.0f, 32.0f, 140.0f, 8500.0f },
        { 1.1f, 8.0f, 28.0f, 22.0f, 72.0f, 80.0f, 45.0f, 180.0f, 7200.0f },
        { 3.8f, 18.0f, 55.0f, 28.0f, 48.0f, 105.0f, 55.0f, 110.0f, 6500.0f },
        { 7.5f, 42.0f, 92.0f, 32.0f, 62.0f, 130.0f, 38.0f, 90.0f, 9500.0f },
        { 2.2f, 12.0f, 48.0f, 30.0f, 35.0f, 145.0f, 68.0f, 160.0f, 11000.0f }
    };

    const auto& v = presets[juce::jlimit(0, 4, index)];
    const char* ids[] = { "decay", "predelay", "size", "mix", "damping", "width", "dirt", "lowcut", "highcut" };
    const float values[] = { v.decay, v.predelay, v.size, v.mix, v.damping, v.width, v.dirt, v.lowcut, v.highcut };

    for (int i = 0; i < 9; ++i)
        if (auto* parameter = parameters.getParameter(ids[i]))
            parameter->setValueNotifyingHost(parameter->convertTo0to1(values[i]));

    currentProgram = juce::jlimit(0, 4, index);
}

void BasementAudioProcessor::setCurrentProgram(int index) { loadPreset(index); }

const juce::String BasementAudioProcessor::getProgramName(int index)
{
    const char* names[] = { "BASEMENT", "GARAGE", "BUNKER", "ABANDONED FACTORY", "DIRTY SPRING" };
    return names[juce::jlimit(0, 4, index)];
}

void BasementAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    std::unique_ptr<juce::XmlElement> xml(parameters.copyState().createXml());
    copyXmlToBinary(*xml, destData);
}

void BasementAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* BasementAudioProcessor::createEditor()
{
    return new BasementAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new BasementAudioProcessor();
}
