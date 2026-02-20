cmake_minimum_required(VERSION 4.2.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

# Set C++23 as the default for all targets
set(CMAKE_CXX_STANDARD 23)
# Ensure the compiler supports the specified standard
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
# Disable compiler-specific extensions by default
set(CMAKE_CXX_EXTENSIONS OFF) 

# Prevent in-source builds
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please create a separate build directory and run CMake from there.")
endif()

# -----------------------------------------------------------------------------------------------
project(EvilAudio VERSION 0.1.0 LANGUAGES C CXX)

include(CMakeDependentOption)
include(CMakePrintHelpers)
include(cmake/EvilAudio.cmake)
include(cmake/EvilAudioLib.cmake)
include(cmake/EvilAudioASIO.cmake)

# Add the JUCE libaray to CMAKE_PREFIX_PATH for find_package

# Don't use any install targets for JUCE, as we want to use the CMake 
# configuration provided by JUCE, but we don't want to install JUCE itself.
##set(CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/source/libs/JUCE/install")

# Find the JUCE package.
# If JUCE is not found, CMake will produce an error.

# Don't use any install targets for JUCE, as we want to 
# use the CMake configuration provided by JUCE, but we don't want to install JUCE itself. 
##find_package(JUCE CONFIG REQUIRED)  
add_subdirectory(external/JUCE)

# Add custom CMake functions for Evil Audio.

evil_detect_asio_sdk()

add_subdirectory(source)
#add_subdirectory(docs/doxygen)

# Set EvilDAW as the startup project in Visual Studio.
# Do this after adding the subdirectories to ensure that 
# the EvilDAW target is defined before we try to set it as the startup project.

if (TARGET EvilDAW)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT EvilDAW)
endif()

# -----------------------------------------------------------------------------------------------