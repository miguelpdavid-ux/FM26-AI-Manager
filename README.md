# FM26 AI Manager V1.6.1

Safety-focused source revision of the automation core.

## Changes from V1.6
- Removed global Ctrl+Shift+F12 hotkey registration.
- Removed `SendInput`, global keyboard injection and forced foreground switching.
- The first autonomous executor addresses only the detected Football Manager 2026 window using targeted Win32 window messages.
- If FM26 is minimized, missing or unresponsive, the executor refuses to act.
- Emergency Stop remains available inside the manager UI.
- The automation loop still follows observe -> act -> verify; a visible state change is required before an action is counted as verified.
- GitHub Actions creates SHA-256 manifests for source and executable alongside the portable build.

## Current scope
V1.6.1 validates the targeted automation architecture. It is not yet the complete autonomous club manager. Transfers, contracts, staff changes and tactical editing remain safety-gated until semantic screen recognition and their individual executors are implemented and verified.

## Build
Upload this source tree to the repository and let GitHub Actions build it on `windows-latest`. No local compiler or Python installation is required on the user's PC.
