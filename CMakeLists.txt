cmake_minimum_required(VERSION 3.22)

project(WarmDrive VERSION 1.0.0)

# ---------------------------------------------------------------------------
# Download JUCE automatically (pinned version) the first time you configure.
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    JUCE
    URL https://github.com/juce-framework/JUCE/archive/refs/tags/9.0.3.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(JUCE)

# Only copy into the system plug-in folder on the Mac (it needs no admin rights
# there; on Windows it would need admin and fail the build).
if(APPLE)
    set(WARMDRIVE_COPY_AFTER_BUILD TRUE)
else()
    set(WARMDRIVE_COPY_AFTER_BUILD FALSE)
endif()

juce_add_plugin(WarmDrive
    COMPANY_NAME "Ortiz"
    BUNDLE_ID com.ortiz.warmdrive
    PLUGIN_MANUFACTURER_CODE Ortz      # 4 chars, at least one upper-case
    PLUGIN_CODE Wdrv                   # 4 chars, exactly one upper-case
    FORMATS AU VST3 Standalone         # AU is ignored on Windows
    PRODUCT_NAME "Warm Drive"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD ${WARMDRIVE_COPY_AFTER_BUILD}
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE)

target_compile_features(WarmDrive PRIVATE cxx_std_17)

target_sources(WarmDrive
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_compile_definitions(WarmDrive
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(WarmDrive
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)
