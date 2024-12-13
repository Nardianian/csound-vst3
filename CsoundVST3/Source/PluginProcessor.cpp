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
    // NB: some hosts don't cope very well if you tell them there are 0
    // programs, so this should be at least 1, even if you're not really
    // implementing programs.
    return 1;
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
    int bytes_read = 0;
    auto csound_host_data = csoundGetHostData(csound_);
    CsoundVST3AudioProcessor *processor = static_cast<CsoundVST3AudioProcessor *>(csound_host_data);
    int messages = 0;
    while (processor->midi_input_fifo.empty() == false)
    {
        char buffer[0x200];
        auto message = processor->midi_input_fifo.back();
        auto timestamp = message.frame - processor->host_block_begin;
        juce::MidiMessage midi_message(message.data, message.size, timestamp);
        if (message.frame >= processor->csound_block_begin)
        {
            auto data = message.data;
            auto size = message.size;
            if (!(0x80 <= data[0]) && (data[0] <= 0xEF))
            {
                processor->midi_input_fifo.pop_back();
                continue;
            }
            messages++;
#if defined(JUCE_DEBUG)
            std::snprintf(buffer, sizeof(buffer), "Input MIDI message %5d: timestamp: %7llu %s", messages, timestamp, midi_message.getDescription().toRawUTF8());
            DBG(buffer);
#endif
            for (int i = 0; i < size; ++i, ++bytes_read)
            {
                midi_buffer[bytes_read] = data[i];
            }
            processor->midi_input_fifo.pop_back();
        }
        else
        {
            break;
        }
    }
    return bytes_read;
}

/**
 * Called by Csound for each output MIDI message, to send  MIDI data to the host.
 */
int CsoundVST3AudioProcessor::midiWrite(CSOUND *csound_, void *userData, const unsigned char *midi_buffer, int midi_buffer_size)
{
    int result = 0;
    auto csound_host_data = csoundGetHostData(csound_);
    CsoundVST3AudioProcessor *processor = static_cast<CsoundVST3AudioProcessor *>(csound_host_data);
    MidiChannelMessage channel_message;
    channel_message.frame = processor->host_audio_buffer_frame;
    channel_message.size = midi_buffer_size;
    for (int i = 0; i < midi_buffer_size; ++i)
    {
        channel_message.data[i] = midi_buffer[i];
    }
    processor->midi_output_fifo.push_front(channel_message);
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
    host_frame = *optional_host_frame_index;
    // Here we see if the host is looping.
    if (host_frame < host_prior_frame)
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
                    csound_frame = 0;
                }
            }
            auto host_frame_seconds = *optional_host_frame_seconds;
            csound.SetScoreOffsetSeconds(host_frame_seconds);
        }
    }
    host_prior_frame = host_frame;
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
    auto input_device_count = midi_input_devices.size();
    for (auto device_index = 0; device_index < input_device_count; ++device_index)
    {
        auto device = midi_input_devices.getReference(device_index);
        juce::String message = juce::String::formatted("MIDI input device:  %3d %-40s (id: %s)", device_index, device.name.toUTF8(), device.identifier.toUTF8());
        message = message + "\n";
        csoundMessage(message);
    }
    auto midi_output_devices = juce::MidiOutput::getAvailableDevices();
    auto output_device_count = midi_output_devices.size();
    for (auto device_index = 0; device_index < output_device_count; ++device_index)
    {
        auto device = midi_output_devices.getReference(device_index);
        juce::String message = juce::String::formatted("MIDI output device: %3d %-40s (id: %s)", device_index, device.name.toUTF8(), device.identifier.toUTF8());
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
            csound_frame = 0;
        }
    }
    iodbfs = 1. / csound.Get0dBFS();
    host_input_channels  = getTotalNumInputChannels();
    host_output_channels = getTotalNumOutputChannels();
    host_channels = std::max(host_input_channels, host_output_channels);
    csound_input_channels = csound.GetNchnlsInput();
    csound_output_channels = csound.GetNchnls();
    host_frame = 0;
    host_prior_frame = 0;
    csound_frames = csound.GetKsmps();
    csound_block_begin = 0;
    csound_block_end = csound_block_begin + csound_frames;
    host_block_begin = 0;
    // Vital! Ensures that Csound's block is in sync with the host's block.
    //csound_frame = csound_frames;
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
    // TODO: the following is a hack, better try something else.
    auto host_description = plugin_host_type.getHostDescription();
    DBG("Host description: " << host_description);
    auto host = juce::String::formatted("Host: %s\n", host_description);
    csoundMessage(host.toUTF8());
    if (plugin_host_type.type == juce::PluginHostType::UnknownHost)
    {
        suspendProcessing(true);
    }
    else
    {
        csoundIsPlaying = true;
        suspendProcessing(false);
   }
    csoundMessage("Ready to play.\n");
 }

/**
 * Calls csoundPerformKsmps to do the actual processing.
 *
 * The number of input channels may not equal the number of output channels.
 * The number of frames in the buffer may not be the same as Csound's ksmps,
 * and may not be the same on every call.
 *
 * Input data in the buffers is replaced by output data, or cleared.
 
 I am going to change the implementation to use FIFOs. For this single-
 threaded use, std::deque is suitablre as peeking is necessary.
 In the processBlock calls, the incoming buffers will have their data moved into:
 plugin_midi_input_queue
 plugin_audio_input_queue
 Csound will wait until there is enough data in audio_input_queue to fill spout.
 Csound will then call performKsmps. This can happen at any time during processBlock.
 spout will be enqueued to plugin_audio_output.
 plugin_midi_output will be enqueued to plugin_midi_output.
 processBlock will then clear host_midi_buffer and push plugin_midi_output into it.
 processBlock will also dequeue plugin_audio_output into host_audio_buffer.
 When host_audio_buffer is full, processBlock will return.
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
        host_block_frame = *optional_time_in_samples;
    }
    //if (play_head_position->getIsPlaying() == false)
    //{
    //    return;
    //}
    synchronizeScore(play_head_position);
    juce::ScopedNoDenormals noDenormals;
    auto host_audio_buffer_frames = host_audio_buffer.getNumSamples();
    host_block_begin = host_frame;
    host_block_end = host_block_begin + host_audio_buffer_frames;
    // Write audio for Csound to process into this buffer.
    auto spin = csound.GetSpin();
    // Read audio that Csound has processed from this buffer.
    auto spout = csound.GetSpout();
    if (spout == nullptr)
    {
        csoundMessage("Null spout...\n");
        return;
    }
    auto host_audio_input_buffer = getBusBuffer(host_audio_buffer, true, 0);
    auto host_audio_output_buffer = getBusBuffer(host_audio_buffer, false, 0);
    
    // Csound's spin and spout buffers are indexed [frame][channel].
    // The host audio buffer is indexed [channel][frame].
    // As far as I can tell, both `getWritePointer` and `setSample` mark the
    // host audio buffer as not clear, and have the same addresses for the
    // same elements. The host buffer channel count is the greater of inputs +
    // side chains and outputs. Input channels are followed by side chain
    // channels. Output channels overlap inputs and possibly side chains.
    // (CsoundVST3 does not use side chains.) The getBusBuffer function
    // gathers the appropriate channel pointers into a channel array for
    // the indicated buffer, but care must be used to respect the outputs
    // overlapping other channels.
        
    // Push all inputs onto FIFOs.
    int messages = 0;
    for (const auto metadata : host_midi_buffer)
    {
        auto message = metadata.getMessage();
        auto timestamp = metadata.samplePosition;
        if (metadata.numBytes <= 4)
        {
            MidiChannelMessage channel_message;
            channel_message.frame = host_block_begin + timestamp;
            channel_message.size = metadata.numBytes;
            for (int i = 0; i < channel_message.size; ++i)
            {
                channel_message.data[i] = metadata.data[i];
            }
            midi_input_fifo.push_front(channel_message);
        }
        char buffer[0x200];
        messages++;
#if defined(JUCE_DEBUG)
        std::snprintf(buffer, sizeof(buffer), "Host  MIDI message %5d: buffer frame:  %7d %s", messages, timestamp, message.getDescription().toRawUTF8());
        DBG(buffer);
#endif
    }
    host_midi_buffer.clear();
    for (int host_audio_buffer_frame = 0; host_audio_buffer_frame < host_audio_buffer_frames; ++host_audio_buffer_frame)
    {
        for (int host_audio_buffer_channel = 0; host_audio_buffer_channel < host_input_channels; ++host_audio_buffer_channel)
        {
            auto sample = host_audio_buffer.getSample(host_audio_buffer_channel, host_audio_buffer_frame);
            audio_input_fifo.push_front(sample);
        }
    }
    host_audio_buffer.clear();
    // FIFOs being loaded, now process.
    // The host block pops audio input...
    for (host_audio_buffer_frame = 0; host_audio_buffer_frame < host_audio_buffer_frames; )
    {
        int channel_index;
        for (channel_index = 0; channel_index < host_input_channels; channel_index++)
        {
            float sample = float(iodbfs * audio_input_fifo.back());
            {
                spin[(csound_frame * csound_input_channels) + channel_index] = double(sample);
            }
            audio_input_fifo.pop_back();
        }
        csound_frame++;
        // ...until spin is full...
        if (csound_frame >= csound_frames)
        {
            csound_frame = 0;
            csound.PerformKsmps();
            // ...then the Csound block pushes audio output.
            // Thus the host block and the Csound block need not be the same
            // size, and the FIFOs keep samples aligned.
            for (int csound_frame = 0; csound_frame < csound_frames; ++csound_frame)
            {
                for (int csound_output_channel = 0; csound_output_channel < csound_output_channels; ++csound_output_channel)
                {
                    auto sample = spout[(csound_frame * csound_output_channels) + csound_output_channel];
                    audio_output_fifo.push_front(sample);
                }
            }
        }
    }
    // Processing the host block being completed,
    // now pop from the output FIFOs until JUCE outputs are full.
    while (true)
    {
        if (midi_output_fifo.empty() == true)
        {
            break;
        }
        auto message = midi_output_fifo.back();
        if (message.frame >= host_block_begin)
        {
            host_midi_buffer.addEvent(message.data, message.size, int(message.frame - host_block_begin));
            midi_output_fifo.pop_back();
        }
        else
        {
            break;
        }
    }
    for (int host_audio_buffer_frame = 0; host_audio_buffer_frame < host_audio_buffer_frames; ++host_audio_buffer_frame)
    {
        for (int host_output_channel = 0; host_output_channel < host_output_channels; ++host_output_channel)
        {
            auto sample = audio_output_fifo.back();
            audio_output_fifo.pop_back();
            host_audio_buffer.setSample(host_audio_buffer_frame, host_output_channel, sample);
        }
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

void CsoundVST3AudioProcessor::play()
{
    auto frames_per_second = getSampleRate();
    auto frame_size = getBlockSize();
    suspendProcessing(true);
    prepareToPlay(frames_per_second, frame_size);
    suspendProcessing(false);
}

void CsoundVST3AudioProcessor::stop()
{
    suspendProcessing(true);
    csoundIsPlaying = false;
    csound.Stop();
    csound.Cleanup();
    csound.Reset();
}


