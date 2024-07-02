

#pragma once

#include "PluginProcessor.h"

#include <JuceHeader.h>

#include "CodaSlider.h"

#include "codaSliderLookAndFeel.h"

#include "FocusSlider.h"

#include "FocusSliderLNF.h"

#include "laf/centerSliderLAF.h"

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
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> centerSliderAtt;
   
    CodaSlider amountSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> amountSliderAtt;
    
    FocusSlider focusSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> focusSliderAtt;
    
    juce::Label  centerLabel;
    
    GradientSlider gradientSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> gradientSliderAtt;
    
    juce::Image GradientStrip;
    
    double mult = 0.75;
    
    CutoffSlider cutoffLAF;
    
    class MainContentComponent   : public juce::Component,
    public juce::Slider::Listener
    {};
        
        
    public:
 
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CodaEditor)
    };

