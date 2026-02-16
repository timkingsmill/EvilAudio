cmake_minimum_required(VERSION 4.2.0)

set(CMAKE_FOLDER EvilAudio/plugins/EvilEQPlugin)

# The top-level CMakeLists.txt file for a project must contain a literal, direct call to the
# `project()` command. `project()` sets up some helpful variables that describe source/binary
# directories, and the current project version. This is a standard CMake command.

project(EvilEQPlugin VERSION 0.0.1)

juce_add_plugin(EvilEQPlugin
    # VERSION ...                               # Set this if the plugin version is different to the project version
    # ICON_BIG ...                              # ICON_* arguments specify a path to an image file to use as an icon for the Standalone
    # ICON_SMALL ...
    # COMPANY_NAME ...                          # Specify the name of the plugin's author
    # IS_SYNTH TRUE/FALSE                       # Is this a synth or an effect?
    # NEEDS_MIDI_INPUT TRUE/FALSE               # Does the plugin need midi input?
    # NEEDS_MIDI_OUTPUT TRUE/FALSE              # Does the plugin need midi output?
    # IS_MIDI_EFFECT TRUE/FALSE                 # Is this plugin a MIDI effect?
    # EDITOR_WANTS_KEYBOARD_FOCUS TRUE/FALSE    # Does the editor need keyboard focus?
    # COPY_PLUGIN_AFTER_BUILD TRUE/FALSE        # Should the plugin be installed to a default location after building?
    PLUGIN_MANUFACTURER_CODE OAUD               # A four-character manufacturer id with at least one upper-case character
    PLUGIN_CODE Eq01                            # A unique four-character plugin id with exactly one upper-case character
                                                # GarageBand 10.3 requires the first letter to be upper-case, and the remaining letters to be lower-case
    ###FORMATS AU VST3 Standalone                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3
    FORMATS Standalone                  # The formats to build. Other valid formats are: AAX Unity VST AU AUv3

    PRODUCT_NAME "EvilEQPlugin")        # The name of the final executable, which can differ from the target name

# Create the JuceHeader.h for this target.
juce_generate_juce_header(EvilEQPlugin)

target_sources(EvilEQPlugin
    PRIVATE
        EvilEQPlugin.cpp
)

if(JUCE_ASIO_SUPPORT)
    message(STATUS "Enabling ASIO support for EvilEQPlugin")
    message(STATUS "ASIO_SDK_DIR is set to: ${ASIO_SDK_DIR}")
    target_compile_definitions(EvilEQPlugin PUBLIC JUCE_ASIO=1)
    target_include_directories(EvilEQPlugin PUBLIC "${ASIO_SDK_DIR}")
endif()

target_compile_definitions(EvilEQPlugin
    PUBLIC
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_plugin` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_plugin` call
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(EvilEQPlugin
    PRIVATE
        evil::evil_audio_eq
)       

target_link_libraries(EvilEQPlugin
    PRIVATE 
        juce::juce_audio_utils 
        juce::juce_audio_devices
        juce::juce_audio_basics 
        juce::juce_audio_plugin_client
        juce::juce_core
        juce::juce_dsp 
        juce::juce_gui_extra 
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)


