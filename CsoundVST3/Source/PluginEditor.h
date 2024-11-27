/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class CsoundVST3AudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Button::Listener
{
public:
    CsoundVST3AudioProcessorEditor (CsoundVST3AudioProcessor&);
    ~CsoundVST3AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CsoundVST3AudioProcessor& audioProcessor;
    juce::CodeDocument codeDocument;

    juce::TextButton openButton{"Load..."};
    juce::TextButton saveButton{"Save"};
    juce::TextButton saveAsButton{"Save as..."};
    juce::TextButton playButton{"Play"};
    juce::TextButton stopButton{"Stop"};
    juce::TextButton aboutButton{"About"};

    juce::Label statusBar;
    juce::StretchableLayoutManager verticalLayout;
    juce::StretchableLayoutResizerBar divider;
    juce::CodeEditorComponent codeEditor;
    juce::TextEditor messageLog;

    void buttonClicked(juce::Button* button) override;
    
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File csd_file;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CsoundVST3AudioProcessorEditor)
};
