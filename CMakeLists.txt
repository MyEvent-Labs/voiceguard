cmake_minimum_required(VERSION 3.22)

project(VoiceGuard
    VERSION 0.0.1
    DESCRIPTION "MyEvent Labs VoiceGuard - real-time speech enhancement for live events"
    HOMEPAGE_URL "https://github.com/MyEvent-Labs/voiceguard"
    LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Universal binary")
endif()

option(VOICEGUARD_BUILD_TESTS "Build offline unit / DSP regression tests" ON)
option(VOICEGUARD_BUILD_TOOLS "Build offline signal generator and benchmark tools" ON)
option(VOICEGUARD_ENABLE_SANITIZERS "Enable ASan/UBSan on supported toolchains" OFF)
set(VOICEGUARD_JUCE_PATH "" CACHE PATH "Path to a local JUCE 8 checkout (offline builds)")

include(cmake/VoiceGuardJuce.cmake)

# ---------------------------------------------------------------------------
# Shared DSP core. No GUI, no plugin-format code. Links into the plugin and
# is exercised directly by the offline test harness.
# ---------------------------------------------------------------------------
add_library(voiceguard_core STATIC
    src/dsp/AutoController.cpp
    src/dsp/Biquad.cpp
    src/dsp/DeEsser.cpp
    src/dsp/DspUtils.cpp
    src/dsp/DynamicCutBank.cpp
    src/dsp/DynamicEq.cpp
    src/dsp/Dynamics.cpp
    src/dsp/HarshnessControl.cpp
    src/dsp/InputConditioning.cpp
    src/dsp/LevelRider.cpp
    src/dsp/NoiseReduction.cpp
    src/dsp/OutputSafety.cpp
    src/dsp/Parameters.cpp
    src/dsp/SpeechAnalyzer.cpp
    src/dsp/VoiceGuardEngine.cpp
    src/state/StateIO.cpp)

target_include_directories(voiceguard_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(voiceguard_core
    PUBLIC
        juce::juce_audio_basics
        juce::juce_dsp
        juce::juce_audio_processors
        juce::juce_data_structures
    PRIVATE
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

target_compile_definitions(voiceguard_core
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_MODAL_LOOPS_PERMITTED=0
        JUCE_REPORT_APP_USAGE=0)

# ---------------------------------------------------------------------------
# Plugin targets
# ---------------------------------------------------------------------------
juce_add_plugin(VoiceGuard
    COMPANY_NAME              "MyEvent Labs"
    BUNDLE_ID                 "com.myeventlabs.voiceguard"
    PLUGIN_MANUFACTURER_CODE  MyEv
    PLUGIN_CODE               Vgrd
    FORMATS                   VST3 AU Standalone
    PRODUCT_NAME              "MyEvent Labs VoiceGuard"
    DESCRIPTION               "Live speech enhancement plugin (ALPHA)"
    IS_SYNTH                  FALSE
    NEEDS_MIDI_INPUT          FALSE
    NEEDS_MIDI_OUTPUT         FALSE
    IS_MIDI_EFFECT            FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD   FALSE
    VST3_CATEGORIES           Fx
    AU_MAIN_TYPE              kAudioUnitType_Effect
    MICROPHONE_PERMISSION_ENABLED TRUE
    MICROPHONE_PERMISSION_TEXT "VoiceGuard standalone uses audio input for local speech analysis and enhancement. Audio never leaves this computer.")

target_sources(VoiceGuard PRIVATE
    src/PluginProcessor.cpp
    src/PluginEditor.cpp
    src/ui/VoiceGuardLookAndFeel.cpp
    src/ui/LevelMeter.cpp)

target_include_directories(VoiceGuard PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(VoiceGuard
    PRIVATE
        voiceguard_core
        juce::juce_audio_utils
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

juce_generate_juce_header(VoiceGuard)

# ---------------------------------------------------------------------------
# Tests and tools
# ---------------------------------------------------------------------------
if(VOICEGUARD_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(VOICEGUARD_BUILD_TOOLS)
    add_subdirectory(tools)
endif()

message(STATUS "VoiceGuard ${PROJECT_VERSION} alpha - tests=${VOICEGUARD_BUILD_TESTS} tools=${VOICEGUARD_BUILD_TOOLS}")
