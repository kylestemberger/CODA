#include <JuceHeader.h>
#include "CodaSlider.h"

//==============================================================================
CodaSlider::CodaSlider() : Slider()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setLookAndFeel(&CodaSliderLookAndFeel);
    
}

CodaSlider::~CodaSlider()
{
    setLookAndFeel(nullptr);
}

void CodaSlider::mouseDown(const juce::MouseEvent& event)
{
    Slider::mouseDown(event);
    setMouseCursor(juce::MouseCursor::NoCursor);
    mousePosition = juce::Desktop::getMousePosition();
}

void CodaSlider::mouseUp(const juce::MouseEvent& event)
{
    Slider::mouseUp(event);
    juce::Desktop::setMousePosition(mousePosition);
    setMouseCursor(juce::MouseCursor::NormalCursor);
}



//==============================================================================


GradientSlider::GradientSlider() : Slider()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setLookAndFeel(&GradientSliderLookAndFeel);
}

GradientSlider::~GradientSlider()
{
    setLookAndFeel(nullptr);
    
}





