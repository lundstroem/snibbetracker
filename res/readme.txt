snibbetracker 
----------------
 
changelog
----------------
build 1 - first test release.

 
introduction
----------------
Functionality may change significantly during the test phase, and there will be no focus on preserving
backwards compatibility (projects made with previous builds may not work as expected).
Subsequent test builds will have an accompanying changelog as a part of this document.
No warranty either implicit or implied is included. Use at your own risk.
 
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

///////////////////////////////////////////// no windows version with save/load yet!
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

* modfier key is cmd on OSX and ctrl on win.
/////////////////////////////////////////////

effects
----------------
0xx - arpeggio (second tone halfsteps, third tone halfsteps) change speed in settings:Arp xx.
3xx - portamento (speed, speed) uses a single value if other is 0 or a multiplication of both. Sets the speed to when new notes will be reached.
4xx - vibrato (speed, depth).
Bxx - lowpass sweep down (linear, sweep) Works best on noise channel. Choose either linear or sweep.
Cxx - lowpass sweep up (linear, sweep) Works best on noise channel. Choose either linear or sweep.
Exx - pitch up (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
Fxx - pitch down (fast, slow) Works on non-noise channels. Both values can be combined to increase effect.
 
BPM - beats per minute.
Active - number of active pattern rows.
Rows - number of active rows in patterns.
Arp - arpeggio speed.
Swing - swing amount.
 
 

