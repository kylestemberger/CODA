#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class CodaSliderLookAndFeel : public juce::LookAndFeel_V4
{
    
public:
    CodaSliderLookAndFeel();
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider) override;
    void drawLabel(juce::Graphics& g, juce::Label& label) override;
    

private:
    juce::Image AmountKnob;
};
    


//===========================================================================


class GradientSliderLookAndFeel : public juce::LookAndFeel_V4
{
    
public:
    GradientSliderLookAndFeel();
    
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider2) override;

    
private:


    juce::Image GradientStrip;
};
    


