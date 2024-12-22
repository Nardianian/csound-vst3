#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <fstream>
#include <sstream>

class AboutDialog : public juce::Component
{
public:
    AboutDialog()
    {
        // Set up app information label
        juce::String build_time = juce::String("Build Date: ") + __DATE__ + " Time: " + __TIME__;
        appInfoLabel.setText("This is CsoundVST3\nVersion 1.0\n" + build_time + "\n(c) 2024 Irreducible Productions", juce::dontSendNotification);
        auto readmeContent = "readme content"; //juce::String(BinaryData::README_md, BinaryData::README_mdSize);

        appInfoLabel.setJustificationType(juce::Justification::topLeft);
        appInfoLabel.setFont(juce::Font(16.0f));
        appInfoLabel.setBorderSize(juce::BorderSize<int>(15));
        addAndMakeVisible(appInfoLabel);

        // Load icon image
        auto imagePath = juce::File::getCurrentWorkingDirectory().getChildFile("icon.png");
        appIcon = juce::ImageFileFormat::loadFrom(imagePath);
        if (appIcon.isValid())
        {
            appIconComponent.setImage(appIcon);
            appIconComponent.setBounds(0, 0, 100, 100); // Set initial size
            addAndMakeVisible(appIconComponent);
        }

        // Load README.md text
        auto readmePath = juce::File::getCurrentWorkingDirectory().getChildFile("README.md");
        if (readmePath.existsAsFile())
        {
            juce::FileInputStream stream(readmePath);
            if (stream.openedOk())
            {
                readmeText = stream.readEntireStreamAsString();
                readmeLabel.setText(readmeText, juce::dontSendNotification);
                readmeLabel.setFont(juce::Font(14.0f));
                readmeLabel.setJustificationType(juce::Justification::topLeft);
                readmeLabel.setColour(juce::Label::backgroundColourId, juce::Colours::lightgrey);
                readmeLabel.setColour(juce::Label::textColourId, juce::Colours::black);
                readmeLabel.setSize(400, 200); // Adjust based on expected README size
                readmeLabel.setBorderSize(juce::BorderSize<int>(5));
                addAndMakeVisible(readmeLabel);
            }
        }
        ///auto readmeContent = juce::String(BinaryData::README_md, BinaryData::README_mdSize);

        // Set up the TextEditor to display the content
        textEditor.setMultiLine(true);
        textEditor.setReadOnly(true);
        textEditor.setScrollbarsShown(true);
        textEditor.setText(readmeContent);

        addAndMakeVisible(textEditor);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto topArea = area.removeFromTop(100); // Reserve space for top row
        auto bounds = getLocalBounds().reduced(10);
        textEditor.setBounds(bounds.removeFromTop(bounds.getHeight() - 40));

        appInfoLabel.setBounds(topArea.removeFromLeft(getWidth() / 2));
        appIconComponent.setBounds(topArea); // Takes the right half of the top row

        readmeLabel.setBounds(area); // Remainder is for the README
    }

private:
    juce::TextEditor textEditor;
    juce::Label appInfoLabel;
    juce::ImageComponent appIconComponent;
    juce::Image appIcon;
    juce::String readmeText;
    juce::Label readmeLabel;
};

// A helper function to create and show the dialog
void showAboutDialog(juce::Component* parent)
{
    juce::DialogWindow::LaunchOptions options;
    options.content.setOwned(new AboutDialog());
    options.content->setSize(600, 400); // Adjust dialog size as needed
    options.dialogTitle = "About CsundVST3";
    options.dialogBackgroundColour = juce::Colours::darkgrey;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = true;

    // Show the dialog
    options.launchAsync();
}

