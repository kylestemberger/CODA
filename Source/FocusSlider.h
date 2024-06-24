#pragma once

#include <JuceHeader.h>
#include "FocusSliderLNF.h"

//==============================================================================
/*
*/
class FocusSlider : public juce::Slider
{
public:
    FocusSlider();
    ~FocusSlider();

    void mouseDown(const juce::MouseEvent& event) override;
    void mouseUp(const juce::MouseEvent& event) override;
    
    
    



private:
    FocusSliderLookAndFeel FocusSliderLookAndFeel;
    juce::Point<int> mousePosition;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FocusSlider)
};





