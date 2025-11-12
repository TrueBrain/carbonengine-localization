cmake_minimum_required(VERSION 3.16)

project(EveLocalization VERSION 1.0.0)

option(BUILD_FLATBUFFERS_TEST_DATA_GENERATOR "Build the flatbuffers test data generator tool" OFF)

if(NOT DEFINED ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "Missing required environment variable CCP_EVE_PERFORCE_BRANCH_PATH")
elseif(NOT IS_ABSOLUTE $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be an absolute path")
elseif(NOT EXISTS $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH is pointing at a non-existing location $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
elseif(NOT IS_DIRECTORY $ENV{CCP_EVE_PERFORCE_BRANCH_PATH})
    message(FATAL_ERROR "CCP_EVE_PERFORCE_BRANCH_PATH needs to be a directory")
else()
    message(STATUS "Using CCP_EVE_PERFORCE_BRANCH_PATH located at $ENV{CCP_EVE_PERFORCE_BRANCH_PATH}")
endif()

file(TO_CMAKE_PATH "$ENV{CCP_EVE_PERFORCE_BRANCH_PATH}" BRANCH_ROOT_DIR)
list(INSERT CMAKE_MODULE_PATH 0 ${BRANCH_ROOT_DIR}/cmake)
include(CcpGlobalSettings)

find_package(Blue CONFIG REQUIRED)
find_package(BlueExposure CONFIG REQUIRED)
find_package(flatbuffers CONFIG REQUIRED)

set(headers
    autoversion.h
    collator.h
    formatters.h
    localization.h
    messages_generated.h
    parser.h
    version.h
    WrapPointList.h
)

set(sources
    collator.cpp
    collator.mm
    collator_blue.cpp
    formatters.cpp
    localization.cpp
    localization_blue.cpp
    parser.cpp
    parser_blue.cpp
    stdafx.cpp
    WrapPointList.cpp
    WrapPointList_Blue.cpp
)

add_library(EveLocalization SHARED
    ${headers}
    ${sources}
)
target_link_libraries(EveLocalization PRIVATE Blue BlueExposure flatbuffers::flatbuffers)

target_precompile_headers(EveLocalization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/stdafx.h)
target_include_directories(EveLocalization PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(EveLocalization PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    OUTPUT_NAME "_evelocalization"
)

if(APPLE)
    enable_language(OBJCXX)
    set_source_files_properties(${sources} PROPERTIES LANGUAGE OBJCXX)

    set_target_properties(
        EveLocalization
        PROPERTIES 
            PREFIX ""
            SUFFIX ".so"
    )
    target_link_libraries(EveLocalization PRIVATE "-framework Foundation")

elseif(WIN32)

    target_link_libraries(EveLocalization PRIVATE usp10)

endif()

# Provide an install target iff this is the top level project only
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    # Install rule to ensure that our runtime and linker files are in the expected, platform-specific folders
    install(
        TARGETS EveLocalization
        CONFIGURATIONS ${CMAKE_CONFIGURATION_TYPES}
        ARCHIVE DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        LIBRARY DESTINATION lib/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
        RUNTIME DESTINATION bin/${CCP_PLATFORM}/${CCP_ARCHITECTURE}/${CCP_TOOLSET}/
    )

    # Install python package
    install(DIRECTORY python/ DESTINATION python
        FILES_MATCHING
        PATTERN "*.py"
        PATTERN "package.yaml"
    )
endif()

if(BUILD_FLATBUFFERS_TEST_DATA_GENERATOR)
    add_executable(generate_flatbuffers_test_data
        generate_flatbuffers_test_data.cpp
    )
    target_link_libraries(generate_flatbuffers_test_data PRIVATE flatbuffers::flatbuffers)
    target_include_directories(generate_flatbuffers_test_data PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()
