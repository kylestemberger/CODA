#include <JuceHeader.h>
#include "FocusSlider.h"

//==============================================================================
FocusSlider::FocusSlider() : Slider()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    setLookAndFeel(&FocusSliderLookAndFeel);
}

FocusSlider::~FocusSlider()
{
    setLookAndFeel(nullptr);
}

void FocusSlider::mouseDown(const juce::MouseEvent& event)
{
    Slider::mouseDown(event);
    setMouseCursor(juce::MouseCursor::NoCursor);
    mousePosition = juce::Desktop::getMousePosition();
}

void FocusSlider::mouseUp(const juce::MouseEvent& event)
{
    Slider::mouseUp(event);
    juce::Desktop::setMousePosition(mousePosition);
    setMouseCursor(juce::MouseCursor::NormalCursor);
}


