
# Set the folder structure for IDEs
cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH parent_path)
cmake_path(GET parent_path FILENAME parent_folder_name)
set(CMAKE_FOLDER ${PROJECT_NAME}/${parent_folder_name}/juce)

# Create static libraries for JUCE modules.

set(JUCE_MODULES_DIR "external/JUCE/modules")
message(STATUS "Searching for JUCE modules in: ${JUCE_MODULES_DIR}")

increment_log_indent()

# create a list of all directories in the JUCE modules directory
file(GLOB module_dirs RELATIVE ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/${JUCE_MODULES_DIR}/*)
list(SORT module_dirs)

foreach(dir ${module_dirs})
    if(IS_DIRECTORY ${dir})
        set(module_path "${PROJECT_SOURCE_DIR}/${dir}")
        message(STATUS "Found module source path: ${module_path}")
        increment_log_indent()
        evil_add_static_lib(${module_path})
        decrement_log_indent()
    endif()
endforeach()

#[[  DEPRICATED: This is no longer needed, as we now use the CMake configuration 
     provided by JUCE, which defines the JUCE modules as imported targets.


add_juce_module_static_library(juce::juce_core)
add_juce_module_static_library(juce::juce_audio_basics)
add_juce_module_static_library(juce::juce_audio_devices)
add_juce_module_static_library(juce::juce_audio_formats)
add_juce_module_static_library(juce::juce_audio_processors) 
add_juce_module_static_library(juce::juce_audio_processors_headless) 
add_juce_module_static_library(juce::juce_audio_utils)
add_juce_module_static_library(juce::juce_dsp)
add_juce_module_static_library(juce::juce_data_structures)
add_juce_module_static_library(juce::juce_events)
add_juce_module_static_library(juce::juce_graphics)
add_juce_module_static_library(juce::juce_gui_basics)
add_juce_module_static_library(juce::juce_gui_extra)

]]