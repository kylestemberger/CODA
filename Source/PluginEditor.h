

#pragma once

#include "PluginProcessor.h"

#include <JuceHeader.h>

#include "CodaSlider.h"


#include "codaSliderLookAndFeel.h"

#include "FocusSlider.h"

#include "FocusSliderLNF.h"









class CodaEditor : public juce::AudioProcessorEditor,
                public juce::Slider::Listener
{
public:
    explicit CodaEditor(CodaProcessor&);
    ~CodaEditor() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
    void sliderValueChanged(juce::Slider *slider) override;

    


private:
    CodaProcessor& processor_;
    
    
    
    juce::Image background;
    
    
    
    juce::Slider centerSlider;
    
   
    CodaSlider amountSlider;
    
   
    
    FocusSlider focusSlider;
    juce::Label  centerLabel;
  
    
    GradientSlider gradientSlider;
    
    
  
    
    juce::Image GradientStrip;
    
    

    
    class MainContentComponent   : public juce::Component,
    public juce::Slider::Listener
    {};
        
        
    public:
 
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CodaEditor)
    };

