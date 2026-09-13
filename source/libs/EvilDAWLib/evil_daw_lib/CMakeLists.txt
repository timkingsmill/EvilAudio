include_guard(GLOBAL)

# -----------------------------------------------------------------------------------------------
# EvilDAWLib static library target.

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_FOLDER EvilAudio/applications/EvilDAW)

project(EvilDAWLib VERSION 0.1.0 LANGUAGES C CXX)

reset_log_indent()
message(STATUS "Configuring static library target: ${PROJECT_NAME}")
increment_log_indent()

add_library(${PROJECT_NAME} STATIC)
add_library(evil::EvilDAWLib ALIAS ${PROJECT_NAME})

target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=0
        DONT_SET_USING_JUCE_NAMESPACE=1
)

target_include_directories(${PROJECT_NAME}      
    PRIVATE 
        $<BUILD_INTERFACE:${JUCE_MODULES_DIR}>
    PUBLIC    
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

target_sources(${PROJECT_NAME}
    PUBLIC
        evil_daw_lib.h

    PRIVATE
        evil_daw_lib.cpp

        application/EvilDAWApplication.h
        application/EvilDAWApplication.cpp

        graphs/edaw_graph_document_component.h
        graphs/edaw_graph_document_component.cpp
        
        settings/editors/edaw_audio_settings_editor.h
        settings/editors/edaw_audio_settings_editor.cpp

)

add_subdirectory(plugins)
add_subdirectory(windows)

# -----------------------------------------------------------------------------------------------
# Organize source files into groups in Visual Studio.
get_target_property(target_sources ${PROJECT_NAME} SOURCES)

# Create the Source files group.
source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}"
    PREFIX "Source Files"
    FILES 
        ${target_sources}
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        evil::evil_eq_editors_lib
        evil::evil_plugins_lib
        evil::evil_patterns_lib
    PUBLIC
        evil::juce_audio_devices_lib
        evil::juce_audio_formats_lib
        evil::juce_audio_processors_lib
        evil::juce_audio_processors_headless_lib
        evil::juce_audio_utils_lib
        #evil::juce_box2d_lib
        evil::juce_core_lib
        #evil::juce_cryptography_lib
        evil::juce_data_structures_lib
        evil::juce_dsp_lib
        #evil::juce_events_lib
        evil::juce_graphics_lib
        evil::juce_gui_basics_lib
        evil::juce_gui_extra_lib
        #evil::juce_javascript_lib
)

# Print the properties of the target for debugging purposes. 
# This is useful to verify that the target has been configured correctly 
# with the expected sources, include directories, compile definitions, and linked libraries.
print_target_properties(${PROJECT_NAME})

decrement_log_indent()
message(STATUS "Finished configuring EvilDAWLib Static Library.")
message(STATUS "=================================================================================")

# -----------------------------------------------------------------------------------------------