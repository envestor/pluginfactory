# Template notes for new plugins

This is the minimal CI skeleton (PLU-20), not yet the full shared template. Anything a new plugin copies from this skeleton must change the following:

## Permanent identifiers (never change once released)
- Manufacturer code: `Bdpo`
- Plugin code (this skeleton only — every new plugin needs its own, unique, four-character code): `Skl1`
- Parameter ID: `gainDb` — the one test parameter's string ID, declared once in `Source/PluginProcessor.h` under `ParamIDs::gainDb`.

## JUCE version
Pinned via `FetchContent` in `CMakeLists.txt` to tag `8.0.4`. Upgrading JUCE for all plugins means bumping this tag deliberately and re-running the full CI matrix, not a casual change.

## CLAP support
Provided by `clap-juce-extensions` (MIT licence, see `THIRD_PARTY.md`), fetched via `FetchContent` pinned in `CMakeLists.txt`. Currently pinned to `main` — replace with a pinned commit SHA before this skeleton becomes the production template, so CLAP builds stay reproducible.

## Licensing
`LICENSE` at the repo root is "All rights reserved." — this is proprietary commercial software, not open source.

## What is intentionally missing from this skeleton
No DSP beyond a single smoothed gain stage, no custom UI (uses JUCE's generic parameter editor), no licence/trial check, no preset system, no AAX. These come in later tasks under PLU-18.
