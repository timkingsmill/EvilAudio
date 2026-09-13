include_guard(GLOBAL)

project(JuceStaticLibs VERSION 0.1.0 LANGUAGES C CXX)

reset_log_indent()
message(STATUS "===============================================================")
message(STATUS "Configuring static library target: ${PROJECT_NAME}")

# Add the cmake file that defines the evil_add_static_lib function,
# which we will use to create static libraries for the JUCE modules.
include(${CMAKE_SOURCE_DIR}/extras/cmake/EvilAudioStaticLibUtils.cmake)

# Set the folder structure for IDEs

increment_log_indent()

# Build the relative path for the current project, which will be used to 
# set the CMAKE_FOLDER property for the targets defined in this CMakeLists.txt file.
cmake_path(GET PROJECT_SOURCE_DIR PARENT_PATH _libs_folder_name)
cmake_path(GET _libs_folder_name STEM _libs_folder_name)
cmake_path(GET PROJECT_SOURCE_DIR STEM _static_libs_folder_name)
set(CMAKE_FOLDER EvilAudio/${_libs_folder_name}/${_static_libs_folder_name})

# Create static libraries for JUCE modules.

set(JUCE_MODULES_DIR "${CMAKE_SOURCE_DIR}/external/JUCE/modules")
message(STATUS "Searching for JUCE modules in: ${JUCE_MODULES_DIR}")

# create a list of all directories in the JUCE modules directory
# Module path is the full path to the module directory 
# (e.g. /path/to/JUCE/modules/juce_core)
file(GLOB module_dirs ${JUCE_MODULES_DIR}/*)
list(SORT module_dirs)

foreach(module_dir ${module_dirs})
    if(IS_DIRECTORY ${module_dir})
        message(STATUS "Found JUCE module directory: ${module_dir}")
        increment_log_indent()
        evil_add_static_lib(${module_dir})
        decrement_log_indent()
    endif()
endforeach()

decrement_log_indent()
decrement_log_indent()
