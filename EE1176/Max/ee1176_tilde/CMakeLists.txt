# ee1176~ Max/MSP external, following the standard min-devkit project
# layout: the folder name becomes the object/target name (via
# max-pretarget.cmake's THIS_FOLDER_NAME logic), so this directory must be
# named exactly "ee1176_tilde" and contain "ee1176_tilde.cpp".

cmake_minimum_required(VERSION 3.19)

# Keep the built .mxo inside the project tree (Max/externals) instead of the
# max-sdk-base default of ../../../externals, which would land outside this
# repository.
if(NOT DEFINED C74_LIBRARY_OUTPUT_DIRECTORY)
    set(C74_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../externals")
endif()

set(C74_MIN_API_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../ThirdParty/min-devkit/source/min-api)
include(${C74_MIN_API_DIR}/script/min-pretarget.cmake)

include_directories(
    "${C74_INCLUDES}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Shared"
)

set(SOURCE_FILES ${PROJECT_NAME}.cpp)

add_library(${PROJECT_NAME} MODULE ${SOURCE_FILES})

include(${C74_MIN_API_DIR}/script/min-posttarget.cmake)
