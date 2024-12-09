/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "csound_threaded.hpp"
#include "atomic_queue/atomic_queue.h"

#include <iostream>
#include <numeric> // For std::accumulate

class AudioFifo : public atomic_queue::AtomicQueue<double, 16384>
{
    AudioFifo()
    {
    }

    std::atomic<int> producer_frames_to_wait;
    std::mutex producer_mutex;
    std::condition_variable producer_condition;
    std::atomic<int> consumer_frames_to_wait;
    std::mutex consumer_mutex;
    std::condition_variable consumer_condition;
};

class CsoundVST3AudioProcessor  : public juce::AudioProcessor, public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    CsoundVST3AudioProcessor();
    ~CsoundVST3AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
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
    
    void play();
    void stop();

    Csound csound;
    bool csoundIsPlaying = false;
    std::function<void(const juce::String&)> messageCallback;
    juce::String csd;
    juce::PluginHostType plugin_host_type;

private:
    static constexpr double inputScale = 32767.0;
    static constexpr double outputScale = (1.0 / 32767.0);
    double iodbfs;
    
    int host_input_channels;
    int host_output_channels;
    int host_channels;
    int csound_input_channels;
    int csound_output_channels;
    
    // These are valid only during processBlock.
    int csound_frames;
    int csound_frame;
    int csound_frame_end;
    int64_t host_frame;
    int64_t host_block_frame;
    int host_audio_buffer_frame;
    int64_t host_prior_frame;
    juce::AudioBuffer<float> plugin_audio_input_buffer;
    juce::MidiBuffer plugin_midi_input_buffer;
    juce::MidiBuffer plugin_midi_output_buffer;
    
    // The host pushes its AudioBuffer samples onto this.
    //AudioFifo audio_input_fifo;
    // The plugin waits for its input block to fill,
    // then pushes its output onto this. The host then pops from this until
    // its AudioBuffer is full of output.
    //AudioFifo audio_output_fifo;
    

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CsoundVST3AudioProcessor)
};
