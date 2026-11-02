#include "PluginEditor.h"

SMProductionCRTEditor::SMProductionCRTEditor(SMProductionCRTProcessor& p)
 : AudioProcessorEditor(&p), processor(p)
{
    setSize(760, 430);
    const char* ids[] = { "DEPTH", "RATE", "NOISE", "FILTER", "CRUSH", "MIX" };
    const char* names[] = { "DEPTH", "RATE", "NOISE", "FILTER", "CRUSH", "MIX" };
    for (int i = 0; i < 6; ++i)
    {
        auto* s = sliders.add(new juce::Slider());
        s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 76, 20);
        s->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd5a45b));
        s->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff343943));
        s->setColour(juce::Slider::thumbColourId, juce::Colour(0xffffe0a3));
        addAndMakeVisible(s);
        auto* l = labels.add(new juce::Label());
        l->setText(names[i], juce::dontSendNotification);
        l->setJustificationType(juce::Justification::centred);
        l->setColour(juce::Label::textColourId, juce::Colour(0xffe8e3d8));
        l->setFont(juce::Font(13.0f, juce::Font::bold));
        addAndMakeVisible(l);
        attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(processor.parameters, ids[i], *s));
    }
    stereo.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe8e3d8));
    tape.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe8e3d8));
    power.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe8e3d8));
    addAndMakeVisible(stereo); addAndMakeVisible(tape); addAndMakeVisible(power);
    stereoAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "STEREO", stereo);
    tapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "TAPE", tape);
    powerAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "POWER", power);
}

void SMProductionCRTEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff17191d));
    g.setColour(juce::Colour(0xff252a31));
    g.fillRoundedRectangle(getLocalBounds().toFloat().reduced(12.0f), 16.0f);
    g.setColour(juce::Colour(0xffd5a45b));
    g.setFont(juce::Font(25.0f, juce::Font::bold));
    g.drawText("SM PRODUCTION", 28, 20, 360, 34, juce::Justification::left);
    g.setColour(juce::Colour(0xffe8e3d8));
    g.setFont(juce::Font(14.0f));
    g.drawText("CRT  /  ANALOG + DIGITAL CHARACTER", 30, 55, 430, 22, juce::Justification::left);
    g.setColour(juce::Colour(0xff444b55));
    g.drawLine(28.0f, 88.0f, 732.0f, 88.0f, 1.0f);
    g.setColour(juce::Colour(0xff8e949d));
    g.setFont(juce::Font(11.0f));
    g.drawText("DEPTH  •  RATE  •  NOISE  •  FILTER  •  CRUSH  •  MIX", 30, 390, 700, 18, juce::Justification::centred);
}

void SMProductionCRTEditor::resized()
{
    const int x0 = 34, y0 = 118, w = 112, gap = 8;
    for (int i = 0; i < sliders.size(); ++i)
    {
        const int x = x0 + i * (w + gap);
        labels[i]->setBounds(x, y0, w, 24);
        sliders[i]->setBounds(x, y0 + 24, w, 155);
    }
    stereo.setBounds(120, 320, 140, 28);
    tape.setBounds(305, 320, 160, 28);
    power.setBounds(520, 320, 130, 28);
}
