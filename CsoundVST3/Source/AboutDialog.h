#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_graphics/juce_graphics.h>
#include <fstream>
#include <sstream>

class AboutDialog : public juce::Component
{
public:
    AboutDialog()
    {
        // Load the image from BinaryData
        auto imageInputStream = std::make_unique<juce::MemoryInputStream>(BinaryData::angel_concert_png, BinaryData::angel_concert_pngSize, false);
        auto loadedImage = juce::ImageFileFormat::loadFrom(*imageInputStream);

        if (loadedImage.isValid())
        {
            appIconComponent.setImage(loadedImage);
            ///appIconComponent.setBounds(0, 0, 200, 200); // Set initial size
            addAndMakeVisible(appIconComponent);
        }
        // Set up app information label
        juce::String build_time = juce::String("Build Date: ") + __DATE__ + " Time: " + __TIME__;
        appInfoLabel.setText("This is CsoundVST3\nVersion 1.0\n" + build_time + "\n(c) 2024 Irreducible Productions", juce::dontSendNotification);
        auto readmeContent = juce::String(BinaryData::README_md, BinaryData::README_mdSize);

        appInfoLabel.setJustificationType(juce::Justification::topLeft);
        appInfoLabel.setFont(juce::Font(16.0f));
        appInfoLabel.setBorderSize(juce::BorderSize<int>(15));
        addAndMakeVisible(appInfoLabel);
        
        // Set up the TextEditor to display the content
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
        // Split the top into two sections: about and icon
        auto topSection = bounds.removeFromTop(120); // Adjust the height of the top section as needed
        auto aboutArea = topSection.removeFromLeft(proportionOfWidth(0.7f));
        appInfoLabel.setBounds(aboutArea);
        auto iconArea = topSection; // Remaining space on the right
        appIconComponent.setBounds(iconArea);
        // Remaining space is for the text editor
        textEditor.setBounds(bounds.reduced(border));
    }

private:
    juce::TextEditor textEditor;
    ///juce::WebBrowserComponent textEditor;
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
    options.dialogTitle = "About CsoundVST3";
    options.dialogBackgroundColour = juce::Colours::darkgrey;
    options.escapeKeyTriggersCloseButton = true;
    options.useNativeTitleBar = true;
    options.resizable = true;

    // Show the dialog
    options.launchAsync();
}

