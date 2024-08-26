

#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "utils.h"
#include <JuceHeader.h>

CodaEditor::CodaEditor(CodaProcessor& p)
    : AudioProcessorEditor(&p), processor_(p)
{
    startTimerHz(60);
    
   centerSlider.setSliderStyle(juce::Slider::LinearBar);
   centerSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
   centerSlider.addListener (this);
   //centerSlider.setSkewFactorFromMidPoint(800.0);
   centerSlider.setTextValueSuffix("hz");
   centerSlider.setDoubleClickReturnValue(true, 130.0f);
   centerSlider.setColour(juce::Slider::thumbColourId, juce::Colours::whitesmoke);
   centerSlider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::white.withAlpha(0.0f));
   centerSlider.setColour(juce::Slider::trackColourId, juce::Colours::white.withAlpha(0.8f));
    auto amountValue = processor_.parameters_.getParameter("filter_freq")->getValue();
    auto from0 = processor_.parameters_.getParameter("filter_freq")->convertFrom0to1(amountValue);
    centerSlider.setValue(from0);
    centerSlider.setLookAndFeel(&cutoffLAF);
   
   // The amount dial
   addAndMakeVisible(amountSlider);
   amountSlider.setRange(0.0f, 40.0f, 1.0f);
   amountSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
   amountSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
   amountSlider.addListener (this);
   amountSlider.isDoubleClickReturnEnabled();
   amountSlider.setDoubleClickReturnValue(true, 10.0f);
   amountSlider.snapValue(processor_.filter_stages_, juce::Slider::absoluteDrag);
    
    amountValue = processor_.parameters_.getParameter("filter_stages")->getValue();
    from0 = processor_.parameters_.getParameter("filter_stages")->convertFrom0to1(amountValue);
   amountSlider.setValue(from0);
   
   // The Gradient dial
   addAndMakeVisible(gradientSlider);
   gradientSlider.setRange(0.0f, 40.0f, 1.0f);
   gradientSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
   gradientSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    
    amountValue = processor_.parameters_.getParameter("filter_stages")->getValue();
    from0 = processor_.parameters_.getParameter("filter_stages")->convertFrom0to1(amountValue);
    gradientSlider.setValue(from0);
  
   // The focus dial
   addAndMakeVisible(focusSlider);
   focusSlider.setRange(0.5f, 20.0f, 0.5f);
   //focusSlider.snapValue(processor_.filter_resonance_, juce::Slider::absoluteDrag);
   focusSlider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
   focusSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
   focusSlider.addListener (this);
   focusSlider.setSkewFactor(0.2);
   focusSlider.isDoubleClickReturnEnabled();
   focusSlider.setDoubleClickReturnValue(true, 0.5f);
    amountValue = processor_.parameters_.getParameter("filter_res")->getValue();
    from0 = processor_.parameters_.getParameter("filter_res")->convertFrom0to1(amountValue);
    focusSlider.setValue(from0);
    
    // The frequency slider
    addAndMakeVisible(centerSlider);
    
    // license stuff
    addChildComponent(bgLabel);
    bgLabel.setInterceptsMouseClicks(false, false);
    
    addChildComponent( labelKeyBased );
    addChildComponent( labelKey );
    labelKey.setJustificationType(juce::Justification::centred);
    //addChildComponent( labelGetTrial );
    //addChildComponent( getTrialButton );
    addChildComponent( activateKeyButton );
    addChildComponent( keyEditor );
    //addChildComponent( labelUser );
    //labelUser.setJustificationType(juce::Justification::centred);
    //addChildComponent( userEditor );
    addChildComponent( labelInfo );
    addChildComponent( deactivateButton );
    addChildComponent( checkButton );
    

    activateKeyButton.addListener( this );
    getTrialButton.addListener( this );
    deactivateButton.addListener( this );
    checkButton.addListener( this );

    activateKeyButton.setComponentID( "Activate key-based" );
    activateUserButton.setComponentID( "Activate user-based" );
    getTrialButton.setComponentID( "Get trial" );
    deactivateButton.setComponentID( "Deactivate" );
    checkButton.setComponentID( "Check" );

    auto& license = processor_.licenseManager->getCurrentLicense();

    if( license != nullptr )
    {
        updateLicenseFields();
        makeInfoVisible();
    }
    else
        makeActivationVisible();
    
    setLookAndFeel(&myLAF);
    
    setSize (548 * mult, 735 * mult);
}

CodaEditor::~CodaEditor() 
{
    setLookAndFeel(nullptr);
    
    centerSlider.setLookAndFeel(nullptr);
    
    activateKeyButton.removeListener( this );
    getTrialButton.removeListener( this );
    deactivateButton.removeListener( this );
    checkButton.removeListener( this );
    
    stopTimer();
}

void CodaEditor::paint(juce::Graphics& g)
{
    background = juce::ImageCache::getFromMemory(BinaryData::CC_Backplate2_png, BinaryData::CC_Backplate2_pngSize);
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
    
    drawFrame(g);
}

void CodaEditor::resized()
{
    auto compX = getWidth() * 0.24;
    auto compY = getHeight() * 0.25;
    auto compWidth = getWidth() * 0.495;
    auto compHeight = getHeight() * 0.335;
    centerSlider.setBounds(compX, compY, compWidth, compHeight);
    
    compX = getWidth() * 0.27;
    compY = getHeight() * 0.27;
    compWidth = getWidth() * 0.65;
    compHeight = getHeight() * 0.4;
    gradientSlider.setBounds(compX, compY, compWidth, compHeight);
    
    compX = getWidth() * 0.24;
    compY = getHeight() * 0.76;
    compWidth = getWidth() * 0.26;
    amountSlider.setBounds(compX, compY, compWidth, compWidth);
    compX += compWidth * 1.1;
    focusSlider.setBounds(compX, compY, compWidth, compWidth);
    
    // license stuff
    auto compBounds = getLocalBounds().withSizeKeepingCentre(getWidth() * 0.5, getHeight() * 0.05);
    keyEditor.setBounds( compBounds );
    labelKey.setBounds(compBounds.withY(keyEditor.getY() - keyEditor.getHeight()));
    activateKeyButton.setBounds(compBounds.withY(keyEditor.getBottom() + getHeight() * 0.018).withX(keyEditor.getX()));
    labelUser.setBounds(compBounds.withY(activateKeyButton.getBottom() + getHeight() * 0.036).withX(labelKey.getX()));
    userEditor.setBounds(compBounds.withY(labelUser.getBottom() + getHeight() * 0.018).withX(keyEditor.getX()));
    getTrialButton.setBounds(compBounds.withY(userEditor.getBottom() + getHeight() * 0.018).withX(keyEditor.getX()));

    //labelInfo.setBounds( 20, 20, getWidth()-40, getHeight()-50 );
    deactivateButton.setBounds( 20, getHeight()-40, getWidth()/2 - 30, 20);
    checkButton.setBounds( getWidth() / 2, getHeight() - 40, getWidth() / 2 - 30, 20 );
    
    bgLabel.setBounds(getLocalBounds());
}

void CodaEditor::sliderValueChanged (juce::Slider *slider)
{
    if (slider == &amountSlider)
    {
        processor_.filter_stages_ = amountSlider.getValue();
        gradientSlider.setValue(amountSlider.getValue());
    }
    
    if (slider == &focusSlider)
    {
        processor_.filter_resonance_ = focusSlider.getValue();
    }
    
    if (slider == &centerSlider)
    {
        processor_.filter_frequency_ = centerSlider.getValue();
    }
}

void CodaEditor::timerCallback()
{
    if (processor_.nextFFTBlockReady)
    {
        processor_.drawNextFrameOfSpectrum();
        processor_.nextFFTBlockReady = false;
        repaint();
    }
}

void CodaEditor::drawFrame(juce::Graphics& g)
{
    // Ensure processor_.scopeSize and processor_.scopeData are valid
    if (!(processor_.scopeSize > 0) ||
        !processor_.audioInitialized.load())
        return;
    
    // Define the drawing region
    int width  = getWidth() * 0.53;
    int height = getHeight() * 0.396;
    int x = getWidth() * 0.24;
    int y = getHeight() * 0.185;

    // Set the clipping region to the bounds of the drawing area
    g.reduceClipRegion(x, y, width, height);

    // Draw the frequency response
    g.setColour(juce::Colours::white);
    
    for (int i = 1; i < processor_.scopeSize; ++i)
    {
        // Ensure the data is within the expected range
        float prevValue = juce::jlimit(0.0f, 1.0f, processor_.scopeData[i - 1]);
        float currValue = juce::jlimit(0.0f, 1.0f, processor_.scopeData[i]);
        
        if (prevValue == 0.0f ||
            prevValue == 1.0f ||
            currValue == 0.0f ||
            currValue == 1.0f)
            return;

        g.drawLine(
            {
                (float) juce::jmap(i - 1, 0, processor_.scopeSize - 1, x, width + x),
                juce::jmap(prevValue, 0.0f, 1.0f, (float) height + (float) y, (float) y),
                (float) juce::jmap(i, 0, processor_.scopeSize - 1, x, width + x),
                juce::jmap(currValue, 0.0f, 1.0f, (float) height + (float) y, (float) y),
            });
    }
}

void CodaEditor::makeInfoVisible()
{
    labelKeyBased.setVisible( false );
    labelUserBased.setVisible( false );
    labelUser.setVisible( false );
    labelKey.setVisible( false );
    labelPassword.setVisible( false );
    labelGetTrial.setVisible( false );
    labelUser2.setVisible( false );
    getTrialButton.setVisible( false );
    activateKeyButton.setVisible( false );
    activateUserButton.setVisible( false );
    keyEditor.setVisible( false );
    userEditor.setVisible( false );
    passwordEditor.setVisible( false );
    userEditor2.setVisible( false );
    image.setVisible( false );
    bgLabel.setVisible( false );

    labelInfo.setVisible( false );
    deactivateButton.setVisible( true );
    checkButton.setVisible( true );
}

void CodaEditor::makeActivationVisible()
{
    labelInfo.setVisible( false );
    deactivateButton.setVisible( false );
    checkButton.setVisible( false );

    bgLabel.setVisible( true );
    labelKeyBased.setVisible( true );
    labelUserBased.setVisible( true );
    labelUser.setVisible( true );
    labelKey.setVisible( true );
    labelPassword.setVisible( true );
    labelGetTrial.setVisible( true );
    labelUser2.setVisible( true );
    getTrialButton.setVisible( true );
    activateKeyButton.setVisible( true );
    activateUserButton.setVisible( true );
    keyEditor.setVisible( true );
    userEditor.setVisible( true );
    passwordEditor.setVisible( true );
    userEditor2.setVisible( true );
    image.setVisible( true );
}

void CodaEditor::buttonClicked( juce::Button* button )
{
    auto buttonId = button->getComponentID();
    if( buttonId == activateKeyButton.getComponentID() )
        activateKeyBased();
    if( buttonId == activateUserButton.getComponentID() )
        activateUserBased();
    if( buttonId == getTrialButton.getComponentID() )
        getTrial();
    if( buttonId == deactivateButton.getComponentID() )
        deactivate();
    if( buttonId == checkButton.getComponentID() )
        checkLicense();
}

void CodaEditor::activateKeyBased()
{
    try
    {
        auto key = keyEditor.getText();
        if( !key.isEmpty() )
        {
            LicenseSpring::LicenseID id =LicenseSpring::LicenseID::fromKey( key.toStdString() ) ;
            auto license = processor_.licenseManager->activateLicense(id );
            if( license->isValid() )
            {
                updateLicenseFields();
                makeInfoVisible();
                processor_.setLicenseIsValid(true);
            }
        }
        else
            AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Warning", "Type key for activation", "Ok" );

    }
    catch( LicenseSpring::LicenseSpringException ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "LicenseSpring exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( std::exception ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Standard exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( ... )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Unknown exception encountered!" ), "Ok" );
    }

}

void CodaEditor::activateUserBased()
{
    try
    {
        auto user = userEditor.getText();
        auto password = passwordEditor.getText();
        if( !user.isEmpty() && !password.isEmpty() )
        {
            auto license = processor_.licenseManager->activateLicense( LicenseSpring::LicenseID::fromUser( user.toStdString(), password.toStdString() ) );
            if( license->isValid() )
            {
                updateLicenseFields();
                makeInfoVisible();
            }
        }
        else
            AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Warning", "Type user emal and password for activation", "Ok" );

    }
    catch( LicenseSpring::LicenseSpringException ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "LicenseSpring exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( std::exception ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Standard exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( ... )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Unknown exception encountered!" ), "Ok" );
    }
}

void CodaEditor::getTrial()
{
    try
    {
        //auto user = userEditor2.getText();
        std::string user = "landonviator@gmail.com";
        auto id = processor_.licenseManager->getTrialLicense( user );
        userEditor.setText( id.user().c_str() );
        passwordEditor.setText( id.password().c_str() );
        keyEditor.setText( id.key().c_str() );
    }
    catch( LicenseSpring::LicenseSpringException ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "LicenseSpring exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( std::exception ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Standard exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( ... )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Unknown exception encountered!" ), "Ok" );
    }
}

void CodaEditor::updateLicenseFields()
{
    try
    {
        auto license = processor_.licenseManager->getCurrentLicense();
        if( license == nullptr )
            return;

        String text = "";

        text += "\n------------- License info -------------\n";

        const auto& licenseOwner = license->owner();
        {
            std::string ownerInfo;
            auto formatInfo = [&ownerInfo]( const std::string& value )
            {
                if( value.empty() )
                    return;
                if( !ownerInfo.empty() )
                    ownerInfo.append( " " );
                ownerInfo.append( value );
            };
            formatInfo( licenseOwner.firstName() );
            formatInfo( licenseOwner.lastName() );
            formatInfo( licenseOwner.email() );
            formatInfo( licenseOwner.company() );
            if( !ownerInfo.empty() )
            {
                text += "Licensed to: ";
                text += ownerInfo.c_str();
                text += "\n";
            }
        }

        if( !license->key().empty() )
        {
            text += "Key = ";
            text += license->key().c_str();
            text += "\n";
        }

        if( !license->user().empty() )
        {
            text += "User = ";
            text += license->user().c_str();
            text += "\n";
        }

        text += "Type = ";
        text += license->type().toFormattedString().c_str();
        text += "\n";
        text += "Status = ";
        text += license->status().c_str();
        text += "\n";
        text += "IsActive = ";
        text += license->isActive() ? "True" : "False";
        text += "\n";
        text += "IsEnabled = ";
        text += license->isEnabled() ? "True" : "False";
        text += "\n";
        text += "IsTrial = ";
        text += license->isTrial() ? "True" : "False";
        text += "\n";
        text += "IsFloating = ";
        text += license->isFloating() ? "True" : "False";
        text += "\n";
        text += "IsOfflineActivated = ";
        text += license->isOfflineActivated() ? "True" : "False";
        text += "\n";
        text += "Times activated = ";
        text += static_cast<int>(license->timesActivated());
        text += "\n";
        text += "Max activations = ";
        text += static_cast<int>(license->maxActivations());
        text += "\n";

        auto TmToStr = []( const tm& dateTime )
        {
            return LicenseSpring::TmToString( dateTime, "%d-%m-%Y %H:%M:%S" );
        };

        text += "Validity Period = ";
        text += TmToStr( license->validityPeriod() ).c_str();
        text += "\n";
        text += "Validity Period UTC = ";
        text += TmToStr( license->validityPeriodUtc() ).c_str();
        text += "\n";
        text += "Days remaining till license expires = ";
        text += license->daysRemaining();
        text += "\n";
        text += "Maintenance period = ";
        text += TmToStr( license->maintenancePeriod() ).c_str();
        text += "\n";
        text += "Maintenance period UTC = ";
        text += TmToStr( license->maintenancePeriodUtc() ).c_str();
        text += "\n";
        text += "Maintenance days remaining = ";
        text += license->maintenanceDaysRemaining();
        text += "\n";
        text += "Last online check date = ";
        text += TmToStr( license->lastCheckDate() ).c_str();
        text += "\n";
        text += "Days passed since last online check = ";
        text += license->daysPassedSinceLastCheck();
        text += "\n";

        auto productFeatures = license->features();
        if( !productFeatures.empty() )
        {
            text += "Product features available for this license:\n";
            for( auto feature : productFeatures )
            {
                text += feature.toString().c_str();
                text += "\n";
            }
        }

        auto dataFields = license->customFields();
        if( !dataFields.empty() )
        {
            text += "Custom data fields available for this license:\n";
            for( const auto& field : dataFields )
            {
                text += "Data field - Name: ";
                text += field.fieldName().c_str();
                text += ", Value: ";
                text += field.fieldValue().c_str();
                text += "\n";
            }
        }

        const auto& userData = license->userData();
        if( !userData.empty() )
        {
            text += "User data for this license: \n";
            for( const auto& field : userData )
            {
                text += "Data field - Name: ";
                text += field.fieldName().c_str();
                text += ", Value: ";
                text += field.fieldValue().c_str();
                text += "\n";
            }
        }

        if( license->type() == LicenseTypeConsumption )
        {
            text += "Total consumptions = ";
            text += license->totalConsumption();
            text += "\n";
            text += "Max consumptions = ";
            text += license->maxConsumption();
            text += "\n";
            text += "Is overages allowed = ";
            text += license->isOveragesAllowed();
            text += "\n";
            if( license->isOveragesAllowed() )
            {
                text += "Max overages = ";
                text += license->maxOverages();
                text += "\n";
            }
        }

        labelInfo.setText( text ,NotificationType::dontSendNotification);
    }
    catch( LicenseSpring::LicenseSpringException ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "LicenseSpring exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( std::exception ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Standard exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( ... )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Unknown exception encountered!" ), "Ok" );
    }
}

void CodaEditor::deactivate()
{
    try
    {
        processor_.licenseManager->getCurrentLicense()->deactivate( true );
        makeActivationVisible();
    }
    catch( LicenseSpring::LicenseSpringException ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "LicenseSpring exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( std::exception ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Standard exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( ... )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Unknown exception encountered!" ), "Ok" );
    }
}

void CodaEditor::checkLicense()
{
    try
    {
        processor_.licenseManager->getCurrentLicense()->check();
        updateLicenseFields();
        AlertWindow::showMessageBoxAsync( AlertWindow::InfoIcon, "Info", "License check sucessful!", "Ok" );
    }
    catch( LicenseSpring::LicenseSpringException ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "LicenseSpring exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( std::exception ex )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Standard exception encountered: " ) + String( ex.what() ), "Ok" );
    }
    catch( ... )
    {
        AlertWindow::showMessageBoxAsync( AlertWindow::WarningIcon, "Error", String( "Unknown exception encountered!" ), "Ok" );
    }
}

void CodaEditor::textEditorTextChanged( TextEditor& editor )
{
    if( editor.getText() == userEditor2.getText() )
        userEditor.setText( userEditor2.getText().toLowerCase() );
    else
        userEditor2.setText( userEditor.getText().toLowerCase() );
}
