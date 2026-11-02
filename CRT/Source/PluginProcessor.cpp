#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <cmath>

namespace
{
constexpr float pi = 3.14159265358979323846f;
float softClip(float x) { return std::tanh(x); }
}

SMProductionCRTProcessor::SMProductionCRTProcessor()
 : AudioProcessor(BusesProperties().withInput("Input", juce::AudioChannelSet::stereo(), true)
                                  .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
   parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

SMProductionCRTProcessor::APVTS::ParameterLayout SMProductionCRTProcessor::createParameterLayout()
{
    APVTS::ParameterLayout p;
    auto knob = [&p](const char* id, const char* name, float lo, float hi, float def, float step = 0.0f)
    {
        p.add(std::make_unique<juce::AudioParameterFloat>(id, name,
            juce::NormalisableRange<float>(lo, hi, step), def));
    };
    knob("DEPTH", "Depth", 0.0f, 1.0f, 0.35f);
    knob("RATE", "Rate", 0.05f, 8.0f, 0.8f);
    knob("NOISE", "Noise", 0.0f, 1.0f, 0.08f);
    knob("FILTER", "Filter", 0.0f, 1.0f, 0.65f);
    knob("CRUSH", "Crush", 0.0f, 1.0f, 0.12f);
    knob("MIX", "Mix", 0.0f, 1.0f, 1.0f);
    p.add(std::make_unique<juce::AudioParameterBool>("STEREO", "Stereo", true));
    p.add(std::make_unique<juce::AudioParameterBool>("TAPE", "Tape Wear", true));
    p.add(std::make_unique<juce::AudioParameterBool>("POWER", "Power", true));
    return p;
}

float SMProductionCRTProcessor::get(const char* id) const
{
    if (auto* v = parameters.getRawParameterValue(id)) return v->load();
    return 0.0f;
}

void SMProductionCRTProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    sr = sampleRate;
    delayBuffer.setSize(2, juce::jmax(8, (int)(sr * 0.08) + samplesPerBlock + 8));
    delayBuffer.clear();
    writePos = 0;
    phase = wowPhase = flutterPhase = env = 0.0f;
    crushCounter = 0;
    crushHoldL = crushHoldR = lpStateL = lpStateR = 0.0f;
    noiseStateL = noiseStateR = 0.0f;
}

void SMProductionCRTProcessor::releaseResources() {}

bool SMProductionCRTProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void SMProductionCRTProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int channels = buffer.getNumChannels();
    if (channels == 0) return;

    const bool powered = get("POWER") > 0.5f;
    const float depth = get("DEPTH");
    const float rate = get("RATE");
    const float noiseAmt = get("NOISE");
    const float filterAmt = get("FILTER");
    const float crush = get("CRUSH");
    const float mix = get("MIX");
    const bool stereo = get("STEREO") > 0.5f;
    const bool tape = get("TAPE") > 0.5f;
    if (!powered) return;

    const float dryGain = std::cos(mix * pi * 0.5f);
    const float wetGain = std::sin(mix * pi * 0.5f);
    const float crushRate = 1.0f + crush * 31.0f;
    const float bitSteps = std::pow(2.0f, 16.0f - crush * 13.0f);
    const float envAttack = std::exp(-1.0f / (0.012f * (float)sr));
    const float envRelease = std::exp(-1.0f / (0.22f * (float)sr));
    const float baseCut = 18000.0f * std::pow(350.0f / 18000.0f, filterAmt * 0.82f);
    const float noiseCoeff = 0.015f + noiseAmt * 0.18f;
    const float depthMs = 0.7f + depth * 11.0f;

    auto* left = buffer.getWritePointer(0);
    auto* right = channels > 1 ? buffer.getWritePointer(1) : nullptr;
    for (int i = 0; i < n; ++i)
    {
        const float inL = left[i];
        const float inR = right ? right[i] : inL;
        const float monoEnvIn = 0.5f * (std::abs(inL) + std::abs(inR));
        const float coeff = monoEnvIn > env ? envAttack : envRelease;
        env = coeff * env + (1.0f - coeff) * monoEnvIn;

        phase += (2.0f * pi * rate) / (float)sr;
        wowPhase += (2.0f * pi * (0.12f + rate * 0.035f)) / (float)sr;
        flutterPhase += (2.0f * pi * (5.1f + rate * 0.4f)) / (float)sr;
        if (phase > 2.0f * pi) phase -= 2.0f * pi;
        if (wowPhase > 2.0f * pi) wowPhase -= 2.0f * pi;
        if (flutterPhase > 2.0f * pi) flutterPhase -= 2.0f * pi;

        const float wow = tape ? (std::sin(wowPhase) * 0.72f + std::sin(flutterPhase) * 0.16f) : 0.0f;
        const float modL = std::sin(phase + (stereo ? 0.0f : 0.15f));
        const float modR = std::sin(phase + (stereo ? 1.15f : 0.15f));
        const float modMsL = depthMs * (0.52f + 0.48f * modL) + wow * depth * 2.8f;
        const float modMsR = depthMs * (0.52f + 0.48f * modR) + wow * depth * 2.8f;
        const int maxDelay = delayBuffer.getNumSamples() - 2;
        const float dL = juce::jlimit(1.0f, (float)maxDelay, modMsL * (float)sr * 0.001f);
        const float dR = juce::jlimit(1.0f, (float)maxDelay, modMsR * (float)sr * 0.001f);
        const int rL = (writePos - (int)dL + maxDelay) % maxDelay;
        const int rR = (writePos - (int)dR + maxDelay) % maxDelay;
        const float delayedL = delayBuffer.getSample(0, rL);
        const float delayedR = delayBuffer.getSample(1, rR);
        delayBuffer.setSample(0, writePos, inL);
        delayBuffer.setSample(1, writePos, inR);
        if (++writePos >= maxDelay) writePos = 0;

        float wetL = inL * (1.0f - depth * 0.32f) + delayedL * depth * 0.32f;
        float wetR = inR * (1.0f - depth * 0.32f) + delayedR * depth * 0.32f;

        const float dynamicCut = juce::jlimit(250.0f, 20000.0f, baseCut * (0.58f + 1.65f * env));
        const float alpha = 1.0f - std::exp(-2.0f * pi * dynamicCut / (float)sr);
        lpStateL += alpha * (wetL - lpStateL);
        lpStateR += alpha * (wetR - lpStateR);
        wetL = wetL * (1.0f - filterAmt * 0.82f) + lpStateL * filterAmt * 0.82f;
        wetR = wetR * (1.0f - filterAmt * 0.82f) + lpStateR * filterAmt * 0.82f;

        if (crush > 0.001f)
        {
            if ((crushCounter++ % (int)crushRate) == 0)
            {
                crushHoldL = std::round(wetL * bitSteps) / bitSteps;
                crushHoldR = std::round(wetR * bitSteps) / bitSteps;
            }
            wetL = crushHoldL;
            wetR = crushHoldR;
        }

        if (noiseAmt > 0.0001f)
        {
            const float whiteL = random.nextFloat() * 2.0f - 1.0f;
            const float whiteR = random.nextFloat() * 2.0f - 1.0f;
            noiseStateL = 0.82f * noiseStateL + 0.18f * whiteL;
            noiseStateR = 0.82f * noiseStateR + 0.18f * whiteR;
            wetL += (whiteL * 0.68f + noiseStateL * 0.32f) * noiseCoeff;
            wetR += (whiteR * 0.68f + noiseStateR * 0.32f) * noiseCoeff;
        }

        if (tape)
        {
            wetL = softClip(wetL * (1.0f + depth * 0.8f)) * (1.0f - depth * 0.07f);
            wetR = softClip(wetR * (1.0f + depth * 0.8f)) * (1.0f - depth * 0.07f);
            const float wear = 1.0f - depth * 0.035f * (0.5f + 0.5f * std::sin(wowPhase * 0.37f));
            wetL *= wear; wetR *= wear;
        }

        if (stereo && right)
        {
            const float mid = 0.5f * (wetL + wetR);
            const float side = 0.5f * (wetL - wetR) * (1.0f + depth * 0.45f);
            wetL = mid + side;
            wetR = mid - side;
        }

        left[i] = dryGain * inL + wetGain * wetL;
        if (right) right[i] = dryGain * inR + wetGain * wetR;
    }
}

juce::AudioProcessorEditor* SMProductionCRTProcessor::createEditor()
{
    return new SMProductionCRTEditor(*this);
}

void SMProductionCRTProcessor::getStateInformation(juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, dest);
}

void SMProductionCRTProcessor::setStateInformation(const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, size));
    if (xml && xml->hasTagName(parameters.state.getType()))
        parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SMProductionCRTProcessor();
}
