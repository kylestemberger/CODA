

#pragma once

#include "PluginProcessor.h"
#include <JuceHeader.h>
#include "CodaSlider.h"
#include "codaSliderLookAndFeel.h"
#include "FocusSlider.h"
#include "FocusSliderLNF.h"
#include "laf/centerSliderLAF.h"
#include <LicenseSpring/LicenseManager.h>

class MyLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MyLookAndFeel()
    {
        setColour(juce::TextButton::buttonColourId, gray);
        setColour(juce::TextButton::buttonOnColourId, lightGray);
        setColour(juce::ComboBox::outlineColourId, lightGray);

        setColour(juce::TextEditor::focusedOutlineColourId, text);
        setColour(juce::TextEditor::backgroundColourId, gray);
        
        setColour(juce::Label::ColourIds::backgroundColourId, darkGray.withAlpha(0.8f));
        setColour(juce::Label::ColourIds::textColourId, text);
    }
    
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight)
    {
        juce::FontOptions options ("Helvetica", juce::jmin (20.0f, (float) buttonHeight * 0.6f), juce::Font::FontStyleFlags::bold);
        return juce::Font(options);
    }
    
    juce::Font getLabelFont (juce::Label& label)
    {
        juce::FontOptions options ("Helvetica", juce::jmin (20.0f, (float) label.getHeight() * 0.6f), juce::Font::FontStyleFlags::bold);
        return juce::Font(options);
    }
    
    void drawTextEditorOutline (juce::Graphics& g, int width, int height, juce::TextEditor& textEditor)
    {
        juce::Rectangle<int> bounds (width, height); // modified
        auto cornerSize = 6.0f; // modified

        if (dynamic_cast<juce::AlertWindow*> (textEditor.getParentComponent()) == nullptr)
        {
            if (textEditor.isEnabled())
            {
                if (textEditor.hasKeyboardFocus (true) && !textEditor.isReadOnly())
                {
                    g.setColour (textEditor.findColour (juce::TextEditor::focusedOutlineColourId));
                    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1);  // modified
                }
                else
                {
                    g.setColour (textEditor.findColour (juce::TextEditor::outlineColourId));
                    g.drawRoundedRectangle(bounds.toFloat().reduced(0.5f, 0.5f), cornerSize, 1);
                }
            }
        }
    }
    
private:
    juce::Colour gray = juce::Colour(42, 42, 45);
    juce::Colour darkGray = juce::Colour(17, 17, 20);
    juce::Colour lightGray = juce::Colour(84, 84, 90);
    juce::Colour text = juce::Colour(249, 250, 251);
};

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


class CodaEditor : public juce::AudioProcessorEditor
, public juce::Slider::Listener
, public juce::TextEditor::Listener
, public juce::Button::Listener
, public juce::Timer
{
public:
    explicit CodaEditor(CodaProcessor&);
    ~CodaEditor() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
    void sliderValueChanged(juce::Slider *slider) override;
    void textEditorTextChanged( juce::TextEditor& editor ) override;
    void buttonClicked( juce::Button* ) override;

private:
    CodaProcessor& processor_;
    
    MyLookAndFeel myLAF;
    
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
    
    //==============================================================================
    // Your private member variables go here...
    
    juce::Label labelKeyBased{{}, "For key-based product enter"};
    juce::Label labelUserBased{{}, "For user-based product enter"};
    juce::Label labelGetTrial{{}, "Get trial"};
    juce::Label labelUser{{}, "Email for trial"};
    juce::Label labelUser2{{}, "User"};
    juce::Label labelKey{{}, "Key"};
    juce::Label labelPassword{{}, "Password"};
    juce::TextButton activateKeyButton{"Activate"};
    juce::TextButton activateUserButton{"Activate"};
    juce::TextButton getTrialButton{"Get trial"};
    juce::TextEditor keyEditor;
    juce::TextEditor userEditor;
    juce::TextEditor userEditor2;
    juce::TextEditor passwordEditor;
    juce::ImageComponent image;

    juce::Label labelInfo{{}, ""};
    juce::TextButton deactivateButton{"Dectivate"};
    juce::TextButton checkButton{"Check license"};
    
    juce::Label bgLabel;


    void activateKeyBased();
    void activateUserBased();
    void getTrial();
    void deactivate();
    void checkLicense();
    void updateLicenseFields();
    void makeInfoVisible();
    void makeActivationVisible();
    
    class MainContentComponent   : public juce::Component,
    public juce::Slider::Listener
    {};
        
        
    public:
 
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CodaEditor)
    };

