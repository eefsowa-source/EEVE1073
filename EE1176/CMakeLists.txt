# EE-1176 -- 1176-style FET compressor emulation.
# See EE1176/Shared/Ee1176CompressorCore.h for the DSP core and the
# top-level README.md for build instructions.

if(EE1176_BUILD_PLUGIN)
    juce_add_plugin(EE1176
        COMPANY_NAME "EON Audio"
        BUNDLE_ID "com.eonaudio.ee1176"
        PLUGIN_MANUFACTURER_CODE Eone
        PLUGIN_CODE Ev76
        FORMATS VST3 AU Standalone
        PRODUCT_NAME "EE-1176"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD TRUE
        VST3_CATEGORIES Fx Dynamics
        AU_MAIN_TYPE kAudioUnitType_Effect)

    target_sources(EE1176 PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

    target_include_directories(EE1176 PRIVATE Shared)

    target_compile_definitions(EE1176 PUBLIC
        JUCE_ALSA=0
        JUCE_DIRECTSOUND=0
        JUCE_PLUGINHOST_VST3=0
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

    target_link_libraries(EE1176 PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
        PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
endif()

if(EE1176_BUILD_MAX_EXTERNAL)
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
