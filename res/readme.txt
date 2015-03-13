snibbetracker 
----------------
 
changelog
----------------
build 6 - 2015-03-13 15.26
- hopefully prevented crashes when saving (directory browsing is disabled for now).
- clear track nodes before loading new project.
- fixed memory leak when using more than 16 rows.
- minimum amount of rows set to 1 instead of 16.
- spacebar is now used for controlling playback.
- enter(return) key is now used for toggling edit 
		and enter/exit instrument view.
- showing current pattern and track in lower right corner (p:0 t:0).
 
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

track format where a = note, b = instrument number, ccc = effects.
a b ccc
 
pattern view
----------------
controls:
- arrow keys: move around grid.
- plus/minus: cycle waveform, pattern numbers, bpm, swing, active etc.
- spacebar: go to instrument view (when gridcursor is at Ins 0-11)
- tab: go to track view.
 
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
3xx - portamento (speed, speed) uses a single value if other is 0 or a multiplication of both. Sets the speed to when new notes will be reached.
4xx - vibrato (speed, depth).
Bxx - lowpass sweep down (linear, sweep) Works best on noise channel. Choose either linear or sweep.
Cxx - lowpass sweep up (linear, sweep) Works best on noise channel. Choose either linear or sweep.
Exx - pitch up (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
Fxx - pitch down (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
dxx - ends pattern.

BPM - beats per minute.
Active - number of active pattern rows.
Rows - number of active rows in patterns.
Arp - arpeggio speed.
Swing - swing amount.
 
 

