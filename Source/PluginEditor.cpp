

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "utils.h"
#include <JuceHeader.h>

CodaEditor::CodaEditor(CodaProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
   // The frequency slider
   addAndMakeVisible(centerSlider);
   
   centerSlider.setRange(20.0f, 20000.0f, 1.0f);
   centerSlider.setSliderStyle(juce::Slider::LinearHorizontal);
   centerSlider.setTextBoxStyle(juce::Slider::TextBoxAbove, false, 100, 30);
   centerSlider.addListener (this);
   centerSlider.setSkewFactor(0.2);
   centerSlider.setTextValueSuffix("hz");
   centerSlider.setDoubleClickReturnValue(true, 130.0f);
   centerSlider.setColour(juce::Slider::thumbColourId, juce::Colours::whitesmoke);
   centerSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::white.withAlpha(0.0f));
   centerSlider.setColour(juce::Slider::trackColourId, juce::Colours::white.withAlpha(0.8f));
    auto amountValue = processor_.parameters_.getParameter("filter_freq")->getValue();
    auto from0 = processor_.parameters_.getParameter("filter_freq")->convertFrom0to1(amountValue);
    centerSlider.setValue(from0);
   
   // The amount dial
   addAndMakeVisible(amountSlider);
   amountSlider.setRange(0.0f, 40.0f, 1.0f);
   amountSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
   amountSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
   amountSlider.addListener (this);
   amountSlider.isDoubleClickReturnEnabled();
   amountSlider.setDoubleClickReturnValue(true, 10.0f);
   amountSlider.snapValue(processor_.filter_stages_, juce::Slider::absoluteDrag);
    
    amountValue = processor_.parameters_.getParameter("filter_stages")->getValue();
    from0 = processor_.parameters_.getParameter("filter_stages")->convertFrom0to1(amountValue);
   amountSlider.setValue(from0);
   
   // The Gradient dial
   addAndMakeVisible(gradientSlider);
   gradientSlider.setRange(0.0f, 40.0f, 1.0f);
   gradientSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
   gradientSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    
    amountValue = processor_.parameters_.getParameter("filter_stages")->getValue();
    from0 = processor_.parameters_.getParameter("filter_stages")->convertFrom0to1(amountValue);
    gradientSlider.setValue(from0);
  
   addAndMakeVisible(processor_.waveViewer);
   processor_.waveViewer.setOpaque(false);
   processor_.waveViewer.setBounds(140, 248, 272, 40);
   processor_.waveViewer.setColours(juce::Colours::white.withAlpha(0.0f), juce::Colours::white);
   
   // The focus dial
   addAndMakeVisible(focusSlider);
   focusSlider.setRange(0.5f, 20.0f, 0.5f);
   //focusSlider.snapValue(processor_.filter_resonance_, juce::Slider::absoluteDrag);
   focusSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
   focusSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
   focusSlider.addListener (this);
   focusSlider.setSkewFactor(0.2);
   focusSlider.isDoubleClickReturnEnabled();
   focusSlider.setDoubleClickReturnValue(true, 0.5f);
    amountValue = processor_.parameters_.getParameter("filter_res")->getValue();
    from0 = processor_.parameters_.getParameter("filter_res")->convertFrom0to1(amountValue);
    focusSlider.setValue(from0);
    
    setSize (548 * mult, 735 * mult);
}

CodaEditor::~CodaEditor() 
{
    
}

void CodaEditor::paint(juce::Graphics& g)
{
    background = juce::ImageCache::getFromMemory(BinaryData::CC_Backplate2_png, BinaryData::CC_Backplate2_pngSize);
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
}

void CodaEditor::resized()

{
    auto compX = getWidth() * 0.18;
    auto compY = getHeight() * 0.52;
    auto compWidth = getWidth() * 0.64;
    auto compHeight = getHeight() * 0.1;
    centerSlider.setBounds(compX, compY, compWidth, compHeight);
    
    compX = getWidth() * 0.17;
    compY = getHeight() * 0.2;
    compWidth = getWidth() * 0.65;
    compHeight = getHeight() * 0.4;
    gradientSlider.setBounds(compX, compY, compWidth, compHeight);
    
    compX = getWidth() * 0.16;
    compY = getHeight() * 0.72;
    compWidth = getWidth() * 0.35;
    amountSlider.setBounds(compX, compY, compWidth, compWidth);
    compX += compWidth;
    focusSlider.setBounds(compX, compY, compWidth, compWidth);
}

void CodaEditor::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &amountSlider)
    {
        processor_.filter_stages_ = amountSlider.getValue();
        gradientSlider.setValue(amountSlider.getValue());
    }
    
    if (slider == &focusSlider)
    {
        processor_.filter_resonance_ = focusSlider.getValue();
    }
    
    if (slider == &centerSlider)
    {
        processor_.filter_frequency_ = centerSlider.getValue();
    }
}
