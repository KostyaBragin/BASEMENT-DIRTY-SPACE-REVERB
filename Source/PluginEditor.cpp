#include "PluginEditor.h"
#include <BinaryData.h>
BasementAudioProcessorEditor::BasementAudioProcessorEditor(BasementAudioProcessor&p):AudioProcessorEditor(&p),proc(p){
 setSize(760,480); backgroundImage = juce::ImageCache::getFromMemory(BinaryData::basement_background_jfif, BinaryData::basement_background_jfifSize); const char* ids[]={"decay","predelay","size","mix","damping","width","dirt","lowcut","highcut"};
 for(int i=0;i<9;++i){knobs[i]=std::make_unique<juce::Slider>();knobs[i]->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knobs[i]->setTextBoxStyle(juce::Slider::TextBoxBelow,false,78,20);
 knobs[i]->setColour(juce::Slider::rotarySliderFillColourId,juce::Colour(0xffd94b35));knobs[i]->setColour(juce::Slider::rotarySliderOutlineColourId,juce::Colour(0xff393735));knobs[i]->setColour(juce::Slider::thumbColourId,juce::Colour(0xfff0e7d8));knobs[i]->setColour(juce::Slider::textBoxTextColourId,juce::Colour(0xffeee6d8));knobs[i]->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colour(0xff252321));addAndMakeVisible(*knobs[i]);links[i]=std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(p.parameters,ids[i],*knobs[i]);}
 preset.addItem("BASEMENT",1);preset.addItem("GARAGE",2);preset.addItem("BUNKER",3);preset.addItem("ABANDONED FACTORY",4);preset.addItem("DIRTY SPRING",5);preset.onChange=[this]{proc.loadPreset(preset.getSelectedId()-1);};preset.setSelectedId(1);addAndMakeVisible(preset);
}
void BasementAudioProcessorEditor::paint(juce::Graphics&g){g.fillAll(juce::Colour(0xff171615));g.setColour(juce::Colour(0xff292624));g.fillRoundedRectangle(16,16,728,448,8);if(backgroundImage.isValid()){g.saveState();g.reduceClipRegion(juce::Rectangle<int>(16,16,728,448));g.drawImage(backgroundImage,16,16,728,448,0,0,backgroundImage.getWidth(),backgroundImage.getHeight());g.setColour(juce::Colours::black.withAlpha(0.62f));g.fillRect(16,16,728,448);g.restoreState();}g.setColour(juce::Colour(0xffd94b35));g.fillRect(16,16,728,5);
 g.setColour(juce::Colour(0xffe9e0d2));g.setFont(juce::Font(juce::FontOptions("Arial",42,juce::Font::bold)));g.drawText("BASEMENT",34,27,390,52,juce::Justification::left);
 g.setColour(juce::Colour(0xffd94b35));g.setFont(juce::Font(juce::FontOptions("Arial",12,juce::Font::bold)));g.drawText("DIRTY SPACE REVERB  /  SM production",37,78,450,20,juce::Justification::left);
 g.setColour(juce::Colour(0xffaaa095));g.setFont(juce::Font(juce::FontOptions("Arial",10,juce::Font::bold)));g.drawText("NO CLEAN ROOMS. NO POLITE REFLECTIONS.",37,102,450,18,juce::Justification::left);
}
void BasementAudioProcessorEditor::resized(){preset.setBounds(500,48,220,30);for(int i=0;i<9;++i){int row=i/5,col=i%5;knobs[i]->setBounds(30+col*143,145+row*145,130,125);}}
