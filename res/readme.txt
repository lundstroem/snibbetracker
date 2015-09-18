snibbetracker BETA
==================

introduction
----------------
snibbetracker is a music tracker written in C using SDL2 that I began working on in 2014 to learn DSP programming.
I learned a lot on the way, and I would probably have made it different with the knowledge I have now, but perhaps it
can be useful for someone.
This is the BETA version of snibbetracker. I will try to keep backwards compatibility as intact as possible so
that old songs will sound the same in newer versions, and generally just maintain existing functionality.
However if something appears to be fundamentally broken, I might consider fixing it in a later version, and
in that case it will be clearly documented in the changelog. Linux version will be available at a later date
depending on interest.
No warranty either implicit or implied is included. Use at your own risk.
(c) Palestone Software.

credits
----------------
lundstroem
salkinitzor
nordloef
Linde
sunfl0wr
Rockard
0c0
OlofsonArcade
goto80
lolloise
crabinfo

contact/support
----------------
If you encounter bugs or other issues, please let me know and if possible the steps to reproduce them.
Feel free to contact if you have other feedback or questions, or if you've made a song
you'd like to share.
palestonesoftware@gmail.com
http://www.palestonesoftware.com/snibbetracker.html

controls overview
----------------
* modfier key is cmd on OSX and ctrl on win.

OSX
----------------
To be able to save/load/export you will have to set a working directory in:
snibbetracker.app/Contents/Resources/config.txt
Edit the json value for "working_dir_path" to an existing folder in you Documents directory like:
"working_dir_path":"/Users/d/Documents/snibbetracker/"

track view
----------------
- return: toggle editing on/off.
- space: play/stop.
- arrow keys: move cursor.
- tab: go to pattern view.
- modifier+left/right: change octave up/down.
- modifier+up/down: move notes below cursor.
- shift+arrow keys: make selection.
- modifier+c/v/x: copy paste or cut note (or selection).
- character keys: play notes or edit effects.
- modifier+f: toggle play cursor follow.
- home/end: go to top / bottom.
- plus/minus: transpose halfnote in selection.
- modifier+plus/minus: transpose octave in selection.
track format explanation:
a = note, b = instrument number, ccc = effects. 6 supported channels.
a b ccc | a b ccc | a b ccc | a b ccc | a b ccc | a b ccc

pattern view
----------------
- arrow keys: move around grid.
- plus/minus: cycle waveform, pattern numbers, active rows etc.
- return: go to instrument view (when gridcursor is at Ins 0-F)
- tab: go to track view.
- e: jump to trackview with current row position.
- m: mute track (or channel if cursor is at the top)
- x: activate/inactivate track.
- s: solo track (or channel if cursor is at the top)
- modifier+up/down: paginate tracks (0-63).
- modifier+c/v: copy paste track data.
- home/end: set cursor to top / bottom.

amp - master amplitude, used both for previewing and exporting.
rows - number of active rows in patterns.
arp - general arpeggio speed.
preview - if notes are audiable when editing.
tempo - open tempo editor.
visual - visualiser.
credits - show credits.
wavetable - connect a channel to lane 0-5 to combine waves together at different speeds. (see effect 9).
cust wave - create custom waveform for the "cust" wave type.

instrument view
----------------
- arrow keys: move node.
- modifier+arrow keys: move node slowly.
- tab: cycle nodes.
- return/esc: go to pattern view.
- shift: toggle editing of envelope or effects.
- home/end: cycle instruments.

custom wave
----------------
- arrow keys: move node.
- tab: cycle nodes.

wavetable view
----------------
- x: activate/inactivate row. first row is always active. toggle loop active/inactive.
- plus/minus: change speed on top row.
- 1-F: change overall speed on top row, or speed per row.

tempo view
----------------
- x: activate/inactivate row. each column must have at least one active row.
- plus/minus: change BPM on top row.
- 1-F: change BPM on top row, or beats.
- modifier+return: switch tempo column. while playing, column will be armed and switched to when the current pattern has finished.

global controls
----------------
- modifier+s: go to save view
- modifier+o: go to load view
- modifier+e: go to export view.
- modifier+i: go to instrument view.
- modifier+t: go to tempo view.
- modifier+r: go to wavetable view.
- modifier+j: go to custom wave view.
- modifier+n: reset project.
- space: toggle playback.
- escape: exit current view.

save view
----------------
- arrow keys: navigate file system.
- character keys: enter filename.
- enter: save file at the current location.
- escape: exit save view.

load view
----------------
- arrow keys: navigate file system.
- enter: load the file.
- escape: exit load view.

effects
----------------
0xx - arpeggio (second tone halfsteps, third tone halfsteps) change speed in settings:Arp xx.
1xx - arpeggio speed (speed, speed) use one of the values or both multiplied.
2xx - delay (speed, feedback)
3xx - portamento (speed, speed) uses a single value if other is 0 or a multiplication of both. Sets the speed to when new notes will be reached.
4xx - vibrato (speed, depth).
5xx - distortion (amp, amp).
6xx - FM (depth, speed).
7xx - detune (amount, amount) 88 is middle.
8xx - PWM (linear position/oscillation depth, oscillation speed) on squarewave. If param2 is present, param1 will be used for osc depth.
9xx - set wavetable/waveform for current channel. param1: set wavetable lane 0-5 or param2: change waveform 0-5.
Axx - (left amplitud, right amplitud) can be used for amplitude, pan och turning off a tone.
Bxx - downsample sweep down (linear, sweep) Works best on noise channel. Choose either linear or sweep.
Cxx - downsample sweep up (linear, sweep) Works best on noise channel. Choose either linear or sweep.
Dxx - ends pattern. D11 - jump to next pattern and reset tempo seq. D1x - reset tempo seq. D2x - switch tempo_seq column. x = tempo seq column (0-5).
Exx - pitch up (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
Fxx - pitch down (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
Gxx - bitcrush, params are multiplied to represent a bit depth. Affects all channels.

changelog
----------------
build 21
- show red lingering pixels in visualiser to display when audio clips.
- reset will now clear wavetable and custom waveform.
- noise table is now predefined instead of randomly generated at runtime.
- fixed preview of downsample sweep effect.
- fixed bug where notes at the end of the song leak into the start when exporting wav.

build 20 - 2015-08-12 16.00
- set default amp to 50% instead of 100%.
- fixed bug where visualiser showed twice the actual amplitude.
- increased max file name length to 40 instead of 20.

build 19 - 2015-08-10 12.27
- fixed reset of downsample.
- added bitcrush effect.
- when removing effect in trackview, the params are also removed.

build 18 - 2015-07-23 13.26
- changed activate button to x for wavetable, tempo and pattern editors.
- changed wavetable 900 to work like 90 to select wavetable lane 0 for current voice/channel. 9-0 to change waveform (param2).
- removed portamento reset on loopback for smoother transition when previewing. Set effect 3 without
    params to reset it manually when needed.
- made FM more chromatic and using now using effect 600.
- removed linked dist.
- fixed crash bug where sometimes noise phase would get a negative value.
- preview in instrument view now works with the current instrument you are editing.
- lowered modifiers for effects 700 and 300.
- refactored input.
- fixed bug where exported wave header indicated wrong size.
- fixed bug where detune would not reset.
- fixed bug where previous arp setting would apply on loopback and initial setting would not trigger.
- fixed bug where wavetable could not be set.

build 17 - 2015-06-22 11.56
- removed reads of uninitialized memory to potentially fix some random crashes.

build 16 - 2015-06-22 09.28
- added wavetable editor
- added custom wave editor.
- changed activate key 'a' to enter key instead to be able to input 0-F in tempo and wavetable view.
- transpose halfnotes with +/- in track view, also works with selections. (hold modifier for octaves).

build 15 - 2015-06-11 14.58
- tempo editor.
- improved preview: amplitude ramp, sustain (with third envelope node > 0), instrument effects added to preview.
    preview is disabled when playback is active.
- shortcuts: ctrl (or cmd on OSX) + i for instruments, + t for tempo + p for visualiser. Exit views with escape.
- editable color scheme.
- delay effect: 2xx (speed, feedback).
- in-editor help file.
- working dir now defaults to username/Documents on OSX.
- lots of tweaks.

build 14 - 2015-05-27 20.24
- fixed resetting bugs of voices and instrument effects.

build 13 - 2015-05-27 18.26
- fixed bug where active_tracks was not saved properly.
- default BPM is now 120
- fixed FM to work better.
- fixed bug where only a part of the song would be rendered at export.
- added preview 0 1 to prefs. (add "preview":false to config if you want to disable preview from start).
- disabled preview on play.
- removed lock from audiothread.
- moved mixing functions from client to engine code.
- fixed bug where audio effect A clipped on loopback, added ramp.

build 12 - 2015-05-26 12.32
- fixed effects so that they are not reset by the next row if there is no new note. (might be more adjustments needed).
- made FM for other waveforms other than square for effect 8. (depth, speed).
- made effect for changing waveform for channel (9). (channel, wavetype)
- copy/paste instruments in pattern editor.
- adjusted slow pitch shift param to be a bit faster.
- decreased detune modifier.

build 11 - 2015-05-21 11.00
- added effect table to instrument editor. These effects (pre-applied effects) will always be applied for the instrument (if not overridden with the same effect in the trackview). All effects will now be turned off after each row (and not as before by effect changes).
- removed effect 9 (stacking) due to the introduction of instrument effects.
- fixed bug where tone at the start of pattern disappears when moving a section of notes upwards.
- instrument numbers on deleted notes get saved to file.
- fixed bug where cursor does not go down when deleting note while playing.
- fixed bug where export ignores config working dir.
- added fullscreen setting to config
- fixed bug where addnote skips rows when adding.
- added deletekey, same use as backspace.
- showing r:0 for example to show which row you are on.
- added beat option for highlighting row intervals.
- home/end keys get to top/bottom rows and cycle through instruments.
- save/load preset effects in json.
- controls for toggling adsr/effects in instrument (left shift).
- clamp/dist effect - one linked / one channel wise.
- added link effect (for pre-mixdown of channels).
- fixed wrong instrument number when adding notes (unconfirmed).
- fix bug where volume does not reset when removing A effect.
- fixed bug where export wav buffer was not zeroed before use.
- only play from track in pattern view if cursor is < 16 etc otherwise from track 0.
- scroll for nodes in envelope.

build 10 - 2015-05-10 11.40
- fixed crash bug on XP when saving/loading/exporting.

build 9 - 2015-05-07 07.48
- added "move all notes in pattern-row below cursor up/down" in trackview. The notes that are moved out of the pattern area gets deleted.
- added copy/cut/paste in trackview.
- copy/paste pattern in patternview.
- ability to type in BPM and arp speed etc for quicker changes. You can still use +/- like before.
- will remember the last filename you saved as (or loaded) when going in to the save/load screen.
- safety pass so you don't accidentally save over an existing song, or load a new song when you have a currently unsaved one.
- first param of PWM effect control modulation depth if the second param (speed) is present.
- changed default instrument envelope to one with fast attack.
- make config file with default save/load path, buffersize etc.
- optimize to use less CPU while running (passive rendering).
- export songs to .wav.
- added config file to executable directory.
- OSX version.

build 8 - 2015-03-20 10.48
- toggle tracks active/inactive with 'a' and 's' 'while in patternview.
- 'e' to jump directly to that track from patternview.
- mute/solo 'm'/'s' on channels.
- cycle tracks in patternview (0-63).
- new effects: PWM, stack effects, arp speed, amp.
- no stacking of effects, except some combinations of arp/arp speed and amp.
- save song fix (again).
- correct loading of channel-waveforms data.
- changed a few colors with with more contrast to make it easier to see what you're doing.

build 7 - 2015-03-15 08.59
- edit cursor disconnected from player cursor. (can be toggled with modifier+F)
- directory browsing completely disabled for saving and loading files, only uses the directory the exe is in.
    when loading files, just type the name without the .json and press enter, same as for saving.

build 6 - 2015-03-13 15.26
- hopefully prevented crashes when saving (directory browsing is disabled for now).
- clear track nodes before loading new project.
- fixed memory leak when using more than 16 rows.
- minimum amount of rows set to 1 instead of 16.
- spacebar is now used for controlling playback.
- enter(return) key is now used for toggling edit and enter/exit instrument view.
- showing current pattern and track in lower right corner (p:0 t:0).
- cursor is now advanced down a row when removing notes and params with backspace.

build 5
- test release.


