# Third-party components

## JUCE
- Version: pinned to release tag `8.0.4`
- Licence: JUCE free "Starter" licence (also compatible with JUCE's AGPLv3 option, but we use the Starter/Indie commercial terms)
- Usage: fetched at build time via CMake `FetchContent` from the official JUCE repository (`https://github.com/juce-framework/JUCE`). Never vendored into this repository, never shipped as source by us.

## CLAP JUCE Extensions
- Source: `https://github.com/free-audio/clap-juce-extensions`
- Licence: MIT
- Usage: fetched at build time via CMake `FetchContent`, pinned to commit `55525c9858d4b25687be7759a5e0f70eccef218e` (no tagged release exists upstream, so a commit SHA is used instead of a branch). Provides the CLAP wrapper target on top of JUCE's `juce_add_plugin`. MIT is compatible with our closed-source distribution; recorded here as required before adding any third-party dependency.

## pluginval
- Source: `https://github.com/Tracktion/pluginval`
- Licence: GPLv3
- Usage: executed as a standalone, pre-built, build-time validation tool only (downloaded binary in CI). Never linked into our plugin binaries, never shipped to customers, no GPL source copied into this repository.
