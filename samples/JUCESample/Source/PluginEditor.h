

#pragma once

#include "PluginProcessor.h"

#include <JuceHeader.h>

#include "CodaSlider.h"

#include "codaSliderLookAndFeel.h"

#include "FocusSlider.h"

#include "FocusSliderLNF.h"

#include "laf/centerSliderLAF.h"

class LogarithmicSlider : public juce::Slider
{
public:
    LogarithmicSlider() 
    {
        setRange(20.0, 20000.0, 1.0);
    }

    double getValueFromText(const juce::String& text) override
    {
        return text.getDoubleValue();
    }

    juce::String getTextFromValue(double value) override
    {
        return juce::String(value, 2);
    }

    double proportionOfLengthToValue(double proportion) override
    {
        double minValue = getRange().getStart();
        double maxValue = getRange().getEnd();
        return minValue * std::pow(maxValue / minValue, proportion);
    }

    double valueToProportionOfLength(double value) override
    {
        double minValue = getRange().getStart();
        double maxValue = getRange().getEnd();
        return std::log(value / minValue) / std::log(maxValue / minValue);
    }
};


class CodaEditor : public juce::AudioProcessorEditor,
                public juce::Slider::Listener,
public juce::Timer
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
    
    LogarithmicSlider centerSlider;
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
    
    void timerCallback() override;
    void drawFrame (juce::Graphics& g);
    
    class MainContentComponent   : public juce::Component,
    public juce::Slider::Listener
    {};
        
        
    public:
 
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CodaEditor)
    };

