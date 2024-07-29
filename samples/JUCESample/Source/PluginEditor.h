

#pragma once

#include "PluginProcessor.h"
#include <JuceHeader.h>
#include "CodaSlider.h"
#include "codaSliderLookAndFeel.h"
#include "FocusSlider.h"
#include "FocusSliderLNF.h"
#include "laf/centerSliderLAF.h"
#include <LicenseSpring/LicenseManager.h>

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
    juce::Label labelUser{{}, "User"};
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

