cmake_minimum_required(VERSION 3.22)

project(PluginFactorySkeleton VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

add_subdirectory(dsp)

include(FetchContent)

# JUCE, pinned to release tag 8.0.4. Never vendored: fetched fresh at configure time.
# See THIRD_PARTY.md for licence details.
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.4
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(JUCE)

# CLAP JUCE extensions, MIT licensed. See THIRD_PARTY.md.
# No tagged release exists upstream; pinned to a specific commit SHA (not a
# branch) so the build is reproducible.
FetchContent_Declare(
    clap-juce-extensions
    GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
    GIT_TAG        55525c9858d4b25687be7759a5e0f70eccef218e
)
FetchContent_MakeAvailable(clap-juce-extensions)

# Permanent plugin identity. Do not change once released.
# Manufacturer code: Bdpo  (The Bespoke Developer PlugOut)
# Plugin code:       Skl1  (CI-A1 minimal skeleton)
set(PLUGIN_MANUFACTURER_CODE "Bdpo")
set(PLUGIN_CODE "Skl1")

juce_add_plugin(PluginFactorySkeleton
    COMPANY_NAME "The Bespoke Developer PlugOut"
    BUNDLE_ID "com.bespokedeveloperplugout.pluginfactoryskeleton"
    PLUGIN_MANUFACTURER_CODE ${PLUGIN_MANUFACTURER_CODE}
    PLUGIN_CODE ${PLUGIN_CODE}
    FORMATS Standalone VST3 AU
    PRODUCT_NAME "PluginFactorySkeleton"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES "Fx"
    AU_MAIN_TYPE "kAudioUnitType_Effect"
)

target_sources(PluginFactorySkeleton
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_definitions(PluginFactorySkeleton
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(PluginFactorySkeleton
    PRIVATE
        juce::juce_audio_utils
        bdpo_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

clap_juce_extensions_plugin(
    TARGET PluginFactorySkeleton
    CLAP_ID "com.bespokedeveloperplugout.pluginfactoryskeleton"
    CLAP_FEATURES audio-effect
    CLAP_MANUFACTURER_NAME "The Bespoke Developer PlugOut"
)
