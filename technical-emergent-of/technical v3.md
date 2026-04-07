
## Technical Series: Master Development Roadmap
Artist Identity: Generative "Machine-Made" Technical Drawings
Influence: Ágoston Nagy (Binaura) — The "Machine as Participant"
Core Features: Emergent DNA Species, Pentatonic Piano Soundscapes, Vivarin Translucency.
------------------------------
## 1. The Development Stack (The "Lab")
For high-performance research and physical installations.

* Visuals: openFrameworks (C++).
* Purpose: Real-time rendering of complex geometry and high-fidelity "Vivarin" translucent gradients (smoother and faster than browser-based WebGL).
* Audio: Pure Data (Pd).
* Purpose: The "Brain." Handles non-linear synthesis, pentatonic mapping, and piano sample mutation.
* The Bridge (C++ $\leftrightarrow$ Pd):
* Method A (Internal - libpd/ofxPd): Embeds Pd inside C++. Best for final apps and low-latency "taps."
   * Method B (External - OSC): Programs talk over a network. Best for prototyping (tweaking audio while visuals run).
* Feedback Loop: Use FFT (Audio Analysis) in Pd to send data back to C++. This makes your drawings "pulse" or "glow" (alpha/transparency) in sync with the piano strokes.

------------------------------
## 2. The Physical & Robotic Stack
For installations involving sensors, plotters, or robotic arms.

* Hardware: Raspberry Pi (The Brain) + Arduino (The Nervous System/Sensors).
* OS: Linux (Ubuntu/Arch). Preferred for 24/7 stability and raw hardware access.
* Control Languages:
* Node.js: For bridging digital DNA code with robotic movements/plotters.
   * Python: For quick integration of GPIO sensors (motion, light, distance).
* Interaction: Use sensors to let the environment "disturb" the species DNA, making the machine a participant with the physical world.

------------------------------
## 3. The Creative Logic: "Emergent DNA"

* **Genotype (canonical):** The **[EmergentDNA](https://github.com/walhimer-studio/EmergentDNA)** reference repo is the genome layer: seeded deterministic **`Rand` / `SeedRng`**, optional trait bundles, and rules in `docs/SPEC.md`. It intentionally excludes rendering, audio engines, and timers — those belong in the **phenotype** (p5, WebGL, Tone.js, installation code).
* DNA struct: Define traits (line weight, color palette, pentatonic root, mutation rate). Drive all random choices from the same canonical PRNG so preview, mint, and gallery match.
* The participant loop (target interaction):
* Tap 1: Generates a line + triggers a pentatonic piano note.
   * Tap 2: Mutates a gene. The "Species" evolves, changing the behavior for all future taps.
* Multi-user DNA: Use WebSockets to allow different "Species" to communicate and exchange DNA traits online.

------------------------------
## 4. The Deployment Stack (The "Release")
Manual rewrite for web-based NFT platforms.

* Visuals: p5.js (WebGL mode). Vivarin-style translucency: layered fills and alpha in WebGL where the piece needs that glassy, luminous look.
* Audio: **Tone.js** — scheduling, envelopes, and sample playback in the browser (closest spirit to Pd-style control in JS).
* **Standalone final HTML:** Ship **Tone.js (and p5.js) vendored or embedded** in the release bundle so the **minted file has no runtime dependency on CDNs or external script URLs** — one self-contained HTML (or ZIP) with no network calls required to load libraries. Build step: bundle/minify/inject; prototypes may use CDNs until the release is frozen.
* Genome: Initialize **`EmergentDNA.Rand` / `SeedRng`** from the platform seed (**fxrand()**, URL hash, **hl-gen.js**, etc.) so DNA traits and drawing RNG stay aligned with the spec.
* Note: C++ cannot be auto-exported to HTML. You must manually port the "soul" of the C++ logic into JavaScript.

------------------------------
## 5. Targeted Platforms

* fx(hash): Primary choice for long-form generative series using fxrand(). Feed that value (or a derived uint32) into the canonical **`Rand`** constructor.
* [Verse.works](https://verse.works/): For curated, high-end gallery series (uses URL hash payloads).
* Highlight.xyz: Uses hl-gen.js for deterministic seeding — same **single seed → same `Rand` stream** rule applies.
* Manifold: For total control over the smart contract and claim pages.
* Note on Objkt: Use only for secondary marketplace listings or pre-generated 1/1s. It does not support the "Master ZIP" minting method natively.

------------------------------
## 6. Lab prototype status (`technical-emergent-of/`)

This folder is the **current** openFrameworks + Pd work area. As of the last pass:

* **openFrameworks (C++):** A **visual** prototype exists (`src/`): seeded **`SeedRng`** (matches EmergentDNA), **`TechnicalTraits`**, queued technical drawing (grid, elevation line, segments, cubes, etc.). **Audio default:** **OSC** to Pure Data (`ofxOsc`: send **`/from-of`** to port **9000**, receive **`/to-of-rms`** on **9001**). **Fallback:** local **`ofSoundPlayer`** + `bin/data/sounds/` (press **`o`** to toggle). Regenerate the project with **ofxOsc** in **addons.make**. Vivarin-style shaders and a generated IDE project are still TBD.
* **Pure Data:** **`pd/`** has **`setup-samples.sh`**, **`abstractions/penta-voice.pd`**, **`main.pd`** (send/receive symbols for future libpd), and **`main-osc.pd`** (OSC + **iemnet** `udpreceive` / `udpsend` — install via Deken).
* **Emergent DNA:** The **canonical PRNG** is implemented in C++ (`EmergentDNA.h`); full **`SpeciesGenome` / `expressIndividual`** from JS is not ported — only what the drawing + audio layers need today.
* **C++ $\leftrightarrow$ Pd (OSC):** **Implemented** for prototyping: each draw step sends **`/from-of`** with degree **0–4** and velocity **1–127**; Pd returns **`/to-of-rms`** for a thin RMS bar in the oF window. **libpd** is still optional for a later single-binary install.

------------------------------
## 7. libpd / ofxPd versus OSC (how to choose the bridge)

Both connect **openFrameworks** to **Pure Data**; they differ in **process model** and **workflow**.

### libpd (embedded Pd inside the C++ app)

**Advantages**

* **One binary, one process** — simpler to ship for installations: no second app to launch.
* **Lower latency** — no UDP hop; audio can be tied to the same clock as your app (subject to correct libpd/audio-thread usage).
* **Tight coupling** — direct `libpd_send_symbol` / `libpd_process_float` style control; good for “tap in sync with line.”
* **Single crash domain** — one thing to profile and deploy (also a downside below).

**Disadvantages**

* **Build/link complexity** — libpd + ofxPd must match your platform; externs in the patch must be supported by libpd (not all Pd libraries work embedded).
* **No built-in Pd GUI** in the running app unless you add UI or use a remote debugger — **live patching** often means re-saving and reloading the patch or a custom reload path.
* **Threading rules** — you must respect libpd’s expectations (what runs on the audio thread vs main thread) to avoid glitches or crashes.
* **Risk coupling** — a bad patch or extern can bring down the whole oF process.

### OSC (Pd as a separate process, e.g. Pd + `netreceive`)

**Advantages**

* **Best for live tweaking** — full **Pure Data GUI** while your sketch runs: edit mappings, modulation, and analysis without recompiling C++.
* **Process isolation** — audio engine can keep running if you restart the visual app (and sometimes the reverse).
* **Flexible topology** — can run Pd on another machine on the LAN if you ever need that for installs or split hardware.
* **Simple C++ side** — send floats/lists with **ofxOsc**; no libpd link step.

**Disadvantages**

* **Higher latency** — UDP + two schedulers; fine for many installations, worse for sub–few-ms tight coupling unless you tune buffers.
* **Two things to start** — operator must launch oF **and** Pd (unless you script it); more moving parts in documentation.
* **Sync** — you must design message rate and timestamps if you need sample-accurate alignment with visuals.
* **Network hygiene** — firewalls, ports, and “localhost only” vs remote must be decided explicitly.

### Practical rule of thumb

* Prefer **OSC** while you are still **designing sound** (mapping, modulation, FFT branches) and want the **Pd editor** open all day.
* Prefer **libpd** when the patch is **stable** and you want **one deployable**, **lower latency**, and **fewer runtime dependencies** for a gallery or long-run install.

You can also **prototype on OSC**, then **port the same message protocol** to libpd for the final build.

------------------------------
## 8. Immediate Action Items

   1. **Audio (Pd):** Pentatonic map + velocity layers + RMS tap exist in **`pd/`**; **OSC to oF is wired** (**`/from-of`**, **`/to-of-rms`**). Next: refine mapping, add modulation / **FFT** (and extra OSC addresses) and use them for visuals — mirror any new sends in **`ofApp`**.
   2. **Visuals (oF):** Extend the existing technical queue toward full parity with the p5/HTML prototype; add Vivarin-style translucency when you move to shaders/FBOs.
   3. **Bridge:** **OSC** path is in place (**ofxOsc** + **`pd/main-osc.pd`**). Next: stabilize patches, add FFT/other sends, then optionally **libpd** for shipping a single binary.
   4. **Web release:** Integrate **`emergent-dna-core.js`** with p5 + **Tone.js**, seeded from the platform; bundle a **standalone HTML** (vendored Tone + p5, no runtime CDN).

------------------------------

