

#include "PluginEditor.h"

#include "PluginProcessor.h"

#include "utils.h"

#include <JuceHeader.h>




CodaEditor::CodaEditor(CodaProcessor& p)
    : AudioProcessorEditor(&p), processor_(p){
        

        
        setSize (548, 735);
        
        
      
        
}

CodaEditor::~CodaEditor() {
    
    

    
    
    
}

void CodaEditor::paint(juce::Graphics& g)
{
    

    
    background = juce::ImageCache::getFromMemory(BinaryData::CC_Backplate2_png, BinaryData::CC_Backplate2_pngSize);
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    
    
 
    
    
  
    
    
    
     // The frequency slider
    
    addAndMakeVisible(centerSlider);
    
    
   centerSlider.setBounds(120, 397, 310, 40);
    centerSlider.setRange(20.0f, 20000.0f, 1.0f);
   centerSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    centerSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100, 30);
    centerSlider.addListener (this);
    centerSlider.setSkewFactor(0.2);
    centerSlider.setTextValueSuffix("hz");
    centerSlider.isDoubleClickReturnEnabled();
    centerSlider.setDoubleClickReturnValue(true, 130.0f);
    centerSlider.setColour(juce::Slider::thumbColourId, juce::Colours::whitesmoke);
    centerSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::white.withAlpha(0.0f));
    centerSlider.setColour(juce::Slider::trackColourId, juce::Colours::white.withAlpha(0.8f));
   
  
 
    
    
    
    // The amount dial
    
    addAndMakeVisible(amountSlider);
    amountSlider.setBounds(118, 544, 156, 156);
    amountSlider.setRange(0.0f, 40.0f, 1.0f);
    amountSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    amountSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    amountSlider.addListener (this);
    amountSlider.isDoubleClickReturnEnabled();
    amountSlider.setDoubleClickReturnValue(true, 10.0f);
    amountSlider.snapValue(processor_.filter_stages_, juce::Slider::absoluteDrag);
    
    // The Gradient dial
    
    addAndMakeVisible(gradientSlider);
    gradientSlider.setBounds(140, 180, 272, 199);
    gradientSlider.setRange(0.0f, 40.0f, 1.0f);
    gradientSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    gradientSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);

    
 
    
   
    addAndMakeVisible(processor_.waveViewer);
    processor_.waveViewer.setOpaque(false);
    processor_.waveViewer.setBounds(140, 248, 272, 40);
    processor_.waveViewer.setColours(juce::Colours::white.withAlpha(0.0f), juce::Colours::white);
   
    
    // The focus dial
    
    
    addAndMakeVisible(focusSlider);
    focusSlider.setBounds(285, 550, 143, 143);
    focusSlider.setRange(0.5f, 20.0f, 0.5f);
    focusSlider.snapValue(processor_.filter_resonance_, juce::Slider::absoluteDrag);
    focusSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    focusSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    focusSlider.addListener (this);
    focusSlider.setSkewFactor(0.2);
    focusSlider.isDoubleClickReturnEnabled();
    focusSlider.setDoubleClickReturnValue(true, 0.5f);

    

    
   
    
    
    
  
   
  
       
 
    
    
    
    
    
    
   

    
    

  
    
    
    

}

void CodaEditor::resized()

{

    
    
}

void CodaEditor::sliderValueChanged (juce::Slider *slider)
{
    
    
    
    if (slider == &amountSlider)
    {
        processor_.filter_stages_ = amountSlider.getValue();
        
        
    };

    
    
    if (slider == &focusSlider)
    {
        processor_.filter_resonance_ = focusSlider.getValue();
    }
    
    if (slider == &centerSlider)
    {
        processor_.filter_frequency_ = centerSlider.getValue();
    }
    
 

    std::make_unique<juce::SliderParameterAttachment>(processor_.filter_stages_, gradientSlider);
    
    

    
}
