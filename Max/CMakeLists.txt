# Max for Live external target, built with Cycling '74's min-devkit
# (https://github.com/Cycling74/min-devkit), the modern C++ framework for
# Max/MSP externals. This produces a `.mxo` object usable inside a Max for
# Live device in Ableton Live.
#
# Setup (see top-level README.md for full instructions):
#   git submodule add --recursive https://github.com/Cycling74/min-devkit ThirdParty/min-devkit
#   cmake -B build -DEEVE_BUILD_MAX_EXTERNAL=ON -DEEVE_BUILD_PLUGIN=OFF
#   cmake --build build --config Release
#
# min-devkit expects to be the CMake source root for its own package/project
# machinery, so this subdirectory just declares the external's sources; the
# actual `min_package_add_target`-style setup happens once min-devkit is
# present and its own CMake includes are pulled in from
# ThirdParty/min-devkit/max-sdk-base/script/min-package.cmake (see min-devkit
# docs — project layout may need `source/projects/eeve1073_tilde/` per
# min-devkit convention; adjust paths below once the submodule is added).

set(MIN_DEVKIT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/min-devkit")

if(NOT EXISTS "${MIN_DEVKIT_DIR}/CMakeLists.txt")
    message(WARNING
        "min-devkit not found at ${MIN_DEVKIT_DIR}. "
        "Run: git submodule add --recursive https://github.com/Cycling74/min-devkit ThirdParty/min-devkit")
    return()
endif()

include("${MIN_DEVKIT_DIR}/max-sdk-base/script/min-package.cmake" OPTIONAL RESULT_VARIABLE MIN_PACKAGE_CMAKE_FOUND)

add_library(eeve1073_tilde MODULE source/eeve1073_tilde.cpp)
target_include_directories(eeve1073_tilde PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../Shared
    ${MIN_DEVKIT_DIR}/min-api/include
    ${MIN_DEVKIT_DIR}/min-lib/include)
set_target_properties(eeve1073_tilde PROPERTIES
    BUNDLE TRUE
    BUNDLE_EXTENSION "mxo"
    OUTPUT_NAME "eeve1073~")
