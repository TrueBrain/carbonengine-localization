
ccp_add_library(EveLocalizationTest SHARED
    flatbuffers_test_data_generator.cpp
)

# Using flatbuffers_generate_headers() causes this header to gain the GENERATED property,
# but this property does not propagate to subdirectories, so we have to set it here manually.
# Trying to build without this property set causes CMake to search for the file before it is generated,
# resulting in an error at configure time.
# Alternatively, we can remove this and set CMP0118 after we require CMake 3.20+.
set_source_files_properties(
    ${CMAKE_BINARY_DIR}/FlatbuffersGeneratedHeaders/messages_generated.h
    PROPERTIES GENERATED TRUE
)

target_link_libraries(EveLocalizationTest PRIVATE BlueExposure flatbuffers FlatbuffersGeneratedHeaders)
target_include_directories(EveLocalizationTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
