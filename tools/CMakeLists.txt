# Offline tools: render the editor to a PNG for visual review.
juce_add_console_app(voiceguard_ui_snapshot
    PRODUCT_NAME "VoiceGuard UI Snapshot")

target_sources(voiceguard_ui_snapshot PRIVATE
    ui_snapshot/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/PluginProcessor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/PluginEditor.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/VoiceGuardLookAndFeel.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/LevelMeter.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../src/ui/ReductionMeter.cpp)

target_include_directories(voiceguard_ui_snapshot PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_libraries(voiceguard_ui_snapshot
    PRIVATE
        voiceguard_core
        juce::juce_audio_utils
        juce::juce_gui_extra
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

target_compile_definitions(voiceguard_ui_snapshot PRIVATE
    JUCE_STANDALONE_APPLICATION=1
    JUCE_USE_CURL=0
    JUCE_WEB_BROWSER=0)
