<p>
<a rel="license" href="http://creativecommons.org/licenses/by-nc-sa/4.0/"><img alt="Creative Commons License" 
style="border-width:0" src="https://i.creativecommons.org/l/by-nc-sa/4.0/88x31.png" />
</a>
<p>All music and examples herein are licensed under the  
<a rel="ccncsa4" href="http://creativecommons.org/licenses/by-nc-sa/4.0/">
Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License</a>.
<p>
<img alt="GNU Affero General License v3" 
style="border-width:0" src="https://www.gnu.org/graphics/agplv3-155x51.png" /> 
</p><p>All code herein is licensed under the  
<a rel="agplv3" href="https://www.gnu.org/licenses/agpl-3.0.html">
GNU Affero General Public License, version 3</a>.

# CsoundVST3
[Michael Gogins](https://michaelgogins.tumblr.com)

## Introduction

CsoundVS3 enables the [Csound audio programming language](https://csound.com/") 
to be used within digital audio workstations as a VST3 plugin instrument. The 
plugin can also be used as a signal processing effect.

CsoundVST3 has audio inputs, audio outputs, MIDI inputs, and MIDI outputs. 
The plugin hosts one .csd file, which can be edited from the plugin's user 
interface. The interface also displays Csound's runtime messages. Csound's 
score time is synchronized with the DAW's playback time, which can loop.



CsoundVST3 has _all_ the power of command-line Csound. CsoundVST3 can read and 
write on the user's filesystem, load plugin opcodes, and execute system 
commands.

CsoundVST's GUI does _not_ provide user-defined widgets for controlling the 
Csound orchestra. However, such controls can be implemented in the DAW using 
MIDI control change messages, or through Csound's network interfaces. All MIDI 
control change messages are automatically mapped to Csound k-rate control 
channels, using a naming convention.

Please log any bug reports or enhancement requests as a GitHub issue.

## Installation

Download the installation archive from <a href="">GitHub<a/> and unzip it.

Copy the CsoundVST3.vst3 directory and its contents to your computer's VST3 
plugins directory. For example, in macOS, that would normally be the user's 
`~/Library/Audio/Plug-Ins/VST3/CsoundVST3.vst3`.

## Usage

 1. Write a Csound .csd file that optionally outputs stereo audio, optionally 
    accepts stereo audio input, and optionally accepts MIDI channel messages. 
    The DAW's MIDI note on and off messages to CsoundVST3 are automatically 
    mapped to Csound `i` statement pfield 1 (as MIDI channel + 1), pfield 4 
    (as MIDI key), and pfield 5 (as MIDI velocity). The DAW's MIDI control 
    change messages to CsoundVST3 are automatically mapped to Csound control 
    channels named `daw_ch<###>_cc<###>`, e.g. `daw_ch000_cc065`. These can 
    include RPNs and NRPNs.

 2. In your DAW, create a new track using CsoundVST3 as a virtual instrument.

 3. Open the CsoundVST3 GUI and either open your .csd file using the
    _**Open...**_ dialog, or paste the .csd code into the edit window.

 5. Click on the **_Play_** button to make sure that the .csd compiles and
    runs. You can use a score in your DAW, or a MIDI controlleer, or a
    virtual keyboard to test the .csd.

 7. Save your DAW project, and re-open it to make sure that your plugin 
    and its .csd have been loaded.

This plugin does not implement presets. The entire state of the plugin is the 
.csd file, which is saved and loaded as part of the DAW project. However, you 
can have as many CsoundVST3 plugins on as many tracks as you like, each with 
its own independent .csd file.




