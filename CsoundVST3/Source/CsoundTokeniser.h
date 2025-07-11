#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>

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
