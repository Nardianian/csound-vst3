cmake_minimum_required(VERSION 3.15)
project(CsoundVST3 VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# JUCE setup
set(JUCE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include/JUCE")
add_subdirectory(${JUCE_ROOT})

# Main plugin target
juce_add_plugin(CsoundVST3
    COMPANY_NAME "Irreducible Productions"
    COMPANY_WEBSITE "https://michaelgogins.tumblr.com"
    COMPANY_EMAIL "michael.gogins@gmail.com"
    IS_SYNTH TRUE
    IS_MIDI_EFFECT TRUE
    NEEDS_MIDI_INPUT TRUE
    FORMATS AU VST3 Standalone
    VST3_CATEGORIES "Fx;Instrument"
    AAX_CATEGORY "16"
    AUMAIN_TYPE "aumu"
    PLUGIN_MANUFACTURER_CODE IrPr
    PLUGIN_CODE Csv3
    PRODUCT_NAME "CsoundVST3"
    COPY_PLUGIN_AFTER_BUILD TRUE
    FILES
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/CsoundTokeniser.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.h
)

# Binary resources
juce_add_binary_data(BinaryData
    SOURCES
        Resources/angel_concert.png
        Resources/angel_concert.icns
)

# Link common libraries and resources
target_link_libraries(CsoundVST3 PRIVATE
    BinaryData

    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)

# JUCE-specific compile definitions
target_compile_definitions(CsoundVST3 PRIVATE
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Framework linking and includes
if(APPLE)
    set(CSOUND_FRAMEWORK_PATH "/Library/Frameworks")
    set(CSOUND_FRAMEWORK_NAME "CsoundLib64")
    set(CSOUND_FRAMEWORK_HEADER_PATH "${CSOUND_FRAMEWORK_PATH}/${CSOUND_FRAMEWORK_NAME}.framework/Headers")

    foreach(tgt CsoundVST3 CsoundVST3_Standalone CsoundVST3_AU CsoundVST3_VST3)
        if(TARGET ${tgt})
            target_include_directories(${tgt} PRIVATE "${CSOUND_FRAMEWORK_HEADER_PATH}")
            target_link_options(${tgt} PRIVATE "-F${CSOUND_FRAMEWORK_PATH}")
            target_link_libraries(${tgt} PRIVATE "-framework ${CSOUND_FRAMEWORK_NAME}"
                BinaryData
                juce::juce_audio_basics
                juce::juce_audio_devices
                juce::juce_audio_formats
                juce::juce_audio_plugin_client
                juce::juce_audio_processors
                juce::juce_audio_utils
                juce::juce_core
                juce::juce_data_structures
                juce::juce_events
                juce::juce_graphics
                juce::juce_gui_basics
                juce::juce_gui_extra
            )
        endif()
    endforeach()
    # macOS app icon
    set_source_files_properties(Resources/angel_concert.icns PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(CsoundVST3 PROPERTIES
        MACOSX_BUNDLE_ICON_FILE angel_concert.icns
        RESOURCE "Resources/angel_concert.icns"
    )
elseif(WIN32)
    include_directories("C:/path/to/csound/include")
    link_directories("C:/path/to/csound/lib")
    target_link_libraries(CsoundVST3 PRIVATE csound64)
elseif(UNIX)
    include_directories("/usr/include/csound" "/opt/homebrew/include/csound")
    link_directories("/usr/lib" "/opt/homebrew/lib")
    target_link_libraries(CsoundVST3 PRIVATE csound64)
endif()

# Optional: Install plugin targets if needed
# install(TARGETS CsoundVST3 DESTINATION ...)

