/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "csound_threaded.hpp"

//==============================================================================
/**
*/
class CsoundVST3AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    CsoundVST3AudioProcessor();
    ~CsoundVST3AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;
    void synchronizeScore();

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    static void csoundMessageCallback_(CSOUND *, int, const char *, va_list);
    void csoundMessage(const juce::String message);
    
    static int midiDeviceOpen(CSOUND *csound, void **userData,
                              const char *devName);
    static int midiDeviceClose(CSOUND *csound, void *userData);
    static int midiRead(CSOUND *csound, void *userData, unsigned char *buf, int nbytes);
    static int midiWrite(CSOUND *csound, void *userData, const unsigned char *buf, int nBytes);
    void synchronizeScore(juce::Optional<juce::AudioPlayHead::PositionInfo> &play_head_position);

    CsoundThreaded csound;
    juce::String csd;
    
private:
    static constexpr double inputScale = 32767.0;
    static constexpr double outputScale = (1.0 / 32767.0);
    
    int host_input_channels;
    int host_output_channels;
    int csound_input_channels;
    int csound_output_channels;
    int input_channels;
    int output_channels;

    // These five are valid only during processBlock.
    int csound_frame_index;
    int64_t host_frame_index;
    int64_t host_prior_frame_index;
    std::shared_ptr< juce::AudioBuffer<float> > audio_buffer;
    std::shared_ptr<juce::MidiBuffer> midi_buffer;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CsoundVST3AudioProcessor)
};
