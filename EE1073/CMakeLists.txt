# EE-1073 -- Neve 1073-style mic preamp + 3-band EQ channel strip emulation.
# See EE1073/Shared/Ee1073ChannelStripCore.h for the DSP core and the
# top-level README.md for build instructions.

if(EE1073_BUILD_PLUGIN)
    juce_add_plugin(EE1073
        COMPANY_NAME "EON Audio"
        BUNDLE_ID "com.eonaudio.ee1073"
        PLUGIN_MANUFACTURER_CODE Eone
        PLUGIN_CODE Ev73
        FORMATS VST3 AU Standalone
        PRODUCT_NAME "EE-1073"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        VST3_CATEGORIES Fx EQ
        AU_MAIN_TYPE kAudioUnitType_Effect)

    target_sources(EE1073 PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Ee1073LookAndFeel.cpp)

    target_include_directories(EE1073 PRIVATE Shared)

    target_compile_definitions(EE1073 PUBLIC
        JUCE_ALSA=0
        JUCE_DIRECTSOUND=0
        JUCE_PLUGINHOST_VST3=0
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

    target_link_libraries(EE1073 PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
endif()

if(EE1073_BUILD_MAX_EXTERNAL)
    set(MIN_DEVKIT_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/min-devkit")
    if(NOT EXISTS "${MIN_DEVKIT_DIR}/source/min-api/script/min-pretarget.cmake")
        message(WARNING
            "min-devkit not found at ${MIN_DEVKIT_DIR}. "
            "Run: git submodule add https://github.com/Cycling74/min-devkit ThirdParty/min-devkit "
            "&& git submodule update --init --recursive")
    else()
        add_subdirectory(Max)
    endif()
endif()
