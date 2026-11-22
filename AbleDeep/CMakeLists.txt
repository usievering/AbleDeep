cmake_minimum_required(VERSION 3.22)

project(AbleDeep VERSION 1.2.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")

# Plugin-Formate (Standard: VST3 + AU auf dem Mac, VST3 auf Windows)
if(APPLE)
    set(ABLEDEEP_FORMATS "VST3;AU;Standalone" CACHE STRING "Plugin formats")
else()
    set(ABLEDEEP_FORMATS "VST3;Standalone" CACHE STRING "Plugin formats")
endif()

# JUCE: entweder lokaler Ordner (-DJUCE_DIR=...) oder automatischer Download
set(JUCE_DIR "" CACHE PATH "Path to a local JUCE checkout (optional)")
if(JUCE_DIR)
    add_subdirectory(${JUCE_DIR} JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG 8.0.4
        GIT_SHALLOW ON)
    FetchContent_MakeAvailable(JUCE)
endif()

juce_add_plugin(AbleDeep
    COMPANY_NAME "AbleDeep Audio"
    COMPANY_WEBSITE ""
    BUNDLE_ID "com.abledeep.abledeep"
    PLUGIN_MANUFACTURER_CODE Abdp
    PLUGIN_CODE Adh1
    FORMATS ${ABLEDEEP_FORMATS}
    PRODUCT_NAME "AbleDeep"
    DESCRIPTION "Deep & Soulful House Instrument"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Synth
    AU_MAIN_TYPE kAudioUnitType_MusicDevice)

target_sources(AbleDeep PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_include_directories(AbleDeep PRIVATE Source)

# Quelltexte sind UTF-8 (Umlaute in Texten der Oberfläche)
if(MSVC)
    target_compile_options(AbleDeep PRIVATE /utf-8)
endif()

target_compile_definitions(AbleDeep PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)

if("Standalone" IN_LIST ABLEDEEP_FORMATS)
    target_link_libraries(AbleDeep PRIVATE juce::juce_audio_utils)
endif()

target_link_libraries(AbleDeep
    PRIVATE
        juce::juce_audio_processors
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
