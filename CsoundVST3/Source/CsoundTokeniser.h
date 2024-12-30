#pragma once
#include <JuceHeader.h>

class CsoundTokeniser : public juce::CodeTokeniser
{
public:
    CsoundTokeniser();
    ~CsoundTokeniser() override = default;

    int readNextToken(juce::CodeDocument::Iterator& source) override;
    juce::CodeEditorComponent::ColourScheme getDefaultColourScheme() override;
    juce::StringArray virtual getTokenTypes();
    juce::CodeEditorComponent::ColourScheme csd_color_scheme;

private:
    enum TokenType
    {
        CsoundKeyword = 0,
        /// CsoundOpcode,
        CsoundNumber,
        CsoundString,
        CsoundComment,
        CsoundIdentifier,
        CsoundOther,
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CsoundTokeniser)
};
