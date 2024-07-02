#include <JuceHeader.h>
#include "codaSliderLookAndFeel.h"

//==============================================================================
CodaSliderLookAndFeel::CodaSliderLookAndFeel() : LookAndFeel_V4()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    AmountKnob = juce::ImageCache::getFromMemory(BinaryData::Amount2_png, BinaryData::Amount2_pngSize);
    



}


void CodaSliderLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider)
{
    if (AmountKnob.isValid())
    {
        const double rotation = (slider.getValue()
                                 - slider.getMinimum())
        
        / (slider.getMaximum()
           - slider.getMinimum());
        
        const int frames = 10;
        const int frameId = (int)ceil(rotation * ((double)frames - 1.0));
        
        int imgWidth = AmountKnob.getWidth();
        int imgHeight = AmountKnob.getHeight() / frames;
        
        g.drawImage(AmountKnob, 0, 0, imgWidth, imgHeight, 0, frameId * imgHeight, imgWidth, imgHeight);
    }
}



void CodaSliderLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label)
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

GradientSliderLookAndFeel::GradientSliderLookAndFeel() : LookAndFeel_V4()
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
    GradientStrip = juce::ImageCache::getFromMemory(BinaryData::GradientStrip_png, BinaryData::GradientStrip_pngSize);
    



}




void GradientSliderLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height, float sliderPos, float rotaryStartAngle, float rotaryEndAngle, juce::Slider& slider2)
{
    
    
    
    const double rotation = (slider2.getValue()
        - slider2.getMinimum())
    
        / (slider2.getMaximum()
            - slider2.getMinimum());

    const int frames = 11;
    const int frameId = (int)ceil(rotation * ((double)frames - 1.0));
    
    int imgWidth1 = GradientStrip.getWidth();
    int imgHeight1 = GradientStrip.getHeight() / frames;
    g.setOpacity(0.75f);
    g.drawImage(GradientStrip, 0, 0, 272, 199, 0, frameId * imgHeight1, imgWidth1, imgHeight1);
}
