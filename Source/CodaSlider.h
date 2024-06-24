#pragma once

#include <JuceHeader.h>
#include "codaSliderLookAndFeel.h"


//==============================================================================
/*
*/
class CodaSlider : public juce::Slider
{
public:
CodaSlider();
    ~CodaSlider();

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    

    
    


private:
    CodaSliderLookAndFeel CodaSliderLookAndFeel;
    juce::Point<int> mousePosition;
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodaSlider)
};



//==============================================================================
/*
*/
class GradientSlider : public juce::Slider
{
public:
    GradientSlider();
    ~GradientSlider();

 
    



private:
    GradientSliderLookAndFeel GradientSliderLookAndFeel;
 
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GradientSlider)
};



//==============================================================================
/*
*/
class FreqSlider : public juce::Slider
{
public:
    FreqSlider();
    ~FreqSlider();

 
    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;



private:
   
    juce::Point<int> mousePosition;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FreqSlider)
};




