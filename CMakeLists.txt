cmake_minimum_required(VERSION 3.22)

# --- Toolchain / platform setup -------------------------------------------
# Universal binary (Intel + Apple Silicon) as recommended in the project
# blueprints. Must be set before project().
if(APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS" FORCE)
endif()
if(APPLE AND NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS deployment version" FORCE)
endif()

project(EEAudioPlugins VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# This repository hosts multiple analog-circuit-emulation plugins, each in
# its own top-level folder with its own JUCE plugin target and (optionally)
# a Max for Live external target:
#
#   EE1176/  1176-style FET compressor (see EE1176/README or the project
#            blueprint PDF "Engineering the 1176...")
#   EE1073/  Neve 1073-style mic preamp + 3-band EQ channel strip (see the
#            project blueprint PDF "From Circuit Simulation to DAW
#            Integration...")
#
# Each subproject's own CMakeLists.txt defines its juce_add_plugin() target
# and Max external, gated by its own *_BUILD_PLUGIN / *_BUILD_MAX_EXTERNAL
# options below.

option(EE1176_BUILD_PLUGIN "Build the EE-1176 JUCE VST3/AU/Standalone plugin" ON)
option(EE1176_BUILD_MAX_EXTERNAL "Build the EE-1176 Max for Live external" OFF)
option(EE1073_BUILD_PLUGIN "Build the EE-1073 JUCE VST3/AU/Standalone plugin" ON)
option(EE1073_BUILD_MAX_EXTERNAL "Build the EE-1073 Max for Live external" OFF)

if(EE1176_BUILD_PLUGIN OR EE1073_BUILD_PLUGIN)
    # Expect JUCE checked out at ThirdParty/JUCE (see README for setup, e.g.
    # `git submodule add https://github.com/juce-framework/JUCE ThirdParty/JUCE`).
    add_subdirectory(ThirdParty/JUCE)
endif()

add_subdirectory(EE1176)
add_subdirectory(EE1073)
