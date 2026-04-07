# openframework-puredata

Workspace for **openFrameworks** apps that talk to **Pure Data** over **OSC**, plus room for future projects alongside the same stack.

## Contents

| Path | Purpose |
|------|---------|
| `technical-emergent-of/` | macOS openFrameworks 0.12 app (`technical-emergent-of.xcodeproj`). Uses **ofxOsc**; audio/control via OSC to Pd. Includes `pd/` patches and sample layout. |

## openFrameworks

This repo holds **source and project files**. It does **not** bundle the openFrameworks SDK.

- Install [openFrameworks](https://openframeworks.cc/) (e.g. `of_v0.12.1_osx_release`).
- Either copy or symlink this app under `apps/myApps/` in your OF tree, or open the `.xcodeproj` and fix **header/library paths** if you keep the project elsewhere.
- Addon: **ofxOsc** (listed in `technical-emergent-of/addons.make`).
- Build in **Xcode** with the shared schemes, e.g. `technical-emergent-of Debug`.

## Pure Data

- Patches live under `technical-emergent-of/pd/` (see `pd/README.txt`).
- The app sends OSC (e.g. to Pd); run Pure Data and load the appropriate main patch when you want sound or routing through Pd.
- Ports and addresses are defined in the app and patch files—keep them in sync when you change them.

## Git remote

After cloning:

```bash
git remote add origin https://github.com/walhimer-studio/openframework-puredata.git
```

SSH is fine too if your key has access to the `walhimer-studio` org.

## Future work

Add new folders under this repo root for other art pieces (each can be its own OF app or other tooling), commit, and push to the same remote.
