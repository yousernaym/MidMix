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

## Native interface (P/Invoke surface)

Declared in [../../VisualMusic/MidMix.cs](../../VisualMusic/MidMix.cs) (`cdecl`, `MidMix.dll`):

- `init()` — initialize the engine / load the soundfont (called at app startup in `App.xaml.cs`).
- `sfLoaded()` — was a soundfont successfully loaded?
- `mixdown(midiPath, mixdownPath)` — render the MIDI file to a WAV mixdown.
- `close()` — shutdown (called at app exit).

See [../../CLAUDE.md](../../CLAUDE.md) for the repo-wide picture and
[../../VisualMusic/CLAUDE.md](../../VisualMusic/CLAUDE.md) for how the mixdown feeds playback and export.
