/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CsoundVST3AudioProcessor::CsoundVST3AudioProcessor()
     : AudioProcessor (BusesProperties()
                        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                        )
{
}

CsoundVST3AudioProcessor::~CsoundVST3AudioProcessor()
{
}

//==============================================================================
const juce::String CsoundVST3AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CsoundVST3AudioProcessor::acceptsMidi() const
{
    return true;
}

bool CsoundVST3AudioProcessor::producesMidi() const
{
    return true;
}

bool CsoundVST3AudioProcessor::isMidiEffect() const
{
     return false;
}

double CsoundVST3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CsoundVST3AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CsoundVST3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void CsoundVST3AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CsoundVST3AudioProcessor::getProgramName (int index)
{
    return {};
}

void CsoundVST3AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void CsoundVST3AudioProcessor::csoundMessage(const juce::String message)
{
    if (messageCallback)
    {
        messageCallback(message);
    }
    sendChangeMessage();
    DBG(message);
}

void CsoundVST3AudioProcessor::csoundMessageCallback_(CSOUND *csound, int level, const char *format, va_list valist)
{
    auto host_data = csoundGetHostData(csound);
    auto processor = static_cast<CsoundVST3AudioProcessor *>(host_data);
    char buffer[0x2000];
    std::vsnprintf(&buffer[0], sizeof(buffer), format, valist);
    processor->csoundMessage(buffer);
}

void CsoundVST3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool CsoundVST3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Check output bus layout
    const auto& mainOutput = layouts.getMainOutputChannelSet();
    if (mainOutput != juce::AudioChannelSet::mono() &&
        mainOutput != juce::AudioChannelSet::stereo())
        return false; // Only mono or stereo output supported

    // Check input bus layout
    const auto& mainInput = layouts.getMainInputChannelSet();
    if (mainInput.isDisabled() || // Allow no input
        mainInput == juce::AudioChannelSet::mono() ||
        mainInput == juce::AudioChannelSet::stereo())
        return true; // Input can be mono, stereo, or none

    return false; // Other configurations not supported
}

int CsoundVST3AudioProcessor::midiDeviceOpen(CSOUND *csound_, void **user_data,
                          const char *devName)
{
    auto csound_host_data = csoundGetHostData(csound_);
    /// CsoundVST3AudioProcessor *processor = static_cast<CsoundVST3AudioProcessor *>(csound_host_data);
    *user_data = (void *)csound_host_data;
    return 0;
}

int CsoundVST3AudioProcessor::midiDeviceClose(CSOUND *csound_, void *user_data)
{
    /// auto csound_host_data = csoundGetHostData(csound_);
    /// CsoundVST3AudioProcessor *processor = static_cast<CsoundVST3AudioProcessor *>(csound_host_data);
    return 0;
}
/**
 * Called by Csound at every kperiod to receive incoming MIDI messages from
 * the host. Only MIDI channel messages are handled. Timing precision is the
 * audio processing block size, so accurate timing requires ksmps of 128 or so.
 */
int CsoundVST3AudioProcessor::midiRead(CSOUND *csound_, void *userData, unsigned char *midi_buffer, int midi_buffer_size)
{
    int midi_input_buffer_index = 0;
    auto csound_host_data = csoundGetHostData(csound_);
    CsoundVST3AudioProcessor *processor = static_cast<CsoundVST3AudioProcessor *>(csound_host_data);
    for (const auto metadata : processor->midi_input_buffer)
    {
        auto data = metadata.data;
        auto size = metadata.numBytes;
        auto status = data[0];
        if (status >= 0x80 && status <= 0xEF)
        {
            for (int i = 0; i < size; ++i, ++midi_input_buffer_index)
            {
                midi_buffer[midi_input_buffer_index] = data[i];
            }
        }
        else
        {
            DBG("Not a MIDI channel message.");
        }
    }
    processor->midi_input_buffer.clear();
    return midi_input_buffer_index;
}

/**
 * Called by Csound for each output MIDI message, to send  MIDI data to the host.
 */
int CsoundVST3AudioProcessor::midiWrite(CSOUND *csound_, void *userData, const unsigned char *midi_buffer, int midi_buffer_size)
{
    int result = 0;
    auto csound_host_data = csoundGetHostData(csound_);
    CsoundVST3AudioProcessor *processor = static_cast<CsoundVST3AudioProcessor *>(csound_host_data);
    // I think the frame index should really be relative to the start of the current block. If so this should be 0 here.
    /// processor->midi_output_buffer.addEvent(midi_buffer, midi_buffer_size, static_cast<int>(processor->host_frame_index));
    processor->midi_output_buffer.addEvent(midi_buffer, midi_buffer_size, 0);
    return result;
}

/**
 * Ensures that Csound's score time tracks the host's performance time. This
 * causes Csound to loop in its own score along with the host.
 *
 * This function is called from processBlock before that function processes any
 * samples. Therefore, the times are always aligned with the start of the block.
 *
 * TODO: What if track has nonzero start -- is that possible?
 */
void CsoundVST3AudioProcessor::synchronizeScore(juce::Optional<juce::AudioPlayHead::PositionInfo> &play_head_position)
{
    /// DBG("Synchronizing score...\n");
    if (play_head_position->getIsPlaying() == false)
    {
        return;
    }
    juce::Optional<int64_t> optional_host_frame_index = play_head_position->getTimeInSamples();
    if (optional_host_frame_index == false)
    {
        return;
    }
    host_frame_index = *optional_host_frame_index;
    // Here we see if the host is looping.
    if (host_frame_index < host_prior_frame_index)
    {
        juce::Optional<double> optional_host_frame_seconds = play_head_position->getTimeInSeconds();
        if (optional_host_frame_seconds == true)
        {
            DBG("Looping...");
            // We recompile to ensure that Csound's alwayson opcode works when
            // looping.
            if (csd.length()  > 0) {
                const char* csd_text = strdup(csd.toRawUTF8());
                if (csd_text) {
                    auto result = csound.CompileCsdText(csd_text);
                    std::free((void *)csd_text);
                    result = csound.Start();
                    csound_frame_index = 0;
                }
            }
            auto host_frame_seconds = *optional_host_frame_seconds;
            csound.SetScoreOffsetSeconds(host_frame_seconds);
        }
    }
    host_prior_frame_index = host_frame_index;
}

/**
 * Compiles the csd and starts Csound.
 */
void CsoundVST3AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::MessageManagerLock lock;
    auto editor = getActiveEditor();
    if (editor)
    {
        auto pluginEditor = reinterpret_cast<CsoundVST3AudioProcessorEditor *>(editor);
        pluginEditor->messageLog.loadContent("");

    }
    csoundMessage("Preparing to play...\r\n");
    if (csoundIsPlaying == true)
    {
        csoundIsPlaying = false;
        csound.Stop();
        csound.Cleanup();
        csound.Reset();
    }
    // (Re-)set the Csound message callback.
    csound.SetHostData(this);
    csound.SetMessageCallback(csoundMessageCallback_);
    // Set up connections with the host.
    csound.SetHostImplementedMIDIIO(1);
    csound.SetHostImplementedAudioIO(1, 0);
    csound.SetExternalMidiInOpenCallback(&CsoundVST3AudioProcessor::midiDeviceOpen);
    csound.SetExternalMidiReadCallback(&CsoundVST3AudioProcessor::midiRead);
    csound.SetExternalMidiInCloseCallback(&CsoundVST3AudioProcessor::midiDeviceClose);
    csound.SetExternalMidiOutOpenCallback(&CsoundVST3AudioProcessor::midiDeviceOpen);
    csound.SetExternalMidiWriteCallback(&CsoundVST3AudioProcessor::midiWrite);
    csound.SetExternalMidiOutCloseCallback(&CsoundVST3AudioProcessor::midiDeviceClose);
    auto midi_input_devices = juce::MidiInput::getAvailableDevices();
    auto device_count = midi_input_devices.size();
    for (auto device_index = 0; device_index < device_count; ++device_index)
    {
        auto device = midi_input_devices.getReference(device_index);
        juce::String message = juce::String::formatted("MIDI input device number: %3d name: %-40s identifier: %s", device_index, device.name.toUTF8(), device.identifier.toUTF8());
        /// DBG(message);
        message = message + "\n";
        csoundMessage(message);
    }
    auto initial_delay_frames = csound.GetKsmps();
    setLatencySamples(initial_delay_frames);
    /*
     Message level for standard (terminal) output. Takes the sum of any of the following values:
     1 = note amplitude messages
     2 = samples out of range message
     4 = warning messages
     128 = print benchmark information
     1024 = suppress deprecated messages
     And exactly one of these to select note amplitude format:
     0 = raw amplitudes, no colours
     32 = dB, no colors
     64 = dB, out of range highlighted with red
     96 = dB, all colors
     256 = raw, out of range highlighted with red
     512 = raw, all colours
     All messages can be suppressed by using message level 16.
     */
    // I suggest: 1 + 2 + 128 + 32 = 162.
    char buffer[0x200];
    // Overrride the csd's sample rate and block size with the host's.
    int host_sample_rate = getSampleRate();
    snprintf(buffer, sizeof(buffer), "--sample-rate=%d", host_sample_rate);
    csound.SetOption(buffer);
    int host_frames_per_block = getBlockSize();
    snprintf(buffer, sizeof(buffer), "--ksmps=%d", host_frames_per_block);
    csound.SetOption(buffer);
    /// snprintf(buffer, sizeof(buffer), "-+msg_color=0");
    /// csound.SetOption(buffer);
    // If there is a csd, compile it.
    if (csd.length()  > 0) {
        const char* csd_text = strdup(csd.toRawUTF8());
        if (csd_text) {
            auto result = csound.CompileCsdText(csd_text);
            std::free((void *)csd_text);
            result = csound.Start();
            csound_frame_index = 0;
        }
    }
    iodbfs = 1. / csound.Get0dBFS();
    host_input_channels  = getTotalNumInputChannels();
    host_output_channels = getTotalNumOutputChannels();
    host_channels = std::max(host_input_channels, host_output_channels);
    csound_input_channels = csound.GetNchnlsInput();
    csound_output_channels = csound.GetNchnls();
    host_frame_index = 0;
    host_prior_frame_index = 0;
    csound_frames = csound.GetKsmps();
    csound_frame_index = 0;
    const int host_input_busses = getBusCount(true);
    const int host_output_busses = getBusCount(false);
    csoundMessage(juce::String::formatted("Host input busses:      %3d\n", host_input_busses));
    csoundMessage(juce::String::formatted("host output busses:     %3d\n", host_output_busses));
    csoundMessage(juce::String::formatted("Host input channels:    %3d\n", host_input_channels));
    csoundMessage(juce::String::formatted("Csound input channels:  %3d\n", csound_input_channels));
    csoundMessage(juce::String::formatted("Csound output channels: %3d\n", csound_output_channels));
    csoundMessage(juce::String::formatted("Host output channels:   %3d\n", host_output_channels));
    csoundMessage(juce::String::formatted("Host channels:          %3d\n", host_channels));
    csoundMessage(juce::String::formatted("Csound ksmps:           %3d\n", csound_frames));
    csoundIsPlaying = true;
    csoundMessage("Ready to play.\r\n");
}

/**
 * Calls csoundPerformKsmps to do the actual processing.
 *
 * The number of input channels may not equal the number of output channels.
 * The number of frames in the buffer may not be the same as Csound's ksmps,
 * and may not be the same on every call.
 *
 * Input data in the buffers is replaced by output data, or zeroed.
 *
 * TODO: Does this code depend on Csound's ksmps equalling host's block size?
 */
void CsoundVST3AudioProcessor::processBlock (juce::AudioBuffer<float>& host_audio_buffer, juce::MidiBuffer& host_midi_buffer)
{
    auto play_head = getPlayHead();
    auto play_head_position = play_head->getPosition();
    if (csoundIsPlaying == false)
    {
        host_audio_buffer.clear();
        host_midi_buffer.clear();
        return;
    }
    auto optional_time_in_samples = play_head_position->getTimeInSamples();
    if (optional_time_in_samples)
    {
        current_block_frame = *optional_time_in_samples;
    }
    //if (play_head_position->getIsPlaying() == false)
    //{
    //    return;
    //}
    synchronizeScore(play_head_position);
    juce::ScopedNoDenormals noDenormals;
    auto host_frames = host_audio_buffer.getNumSamples();
    auto spin = csound.GetSpin();
    auto spout = csound.GetSpout();
    if (spout == nullptr)
    {
        csoundMessage("Null spout...\n");
        return;
    }
    auto host_audio_input_buffer = getBusBuffer(host_audio_buffer, true, 0);
    plugin_audio_input_buffer.makeCopyOf(host_audio_input_buffer);
    auto host_audio_output_buffer = getBusBuffer(host_audio_buffer, false, 0);
    host_audio_buffer.clear();
    midi_input_buffer.clear();
    midi_input_buffer.addEvents(host_midi_buffer, 0, host_midi_buffer.getNumEvents(), 0);
    // Csound's spin and spout buffers are indexed [frame][channel].
    // The host audio buffer is indexed [channel][frame].
    // As far as I can tell, both `getWritePointer` and `setSample` mark the
    // buffer as not clear, and have the same address for the same element.
    // The host buffer channel count is inputs + [side chains + ] outputs, but
    // inputs and outputs are overlaid in the buffer, so some channels are
    // re-used. The getBusBuffer function selects the appropriate channel
    // pointers for the indicated buffer.
    for (auto audio_buffer_frame = 0; audio_buffer_frame < host_frames; )
    {
        csound_frame_index++;
        if (csound_frame_index >= csound_frames)
        {
            DBG(juce::String::formatted("at performKsmps processed: buffer frames: %7d host frames: %7d csound frames: %7d", audio_buffer_frame, host_frame_index, csound_frame_index));
            csound_frame_index = 0;
            csound.PerformKsmps();
        }
        int channel_index;
        for (channel_index = 0; channel_index < host_input_channels; channel_index++)
        {
            float sample = float(iodbfs * plugin_audio_input_buffer.getSample(channel_index, audio_buffer_frame));
            if (channel_index < csound_input_channels)
            {
                spin[(csound_frame_index * csound_input_channels) + channel_index] = double(sample);
            }
         }
        for (channel_index = 0; channel_index < host_output_channels; channel_index++)
        {
            if (channel_index < csound_output_channels)
            {
                float sample = float(iodbfs * spout[(csound_frame_index * host_output_channels) + csound_frame_index]);
                host_audio_output_buffer.setSample(channel_index, audio_buffer_frame, sample);
                spout[(csound_frame_index * csound_output_channels) + channel_index] = double(0);
            }
        }
        audio_buffer_frame++;
        host_frame_index++;
    }
    // Copy Csound's output MIDI buffer to the host's MIDI (output) buffer.
    if (midi_output_buffer.isEmpty() == false)
    {
        host_midi_buffer.clear();
        host_midi_buffer.swapWith(midi_output_buffer);
    }
    else
    {
        host_midi_buffer.clear();
    }
}

//==============================================================================
bool CsoundVST3AudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* CsoundVST3AudioProcessor::createEditor()
{
    return new CsoundVST3AudioProcessorEditor (*this);
}

//==============================================================================
void CsoundVST3AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ValueTree state("CsoundVstState");
    state.setProperty("csd", csd, nullptr);
    juce::MemoryOutputStream stream(destData, false);
    state.writeToStream(stream);
}

void CsoundVST3AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MessageManagerLock lock;
    juce::ValueTree state = juce::ValueTree::readFromData(data, static_cast<size_t>(sizeInBytes));
    if (state.isValid() && state.hasType("CsoundVstState"))
    {
        csd = state.getProperty("csd", "").toString();
        auto editor = getActiveEditor();
        if (editor) {
            auto pluginEditor = reinterpret_cast<CsoundVST3AudioProcessorEditor *>(editor);
            pluginEditor->codeEditor.loadContent(csd);
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CsoundVST3AudioProcessor();
}


