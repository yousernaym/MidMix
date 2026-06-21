# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Role

`MidMix` is the native C++ MIDI→audio mixdown layer for **Visual Music**. It is a git submodule (separate
repo: `yousernaym/MidMix`) that builds `MidMix.dll`, which the app calls via P/Invoke. It renders a MIDI
file to a WAV mixdown using **Fluidsynth** and a SoundFont.

Fluidsynth is supplied through vcpkg in **manifest mode**: the dependency (`fluidsynth`) is declared in
[vcpkg.json](vcpkg.json) at this submodule's root, with versions pinned via `builtin-baseline`. The x64
build auto-restores it into a local `vcpkg_installed/` (enabled by `VcpkgEnableManifest` in
[MidMix/MidMix.vcxproj](MidMix/MidMix.vcxproj)); `VcpkgAutoLink` then links the libs and applocal copies the
DLLs next to `MidMix.dll`. See the repo-root [README.md](../../README.md).
The SoundFont is a `soundfont.sf2` placed next to `VM.exe` at runtime; `sfLoaded()` reports whether it was found.

## Build & output

- Project: [MidMix/MidMix.vcxproj](MidMix/MidMix.vcxproj) (`DynamicLibrary`, x64/x86). Standalone solution:
  [MidMix.sln](MidMix.sln).
- Built as part of the repo-root `VisualMusic.sln`; `MidMix.dll` lands in the repo-root `x64\<Config>\` and
  VisualMusic's post-build copies it into the app output.

## Updating Fluidsynth

The Fluidsynth version is whatever the `builtin-baseline` in [vcpkg.json](vcpkg.json) resolves to (no
`vcpkg install`/`vcpkg upgrade` in manifest mode). To move to a newer Fluidsynth:

1. Update the vcpkg checkout to a newer snapshot and re-bootstrap (from the vcpkg root, e.g. `D:\dev\vcpkg`):
   `git fetch origin && git checkout <newer-release-tag-or-origin/master> && .\bootstrap-vcpkg.bat`.
2. Bump this manifest's baseline to the new snapshot: `vcpkg x-update-baseline --x-manifest-root=<this dir>`
   (or set `builtin-baseline` by hand to `git -C <vcpkg root> rev-parse HEAD`). Media's manifest is separate,
   so bumping here moves Fluidsynth (and its transitive deps like glib) only.
3. Rebuild `VisualMusic.sln` — the new version installs automatically (compiles from source once, then cached).
4. **Test a real MIDI mixdown** to confirm `init`/`mixdown` still behave (a major-version bump can shift the
   Fluidsynth API).

To pin an exact version instead of the baseline default, add an `overrides` entry to [vcpkg.json](vcpkg.json),
e.g. `"overrides": [ { "name": "fluidsynth", "version": "2.5.4" } ]`; the version must exist at/after the
baseline (browse `<vcpkg root>\versions\f-\fluidsynth.json`).

## Native interface (P/Invoke surface)

Declared in [../../VisualMusic/MidMix.cs](../../VisualMusic/MidMix.cs) (`cdecl`, `MidMix.dll`):

- `init()` — initialize the engine / load the soundfont (called at app startup in `App.xaml.cs`).
- `sfLoaded()` — was a soundfont successfully loaded?
- `mixdown(midiPath, mixdownPath)` — render the MIDI file to a WAV mixdown.
- `close()` — shutdown (called at app exit).

See [../../CLAUDE.md](../../CLAUDE.md) for the repo-wide picture and
[../../VisualMusic/CLAUDE.md](../../VisualMusic/CLAUDE.md) for how the mixdown feeds playback and export.
