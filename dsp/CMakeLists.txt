# Shared DSP library. No JUCE dependency: plain C++17 so it can be unit
# tested offline (SAT-1) and reused by future plugins without pulling in
# plugin-framework code.

add_library(bdpo_dsp STATIC
    saturation/SaturationStage.h
    saturation/SaturationStage.cpp
)

target_include_directories(bdpo_dsp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(bdpo_dsp PUBLIC cxx_std_17)

add_executable(bdpo_dsp_tests tests/SaturationStageTest.cpp)
target_link_libraries(bdpo_dsp_tests PRIVATE bdpo_dsp)

enable_testing()
add_test(NAME SaturationStageOfflineTest COMMAND bdpo_dsp_tests)
