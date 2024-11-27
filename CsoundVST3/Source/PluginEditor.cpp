/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "csound_threaded.hpp"


//==============================================================================
CsoundVST3AudioProcessorEditor::CsoundVST3AudioProcessorEditor (CsoundVST3AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
    divider(&verticalLayout, 1, false), codeEditor(codeDocument, nullptr)
{
    CsoundThreaded csound;
    // Menu Bar Buttons
    addAndMakeVisible(openButton);
    addAndMakeVisible(saveButton);
    addAndMakeVisible(saveAsButton);
    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(aboutButton);

    // Attach listeners
    openButton.addListener(this);
    saveButton.addListener(this);
    saveAsButton.addListener(this);
    playButton.addListener(this);
    stopButton.addListener(this);
    aboutButton.addListener(this);

    // Status Bar
    statusBar.setText("Ready", juce::dontSendNotification);
    statusBar.setJustificationType(juce::Justification::left);
    addAndMakeVisible(statusBar);

    // Code Editor
    addAndMakeVisible(codeEditor);
    codeEditor.setFont(juce::FontOptions("Courier",14.0f, juce::Font::plain));
    codeEditor.setReadOnly(false);
 
    // Message Log
    addAndMakeVisible(messageLog);
    messageLog.setReadOnly(true);

    // Vertical Layout
    verticalLayout.setItemLayout(0, -0.1, -0.9, -0.5); // Top window
    verticalLayout.setItemLayout(1, 8, 8, 8);          // Divider
    verticalLayout.setItemLayout(2, -0.1, -0.9, -0.5); // Bottom window

    addAndMakeVisible(divider);

    setSize(800, 600);
}

CsoundVST3AudioProcessorEditor::~CsoundVST3AudioProcessorEditor()
{
}

//==============================================================================
void CsoundVST3AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void CsoundVST3AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // Menu Bar
    auto menuBar = bounds.removeFromTop(30);
    openButton.setBounds(menuBar.removeFromLeft(80));
    saveButton.setBounds(menuBar.removeFromLeft(80));
    saveAsButton.setBounds(menuBar.removeFromLeft(100));
    playButton.setBounds(menuBar.removeFromLeft(80));
    stopButton.setBounds(menuBar.removeFromLeft(80));
    aboutButton.setBounds(menuBar.removeFromLeft(100));

    // Status Bar
    auto statusBarHeight = 20;
    statusBar.setBounds(bounds.removeFromBottom(statusBarHeight));

    juce::Component *components[] = {&codeEditor, &divider, &messageLog};
    verticalLayout.layOutComponents(components, 3, bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight(), true, true) ;
    setWantsKeyboardFocus(true);
 }

// Button Click Handler
void CsoundVST3AudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &openButton)
    {
        statusBar.setText("Open button clicked", juce::dontSendNotification);
    }
    else if (button == &saveButton)
    {
        statusBar.setText("Save button clicked", juce::dontSendNotification);
    }
    else if (button == &saveAsButton)
    {
        statusBar.setText("Save As button clicked", juce::dontSendNotification);
    }
    else if (button == &playButton)
    {
        statusBar.setText("Play button clicked", juce::dontSendNotification);
    }
    else if (button == &stopButton)
    {
        statusBar.setText("Stop button clicked", juce::dontSendNotification);
    }
    else if (button == &aboutButton)
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                               "About",
                                               "This is CsoundVST.vst3 by Michael Gogins. For help see:\n\nhttps://github.com/gogins/csound-vst3");
    }

}

