Pentatonic Salamander + Pure Data (openFrameworks + OSC)
==========================================================

Musical set (5 pitches x 16 velocity layers)
--------------------------------------------
Salamander pitch names in this build: C, D#, F#, A (plus C up an octave).

  Degree 0 = C4   Degree 1 = D#4   Degree 2 = F#4   Degree 3 = A4   Degree 4 = C5

Velocity 1-127 maps to layer v01-v16 (see abstractions/penta-voice.pd).

Setup
-----
1. chmod +x setup-samples.sh && ./setup-samples.sh
2. Install iemnet (Pure Data: Help -> Find externals -> search "iemnet").
3. Open pd/main-osc.pd (OSC). Use main.pd only for libpd-style send/receive testing without OSC.

OSC ports (match ofApp defaults)
--------------------------------
  oF -> Pd   UDP 9000   message /from-of   args: int degree (0-4), int velocity (1-127)
  Pd -> oF   UDP 9001   message /to-of-rms args: float (0..1 approx)

openFrameworks
--------------
  addons.make lists ofxOsc. Regenerate the project with the oF projectGenerator
  so ofxOsc is linked. Run the app, then start Pd with main-osc.pd and DSP on.

Keys in oF
----------
  o / O   Toggle OSC (Pd) vs local ofSoundPlayer samples
  Space   New random seed
  R       Same seed
  N       seed+1

Paths
-----
  Symlinks pd/wav/n0_v01.wav ... n4_v16.wav avoid "#" in Pd file paths.
