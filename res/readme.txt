snibbetracker test
==================
 

introduction
----------------
This is a test version, please do not re-distribute. Functionality may change significantly 
during the test phase, and there will be no focus on preserving backwards compatibility 
(projects made with previous builds may not work as expected). Subsequent test builds will 
have an accompanying changelog as a part of this document.
No warranty either implicit or implied is included. Use at your own risk.
snibbetracker (c) Palestone Software.

contact/support
----------------
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

trackview
----------------
- spacebar: toggle editing on/off.
- enter: play/stop.
- arrow keys: move cursor.
- tab: go to pattern view.
- modifier+left/right: change octave up/down.
- modifier+up/down: move notes below cursor.
- shift+arrow keys: make selection.
- modifier+c/v: copy paste note (or selection).
- character keys: play notes or edit effects.
- modifier+f: toggle play cursor follow.
track format explanation:
a = note, b = instrument number, ccc = effects. 6 supported channels.
a b ccc | a b ccc | a b ccc | a b ccc | a b ccc | a b ccc

 
patternview
----------------
- arrow keys: move around grid.
- plus/minus: cycle waveform, pattern numbers, bpm, swing, active etc.
- spacebar: go to instrument view (when gridcursor is at Ins 0-F)
- tab: go to track view.
- e: jump to trackview with current position.
- m: mute track (or channel if cursor is at the top)
- a: activate/inactivate track.
- s: solo track (or channel if cursor is at the top)
- modifier+up/down: cycle tracks (0-63).
- modifier+c/v: copy paste track data.

instrument view
----------------
- arrow keys: move node.
- modifier+arrow keys: move node slow.
- tab: cycle nodes.
- spacebar: go to pattern view.

global controls
----------------
- modifier+s: go to save view
- modifier+o: go to load view
- modifier+e: export to wav.

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
3xx - portamento (speed, speed) uses a single value if other is 0 or a multiplication of both. Sets the speed to when new notes will be reached.
4xx - vibrato (speed, depth).
5xx - distortion (clamp, clamp).
6xx - link distortion (channel, amp)
7xx - detune (amount, amount) 88 is middle
8xx - PWM (linear position/oscillation depth, oscillation speed) works only on squarewave. If param2 is present, param1 will be used for osc depth.
9xx - stack effects (positivt value activates, 0 eller - turns off.)
Axx - (left amplitud, right amplitud) can be used for amplitude, pan och turning off a tone.
Bxx - downsample sweep down (linear, sweep) Works best on noise channel. Choose either linear or sweep.
Cxx - downsample sweep up (linear, sweep) Works best on noise channel. Choose either linear or sweep.
Dxx - ends pattern.
Exx - pitch up (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
Fxx - pitch down (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.


BPM - beats per minute.
Amp - master amplitude, used both for previewing and exporting. Shows red if clipping.
Active - number of active pattern rows.
Rows - number of active rows in patterns.
Arp - arpeggio speed.
Swing - swing amount.
Preview - toggle for if notes should be audiable when playing on the keyboard.
 

changelog
----------------

build 9 - 2015-05-07 07.48
- added "move all notes in pattern-row below cursor up/down" in trackview. The notes that are moved out of the pattern area gets deleted.
- added copy/cut/paste in trackview.
- copy/paste pattern in patternview.
- ability to type in BPM and arp speed etc for quicker changes. You can still use +/- like before.
- will remember the last filename you saved as (or loaded) when going in to the save/load screen.
- safety pass so you don't accidentally save over an existing song, or load a new song when you have a currently unsaved one.
- first param of PWM effect control modulation depth if the second param (speed) is present.
- changed default instrument envelop to one with fast attack.
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


