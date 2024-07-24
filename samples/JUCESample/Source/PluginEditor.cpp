

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "utils.h"
#include <JuceHeader.h>

CodaEditor::CodaEditor(CodaProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    startTimerHz(60);
    
   centerSlider.setSliderStyle(juce::Slider::LinearBar);
   centerSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
   centerSlider.addListener (this);
   //centerSlider.setSkewFactorFromMidPoint(800.0);
   centerSlider.setTextValueSuffix("hz");
   centerSlider.setDoubleClickReturnValue(true, 130.0f);
   centerSlider.setColour(juce::Slider::thumbColourId, juce::Colours::whitesmoke);
   centerSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::white.withAlpha(0.0f));
   centerSlider.setColour(juce::Slider::trackColourId, juce::Colours::white.withAlpha(0.8f));
    auto amountValue = processor_.parameters_.getParameter("filter_freq")->getValue();
    auto from0 = processor_.parameters_.getParameter("filter_freq")->convertFrom0to1(amountValue);
    centerSlider.setValue(from0);
    centerSlider.setLookAndFeel(&cutoffLAF);
   
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
    
    // The frequency slider
    addAndMakeVisible(centerSlider);
    
    setSize (548 * mult, 735 * mult);
}

CodaEditor::~CodaEditor() 
{
    centerSlider.setLookAndFeel(nullptr);
    
    stopTimer();
}

void CodaEditor::paint(juce::Graphics& g)
{
    background = juce::ImageCache::getFromMemory(BinaryData::CC_Backplate2_png, BinaryData::CC_Backplate2_pngSize);
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    
    drawFrame(g);
}

void CodaEditor::resized()
{
    auto compX = getWidth() * 0.24;
    auto compY = getHeight() * 0.25;
    auto compWidth = getWidth() * 0.495;
    auto compHeight = getHeight() * 0.335;
    centerSlider.setBounds(compX, compY, compWidth, compHeight);
    
    compX = getWidth() * 0.27;
    compY = getHeight() * 0.27;
    compWidth = getWidth() * 0.65;
    compHeight = getHeight() * 0.4;
    gradientSlider.setBounds(compX, compY, compWidth, compHeight);
    
    compX = getWidth() * 0.24;
    compY = getHeight() * 0.76;
    compWidth = getWidth() * 0.26;
    amountSlider.setBounds(compX, compY, compWidth, compWidth);
    compX += compWidth * 1.1;
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

void CodaEditor::timerCallback()
{
    if (processor_.nextFFTBlockReady)
    {
        processor_.drawNextFrameOfSpectrum();
        processor_.nextFFTBlockReady = false;
        repaint();
    }
}

void CodaEditor::drawFrame(juce::Graphics& g)
{
    // Define the drawing region
    int width  = getWidth() * 0.53;
    int height = getHeight() * 0.396;
    int x = getWidth() * 0.24;
    int y = getHeight() * 0.185;

    // Set the clipping region to the bounds of the drawing area
    g.reduceClipRegion(x, y, width, height);

    // Ensure processor_.scopeSize and processor_.scopeData are valid
    if (processor_.scopeSize <= 0)
        return;

    // Draw the frequency response
    g.setColour(juce::Colours::white);
    for (int i = 1; i < processor_.scopeSize; ++i)
    {
        // Ensure the data is within the expected range
        float prevValue = juce::jlimit(0.0f, 1.0f, processor_.scopeData[i - 1]);
        float currValue = juce::jlimit(0.0f, 1.0f, processor_.scopeData[i]);

        g.drawLine(
            {
                (float) juce::jmap(i - 1, 0, processor_.scopeSize - 1, x, width + x),
                juce::jmap(prevValue, 0.0f, 1.0f, (float) height + (float) y, (float) y),
                (float) juce::jmap(i, 0, processor_.scopeSize - 1, x, width + x),
                juce::jmap(currValue, 0.0f, 1.0f, (float) height + (float) y, (float) y),
            });
    }
}

