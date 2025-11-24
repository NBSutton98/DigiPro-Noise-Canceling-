cmake_minimum_required(VERSION 3.22)

project(DigiPro_Noise_Canceling VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(JUCE)

juce_add_plugin(NClite
    COMPANY_NAME "BCIT Students"    
    IS_SYNTH FALSE               
    NEEDS_MIDI_INPUT FALSE      
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE

    FORMATS VST3 Standalone       
    
    PLUGIN_CODE NClt            
    PRODUCT_NAME "NClite Noise Reducer"
)

target_sources(NClite PRIVATE         
    MainComponent.cpp
    AudioEngine.cpp
    NLMSFilter.cpp
    STFTProcessor.cpp
    WienerPost.cpp
    PluginProcessor.cpp
)

target_include_directories(NClite PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_features(NClite PRIVATE cxx_std_20)

target_compile_definitions(NClite PRIVATE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    JUCE_MODAL_LOOPS_PERMITTED=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0
    _CRT_SECURE_NO_WARNINGS
    JUCE_VST3_CAN_REPLACE_VST2=0
)

if (MSVC)
    target_compile_options(NClite PRIVATE
        /Zc:__cplusplus
        /permissive-
        /bigobj
        /W3
    )
endif()

target_link_libraries(NClite PRIVATE
    juce::juce_dsp
    juce::juce_audio_utils
    juce::juce_gui_extra
    juce::juce_graphics
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_core
    juce::juce_audio_processors 
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)