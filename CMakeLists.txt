cmake_minimum_required(VERSION 4.2.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(JUCE_ENABLE_MODULE_SOURCE_GROUPS "Enable Module Source Groups" ON)

# Set C++23 as the default for all targets
set(CMAKE_CXX_STANDARD 23)
# Ensure the compiler supports the specified standard
set(CMAKE_CXX_STANDARD_REQUIRED ON) 
# Disable compiler-specific extensions by default
set(CMAKE_CXX_EXTENSIONS OFF) 

project(EvilAudio VERSION 0.1.0 LANGUAGES C CXX)

include(CMakeDependentOption)

# Add the JUCE libaray to CMAKE_PREFIX_PATH for find_package
set(CMAKE_PREFIX_PATH "/source/EvilAudio/libs/JUCE/install")

#set(
#    JUCE_ASIO_SUPPORT TRUE
#    CACHE BOOL "Enable ASIO support in JUCE"
#)

# Find the JUCE package.
# If JUCE is not found, CMake will produce an error.
find_package(JUCE CONFIG REQUIRED)  

# Add custom CMake functions for Evil Audio.
include(cmake/EvilAudio.cmake)

# Prevent in-source builds
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "In-source builds not allowed. Please create a separate build directory and run CMake from there.")
endif()

# Test the logging with indentation functions.
#[[
increment_log_indent()
message(STATUS "Single Indented Message")      

increment_log_indent()
message(STATUS "Double Indented Message")      

decrement_log_indent()
#decrement_log_indent()
message(STATUS "Unindented Message")     
]] 

# Configure ASIO SDK path
set(ASIO_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/ASIOSDK/common")
if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/libs/ASIOSDK/common")
    if (WIN32)
        message(STATUS "ASIO SDK found at ${ASIO_SDK_DIR}")
        set(ASIO_SDK_PATH "${ASIO_SDK_DIR}" CACHE PATH "Path to the ASIO SDK")
        set(JUCE_ASIO_SUPPORT TRUE CACHE BOOL "Enable ASIO support in JUCE")
    endif()
else()
    message(WARNING "ASIO SDK not found in ${ASIO_SDK_DIR}. ASIO support will be disabled.")
    set(ASIO_SDK_PATH "" CACHE PATH "Path to the ASIO SDK")
endif() 

# -----------------------------------------------------------------------------------------------

add_subdirectory(libs)
add_subdirectory(source)

# -----------------------------------------------------------------------------------------------