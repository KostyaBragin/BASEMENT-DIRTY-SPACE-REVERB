cmake_minimum_required(VERSION 3.22)
project(SMProductionGlue VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(FetchContent)
FetchContent_Declare(JUCE GIT_REPOSITORY https://github.com/juce-framework/JUCE.git GIT_TAG 8.0.6 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(JUCE)
juce_add_plugin(SMProductionGlue
 COMPANY_NAME "SM Production"
 BUNDLE_ID com.smproduction.glue
 PLUGIN_MANUFACTURER_CODE SMPD
 PLUGIN_CODE SGLU
 FORMATS VST3
 PRODUCT_NAME "SM Production GLUE"
 IS_SYNTH FALSE NEEDS_MIDI_INPUT FALSE NEEDS_MIDI_OUTPUT FALSE IS_MIDI_EFFECT FALSE
 EDITOR_WANTS_KEYBOARD_FOCUS FALSE COPY_PLUGIN_AFTER_BUILD FALSE)
target_sources(SMProductionGlue PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp)
target_compile_definitions(SMProductionGlue PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(SMProductionGlue PRIVATE juce::juce_audio_utils juce::juce_dsp PUBLIC juce_recommended_config_flags juce_recommended_lto_flags juce_recommended_warning_flags)