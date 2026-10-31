#include "PluginEditor.h"

BasementAudioProcessorEditor::BasementAudioProcessorEditor(BasementAudioProcessor& processor)
    : juce::AudioProcessorEditor(&processor), audioProcessor(processor)
{
    setSize(720, 500);
    setResizable(false, false);

    title.setText("BASEMENT", juce::dontSendNotification);
    title.setFont(juce::Font(juce::FontOptions(25.0f, juce::Font::bold)));
    title.setJustificationType(juce::Justification::centredLeft);
    title.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(title);

    presetLabel.setText("MODE", juce::dontSendNotification);
    presetLabel.setJustificationType(juce::Justification::centredRight);
    presetLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(presetLabel);

    presetSelector.addItem("BASEMENT", 1);
    presetSelector.addItem("GARAGE", 2);
    presetSelector.addItem("BUNKER", 3);
    presetSelector.addItem("ABANDONED FACTORY", 4);
    presetSelector.addItem("DIRTY SPRING", 5);
    presetSelector.setSelectedId(audioProcessor.getCurrentProgram() + 1, juce::dontSendNotification);
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
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::white);
        slider.setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff292929));
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xffc36b36));
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colour(0xff555555));
        addAndMakeVisible(slider);

        labels[i].setText(parameterNames[i], juce::dontSendNotification);
        labels[i].setJustificationType(juce::Justification::centred);
        labels[i].setColour(juce::Label::textColourId, juce::Colours::lightgrey);
        addAndMakeVisible(labels[i]);

        sliderAttachments[i] = std::make_unique<BasementAudioProcessor::APVTS::SliderAttachment>(
            audioProcessor.parameters, parameterIds[i], slider);
    }

    freezeButton.setButtonText("FREEZE");
    freezeButton.setColour(juce::ToggleButton::textColourId, juce::Colours::white);
    addAndMakeVisible(freezeButton);
    freezeAttachment = std::make_unique<BasementAudioProcessor::APVTS::ButtonAttachment>(
        audioProcessor.parameters, "freeze", freezeButton);
}

void BasementAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff181818));
    g.setColour(juce::Colour(0xff343434));
    g.drawLine(20.0f, 62.0f, 700.0f, 62.0f, 1.0f);
    g.drawLine(20.0f, 440.0f, 700.0f, 440.0f, 1.0f);
}

void BasementAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(20);
    auto header = area.removeFromTop(42);
    title.setBounds(header.removeFromLeft(260));
    presetLabel.setBounds(header.removeFromRight(55));
    presetSelector.setBounds(header.removeFromRight(205).reduced(0, 5));

    area.removeFromTop(12);
    const int cellWidth = area.getWidth() / 3;
    const int cellHeight = 112;

    for (size_t i = 0; i < sliders.size(); ++i)
    {
        const int column = static_cast<int>(i % 3);
        const int row = static_cast<int>(i / 3);
        auto cell = juce::Rectangle<int>(
            area.getX() + column * cellWidth,
            area.getY() + row * cellHeight,
            cellWidth,
            cellHeight);

        labels[i].setBounds(cell.removeFromTop(22));
        sliders[i].setBounds(cell.reduced(8, 0));
    }

    freezeButton.setBounds(24, 447, 140, 30);
}
