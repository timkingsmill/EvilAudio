# -----------------------------------------------------------------------------------------------
# EvilDAW executable target.

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FOLDER EvilAudio/applications/EvilDAW)

project(EvilDAW VERSION 0.1.0 LANGUAGES C CXX)

# Set EvilDAW as the startup project in Visual Studio
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT EvilDAW)

message(STATUS "Configuring EvilDAW...")
increment_log_indent()
messageWithColor(COLOR BLUE "Configuring EvilDAW...")

evil_audio_add_gui_app(EvilDAW
    PRODUCT_NAME "EvilDAW"
    VERSION ${PROJECT_VERSION}
    BUNDLE_ID "com.evil_audio.evildaw"
    COMPANY_PRODUCT_NAME "Evil Audio DAW"
    COMPANY_NAME "Evil Audio"
)

#
# Create the JuceHeader.h for this target.
#juce_generate_juce_header(EvilDAW)

# Define preprocessor definitions for this target.
target_compile_definitions(EvilDAW
    PRIVATE
        #JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
        DONT_SET_USING_JUCE_NAMESPACE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:EvilDAW,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:EvilDAW,JUCE_VERSION>"
)

# Set the include directories for this target.
# Note: The `include` directory is added as a private include 
# directory, which means that it will only be used when 
# compiling the source files of the EvilDAW target. 
# This is a common practice to keep the public interface 
# of the target clean and to avoid exposing internal implementation 
# details to other targets that might link against EvilDAW.
target_include_directories(EvilDAW      
    INTERFACE 
        include
)
        
message(STATUS "Adding target sources for EvilDAW...")

target_sources(EvilDAW
    PRIVATE
        evil_daw_main.h
        evil_daw_main.cpp

        gui/evil_daw_main_window.h
        gui/evil_daw_main_window.cpp
        
        include/LiveScrollingAudioVisualiser.h
        include/MainWindowContent.h
        include/AudioSettingsComponent.h
)

# Organize source files into groups in Visual Studio.
get_target_property(target_sources EvilDAW SOURCES)

increment_log_indent()

# Add header files to a separate source group.
set(include_files ${target_sources})
list(FILTER include_files INCLUDE REGEX ".*\\.h$|.*\\.hpp$")
foreach(src IN LISTS include_files)
    message(STATUS "Found header file: ${src}")
endforeach()

# Add source files to a separate source group.
set(source_files ${target_sources})
list(FILTER source_files INCLUDE REGEX ".*\\.cpp$|.*\\.cxx$|.*\\.c$|.*\\.mm$|.*\\.m$")
foreach(src IN LISTS source_files)
    message(STATUS "Found source file: ${src}")
endforeach()

decrement_log_indent()

# Create the Header files group.
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Header Files"
    FILES ${include_files}
)

# Create the Source files group.
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Source Files"
    FILES ${source_files}
)

# Create the JuceHeader.h for this target.
juce_generate_juce_header(EvilDAW)

target_link_static_libraries(EvilDAW
    evil::juce_audio_devices_lib
    evil::juce_audio_utils_lib
    evil::juce_dsp_lib

    evil::evil_audio_core_lib
    evil::evil_audio_lookandfeel_lib
    evil::evil_audio_eq_lib
)

# -----------------------------------------------------------------------------------------------