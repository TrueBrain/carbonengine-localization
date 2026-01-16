
ccp_add_library(EveLocalizationTest SHARED
    flatbuffers_test_data_generator.cpp
)

# FlatbuffersGeneratedHeaders is an INTERFACE library. The actual header generation
# happens in GENERATE_FlatbuffersGeneratedHeaders. Since we're in a subdirectory,
# we must explicitly depend on it to avoid a race condition.
# See the documentation for flatbuffers_generate_headers() here:
# https://github.com/google/flatbuffers/blob/8914d06ab7123167424438cf293bb349833bcb7d/CMake/BuildFlatBuffers.cmake#L163
# Which says:
# When the target_link_libraries is done within a different directory than
# flatbuffers_generate_headers is called, then the target should also be dependent
# the custom generation target called GENERATE_<TARGET>.
add_dependencies(EveLocalizationTest GENERATE_FlatbuffersGeneratedHeaders)

target_link_libraries(EveLocalizationTest PRIVATE BlueExposure flatbuffers FlatbuffersGeneratedHeaders)
target_include_directories(EveLocalizationTest PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
