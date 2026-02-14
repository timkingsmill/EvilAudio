# -----------------------------------------------------------------------------------------------
# EvilDAWLib static library target.

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FOLDER EvilAudio/applications/EvilDAW)

project(EvilDAWLib VERSION 0.1.0 LANGUAGES C CXX)

message(STATUS "Configuring EvilDAWLib...")
increment_log_indent()
messageWithColor(COLOR BLUE "Configuring EvilDAWLib Static Library...")

add_library(EvilDAWLib)

# Define preprocessor definitions for this target.
target_compile_definitions(EvilDAWLib
    PRIVATE
        #JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
        DONT_SET_USING_JUCE_NAMESPACE=1
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
        JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
)

# Set the include directories for this target.
# Note: The `include` directory is added as a private include 
# directory, which means that it will only be used when 
# compiling the source files of the EvilDAW target. 
# This is a common practice to keep the public interface 
# of the target clean and to avoid exposing internal implementation 
# details to other targets that might link against EvilDAWLib.

target_include_directories(EvilDAWLib      
    PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
)

message(STATUS "Adding target sources for EvilDAWLib...")

target_sources(EvilDAWLib
    PRIVATE
        application/evil_daw_main.h
        application/evil_daw_main.cpp

        application/evil_daw_application.h
        application/evil_daw_application.cpp

        application/evil_daw_application_info.h
        application/evil_daw_application_info.cpp

        application/evil_daw_application_settings.h
        application/evil_daw_application_settings.cpp

        settings/evil_daw_appearance_settings.h
        settings/evil_daw_appearance_settings.cpp

        windows/main/evil_daw_main_window.h
        windows/main/evil_daw_main_window.cpp

        windows/main/evil_daw_main_window_content.h
        windows/main/evil_daw_main_window_content.cpp

        windows/main/evil_daw_main_window_menu.h
        windows/main/evil_daw_main_window_menu.cpp
       
        windows/about/evil_daw_about_window.h
        windows/about/evil_daw_about_window.cpp

        include/LiveScrollingAudioVisualiser.h
        include/MainWindowContent.h
        include/AudioSettingsComponent.h
)

# Organize source files into groups in Visual Studio.
get_target_property(target_sources EvilDAWLib SOURCES)

# Create the Source files group.
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Source Files"
    FILES 
        ${target_sources}
)

# Don't need to specify the juce header file for this.
# Use include directories instead to make it available for the target's source files.
#### juce_generate_juce_header(EvilDAWLib)

#target_link_static_libraries(EvilDAWLib

target_link_libraries(EvilDAWLib
    PUBLIC
        evil::juce_audio_devices_lib    
        evil::juce_audio_utils_lib
        evil::juce_dsp_lib
        evil::evil_audio_core_lib
        evil::evil_audio_lookandfeel_lib
        evil::evil_audio_eq_lib
)

# -----------------------------------------------------------------------------------------------