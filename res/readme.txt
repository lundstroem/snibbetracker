snibbetracker test
----------------
 
changelog
----------------

build 8 - 2015-03-20 10.48
pattern view
- toggle track active/inactive with 'a' when in patternview.
- toggle track solo with 's'.
- press 'e' on a track (in patternview) to jump tp that track in the trackview.
- set mute/solo on channels with 'm' and 's' when the cursor is at the top with sine, saw, square etc..
- scroll tracks with modifier+up/down. They range from 0-63.

effects
no stacking of effects, except some combinations of arp/arp speed and amp.

8xx PWM (linear position, oscillator speed) works only on squarewave.
9xx stack effects (positivt value activates, 0 eller - turns off.)
1xx arpeggio speed (speed, speed) use one of the values or both multiplied.
Axx (left amplitud, right amplitud) can be used for amplitude, pan och turning off a tone.

fixed bugs
---------------
- save song (again)
- should now load channel-waveform settings correctly.

other
- changed a few colors with with more contrast to make it easier to see what you're doing.
- old songs cannot be loaded with this version because of significant changes in the file format.



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
 
introduction
----------------
This is a test version, please do not re-distribute. Functionality may change significantly 
during the test phase, and there will be no focus on preserving backwards compatibility 
(projects made with previous builds may not work as expected). Subsequent test builds will 
have an accompanying changelog as a part of this document.
No warranty either implicit or implied is included. Use at your own risk.
snibbetracker (c) Palestone Software.

contact
----------------
palestonesoftware@gmail.com
http://www.palestonesoftware.com/snibbetracker.html

controls overview
----------------
* modfier key is cmd on OSX and ctrl on win.

track view
----------------
controls:
- spacebar: toggle editing on/off.
- enter: play/stop.
- arrow keys: move cursor.
- tab: go to pattern view.
- modifier+plus/minus: change octave up/down.
- character keys: play notes or edit effects.
- modifier+f: toggle play cursor follow.

track format where a = note, b = instrument number, ccc = effects.
a b ccc
 
pattern view
----------------
controls:
- arrow keys: move around grid.
- plus/minus: cycle waveform, pattern numbers, bpm, swing, active etc.
- spacebar: go to instrument view (when gridcursor is at Ins 0-F)
- tab: go to track view.
- e: jump to trackview with current position.
- m: mute track (or channel if cursor is at the top)
- a: activate/inactivate track.
- s: solo track (or channel if cursor is at the top)
 
instrument view
----------------
controls:
- arrow keys: move node.
- modifier+arrow keys: move node slow.
- tab: cycle nodes.
- spacebar: go to pattern view.

global controls
----------------
- modifier+s: go to save view
- modifier+o: go to load view

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
8xx - PWM (linear position/oscillation depth, oscillation speed) works only on squarewave. If param2 is present, param1 will be used for osc depth.
9xx - stack effects (positivt value activates, 0 eller - turns off.)
Axx - (left amplitud, right amplitud) can be used for amplitude, pan och turning off a tone.
Bxx - downsample sweep down (linear, sweep) Works best on noise channel. Choose either linear or sweep.
Cxx - downsample sweep up (linear, sweep) Works best on noise channel. Choose either linear or sweep.
Dxx - ends pattern.
Exx - pitch up (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
Fxx - pitch down (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.


BPM - beats per minute.
Active - number of active pattern rows.
Rows - number of active rows in patterns.
Arp - arpeggio speed.
Swing - swing amount.
 
 

