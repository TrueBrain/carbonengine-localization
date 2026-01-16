
ccp_add_library(EveLocalizationTest SHARED
    flatbuffers_test_data_generator.cpp
)

set_source_files_properties(
    ${CMAKE_BINARY_DIR}/FlatbuffersGeneratedHeaders/messages_generated.h
    PROPERTIES GENERATED TRUE
)

target_link_libraries(EveLocalizationTest PRIVATE BlueExposure flatbuffers FlatbuffersGeneratedHeaders)
target_include_directories(EveLocalizationTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
