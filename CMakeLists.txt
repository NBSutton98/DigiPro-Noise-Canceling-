cmake_minimum_required(VERSION 3.22)
project(DigiPro_Noise_Canceling VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# JUCE from submodule
add_subdirectory(JUCE)

# Create a JUCE GUI app target
juce_add_gui_app(NCliteApp
        PRODUCT_NAME "NClite"
)

# List your sources (no src/ prefix)
target_sources(NCliteApp PRIVATE
        Main.cpp
        MainComponent.cpp
        AudioEngine.cpp
        NLMSFilter.cpp
        STFTProcessor.cpp
        WienerPost.cpp
)

target_include_directories(NCliteApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(NCliteApp PRIVATE
        juce::juce_dsp
        juce::juce_audio_utils
        juce::juce_gui_extra
        juce::juce_graphics
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_core
)

# Optional platform tweaks
if (WIN32)
    target_link_options(NCliteApp PRIVATE /SUBSYSTEM:WINDOWS)
endif()

if (APPLE)
    set_target_properties(NCliteApp PROPERTIES MACOSX_BUNDLE TRUE)
endif()
