/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CsoundTokeniser.h"
#include "csoundvst3_version.h"

class SearchAndReplaceDialog : public juce::Component,
                               private juce::Button::Listener
{
public:
    SearchAndReplaceDialog(juce::CodeEditorComponent& editor)
        : codeEditor(editor)
    {
        addAndMakeVisible(searchLabel);
        searchLabel.setText("Search:", juce::dontSendNotification);

        addAndMakeVisible(searchField);

        addAndMakeVisible(replaceLabel);
        replaceLabel.setText("Replace:", juce::dontSendNotification);

        addAndMakeVisible(replaceField);

        addAndMakeVisible(searchButton);
        searchButton.setButtonText("Search");
        searchButton.addListener(this);

        addAndMakeVisible(replaceButton);
        replaceButton.setButtonText("Replace");
        replaceButton.addListener(this);

        addAndMakeVisible(closeButton);
        closeButton.setButtonText("Close");
        closeButton.addListener(this);

        setSize(400, 150);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced(10);
        auto row = bounds.removeFromTop(30);

        searchLabel.setBounds(row.removeFromLeft(70));
        searchField.setBounds(row);

        row = bounds.removeFromTop(30);
        replaceLabel.setBounds(row.removeFromLeft(70));
        replaceField.setBounds(row);

        row = bounds.removeFromTop(30);
        searchButton.setBounds(row.removeFromLeft(120));
        replaceButton.setBounds(row.removeFromLeft(120));
        closeButton.setBounds(row);
    }

private:
    juce::CodeEditorComponent& codeEditor;

    juce::Label searchLabel, replaceLabel;
    juce::TextEditor searchField, replaceField;
    juce::TextButton searchButton, replaceButton, closeButton;

    void buttonClicked(juce::Button* button) override
    {
        auto& document = codeEditor.getDocument();
        auto searchText = searchField.getText();

        if (button == &searchButton)
        {
            auto start = codeEditor.getHighlightedRegion().getEnd(); // Start after current selection
            auto content = document.getAllContent();
            auto foundPos = content.indexOf(start, searchText);

            if (foundPos != -1)
            {
                codeEditor.setHighlightedRegion({ foundPos, foundPos + searchText.length() });
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                                  "Search", "Text not found.");
            }
        }
        else if (button == &replaceButton)
        {
            auto replaceText = replaceField.getText();
            auto start = codeEditor.getHighlightedRegion().getEnd();
            auto content = document.getAllContent();
            auto foundPos = content.indexOf(start, searchText);

            if (foundPos != -1)
            {
                document.deleteSection(foundPos, foundPos + searchText.length());
                document.insertText(foundPos, replaceText);
                codeEditor.setHighlightedRegion({ foundPos, foundPos + replaceText.length() });
            }
            else
            {
                juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                                  "Replace", "Text not found.");
            }
        }
        else if (button == &closeButton)
        {
            getParentComponent()->removeChildComponent(this);
        }
    }
};

class ContextMenuHandler : public juce::MouseListener
{
public:
    ContextMenuHandler(juce::CodeEditorComponent& editor)
        : codeEditor(editor)
    {
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        if (e.mods.isPopupMenu())
        {
            juce::PopupMenu menu;
            menu.addItem("Search and Replace", [&]()
            {
                auto* dialog = new juce::DialogWindow("Search and Replace", juce::Colours::lightgrey, true);
                dialog->setContentOwned(new SearchAndReplaceDialog(codeEditor), true);
                dialog->centreWithSize(400, 150);
                dialog->setVisible(true);
            });
            menu.showMenuAsync({});
           // menu.show(0, 0, 0, 0, nullptr);
        }
    }

private:
    juce::CodeEditorComponent& codeEditor;
};

inline void attachContextMenuToCodeEditor(juce::CodeEditorComponent& codeEditor)
{
    auto* handler = new ContextMenuHandler(codeEditor);
    codeEditor.addMouseListener(handler, true);
}

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
    std::unique_ptr<CsoundTokeniser> csd_code_tokeniser;
    std::unique_ptr<juce::CodeEditorComponent> codeEditor;
    std::unique_ptr<juce::CodeEditorComponent> messageLog;

private:
    void buttonClicked(juce::Button* button) override;
    juce::TooltipWindow tooltipWindow { this }; // Enable tooltips
    std::unique_ptr<juce::FileChooser> fileChooser;
    juce::File csd_file;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CsoundVST3AudioProcessorEditor)
};
