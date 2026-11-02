#include "PluginEditor.h"

#include "PluginEditor.h"

SMProductionCRTEditor::SMProductionCRTEditor(SMProductionCRTProcessor& p)
 : AudioProcessorEditor(&p), processor(p)
{
    setSize(900, 590);

    const char* ids[] = { "DEPTH", "RATE", "NOISE", "FILTER", "CRUSH", "MIX" };
    const char* names[] = { "DEPTH", "RATE", "NOISE", "FILTER", "CRUSH", "MIX" };
    for (int i = 0; i < 6; ++i)
    {
        auto* s = sliders.add(new juce::Slider());
        s->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 72, 18);
        s->setRotaryParameters(juce::MathConstants<float>::pi * 1.25f,
                               juce::MathConstants<float>::pi * 2.75f, true);
        s->setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffd5a45b));
        s->setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff555b64));
        s->setColour(juce::Slider::thumbColourId, juce::Colour(0xffffe0a3));
        s->setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xffe8e3d8));
        s->setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff171a20));
        s->setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff454b54));
        addAndMakeVisible(s);

        auto* l = labels.add(new juce::Label());
        l->setText(names[i], juce::dontSendNotification);
        l->setJustificationType(juce::Justification::centred);
        l->setColour(juce::Label::textColourId, juce::Colour(0xffe8e3d8));
        l->setFont(juce::Font(12.0f, juce::Font::bold));
        addAndMakeVisible(l);

        attachments.add(new juce::AudioProcessorValueTreeState::SliderAttachment(processor.parameters, ids[i], *s));
    }

    stereo.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe8e3d8));
    tape.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe8e3d8));
    power.setColour(juce::ToggleButton::textColourId, juce::Colour(0xffe8e3d8));
    stereo.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffd5a45b));
    tape.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffd5a45b));
    power.setColour(juce::ToggleButton::tickColourId, juce::Colour(0xffd5a45b));
    addAndMakeVisible(stereo);
    addAndMakeVisible(tape);
    addAndMakeVisible(power);

    stereoAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "STEREO", stereo);
    tapeAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "TAPE", tape);
    powerAttach = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(processor.parameters, "POWER", power);

    resized();
}

void SMProductionCRTEditor::paint(juce::Graphics& g)
{
    const auto bounds = getLocalBounds().toFloat();
    g.fillAll(juce::Colour(0xff111317));
    g.setColour(juce::Colour(0xff252a30));
    g.fillRoundedRectangle(bounds.reduced(10.0f), 18.0f);

    // Reimagined CRT television screen (original SM Production artwork).
    auto screen = juce::Rectangle<float>(34.0f, 28.0f, 832.0f, 318.0f);
    g.setColour(juce::Colour(0xff090b12));
    g.fillRoundedRectangle(screen.expanded(9.0f), 20.0f);
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xff1737b8), screen.getX(), screen.getY(),
                                            juce::Colour(0xff07134c), screen.getRight(), screen.getBottom(), false));
    g.fillRoundedRectangle(screen, 15.0f);
    g.setColour(juce::Colour(0x22b8d5ff));
    for (int y = 42; y < 338; y += 5)
        g.drawHorizontalLine(y, screen.getX() + 12.0f, screen.getRight() - 12.0f);
    g.setColour(juce::Colour(0x55d9e7ff));
    g.drawHorizontalLine(188, screen.getX() + 8.0f, screen.getRight() - 8.0f);
    g.setColour(juce::Colour(0xffe4e9f5));
    g.setFont(juce::Font(18.0f, juce::Font::bold));
    g.drawText("SM PRODUCTION  /  CRT", 62, 52, 360, 28, juce::Justification::left);
    g.setFont(juce::Font(12.0f));
    g.drawText("ANALOG  +  DIGITAL CHARACTER", 64, 82, 320, 20, juce::Justification::left);
    g.setColour(juce::Colour(0x99ffffff));
    g.drawEllipse(443.0f, 181.0f, 14.0f, 14.0f, 1.5f);

    g.setColour(juce::Colour(0xff343940));
    g.fillRoundedRectangle(34.0f, 365.0f, 832.0f, 42.0f, 7.0f);
    g.setColour(juce::Colour(0xff565d66));
    g.drawRoundedRectangle(34.0f, 365.0f, 832.0f, 42.0f, 7.0f, 1.0f);
    g.setColour(juce::Colour(0xffd5a45b));
    g.setFont(juce::Font(11.0f, juce::Font::bold));
    g.drawText("SM  /  PRODUCTION", 42, 555, 220, 18, juce::Justification::left);
}

void SMProductionCRTEditor::resized()
{
    stereo.setBounds(82, 372, 150, 28);
    tape.setBounds(300, 372, 180, 28);
    power.setBounds(34, 474, 90, 30);

    const int x0 = 135;
    const int width = 112;
    const int gap = 10;
    for (int i = 0; i < sliders.size(); ++i)
    {
        const int x = x0 + i * (width + gap);
        labels[i]->setBounds(x, 414, width, 22);
        sliders[i]->setBounds(x, 435, width, 112);
    }
}
