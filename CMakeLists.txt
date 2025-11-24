cmake_minimum_required(VERSION 3.22)

project(DigiPro_Noise_Canceling VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_gui_app(NCliteApp
    PRODUCT_NAME "NClite"
)

target_sources(NCliteApp PRIVATE
    Main.cpp
    MainComponent.cpp
    AudioEngine.cpp
    NLMSFilter.cpp
    STFTProcessor.cpp
    WienerPost.cpp
)

target_include_directories(NCliteApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(NCliteApp PRIVATE cxx_std_20)

target_compile_definitions(NCliteApp PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    _CRT_SECURE_NO_WARNINGS
)

if (MSVC)
    target_compile_options(NCliteApp PRIVATE
        /Zc:__cplusplus
        /permissive-
        /bigobj
        /W3
    )
endif()

target_link_libraries(NCliteApp PRIVATE
    juce::juce_dsp
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_graphics
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_core
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)

if (WIN32)
    target_link_options(NCliteApp PRIVATE /SUBSYSTEM:WINDOWS)
endif()

if (APPLE)
    set_target_properties(NCliteApp PROPERTIES MACOSX_BUNDLE TRUE)
endif()
