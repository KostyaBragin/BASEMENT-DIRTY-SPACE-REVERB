#include "PluginEditor.h"

namespace
{
    const juce::Colour backgroundColour (0xff171615);
    const juce::Colour panelColour      (0xff242220);
    const juce::Colour accentColour     (0xffd94b35);
    const juce::Colour textColour       (0xffeee6d8);
    const juce::Colour mutedColour      (0xffaaa095);
}

BasementAudioProcessorEditor::BasementAudioProcessorEditor(BasementAudioProcessor& processor)
    : juce::AudioProcessorEditor(&processor), audioProcessor(processor)
{
    setSize(760, 520);
    setResizable(false, false);

    title.setText("BASEMENT", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(38.0f, juce::Font::bold)));
    title.setJustificationType(juce::Justification::centredLeft);
    title.setColour(juce::Label::textColourId, textColour);
    addAndMakeVisible(title);

    presetLabel.setText("MODE", juce::dontSendNotification);
    presetLabel.setFont(juce::Font(juce::FontOptions(12.0f, juce::Font::bold)));
    presetLabel.setJustificationType(juce::Justification::centredRight);
    presetLabel.setColour(juce::Label::textColourId, mutedColour);
    addAndMakeVisible(presetLabel);

    presetSelector.addItem("BASEMENT", 1);
    presetSelector.addItem("GARAGE", 2);
    presetSelector.addItem("BUNKER", 3);
    presetSelector.addItem("ABANDONED FACTORY", 4);
    presetSelector.addItem("DIRTY SPRING", 5);
    presetSelector.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
    presetSelector.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff302d2a));
    presetSelector.setColour(juce::ComboBox::textColourId, textColour);
    presetSelector.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff514941));
    presetSelector.setColour(juce::ComboBox::arrowColourId, accentColour);
    presetSelector.onChange = [this]
    {
        const int selected = presetSelector.getSelectedItemIndex();
        if (selected >= 0)
            audioProcessor.loadPreset(selected);
    };
    addAndMakeVisible(presetSelector);

    const char* parameterIds[] = {
        "decay", "predelay", "size",
        "mix", "damping", "width",
        "dirt", "lowcut", "highcut"
    };
    const char* parameterNames[] = {
        "DECAY", "PRE-DELAY", "SIZE",
        "MIX", "DAMPING", "WIDTH",
        "DIRT", "LOW CUT", "HIGH CUT"
    };

    for (size_t i = 0; i < sliders.size(); ++i)
    {
        auto& slider = sliders[i];
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 82, 20);
        slider.setColour(juce::Slider::textBoxTextColourId, textColour);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff252321));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff403a35));
        slider.setColour(juce::Slider::rotarySliderFillColourId, accentColour);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff514b45));
        slider.setColour(juce::Slider::thumbColourId, textColour);
        addAndMakeVisible(slider);

        labels[i].setText(parameterNames[i], juce::dontSendNotification);
        labels[i].setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
        labels[i].setJustificationType(juce::Justification::centred);
        labels[i].setColour(juce::Label::textColourId, mutedColour);
        addAndMakeVisible(labels[i]);

        sliderAttachments[i] = std::make_unique<BasementAudioProcessor::APVTS::SliderAttachment>(
            audioProcessor.parameters, parameterIds[i], slider);
    }

    freezeButton.setButtonText("FREEZE");
    freezeButton.setColour(juce::ToggleButton::textColourId, textColour);
    freezeButton.setColour(juce::ToggleButton::tickColourId, accentColour);
    freezeButton.setColour(juce::ToggleButton::tickDisabledColourId, mutedColour);
    addAndMakeVisible(freezeButton);
    freezeAttachment = std::make_unique<BasementAudioProcessor::APVTS::ButtonAttachment>(
        audioProcessor.parameters, "freeze", freezeButton);
}

void BasementAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(backgroundColour);

    auto panel = getLocalBounds().toFloat().reduced(12.0f);
    g.setColour(panelColour);
    g.fillRoundedRectangle(panel, 8.0f);

    g.setColour(accentColour);
    g.fillRoundedRectangle(16.0f, 16.0f, 728.0f, 5.0f, 2.0f);

    g.setColour(accentColour);
    g.setFont(juce::Font(juce::FontOptions(11.0f, juce::Font::bold)));
    g.drawText("DIRTY SPACE REVERB  /  AUDIO PUNKS",
               36, 76, 450, 18, juce::Justification::left);

    g.setColour(mutedColour);
    g.setFont(juce::Font(juce::FontOptions(10.0f, juce::Font::bold)));
    g.drawText("NO CLEAN ROOMS. NO POLITE REFLECTIONS.",
               36, 96, 450, 16, juce::Justification::left);

    g.setColour(juce::Colour(0xff393430));
    g.drawLine(28.0f, 120.0f, 732.0f, 120.0f, 1.0f);
    g.drawLine(28.0f, 456.0f, 732.0f, 456.0f, 1.0f);

    g.setColour(juce::Colour(0xff302d2a));
    for (int x = 268; x < 720; x += 232)
        g.drawLine(static_cast<float>(x), 132.0f, static_cast<float>(x), 446.0f, 1.0f);
}

void BasementAudioProcessorEditor::resized()
{
    title.setBounds(34, 26, 390, 50);
    presetLabel.setBounds(488, 42, 58, 26);
    presetSelector.setBounds(552, 40, 172, 30);

    const int gridX = 28;
    const int gridY = 128;
    const int cellWidth = 234;
    const int cellHeight = 108;

    for (size_t i = 0; i < sliders.size(); ++i)
    {
        const int column = static_cast<int>(i % 3);
        const int row = static_cast<int>(i / 3);
        auto cell = juce::Rectangle<int>(
            gridX + column * cellWidth,
            gridY + row * cellHeight,
            cellWidth,
            cellHeight);

        labels[i].setBounds(cell.removeFromTop(20));
        sliders[i].setBounds(cell.reduced(18, 0));
    }

    freezeButton.setBounds(34, 468, 140, 28);
}
