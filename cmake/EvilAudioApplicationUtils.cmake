include_guard(GLOBAL)

# @brief Configures a GUI application target with JUCE framework settings.
#
# This function sets up a GUI application for cross-platform compilation using JUCE.
# On Android, it creates a shared library; on other platforms, it creates an executable.
# The target is configured with JUCE-specific properties and output naming.
#
# @param target The target name of the GUI application to configure.
# @param ARGN Additional arguments passed to _juce_initialise_target().
#
# @details
# - Sets JUCE_STANDALONE_APPLICATION=1 as a private compile definition
# - Initializes the target with JUCE settings
# - Configures the output name using the JUCE_PRODUCT_NAME property
# - Sets the JUCE_TARGET_KIND_STRING property to "App"
# - Configures platform-specific bundle settings
# - Does not automatically add resources.rc (can be customized separately)
#
# @note Resources.rc is intentionally not added automatically to allow for customization.
#
# @see _juce_initialise_target()
# @see _juce_set_output_name()
# @see _juce_configure_bundle()
# @see _juce_configure_app_bundle()
function(evil_audio_add_gui_app target)
    if(CMAKE_SYSTEM_NAME STREQUAL "Android")
        add_library(${target} SHARED)
    else()
        add_executable(${target})
    endif()

    target_compile_definitions(${target} PRIVATE JUCE_STANDALONE_APPLICATION=1)
    _juce_initialise_target(${target} ${ARGN})
    _juce_set_output_name(${target} $<TARGET_PROPERTY:${target},JUCE_PRODUCT_NAME>)
    set_target_properties(${target} PROPERTIES JUCE_TARGET_KIND_STRING "App")
    _juce_configure_bundle(${target} ${target})
    _juce_configure_app_bundle(${target} ${target})
    # Don't add resources.rc automatically, as we may want to customize it.
    #_juce_add_resources_rc(${target} ${target})
endfunction()