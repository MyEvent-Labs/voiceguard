# Offline test harness. Runs headless in CI with no audio interface.
juce_add_console_app(voiceguard_tests
    PRODUCT_NAME "VoiceGuard Tests")

target_sources(voiceguard_tests PRIVATE
    TestMain.cpp
    unit/DspUtilsTests.cpp
    unit/StateTests.cpp
    unit/ParametersTests.cpp
    unit/SpeechAnalyzerTests.cpp)

target_include_directories(voiceguard_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(voiceguard_tests
    PRIVATE
        voiceguard_core
        juce::juce_dsp
        juce::juce_audio_processors
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

target_compile_definitions(voiceguard_tests PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0)

if(VOICEGUARD_ENABLE_SANITIZERS AND (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU"))
    target_compile_options(voiceguard_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(voiceguard_tests PRIVATE -fsanitize=address,undefined)
endif()

add_test(NAME voiceguard_tests COMMAND voiceguard_tests)
