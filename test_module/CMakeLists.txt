
ccp_add_library(EveLocalizationTest SHARED
    flatbuffers_test_data_generator.cpp
)

# FlatbuffersGeneratedHeaders is an INTERFACE target, so it needs an explicit dependency
# to avoid a race condition between this target and the header generation
add_dependencies(EveLocalizationTest FlatbuffersGeneratedHeaders)

target_link_libraries(EveLocalizationTest PRIVATE BlueExposure flatbuffers FlatbuffersGeneratedHeaders)
target_include_directories(EveLocalizationTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
