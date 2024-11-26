/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class CsoundVST3AudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener
{
public:
    CsoundVST3AudioProcessorEditor (CsoundVST3AudioProcessor&);
    ~CsoundVST3AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CsoundVST3AudioProcessor& audioProcessor;
    juce::CodeDocument codeDocument;

    // Menu bar buttons
    juce::TextButton openButton{"Open"};
    juce::TextButton saveButton{"Save"};
    juce::TextButton saveAsButton{"Save as..."};
    juce::TextButton playButton{"Play"};
    juce::TextButton stopButton{"Stop"};
    juce::TextButton aboutButton{"About"};

    // Components
    juce::Label statusBar;
    juce::StretchableLayoutManager verticalLayout;
    juce::StretchableLayoutResizerBar divider;
    juce::CodeEditorComponent codeEditor;
    juce::TextEditor messageLog;

    void buttonClicked(juce::Button* button) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CsoundVST3AudioProcessorEditor)
};
