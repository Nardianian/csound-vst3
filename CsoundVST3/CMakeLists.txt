cmake_minimum_required(VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ----------------------------------------
# VARIABLES
set(PROJECT_NAME "CsoundVST3")
set(TARGET_NAME "CsoundVST3")
set(VERSION_NUMBER 0.1.0)

# ----------------------------------------

# PROJECT INIT
project(${PROJECT_NAME} VERSION ${VERSION_NUMBER})

# PACKAGE INCLUDE
add_subdirectory(include/JUCE JUCE)

# JUCE CONFIG
juce_add_plugin(${TARGET_NAME}
  PRODUCT_NAME "${TARGET_NAME}"

  # manufacturer
  COMPANY_NAME "Irreducible Productions"
  PLUGIN_MANUFACTURER_CODE "MGIP"
  BUNDLE_ID "com.manufacturer.plugin" # [EDIT]

  # plugin
  PLUGIN_NAME "CsoundVST3"
  PLUGIN_CODE "VST3"
  VST3_CATEGORIES Fx, Instrument
  AU_MAIN_TYPE kAudioUnitType_MusicDevice

  # executable
  FORMATS Standalone VST3 AU
  COPY_PLUGIN_AFTER_BUILD TRUE

  IS_SYNTH TRUE
)

target_compile_definitions(${TARGET_NAME}
  PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0

  DONT_SET_USING_JUCE_NAMESPACE=1

  CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
  VERSION="${CURRENT_VERSION}"

  PRODUCT_NAME_WITHOUT_VERSION="${TARGET_NAME}"
)

# ADD FILES
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode/*.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h"
)
target_sources(${TARGET_NAME} PRIVATE ${SOURCE_FILES})

target_include_directories(${TARGET_NAME}
    PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/JuceLibraryCode"
    "C:/Program Files/Csound6_x64/include/csound"
    )

# BINARY DATA
file(GLOB_RECURSE BINARY_FILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*")
list(FILTER BINARY_FILES EXCLUDE REGEX "/\\.DS_Store$")
juce_add_binary_data(BinaryData SOURCES ${BINARY_FILES})

# ADD LIBRARIES
target_link_libraries(${TARGET_NAME}
  PRIVATE
  BinaryData

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

  PUBLIC
  juce::juce_recommended_config_flags
  juce::juce_recommended_lto_flags
  juce::juce_recommended_warning_flags

  "C:/Users/daves/Documents/libsndfile-1.2.2-win64/lib/sndfile.lib"
  "C:/Program Files/Csound6_x64/lib/libcsound64.lib"
)