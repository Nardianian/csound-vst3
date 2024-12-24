#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <fstream>
#include <sstream>

class AboutDialog : public juce::Component
{
public:
    AboutDialog()
    {
        auto imageInputStream = std::make_unique<juce::MemoryInputStream>(BinaryData::angel_concert_png, BinaryData::angel_concert_pngSize, false);
        auto loadedImage = juce::ImageFileFormat::loadFrom(*imageInputStream);

        if (loadedImage.isValid())
        {
            appIconComponent.setImage(loadedImage);
            addAndMakeVisible(appIconComponent);
        }
        juce::String build_time = juce::String("Build Date: ") + __DATE__ + " Time: " + __TIME__;
        appInfoLabel.setText("This is CsoundVST3\nVersion 1.0\n" + build_time + "\n(c) 2024 Irreducible Productions", juce::dontSendNotification);
        auto readmeContent = juce::String(BinaryData::README_md, BinaryData::README_mdSize);

        appInfoLabel.setJustificationType(juce::Justification::topLeft);
        appInfoLabel.setBorderSize(juce::BorderSize<int>(15));
        addAndMakeVisible(appInfoLabel);
        
        textEditor.setMultiLine(true);
        textEditor.setReadOnly(true);
        textEditor.setCaretVisible(false);
        textEditor.setScrollbarsShown(true);
        textEditor.setText(readmeContent);
        addAndMakeVisible(textEditor);
    }

    void resized() override
    {
        const int border = 15;
        auto bounds = getLocalBounds().reduced(border);
        // Split the top into two sections: about and icon.
        auto topSection = bounds.removeFromTop(120);
        auto aboutArea = topSection.removeFromLeft(proportionOfWidth(0.7f));
        appInfoLabel.setBounds(aboutArea);
        auto iconArea = topSection; 
        appIconComponent.setBounds(iconArea);
        // Remaining space is for the text editor.
        textEditor.setBounds(bounds.reduced(border));
    }

private:
    juce::TextEditor textEditor;
    juce::Label appInfoLabel;
    juce::ImageComponent appIconComponent;
    juce::Image appIcon;
    juce::String readmeText;
    juce::Label readmeLabel;
};

void showAboutDialog(juce::Component* parent)
{
    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(new AboutDialog());
    options.content->setSize(600, 400);
    options.dialogTitle = "About CsoundVST3";
    options.dialogBackgroundColour = juce::Colours::darkgrey;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = true;
    options.launchAsync();
}

