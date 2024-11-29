/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CsoundVST3AudioProcessor::CsoundVST3AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
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
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CsoundVST3AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CsoundVST3AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
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
    const juce::MessageManagerLock lock;
    auto editor = getActiveEditor();
    if (editor) {
        auto pluginEditor = reinterpret_cast<CsoundVST3AudioProcessorEditor *>(editor);
        pluginEditor->messageLog.moveCaretToEnd(false);
        pluginEditor->messageLog.insertTextAtCaret(message);
    }
}

void CsoundVST3AudioProcessor::csoundMessageCallback_(CSOUND *csound, int level, const char *format, va_list valist)
{
    auto host_data = csoundGetHostData(csound);
    auto processor = static_cast<CsoundVST3AudioProcessor *>(host_data);
    char buffer[0x2000];
    std::vsnprintf(&buffer[0], sizeof(buffer), format, valist);
    ((CsoundVST3AudioProcessor *)host_data)->csoundMessage(buffer);
}

/**
 * Compiles the csd and starts Csound.
 */
void CsoundVST3AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    auto editor = getActiveEditor();
    if (editor)
    {
        auto pluginEditor = reinterpret_cast<CsoundVST3AudioProcessorEditor *>(editor);
        pluginEditor->messageLog.loadContent("");

    }
    csoundMessage("Preparing to play...\r\n");
    csound.Stop();
    // (Re-)set the Csound message callback.
    csound.SetHostData(this);
    csound.SetMessageCallback(csoundMessageCallback_);
    // Set up connections with the host.
    csound.SetHostImplementedAudioIO(1, 0);
    csound.SetHostImplementedMIDIIO(1);
    csound.SetExternalMidiInOpenCallback(&CsoundVST3AudioProcessor::midiDeviceOpen);
    csound.SetExternalMidiReadCallback(&CsoundVST3AudioProcessor::midiRead);
    csound.SetExternalMidiInCloseCallback(&CsoundVST3AudioProcessor::midiDeviceClose);
    csound.SetExternalMidiOutOpenCallback(&CsoundVST3AudioProcessor::midiDeviceOpen);
    csound.SetExternalMidiWriteCallback(&CsoundVST3AudioProcessor::midiWrite);
    csound.SetExternalMidiOutCloseCallback(&CsoundVST3AudioProcessor::midiDeviceClose);
    auto initial_delay_frames = csound.GetKsmps();
    setLatencySamples(initial_delay_frames);
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
    host_frame_index = 0;
    host_prior_frame_index = 0;
    csoundMessage("Preparing to play.\r\n");
}

void CsoundVST3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CsoundVST3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

int CsoundVST3AudioProcessor::midiDeviceOpen(CSOUND *csound, void **userData,
                          const char *devName)
{
    return 1;
}

int CsoundVST3AudioProcessor::midiDeviceClose(CSOUND *csound, void *userData)
{
    return 1;
}
/**
 * Sends the processor's pending MIDI data, received from the host, to Csound
 * as a block of binary MIDI data.
 */
int CsoundVST3AudioProcessor::midiRead(CSOUND *csound_, void *userData, unsigned char *midi_input_buffer, int midi_input_buffer_size)
{
    int midi_input_buffer_index = 0;
    auto csound_host_data = csoundGetHostData(csound_);
    CsoundVST3AudioProcessor *processor = static_cast<CsoundVST3AudioProcessor *>(csound_host_data);
    auto &midi_buffer = *processor->midi_buffer;
    for (const juce::MidiMessageMetadata metadata : midi_buffer)
    {
        auto data = metadata.data;
        auto size = metadata.numBytes;
        for (int i = 0; i < size; ++i, ++midi_input_buffer_index)
        {
            midi_input_buffer[midi_input_buffer_index++] = data[i];
        }
    }
    return midi_input_buffer_index;
}

/**
 * Csound's MIDI driver calls this for each MIDI output channel message, and
 * the processor adds the  MIDI output buffer to the processor's midi_buffer.
 */
int CsoundVST3AudioProcessor::midiWrite(CSOUND *csound_, void *userData, const unsigned char *midi_output_buffer, int midi_output_buffer_size)
{
    int result = 0;
    auto csound_host_data = csoundGetHostData(csound_);
    CsoundVST3AudioProcessor *processor = static_cast<CsoundVST3AudioProcessor *>(csound_host_data);
    processor->midi_buffer->addEvent(midi_output_buffer, midi_output_buffer_size, static_cast<int>(processor->host_frame_index));
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
 * Calls csoundPerformKsmps to do the actual synthesis.
 *
 * The number of input channels may not equal the number of output channels.
 * The number of frames in the buffer may not be the same as Csound's ksmps,
 * and may not be the same on every call.
 *
 * Input data in the buffers is replaced by output data, or zeroed.
 */
void CsoundVST3AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    auto play_head = getPlayHead();
    auto play_head_position = play_head->getPosition();
    if (play_head_position->getIsPlaying() == false)
    {
        return;
    }
    if (csound.IsPlaying() == false)
    {
        return;
    }
    synchronizeScore(play_head_position);
    juce::ScopedNoDenormals noDenormals;
    audio_buffer = std::make_shared<juce::AudioBuffer<float>>(buffer);
    midi_buffer = std::make_shared<juce::MidiBuffer>(midiMessages);
    auto host_input_channels  = getTotalNumInputChannels();
    auto host_output_channels = getTotalNumOutputChannels();
    auto csound_input_channels = csound.GetNchnlsInput();
    auto csound_output_channels = csound.GetNchnls();
    auto host_frames = buffer.getNumSamples();
    auto csound_frames = csound.GetKsmps();
    auto csound_frame_end = csound_frames + 1;
    auto spin = csound.GetSpin();
    auto spout = csound.GetSpout();
    for(auto host_frame_index_ = 0; host_frame_index_ < host_frames; host_frame_index_++)
    {
        // Copy the host's _current_ audio output to Csound's audio input.
        for(auto channel_index = 0; channel_index < host_input_channels; channel_index++)
        {
            spin[(csound_frame_index * channel_index) + channel_index] = buffer.getSample(channel_index, host_frame_index_);//hostInput[channelI][hostFrameI];
        }
        // Copy Csound's _previous_ audio output to the host's audio input.
        for(auto channel_index = 0; channel_index < host_output_channels; channel_index++) {
            buffer.setSample(channel_index, host_frame_index_, spout[(csound_frame_index * csound_output_channels) + csound_frame_index] * outputScale);
            spout[(channel_index * csound_output_channels) + channel_index] = 0.0;
        }
        csound_frame_index++;
        // If Csound's output buffer has been filled, reset Csound's frame index,
        // and perform the next buffer.
        if(csound_frame_index > csound_frame_end) {
            csound_frame_index = 0;
            csound.PerformKsmps();
        }
    }
    // Clear the host buffers.
    buffer.clear();
    midiMessages.clear();
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
