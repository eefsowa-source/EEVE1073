cmake_minimum_required(VERSION 3.22)

# --- Toolchain / platform setup -------------------------------------------
# Universal binary (Intel + Apple Silicon) as recommended in the project
# blueprint. Must be set before project().
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS" FORCE)
endif()
if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment version" FORCE)
endif()

project(EEVE1073 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(EEVE_BUILD_PLUGIN "Build the JUCE VST3/AU/Standalone plugin" ON)
option(EEVE_BUILD_MAX_EXTERNAL "Build the Max for Live (Max external) target" OFF)

# --- JUCE plugin target -----------------------------------------------------
if(EEVE_BUILD_PLUGIN)
    # Expect JUCE checked out at ThirdParty/JUCE (see README for setup, e.g.
    # `git submodule add https://github.com/juce-framework/JUCE ThirdParty/JUCE`).
    add_subdirectory(ThirdParty/JUCE)

    juce_add_plugin(EEVE1073
        COMPANY_NAME "EON Audio"
        PLUGIN_MANUFACTURER_CODE Eone
        PLUGIN_CODE Ev73
        FORMATS VST3 AU Standalone
        PRODUCT_NAME "EEVE1073"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        VST3_CATEGORIES Fx Dynamics
        AU_MAIN_TYPE kAudioUnitType_Effect)

    target_sources(EEVE1073 PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

    target_include_directories(EEVE1073 PRIVATE Shared)

    target_compile_definitions(EEVE1073 PUBLIC
        JUCE_ALSA=0
        JUCE_DIRECTSOUND=0
        JUCE_PLUGINHOST_VST3=0
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

    target_link_libraries(EEVE1073 PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
endif()

# --- Max for Live external target ------------------------------------------
if(EEVE_BUILD_MAX_EXTERNAL)
    # Expect the min-devkit checked out at ThirdParty/min-devkit (see README):
    # `git submodule add --recursive https://github.com/Cycling74/min-devkit ThirdParty/min-devkit`
    add_subdirectory(Max)
endif()
