cmake_minimum_required(VERSION 3.15)

project(CsoundVST3 VERSION 1.0.0)

# Require C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set JUCE root (update path if necessary)
set(JUCE_ROOT "${CMAKE_SOURCE_DIR}/include/JUCE")
add_subdirectory(${JUCE_ROOT})

# First, find Csound for non-macOS platforms.

if (NOT APPLE)
    # Allow the user to specify a custom root path for Csound
    set(CSOUND_ROOT "" CACHE PATH "Root directory of the Csound installation")

    # Platform-specific hints
    if(WIN32)
        set(CSOUND_POSSIBLE_PATHS
            "${CSOUND_ROOT}"
            "C:/Program Files/Csound6_x64"
            "C:/Program Files (x86)/Csound6_x86"
        )
    elseif(UNIX)
        set(CSOUND_POSSIBLE_PATHS
            "${CSOUND_ROOT}"
            "/usr"
            "/usr/local"
            "/opt"
        )
    endif()

    # Find include directory
    find_path(CSOUND_INCLUDE_DIR
        NAMES csound/csound.hpp
        PATH_SUFFIXES include
        PATHS ${CSOUND_POSSIBLE_PATHS}
        NO_DEFAULT_PATH
    )

    # Find library
    find_library(CSOUND_LIBRARY
        NAMES csound64 CsoundLib64 csound
        PATH_SUFFIXES lib
        PATHS ${CSOUND_POSSIBLE_PATHS}
        NO_DEFAULT_PATH
    )

    # Error out if not found
    if(NOT CSOUND_INCLUDE_DIR OR NOT CSOUND_LIBRARY)
        message(FATAL_ERROR "Csound library or headers not found. Set CSOUND_ROOT or install Csound.")
    endif()
endif()

# Define plugin
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
)

juce_add_binary_data(CsoundBinaryData
    SOURCES
        Resources/angel_concert.png
        Resources/angel_concert.icns
        # Resources/README.md
)

# Add source files
target_sources(CsoundVST3 PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/CsoundTokeniser.cpp
)

target_include_directories(CsoundVST3 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# JUCE module dependencies
target_link_libraries(CsoundVST3 PRIVATE 
    CsoundBinaryData
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

# Compile definitions
target_compile_definitions(CsoundVST3 PRIVATE
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_VST3_CAN_REPLACE_VST2=0
)

# Add icon and other resource files
set(resources
    Resources/angel_concert.png
    Resources/angel_concert.icns
    Resources/README.md
)

# macOS specific bundle properties
if(APPLE)
    # List of plugin format targets to individually set.
    set(CSOUNDVST3_TARGETS
        CsoundVST3
        CsoundVST3_Standalone
        CsoundVST3_VST3
        CsoundVST3_AU
    )

    foreach(target IN LISTS CSOUNDVST3_TARGETS)
        if(TARGET ${target})
            target_include_directories(${target} PRIVATE /Library/Frameworks/CsoundLib64.framework/Headers)
            target_link_libraries(${target} PRIVATE "-framework CsoundLib64")
            target_link_options(${target} PRIVATE -F/Library/Frameworks)
        endif()
    endforeach()


    set(resources
        Resources/angel_concert.png
        Resources/angel_concert.icns
    )

    foreach(target IN ITEMS CsoundVST3 CsoundVST3_VST3 CsoundVST3_AU CsoundVST3_Standalone)
        if(TARGET ${target})
            set_source_files_properties(Resources/angel_concert.icns PROPERTIES
                MACOSX_PACKAGE_LOCATION "Resources"
            )
            set_target_properties(${target} PROPERTIES
                MACOSX_BUNDLE TRUE
                MACOSX_BUNDLE_ICON_FILE angel_concert.icns
                RESOURCE "${resources}"
            )
        endif()
    endforeach()

    # Code signing
    if(CMAKE_GENERATOR STREQUAL "Xcode")
        set_xcode_property(CsoundVST3 CODE_SIGN_IDENTITY "@michael_gogins" "ALL")
    endif()
elseif(WIN32)
    include_directories("C:/path/to/csound/include")
    link_directories("C:/path/to/csound/lib")
    target_link_libraries(CsoundVST3 PRIVATE ${CSOUND_LIBRARY})
elseif(UNIX)
    include_directories("/usr/include/csound" "/opt/homebrew/include/csound")
    link_directories("/usr/lib" "/opt/homebrew/lib")
    target_link_libraries(CsoundVST3 PRIVATE ${CSOUND_LIBRARY})
endif()

# Code signing for Xcode builds
if(APPLE AND CMAKE_GENERATOR STREQUAL "Xcode")
    set_xcode_property(CsoundVST3 CODE_SIGN_IDENTITY "@michael_gogins" "ALL")
endif()

if(CSOUND_INCLUDE_DIR)
    target_include_directories(CsoundVST3 PRIVATE ${CSOUND_INCLUDE_DIR})
else()
    message(WARNING "Csound headers not found; assuming they are not needed directly.")
endif()

# Install target (optional)
install(TARGETS CsoundVST3
    BUNDLE DESTINATION bin
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)
