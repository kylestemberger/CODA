#pragma once

#include <JuceHeader.h>
using namespace juce;

class CutoffSlider : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                                           float sliderPos,
                                           float minSliderPos,
                                           float maxSliderPos,
                                           const Slider::SliderStyle style, Slider& slider)
    {
        Point<float> startPoint (slider.isHorizontal() ? (float) x : (float) x + (float) width * 0.5f,
                                 slider.isHorizontal() ? (float) height : (float) (height + y));

        Point<float> endPoint (slider.isHorizontal() ? (float) (width + x) : startPoint.x,
                               slider.isHorizontal() ? startPoint.y : (float) y);

        g.setColour(juce::Colours::white);
        g.drawLine(0.0f, height, width, height);

        // black value bar
        g.setColour(juce::Colours::black.withAlpha(0.25f));
        auto rectX = slider.getPositionOfValue(slider.getValue()) - width * 0.05f;
        juce::Rectangle<float> blackBlock (static_cast<float>(rectX),
                                           1.0f,
                                           width * 0.1f,
                                           static_cast<float>(height) - height * 0.02f);
        
        juce::ColourGradient gradient(juce::Colours::black.withAlpha(0.2f), blackBlock.getX(), blackBlock.getY(), juce::Colours::black.withAlpha(0.7f), blackBlock.getRight(), blackBlock.getBottom(), false);

        g.setGradientFill(gradient);
        g.fillRoundedRectangle(blackBlock, 10.0f);
        g.fillRect(blackBlock.withHeight(10).withY(blackBlock.getHeight() - 7));
        
//        // white border
//        g.setColour(juce::Colours::whitesmoke.withAlpha(0.05f));
//        g.drawRoundedRectangle(static_cast<float>(rectX - 1),
//                   0.0f,
//                   width * 0.1f + 1,
//                   static_cast<float>(height) - height * 0.02f, 10.0f, 1.0f);
        
        // white thumb square
        g.setColour(juce::Colours::whitesmoke);
        juce::Rectangle<float> whiteBlock (static_cast<float>(rectX + width * 0.015f),
                                           height - width * 0.0625f * 2,
                                           width * 0.0625f,
                                           width * 0.0625f);
        g.fillRoundedRectangle(whiteBlock,
                               3.0f);
        
        // black lines
        g.setColour(juce::Colours::black);
        auto padding = width * 0.0125f;
        auto lineX = whiteBlock.getX() + padding;
        auto linY = whiteBlock.getY() + padding;
        for (int i = 0; i < 4; ++i)
        {
            g.drawLine(lineX, linY, lineX, linY + width * 0.035f, 1.0f);
            lineX += padding;
        }
        
        // text
        g.setColour(juce::Colours::whitesmoke.withAlpha(0.8f));
        g.setFont(juce::Font("Helvetica", height * 0.06, juce::Font::FontStyleFlags::bold));

        auto text = juce::String(slider.getValue()) + " HZ";

        auto textBounds = blackBlock.withWidth(width).toNearestInt();

        auto mult = height * 0.35;
        g.addTransform(juce::AffineTransform::rotation(juce::degreesToRadians(270.0f),
                                                       blackBlock.getCentreX() + mult, blackBlock.getCentreY() + mult * 0.98f));
        g.drawFittedText(text, textBounds, juce::Justification::centred, 1);
    }
};
    


