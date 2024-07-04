#include <JuceHeader.h>
#include "FocusSliderLNF.h"

//==============================================================================
FocusSliderLookAndFeel::FocusSliderLookAndFeel() : LookAndFeel_V4()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    FocusKnob = juce::ImageCache::getFromMemory(BinaryData::Focus2_png, BinaryData::Focus2_pngSize);


}


void FocusSliderLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    if (FocusKnob.isValid())
    {
        const double rotation = (slider.getValue()
            - slider.getMinimum())
        
            / (slider.getMaximum()
                - slider.getMinimum());

        const int frames = 10;
        const int frameId = (int)ceil(rotation * ((double)frames - 1.0));
    


        int imgWidth = FocusKnob.getWidth();
        int imgHeight = FocusKnob.getHeight() / frames;
        g.drawImage(FocusKnob, 0, 0, width, height, 0, frameId * imgHeight, imgWidth, imgHeight);
    }
    else
    {
        static const float textPpercent = 0.35f;
        juce::Rectangle<float> text_bounds(1.0f + width * (1.0f - textPpercent) / 2.0f, 0.5f * height, width * textPpercent, 0.5f * height);

        g.setColour(juce::Colours::white);

        g.drawFittedText(juce::String("No Image"), text_bounds.getSmallestIntegerContainer(), juce::Justification::horizontallyCentred | juce::Justification::centred, 1);
    }
}

void FocusSliderLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
{
    g.setColour(juce::Colour::fromRGB(255, 255, 255));
    g.fillRoundedRectangle(label.getLocalBounds().toFloat(), 3.0f);


    if (!label.isBeingEdited())
    {
      
        const juce::Font font(getLabelFont(label));

        g.setColour(juce::Colour::fromRGB(0, 0, 0));
        g.setFont(font);

        auto textArea = getLabelBorderSize(label).subtractedFrom(label.getLocalBounds());

        g.drawFittedText(label.getText(), textArea, label.getJustificationType(),
                         fmax(1, (int)(textArea.getHeight() / font.getHeight())),
            label.getMinimumHorizontalScale());

        g.setColour(juce::Colour::fromRGB(255, 255, 255));
    }
    else if (label.isEnabled())
    {
        g.setColour(label.findColour(juce::Label::outlineColourId));
    }
    
    


    //g.fillRoundedRectangle(label.getLocalBounds().toFloat(), 3.0f);
}






