# Max for Live external target, built with Cycling '74's min-devkit
# (https://github.com/Cycling74/min-devkit), the modern C++ framework for
# Max/MSP externals. This produces a `.mxo` object usable inside a Max for
# Live device in Ableton Live.
#
# Setup (see top-level README.md for full instructions):
#   git submodule add https://github.com/Cycling74/min-devkit ThirdParty/min-devkit
#   git submodule update --init --recursive
#   cmake -B build-max -GXcode -DEEVE_BUILD_PLUGIN=OFF -DEEVE_BUILD_MAX_EXTERNAL=ON
#   cmake --build build-max --config Release
#
# The actual object lives in eeve1073_tilde/, named to match min-devkit's
# project layout convention (the containing folder name becomes the
# object/target name).

set(MIN_DEVKIT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../ThirdParty/min-devkit")

if(NOT EXISTS "${MIN_DEVKIT_DIR}/source/min-api/script/min-pretarget.cmake")
    message(WARNING
        "min-devkit not found at ${MIN_DEVKIT_DIR}. "
        "Run: git submodule add https://github.com/Cycling74/min-devkit ThirdParty/min-devkit "
        "&& git submodule update --init --recursive")
    return()
endif()

add_subdirectory(eeve1073_tilde)
