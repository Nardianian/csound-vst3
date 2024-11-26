<p>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" 
style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" />
</a>
<p>All music and examples herein are licensed under the  
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">
Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.

# CsoundVST3
[Michael Gogins](https://michaelgogins.tumblr.com)

This is a VST3 plugin version of Csound, licensed under the Affero General 
Public License, version 3.

## Introduction

CsoundVST enables the Csound andio programming language to be used within 
digital audio workstations as a VST3 plugin instrument. The plugin has audio 
inputs, audio outputs, MIDI inputs, and MIDI outputs. The plugin hosts one 
.csd file that can be edited from the plugin's user interface. The interface 
also displays Csound's runtime messages. Csound's score time is synchronized 
with the DAW's playback time, which can loop.

The plugin can also be used as a signal processing effect.

CsoundVST's GUI does not provide user-defined widgets for controlling the 
Csound orchestra. Such controls can be implemented in the host using MIDI 
controllers, or through Csound's network interfaces. All MIDI controllers 
are automatically mapped to Csound k-rate control channels, using a naming 
convention.

Please log any bug reports or enhancement requests as a GitHub issue.

## Installation

Copy the CsoundVST3.vst3 directory and its contents to your computer's VST3 
plugins directory. For example, on the Macintosh, that would normally be the 
user's `~/Library/Audio/Plug-Ins/VST3/CsoundVST3.vst3` directory.

## Usage

 1. Write a Csound .csd file that outputs stereo audio, optionally accepts 
    stereo audio input, and optionally accepts MIDI channel messages. The 
    DAW's MIDI note on and off messages to CsoundVST3 are automatically mapped 
    to Csound `i` statement pfield 1 (as MIDI channel + 1), pfield 4 (as MIDI 
    key), and pfield 5 (as MIDI velocity). The DAW's MIDI control change 
    messages to CsoundVST3 are automatically mapped to Csound control channels 
    named `daw_<ch#>_<cc##>`.

 2. In your DAW, create a new track using CsoundVST3 as a virtual instrument.

 3. Open the CsoundVST3 GUI and either open your .csd file using the Open... 
    dialog, or paste the .csd code into the edit window.

 4. Click on the Play button to make sure that the .csd compiles and runs. 
    You can use a score in your DAW, or a MIDI keyboard, or a virtual keyboard 
    to test it.

 5. Save your DAW project, and re-open it to make sure that your plugin 
    and its .csd have been loaded.

This plugin does not implement presets. The entire state of the plugin is the 
.csd file, which is saved and loaded as part of the DAW project. However, you 
can have as many CsoundVST3 plugins on as many tracks as you like.




