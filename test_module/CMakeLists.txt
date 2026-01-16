
ccp_add_library(EveLocalizationTest SHARED
    flatbuffers_test_data_generator.cpp
)

target_link_libraries(EveLocalizationTest PRIVATE BlueExposure flatbuffers FlatbuffersGeneratedHeaders)
target_include_directories(EveLocalizationTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
