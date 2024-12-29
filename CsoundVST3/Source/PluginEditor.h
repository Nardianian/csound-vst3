/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "csoundvst3_version.h"

class CsoundVST3AudioProcessorEditor  : public juce::AudioProcessorEditor,
public juce::Button::Listener,
public juce::ChangeListener,
public juce::Timer
// public juce::TooltipWindow
{
public:
    CsoundVST3AudioProcessorEditor (CsoundVST3AudioProcessor&);
    ~CsoundVST3AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    void changeListenerCallback(juce::ChangeBroadcaster*) override;
    void timerCallback() override;
private:
    CsoundVST3AudioProcessor& audioProcessor;
    juce::CodeDocument csd_document;
    juce::CodeDocument messages_document;

    juce::TextButton openButton{"Open..."};
    juce::TextButton saveButton{"Save"};
    juce::TextButton saveAsButton{"Save as..."};
    juce::TextButton playButton{"Play"};
    juce::TextButton stopButton{"Stop"};
    juce::TextButton aboutButton{"About"};

    juce::Label statusBar;
    juce::StretchableLayoutManager verticalLayout;
    juce::StretchableLayoutResizerBar divider;
    
    
public:
    juce::CodeEditorComponent codeEditor;
    juce::CodeEditorComponent messageLog;

private:
    void buttonClicked(juce::Button* button) override;
    

    juce::TooltipWindow tooltipWindow { this }; // Enable tooltips

    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File csd_file;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CsoundVST3AudioProcessorEditor)
};
